/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
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
#include <QMessageBox>
#include <QTextEdit>
#include <QMenu>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCryptographicHash>
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

  //qDebug() << "getUpdateDetails" << package.id << PackageKit::Daemon::packageName(package.id);
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

  installButton = new QToolButton;
  installButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  installButton->setText(i18n("Install"));
  installButton->setEnabled(false);
  connect(installButton, &QToolButton::clicked, [this]() { afterInstall = AfterInstall::Nothing; install(); });

  // more install options
  installMenu = new QMenu;
  installMenu->addAction(i18n("Install, then Sleep"), this, [this]() { afterInstall = AfterInstall::Sleep; install(); });
  installMenu->addAction(i18n("Install, then Shutdown"), this, [this]() { afterInstall = AfterInstall::Shutdown; install(); });
  installButton->setMenu(installMenu);

  refreshButton = new QToolButton;
  refreshButton->setText(i18n("Refresh"));
  connect(refreshButton, &QToolButton::clicked, this, [this]() { setPackages(PkUpdates::PackageList()); emit refreshRequested(); });

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
  QSize s;

  if ( savedSize.isValid() )
    s = savedSize;
  else
  {
    s = scrollArea->widget()->sizeHint() + QSize(2 * scrollArea->frameWidth(), 2 * scrollArea->frameWidth());
    s.setWidth(s.width() + scrollArea->verticalScrollBar()->sizeHint().width());
    s.setHeight(layout()->contentsMargins().top() + installButton->sizeHint().height() +
                ((layout()->spacing() == -1) ? style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing) : layout()->spacing()) +
                s.height() + layout()->contentsMargins().bottom());

    s = s.expandedTo(QSize(500, 300));
  }

  // Qt doc: The size of top-level widgets are constrained to 2/3 of the desktop's height and width.
  QSize screen = DesktopWidget::availableSize();
  s = s.boundedTo(screen * 2 / 3);

  return s;
}

//--------------------------------------------------------------------------------

void PkUpdateList::hideEvent(QHideEvent *event)
{
  Q_UNUSED(event)

  if ( itemsLayout->count() )
    savedSize = size();
  else
    savedSize = QSize();
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

bool comparePackageData(const PkUpdates::PackageData &a, const PkUpdates::PackageData &b)
{
  return a.summary.localeAwareCompare(b.summary) < 0;
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
  std::sort(list.begin(), list.end(), comparePackageData);
  for (const PkUpdates::PackageData &data : list)
  {
    auto *item = new PkUpdateListItem(itemsLayout->parentWidget(), PackageKit::Transaction::InfoSecurity, data);
    connect(item, &PkUpdateListItem::toggled, this, &PkUpdateList::countChecked);
    itemsLayout->addWidget(item);
    item->show();
  }

  list = packages.values(PackageKit::Transaction::InfoImportant);
  std::sort(list.begin(), list.end(), comparePackageData);
  for (const PkUpdates::PackageData &data : list)
  {
    auto *item = new PkUpdateListItem(itemsLayout->parentWidget(), PackageKit::Transaction::InfoImportant, data);
    connect(item, &PkUpdateListItem::toggled, this, &PkUpdateList::countChecked);
    itemsLayout->addWidget(item);
    item->show();
  }

  list = packages.values(PackageKit::Transaction::InfoBugfix);
  std::sort(list.begin(), list.end(), comparePackageData);
  for (const PkUpdates::PackageData &data : list)
  {
    auto *item = new PkUpdateListItem(itemsLayout->parentWidget(), PackageKit::Transaction::InfoBugfix, data);
    connect(item, &PkUpdateListItem::toggled, this, &PkUpdateList::countChecked);
    itemsLayout->addWidget(item);
    item->show();
  }

  // all the others
  list.clear();
  for (PkUpdates::PackageList::const_iterator it = packages.constBegin(); it != packages.constEnd(); ++it)
  {
    if ( (it.key() != PackageKit::Transaction::InfoSecurity) &&
         (it.key() != PackageKit::Transaction::InfoImportant) &&
         (it.key() != PackageKit::Transaction::InfoBugfix) )
    {
      list.append(it.value());
    }
  }
  std::sort(list.begin(), list.end(), comparePackageData);
  for (const PkUpdates::PackageData &data : list)
  {
    auto *item = new PkUpdateListItem(itemsLayout->parentWidget(), PackageKit::Transaction::InfoUnknown, data);
    connect(item, &PkUpdateListItem::toggled, this, &PkUpdateList::countChecked);
    itemsLayout->addWidget(item);
    item->show();
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
    installButton->setMenu(installMenu);
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

void PkUpdateList::enqueue(const QString &packageID)
{
  // find and add item with packageID
  for (int i = 0; i < itemsLayout->count(); i++)
  {
    QPointer<PkUpdateListItem> item = qobject_cast<PkUpdateListItem *>(itemsLayout->itemAt(i)->widget());

    if ( item && (item->package.id == packageID) )
    {
      item->showProgress(true);
      item->errorLabel->hide();
      item->detailsLabel->hide();

      installQ.enqueue(item);
      break;
    }
  }

  if ( !installQ.isEmpty() )
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

      if ( ((restart == PackageKit::Transaction::RestartSystem) ||
            (restart == PackageKit::Transaction::RestartSecuritySystem)) &&
           (afterInstall != AfterInstall::Shutdown) )  // when we shutdown, no need to tell the user to restart
      {
        KNotification *notif = new KNotification("restart needed", KNotification::Persistent);
        notif->setWindow(parentWidget()->windowHandle());

        notif->setTitle(i18n("System Reboot Required"));
        notif->setText(i18n("One of the installed packages requires a system reboot"));
        KNotificationAction *action = notif->addAction(i18n("Reboot System"));

        connect(action, &KNotificationAction::activated, this,
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
        KNotification *notif = new KNotification("restart needed", KNotification::Persistent);
        notif->setWindow(parentWidget()->windowHandle());

        notif->setTitle(i18n("Session Restart Required"));
        notif->setText(i18n("One of the installed packages requires you to logout"));
        KNotificationAction *action = notif->addAction(i18n("Logout"));

        connect(action, &KNotificationAction::activated, this,
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

    if ( eulaDialogs.isEmpty() )
    {
      if ( afterInstall == AfterInstall::Sleep )
      {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.PowerManagement",
                                                          "/org/freedesktop/PowerManagement",
                                                          "org.freedesktop.PowerManagement", "Suspend");

        QDBusConnection::sessionBus().send(msg);
      }
      else if ( afterInstall == AfterInstall::Shutdown )
      {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.ksmserver", "/KSMServer",
                                                          "org.kde.KSMServerInterface", "logout");
        msg << 0/*no confirm*/ << 2/*halt*/ << 0; // plasma-workspace/libkworkspace/kworkspace.h

        QDBusConnection::sessionBus().send(msg);
      }
    }

    return;
  }

  QPointer<PkUpdateListItem> item = installQ.head();

  if ( !item )
    return;

  installButton->setText(i18n("Cancel Installation"));
  installButton->setMenu(nullptr);
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

  connect(transaction.data(), &PackageKit::Transaction::allowCancelChanged, this,
          [this]()
          {
            installButton->setEnabled(transaction->allowCancel());
          });

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

  connect(transaction.data(), &PackageKit::Transaction::eulaRequired, this, &PkUpdateList::askEULA);

  connect(transaction.data(), &PackageKit::Transaction::repoSignatureRequired, this,
          [this](const QString &packageID,
                 const QString &repoName,
                 const QString &keyUrl,
                 const QString &keyUserid,
                 const QString &keyId,
                 const QString &keyFingerprint,
                 const QString &keyTimestamp,
                 PackageKit::Transaction::SigType type)
          {
            if ( QMessageBox::question(this, i18n("Validate Signature"),
                                       QString("%1\n%2\n%3\n%4\n%5\n%6\n%7")
                                               .arg(packageID)
                                               .arg(repoName)
                                               .arg(keyUrl)
                                               .arg(keyUserid)
                                               .arg(keyId)
                                               .arg(keyFingerprint)
                                               .arg(keyTimestamp)) == QMessageBox::Yes )
            {
              PackageKit::Daemon::installSignature(type, keyId, packageID);
            }
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

void PkUpdateList::askEULA(const QString &eulaID, const QString &packageID,
                           const QString &vendor, const QString &licenseAgreement)
{
  // I get several different eulaIDs but each having the same licenseAgreement text. Don't bother the user
  // showing multiple times the same licenseAgreement
  QByteArray hash = QCryptographicHash::hash(licenseAgreement.trimmed().toUtf8(), QCryptographicHash::Sha1);

  //qDebug() << "askEULA" << eulaID << packageID << hash.toBase64();

  // we can get multiple signals for the same eulaID
  // when installing multiple packages using the same EULA
  // or we get different eulaIDs with the same text
  if ( eulaDialogs.contains(hash) )
  {
    bool found = false;
    for (const EulaPackage &ep : eulaDialogs.values(hash))
    {
      if ( ep.eulaID == eulaID )
      {
        found = true;
        break;
      }
    }

    if ( !found )
      eulaDialogs.insert(hash, EulaPackage(eulaID, packageID));

    return;
  }

  if ( acceptedEula.contains(hash) )
  {
    //qDebug() << "already accepted - enqueue again";
    PackageKit::Daemon::acceptEula(eulaID);
    enqueue(packageID);
    return;
  }

  eulaDialogs.insert(hash, EulaPackage(eulaID, packageID));

  QDialog *dialog = new QDialog;
  dialog->setModal(false);
  dialog->setWindowTitle(i18n("Accept EULA"));

  QVBoxLayout *vbox = new QVBoxLayout(dialog);
  QHBoxLayout *hbox = new QHBoxLayout;

  QLabel *iconLabel = new QLabel;
  iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  iconLabel->setAlignment(Qt::AlignTop);
  QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxQuestion, nullptr, this);
  int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, this);
  iconLabel->setPixmap(icon.pixmap(QSize(iconSize, iconSize)));

  QLabel *label = new QLabel(i18n("<html>Do you accept the following license agreement "
                                  "for package<br>%1<br>from vendor<br>%2 ?</html>").arg(packageID, vendor));
  label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  label->setTextInteractionFlags(Qt::TextBrowserInteraction);

  hbox->addWidget(iconLabel);
  hbox->addWidget(label);

  vbox->addLayout(hbox);

  QTextEdit *textEdit = new QTextEdit;
  textEdit->setPlainText(licenseAgreement);
  textEdit->setReadOnly(true);
  vbox->addWidget(textEdit);

  QDialogButtonBox *buttonBox = new QDialogButtonBox();
  buttonBox->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons, nullptr, this));
  buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
  buttonBox->button(QDialogButtonBox::Yes)->setDefault(true);

  connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
  vbox->addWidget(buttonBox);

  connect(dialog, &QDialog::finished, this,
          [this, dialog, hash](int result)
          {
            if ( result == QDialog::Accepted )
            {
              for (const EulaPackage &ep : eulaDialogs.values(hash))
              {
                PackageKit::Daemon::acceptEula(ep.eulaID);
                enqueue(ep.packageID);
              }

              acceptedEula.insert(hash);
            }

            eulaDialogs.remove(hash);
            dialog->deleteLater();
          });

  // The dialog will be shown non modal and used asynchronously to the installation.
  // PackageKit Daemon will already finish this package with an error of
  // something like "you need to agree/decline the license"
  // So when the user decides what to do, he must afterwards start the installation
  // for this package again
  dialog->show();
}

//--------------------------------------------------------------------------------
