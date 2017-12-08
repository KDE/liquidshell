#include <PkUpdateList.hxx>

#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QToolButton>
#include <QIcon>
#include <QPointer>
#include <QDebug>

#include <KLocalizedString>

//--------------------------------------------------------------------------------

PkUpdateListItem::PkUpdateListItem(QWidget *parent, PackageKit::Transaction::Info info, const PkUpdates::PackageData &data)
  : QWidget(parent), package(data)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QGridLayout *grid = new QGridLayout(this);
  grid->setContentsMargins(QMargins());
  grid->setVerticalSpacing(0);

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

  QToolButton *label = new QToolButton;
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
    progress = new QProgressBar;
    progress->setFixedWidth(300);
    progressHbox->addWidget(progress);

    cancelButton = new QToolButton;
    cancelButton->setIcon(QIcon::fromTheme("process-stop"));
    progressHbox->addWidget(cancelButton);

    errorLabel = new QLabel;
    errorLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    errorLabel->hide();
    progressHbox->addWidget(errorLabel);

    grid->addLayout(progressHbox, 2, 2, Qt::AlignLeft);
    showProgress(false);
  }
}

//--------------------------------------------------------------------------------

void PkUpdateListItem::showProgress(bool yes)
{
  progress->setVisible(yes);
  cancelButton->setVisible(yes);
}

//--------------------------------------------------------------------------------

void PkUpdateListItem::getUpdateDetails()
{
  qDebug() << "getUpdateDetails" << package.id;
  PackageKit::Transaction *transaction = PackageKit::Daemon::getUpdateDetail(package.id);
  detailsLabel->setText(i18n("Gettings details ..."));
  detailsLabel->show();

  connect(transaction, &PackageKit::Transaction::updateDetail, this, &PkUpdateListItem::updateDetail);

  connect(transaction, &PackageKit::Transaction::errorCode,
          [this, transaction](PackageKit::Transaction::Error error, const QString &details)
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
  Q_UNUSED(restart)
  Q_UNUSED(changelog)
  Q_UNUSED(state)
  Q_UNUSED(issued)
  Q_UNUSED(updated)

  detailsLabel->setText(updateText);
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

PkUpdateList::PkUpdateList(QWidget *parent)
  : QWidget(parent)
{
  setWindowFlags(windowFlags() | Qt::Tool);
  setWindowTitle(i18n("Software Updates"));

  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(QMargins(0, -1, 0, 0));

  // action buttons
  QHBoxLayout *hbox = new QHBoxLayout;

  QCheckBox *checkAll = new QCheckBox(i18n("All"));
  connect(checkAll, &QCheckBox::toggled, this, &PkUpdateList::checkAll);

  QLineEdit *filterEdit = new QLineEdit;
  filterEdit->setPlaceholderText(i18n("Filter"));
  filterEdit->setClearButtonEnabled(true);
  connect(filterEdit, &QLineEdit::textEdited, this, &PkUpdateList::filterChanged);

  installButton = new QPushButton(i18n("Install"));
  installButton->setEnabled(false);
  connect(installButton, &QPushButton::clicked, this, &PkUpdateList::install);

  QPushButton *refresh = new QPushButton(i18n("Refresh"));
  connect(refresh, &QPushButton::clicked, [this]() { setPackages(PkUpdates::PackageList()); emit refreshRequested(); });

  hbox->addWidget(checkAll);
  hbox->addWidget(filterEdit);
  hbox->addWidget(installButton);
  hbox->addWidget(refresh);
  vbox->addLayout(hbox);

  // list of items in the order: security, important, bugfix, others
  QScrollArea *scrollArea = new QScrollArea;
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

void PkUpdateList::setPackages(const PkUpdates::PackageList &packages)
{
  QLayoutItem *child;
  while ( (child = itemsLayout->takeAt(0)) )
  {
    delete child->widget();
    delete child;
  }

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
}

//--------------------------------------------------------------------------------

void PkUpdateList::filterChanged(const QString &text)
{
  itemsLayout->setEnabled(false);

  for (int i = 0; i < itemsLayout->count(); i++)
  {
    PkUpdateListItem *item = qobject_cast<PkUpdateListItem *>(itemsLayout->itemAt(i)->widget());

    item->setVisible(text.isEmpty() || (item->package.summary.indexOf(text, 0, Qt::CaseInsensitive) != -1));
  }

  itemsLayout->setEnabled(true);
  countChecked();
}

//--------------------------------------------------------------------------------

void PkUpdateList::install()
{
  for (int i = 0; i < itemsLayout->count(); i++)
  {
    QPointer<PkUpdateListItem> item = qobject_cast<PkUpdateListItem *>(itemsLayout->itemAt(i)->widget());

    if ( item && !item->isHidden() && item->checkBox->isChecked() )
    {
      item->showProgress(true);
      item->errorLabel->hide();

      PackageKit::Transaction *transaction = PackageKit::Daemon::updatePackage(item->package.id);
      qDebug() << "installing" << item->package.id;

      connect(item->cancelButton, &QToolButton::clicked, transaction, &PackageKit::Transaction::cancel);

      connect(transaction, &PackageKit::Transaction::allowCancelChanged,
              [item, transaction]() { if ( item ) item->cancelButton->setEnabled(transaction->allowCancel()); });

      connect(transaction, &PackageKit::Transaction::statusChanged,
              [item, transaction]()
              {
                if ( !item )  // already deleted
                  return;

                qDebug() << "status" << transaction->status();
                QString text;
                switch ( transaction->status() )
                {
                  case PackageKit::Transaction::StatusUpdate: text = i18n("Updating"); break;
                  case PackageKit::Transaction::StatusInstall: text = i18n("Installing"); break;
                  case PackageKit::Transaction::StatusDownload: text = i18n("Downloading "); break;
                  case PackageKit::Transaction::StatusCancel: text = i18n("Canceling "); break;
                  default: ;
                }

                if ( text.isEmpty() )
                  item->errorLabel->hide();
                else
                {
                  item->errorLabel->setText(text);
                  item->errorLabel->show();
                }
              });

      connect(transaction, &PackageKit::Transaction::percentageChanged,
              [item, transaction]() { if ( item ) item->progress->setValue(transaction->percentage()); });

      connect(transaction, &PackageKit::Transaction::errorCode,
              [item, transaction](PackageKit::Transaction::Error error, const QString &details)
              {
                Q_UNUSED(error)

                if ( !item )
                  return;

                item->showProgress(false);
                item->errorLabel->setText(details);
                item->errorLabel->show();
              });

      connect(transaction, &PackageKit::Transaction::finished,
              [this, item, transaction](PackageKit::Transaction::Exit status, uint runtime)
              {
                Q_UNUSED(runtime)

                if ( status == PackageKit::Transaction::ExitSuccess )
                {
                  item->checkBox->setChecked(false);  // re-count packages
                  item->deleteLater();
                  emit packageInstalled(item->package.id);
                }
                else
                  item->showProgress(false);
              });
    }
  }
}

//--------------------------------------------------------------------------------
