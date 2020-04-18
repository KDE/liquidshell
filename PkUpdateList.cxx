// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 - 2020 Martin Koller, kollix@aon.at

  This file is part of liquidshell.

  liquidshell is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  liquidshell is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with liquidshell.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <PkUpdateList.hxx>
#include <DesktopWidget.hxx>

#include <QScrollBar>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QToolButton>
#include <QIcon>
#include <QDBusConnection>
#include <QStyle>
#include <QApplication>
#include <QScreen>
#include <QDebug>

#include <KLocalizedString>
#include <KNotification>

//#define TEST_LOGOUT
//#define TEST_REBOOT

//--------------------------------------------------------------------------------

PkUpdateListItem::PkUpdateListItem(QWidget *parent, PackageKit::Transaction::Info info, const PkUpdates::PackageData &data)
  : QWidget(parent), package(data)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QGridLayout *grid = new QGridLayout(this);
  grid->setContentsMargins(QMargins());
  grid->setSpacing(0);

  checkBox = new QCheckBox;
  checkBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  grid->addWidget(checkBox, 0, 0);

  QString icon;
  switch ( info )
  {
    case PackageKit::Transaction::InfoSecurity: icon = "update-high"; break;
    case PackageKit::Transaction::InfoImportant: icon = "update-medium"; break;
    case PackageKit::Transaction::InfoBugfix: icon = "update-low"; break;
    default: ;
  }
  QLabel *iconLabel = new QLabel;
  iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  iconLabel->setPixmap(QIcon::fromTheme(icon).pixmap(16));
  grid->addWidget(iconLabel, 0, 1);

  checkBox->setChecked(!icon.isEmpty());  // auto-check the important ones
  connect(checkBox, &QCheckBox::toggled, this, &PkUpdateListItem::toggled);

  label = new QToolButton;
  label->setAutoRaise(true);
  label->setText(package.summary + " [" +
                 PackageKit::Daemon::packageName(package.id) + ", " +
                 PackageKit::Daemon::packageVersion(package.id) + ']');
  grid->addWidget(label, 0, 2, Qt::AlignLeft);

  detailsLabel = new QLabel;
  detailsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
  detailsLabel->setIndent(30);
  detailsLabel->setAlignment(Qt::AlignLeft);
  grid->addWidget(detailsLabel, 1, 2);
  detailsLabel->hide();

  connect(label, &QToolButton::clicked, this, &PkUpdateListItem::getUpdateDetails);

  {
    QHBoxLayout *progressHbox = new QHBoxLayout;
    progressHbox->setSpacing(10);
    progress = new QProgressBar;
    progress->setFixedWidth(300);
    progressHbox->addWidget(progress);

    errorLabel = new QLabel;
    errorLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    errorLabel->hide();
    progressHbox->addWidget(errorLabel);

    packageLabel = new QLabel;
    packageLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    packageLabel->hide();
    progressHbox->addWidget(packageLabel);

    grid->addLayout(progressHbox, 2, 2, Qt::AlignLeft);
    showProgress(false);
  }
}

//--------------------------------------------------------------------------------

void PkUpdateListItem::showProgress(bool yes)
{
  progress->setValue(0);
  progress->setVisible(yes);
}

//--------------------------------------------------------------------------------

void PkUpdateListItem::getUpdateDetails()
{
  if ( !detailsLabel->isHidden() )
  {
    detailsLabel->hide();
    return;
  }

  //qDebug() << "getUpdateDetails" << package.id;
  PackageKit::Transaction *transaction = PackageKit::Daemon::getUpdateDetail(package.id);
  detailsLabel->setText(i18n("Getting details ..."));
  detailsLabel->show();

  connect(transaction, &PackageKit::Transaction::updateDetail, this, &PkUpdateListItem::updateDetail);

  connect(transaction, &PackageKit::Transaction::errorCode, this,
          [this](PackageKit::Transaction::Error error, const QString &details)
          {
            Q_UNUSED(error)
            detailsLabel->setText(details);
          });
}

//--------------------------------------------------------------------------------

void PkUpdateListItem::updateDetail(const QString &packageID,
                                    const QStringList &updates,
                                    const QStringList &obsoletes,
                                    const QStringList &vendorUrls,
                                    const QStringList &bugzillaUrls,
                                    const QStringList &cveUrls,
                                    PackageKit::Transaction::Restart restart,
                                    const QString &updateText,
                                    const QString &changelog,
                                    PackageKit::Transaction::UpdateState state,
                                    const QDateTime &issued,
                                    const QDateTime &updated)
{
  Q_UNUSED(packageID)
  Q_UNUSED(updates)
  Q_UNUSED(obsoletes)
  Q_UNUSED(vendorUrls)
  Q_UNUSED(bugzillaUrls)
  Q_UNUSED(cveUrls)
  Q_UNUSED(changelog)
  Q_UNUSED(state)
  Q_UNUSED(issued)
  Q_UNUSED(updated)

  QString text;

  if ( restart == PackageKit::Transaction::RestartSession )
    text = i18n("<b>Session restart required</b><br>");
  else if ( restart == PackageKit::Transaction::RestartSystem )
    text = i18n("<b>Reboot required</b><br>");

  text += updateText.trimmed();
  text.replace("\n", "<br>");

  detailsLabel->setText(text);
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

PkUpdateList::PkUpdateList(QWidget *parent)
  : QWidget(parent), restart(PackageKit::Transaction::RestartNone)
{
  setWindowFlags(windowFlags() | Qt::Tool);
  setWindowTitle(i18n("Software Updates"));

  vbox = new QVBoxLayout(this);
  QMargins margins = vbox->contentsMargins();
  vbox->setContentsMargins(QMargins(0, -1, 0, 0));

  // action buttons
  QHBoxLayout *hbox = new QHBoxLayout;
  margins.setTop(0);
  margins.setBottom(0);
  hbox->setContentsMargins(margins);

  checkAllBox = new QCheckBox(i18n("All"));
  connect(checkAllBox, &QCheckBox::toggled, this, &PkUpdateList::checkAll);

  filterEdit = new QLineEdit;
  filterEdit->setPlaceholderText(i18n("Filter"));
  filterEdit->setClearButtonEnabled(true);
  connect(filterEdit, &QLineEdit::textEdited, this, &PkUpdateList::filterChanged);

  // create "busy" indicator
  progressBar = new QProgressBar;

  installButton = new QPushButton(i18n("Install"));
  installButton->setEnabled(false);
  connect(installButton, &QPushButton::clicked, this, &PkUpdateList::install);

  refreshButton = new QPushButton(i18n("Refresh"));
  connect(refreshButton, &QPushButton::clicked, this, [this]() { setPackages(PkUpdates::PackageList()); emit refreshRequested(); });

  hbox->addWidget(checkAllBox);
  hbox->addWidget(filterEdit);
  hbox->addWidget(progressBar);
  hbox->addWidget(installButton);
  hbox->addWidget(refreshButton);
  vbox->addLayout(hbox);

  // list of items in the order: security, important, bugfix, others
  scrollArea = new QScrollArea;
  scrollArea->setWidgetResizable(true);

  vbox->addWidget(scrollArea);

  QWidget *w = new QWidget;
  vbox = new QVBoxLayout(w);
  itemsLayout = new QVBoxLayout;
  itemsLayout->setSpacing(0);
  vbox->addLayout(itemsLayout);
  vbox->addStretch();

  scrollArea->setWidget(w);
}

//--------------------------------------------------------------------------------

QSize PkUpdateList::sizeHint() const
{
  QSize s = scrollArea->widget()->sizeHint() + QSize(2 * scrollArea->frameWidth(), 2 * scrollArea->frameWidth());
  s.setWidth(s.width() + scrollArea->verticalScrollBar()->sizeHint().width());
  s.setHeight(layout()->contentsMargins().top() + installButton->sizeHint().height() +
              ((layout()->spacing() == -1) ? style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing) : layout()->spacing()) +
              s.height() + layout()->contentsMargins().bottom());

  s = s.expandedTo(QSize(500, 300));

  // Qt doc: The size of top-level widgets are constrained to 2/3 of the desktop's height and width.
  QSize screen = DesktopWidget::availableSize();
  s = s.boundedTo(screen * 2 / 3);

  return s;
}

//--------------------------------------------------------------------------------

void PkUpdateList::setRefreshProgress(int progress)
{
  if ( progress >= 100 )
  {
    filterEdit->show();
    progressBar->hide();
    refreshButton->setEnabled(true);
  }
  else if ( progress == 0 )
  {
    // we don't know if there will be a value between 0..100
    // Until the first value arrives, use a busy indicator
    progressBar->setMinimum(0);
    progressBar->setMaximum(0);
    progressBar->setValue(0);
    progressBar->show();
    filterEdit->hide();
    refreshButton->setEnabled(false);
  }
  else
  {
    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->setValue(progress);
  }
}

//--------------------------------------------------------------------------------

void PkUpdateList::setPackages(const PkUpdates::PackageList &packages)
{
  itemsLayout->parentWidget()->layout()->setEnabled(false);
  itemsLayout->setEnabled(false);

  QLayoutItem *child;
  while ( (child = itemsLayout->takeAt(0)) )
  {
    delete child->widget();
    delete child;
  }

  checkAllBox->setChecked(false);

  QList<PkUpdates::PackageData> list = packages.values(PackageKit::Transaction::InfoSecurity);
  for (const PkUpdates::PackageData &data : list)
  {
    auto *item = new PkUpdateListItem(itemsLayout->parentWidget(), PackageKit::Transaction::InfoSecurity, data);
    connect(item, &PkUpdateListItem::toggled, this, &PkUpdateList::countChecked);
    itemsLayout->addWidget(item);
    item->show();
  }

  list = packages.values(PackageKit::Transaction::InfoImportant);
  for (const PkUpdates::PackageData &data : list)
  {
    auto *item = new PkUpdateListItem(itemsLayout->parentWidget(), PackageKit::Transaction::InfoImportant, data);
    connect(item, &PkUpdateListItem::toggled, this, &PkUpdateList::countChecked);
    itemsLayout->addWidget(item);
    item->show();
  }

  list = packages.values(PackageKit::Transaction::InfoBugfix);
  for (const PkUpdates::PackageData &data : list)
  {
    auto *item = new PkUpdateListItem(itemsLayout->parentWidget(), PackageKit::Transaction::InfoBugfix, data);
    connect(item, &PkUpdateListItem::toggled, this, &PkUpdateList::countChecked);
    itemsLayout->addWidget(item);
    item->show();
  }

  // all the others
  for (PkUpdates::PackageList::const_iterator it = packages.constBegin(); it != packages.constEnd(); ++it)
  {
    if ( (it.key() != PackageKit::Transaction::InfoSecurity) &&
         (it.key() != PackageKit::Transaction::InfoImportant) &&
         (it.key() != PackageKit::Transaction::InfoBugfix) )
    {
      auto *item = new PkUpdateListItem(itemsLayout->parentWidget(), it.key(), it.value());
      connect(item, &PkUpdateListItem::toggled, this, &PkUpdateList::countChecked);
      itemsLayout->addWidget(item);
      item->show();
    }
  }
  itemsLayout->setEnabled(true);

  filterChanged(filterEdit->text());

  countChecked();
}

//--------------------------------------------------------------------------------

void PkUpdateList::checkAll(bool on)
{
  for (int i = 0; i < itemsLayout->count(); i++)
  {
    PkUpdateListItem *item = qobject_cast<PkUpdateListItem *>(itemsLayout->itemAt(i)->widget());

    if ( item && !item->isHidden() )
    {
      item->checkBox->blockSignals(true);
      item->checkBox->setChecked(on);
      item->checkBox->blockSignals(false);
    }
  }
  countChecked();
}

//--------------------------------------------------------------------------------

void PkUpdateList::countChecked()
{
  int count = 0;

  for (int i = 0; i < itemsLayout->count(); i++)
  {
    PkUpdateListItem *item = qobject_cast<PkUpdateListItem *>(itemsLayout->itemAt(i)->widget());

    if ( item && !item->isHidden() && item->checkBox->isChecked() )
      count++;
  }

  if ( count )
  {
    installButton->setText(i18np("Install %1 package", "Install %1 packages", count));
    installButton->setEnabled(true);
  }
  else
  {
    installButton->setText(i18n("Install"));
    installButton->setEnabled(false);
  }
  installButton->setIcon(QIcon());
  refreshButton->setEnabled(true);
}

//--------------------------------------------------------------------------------

void PkUpdateList::filterChanged(const QString &text)
{
  itemsLayout->parentWidget()->layout()->setEnabled(false);

  for (int i = 0; i < itemsLayout->count(); i++)
  {
    PkUpdateListItem *item = qobject_cast<PkUpdateListItem *>(itemsLayout->itemAt(i)->widget());

    item->setVisible(text.isEmpty() || (item->label->text().indexOf(text, 0, Qt::CaseInsensitive) != -1));
  }

  itemsLayout->parentWidget()->layout()->setEnabled(true);
  countChecked();
}

//--------------------------------------------------------------------------------

void PkUpdateList::install()
{
  if ( !installQ.isEmpty() )  // installation in progress; cancel it
  {
    QPointer<PkUpdateListItem> currentItem = installQ.head();

    if ( transaction )
    {
      QDBusPendingReply<> reply = transaction->cancel();
      reply.waitForFinished();
      if ( reply.isError() && currentItem )
      {
        currentItem->errorLabel->setText(reply.error().message());
        currentItem->errorLabel->show();
      }
    }

    for (int i = 0; i < itemsLayout->count(); i++)
    {
      PkUpdateListItem *item = qobject_cast<PkUpdateListItem *>(itemsLayout->itemAt(i)->widget());

      if ( !transaction || (item != currentItem) )
      {
        item->showProgress(false);
        item->errorLabel->hide();
      }
    }

    installQ.clear();
    emit packageCountToInstall(0);

    countChecked();
    return;
  }

  restart = PackageKit::Transaction::RestartNone;

  for (int i = 0; i < itemsLayout->count(); i++)
  {
    QPointer<PkUpdateListItem> item = qobject_cast<PkUpdateListItem *>(itemsLayout->itemAt(i)->widget());

    if ( item && !item->isHidden() && item->checkBox->isChecked() )
    {
      item->showProgress(true);
      item->errorLabel->hide();
      item->detailsLabel->hide();

      installQ.enqueue(item);
    }
  }

  installOne();
}

//--------------------------------------------------------------------------------

void PkUpdateList::installOne()
{
  emit packageCountToInstall(installQ.count());

  if ( installQ.isEmpty() ) // installation finished
  {
    if ( restart != PackageKit::Transaction::RestartNone )
    {
      QString text;

      if ( (restart == PackageKit::Transaction::RestartSystem) ||
           (restart == PackageKit::Transaction::RestartSecuritySystem) )
      {
        KNotification *notif = new KNotification("restart needed", parentWidget(), KNotification::Persistent);

        notif->setTitle(i18n("System Reboot Required"));
        notif->setText(i18n("One of the installed packages requires a system reboot"));
        notif->setActions(QStringList() << i18n("Reboot System"));

        connect(notif, &KNotification::action1Activated, this,
                []()
                {
                  QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.ksmserver", "/KSMServer",
                                                                    "org.kde.KSMServerInterface", "logout");
                  msg << 0/*no confirm*/ << 1/*reboot*/ << 0; // plasma-workspace/libkworkspace/kworkspace.h

                  QDBusConnection::sessionBus().send(msg);
                });

        notif->sendEvent();
      }
      else if ( (restart == PackageKit::Transaction::RestartSession) ||
                (restart == PackageKit::Transaction::RestartSecuritySession) )
      {
        KNotification *notif = new KNotification("restart needed", parentWidget(), KNotification::Persistent);

        notif->setTitle(i18n("Session Restart Required"));
        notif->setText(i18n("One of the installed packages requires you to logout"));
        notif->setActions(QStringList() << i18n("Logout"));

        connect(notif, &KNotification::action1Activated, this,
                []()
                {
                  QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.ksmserver", "/KSMServer",
                                                                    "org.kde.KSMServerInterface", "logout");
                  msg << 0/*no confirm*/ << 0/*logout*/ << 0; // plasma-workspace/libkworkspace/kworkspace.h

                  QDBusConnection::sessionBus().send(msg);
                });

        notif->sendEvent();
      }
    }

    countChecked();
    return;
  }

  QPointer<PkUpdateListItem> item = installQ.head();

  if ( !item )
    return;

  installButton->setText(i18n("Cancel Installation"));
  installButton->setIcon(QIcon::fromTheme("process-stop"));
  refreshButton->setEnabled(false);

  PackageKit::Transaction::TransactionFlag flag = PackageKit::Transaction::TransactionFlagOnlyTrusted;
#ifdef TEST_LOGOUT
  flag |= PackageKit::Transaction::TransactionFlagSimulate;
  restart = PackageKit::Transaction::RestartSession;
#elif defined TEST_REBOOT
  flag |= PackageKit::Transaction::TransactionFlagSimulate;
  restart = PackageKit::Transaction::RestartSystem;
#endif
  transaction = PackageKit::Daemon::updatePackage(item->package.id, flag);
  packageNoLongerAvailable = false;
  //qDebug() << "installing" << item->package.id;

  connect(transaction.data(), &PackageKit::Transaction::statusChanged, this,
          [item, this]()
          {
            if ( !item )  // already deleted
              return;

            //qDebug() << "status" << QMetaEnum::fromType<PackageKit::Transaction::Status>().valueToKey(transaction->status());
            QString text;
            switch ( transaction->status() )
            {
              case PackageKit::Transaction::StatusWait: text = i18n("Waiting"); break;
              case PackageKit::Transaction::StatusWaitingForAuth: text = i18n("Waiting for authentication"); break;
              case PackageKit::Transaction::StatusDepResolve: text = i18n("Resolving dependencies"); break;
              case PackageKit::Transaction::StatusUpdate: text = i18n("Updating"); break;
              case PackageKit::Transaction::StatusInstall: text = i18n("Installing"); break;
              case PackageKit::Transaction::StatusDownload: text = i18n("Downloading"); break;
              case PackageKit::Transaction::StatusCancel: text = i18n("Canceling"); break;
              case PackageKit::Transaction::StatusFinished: return;  // don't hide error label
              default: return;
            }

            if ( text.isEmpty() )
              item->errorLabel->hide();
            else
            {
              item->errorLabel->setText(text);
              item->errorLabel->show();
            }
          });

  connect(transaction.data(), &PackageKit::Transaction::itemProgress, this,
          [item](const QString &itemID, PackageKit::Transaction::Status status, uint percentage)
          {
            Q_UNUSED(status)

            if ( !item )  // already deleted
              return;

            item->packageLabel->setText(PackageKit::Daemon::packageName(itemID));
            item->packageLabel->show();

            if ( percentage <= 100 )  // 101 .. unknown
              item->progress->setValue(percentage);
          });

  connect(transaction.data(), &PackageKit::Transaction::requireRestart, this,
          [this](PackageKit::Transaction::Restart type, const QString &/*packageID*/)
          {
            // keep most important restart type: System, Session
            if ( (type == PackageKit::Transaction::RestartSystem) ||
                 (type == PackageKit::Transaction::RestartSecuritySystem) ||

                 (((type == PackageKit::Transaction::RestartSession) ||
                   (type == PackageKit::Transaction::RestartSecuritySession)) &&
                   (restart != PackageKit::Transaction::RestartSystem) &&
                   (restart != PackageKit::Transaction::RestartSecuritySystem)) )
              restart = type;
          });

  connect(transaction.data(), &PackageKit::Transaction::errorCode, this,
          [item, this](PackageKit::Transaction::Error error, const QString &details)
          {
            if ( !item )
              return;

            item->showProgress(false);
            item->errorLabel->setText(details);
            item->errorLabel->show();
            //qDebug() << "errorCode" << details << QMetaEnum::fromType<PackageKit::Transaction::Error>().valueToKey(error);

            if ( (error == PackageKit::Transaction::ErrorDepResolutionFailed) &&
                 (transaction->status() == PackageKit::Transaction::StatusSetup) )
            {
              // when a package was already installed due to another dependency, then it's no more an update candidate.
              // Still the finished() signal will arrive which will do cleanup (delete item, dequeue, etc.).
              packageNoLongerAvailable = true;
            }
          });

  connect(transaction.data(), &PackageKit::Transaction::finished, this,
          [item, this](PackageKit::Transaction::Exit status, uint runtime)
          {
            Q_UNUSED(runtime)

            if ( !item )
              return;

            if ( (status == PackageKit::Transaction::ExitSuccess) || packageNoLongerAvailable )
            {
              item->hide();
              item->deleteLater();
              emit packageInstalled(item->package.id);
            }
            else
            {
              item->showProgress(false);
              item->detailsLabel->setText(i18n("Update failed"));
              item->detailsLabel->show();
            }

            if ( !installQ.isEmpty() )  // might have been cancelled, q cleared
              installQ.dequeue();

            installOne();
          });
}

//--------------------------------------------------------------------------------
