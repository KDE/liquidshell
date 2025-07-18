/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <PkUpdates.hxx>
#include <PkUpdateList.hxx>

#include <QIcon>
#include <QDateTime>
#include <QPainter>
#include <QDebug>

#include <KLocalizedString>
#include <KIconLoader>
#include <KConfig>
#include <KConfigGroup>

//--------------------------------------------------------------------------------

PkUpdates::PkUpdates(QWidget *parent)
  : SysTrayItem(parent)
{
  KConfig config;
  KConfigGroup group = config.group("SoftwareUpdates");
  if ( !group.hasKey("enabled") )  // create config entry so that one knows it exists
    group.writeEntry("enabled", true);

  bool isEnabled = group.readEntry("enabled", true);

  if ( !isEnabled )
  {
    hide();
    return;
  }

  setPixmap(currentPixmap = QIcon::fromTheme("system-software-update").pixmap(size()));
  connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, [this]() { createToolTip(); });

  // check every hour if the next checkpoint was reached. This ensures that
  // we check for updates even when the computer was suspended for a while
  // and therefore a normal QTimer timeout for 1 day will not be reached in 1 day
  // TODO check again. start() was missing
  updateTimer.setInterval(3600 * 1000);
  updateTimer.start();
  connect(&updateTimer, &QTimer::timeout, this, &PkUpdates::checkForUpdatesReached);

  QString nextCheck = locale().toString(QDateTime::currentDateTime().addMSecs(updateTimer.interval()),
                                        QLocale::ShortFormat);
  setToolTip(i18n("Next check: %1", nextCheck));

  //QTimer::singleShot(0, this, &PkUpdates::checkForUpdatesReached);
}

//--------------------------------------------------------------------------------

void PkUpdates::checkForUpdatesReached()
{
  if ( updateList && updateList->isInstallInProgress() )
    return;

  QDateTime current = QDateTime::currentDateTime();

  if ( !nextCheck.isValid() || (current >= nextCheck) )
  {
    checkForUpdates();
    nextCheck = current.addDays(1);
  }
}

//--------------------------------------------------------------------------------

void PkUpdates::checkForUpdates()
{
  setPixmap(currentPixmap = QIcon::fromTheme("system-software-update").pixmap(size()));
  setToolTip(i18n("Checking for updates ..."));

  packages.clear();
  setRefreshProgress(0);

  PackageKit::Transaction *transaction = PackageKit::Daemon::refreshCache(true);

  connect(transaction, &PackageKit::Transaction::errorCode, this, &PkUpdates::transactionError);
  connect(transaction, &PackageKit::Transaction::finished, this, &PkUpdates::refreshFinished);

  connect(transaction, &PackageKit::Transaction::percentageChanged, this,
          [this, transaction]()
          {
            if ( (transaction->percentage() <= 100) && (transaction->status() != PackageKit::Transaction::StatusFinished) )
            {
              setRefreshProgress(transaction->percentage() / 2);  // first step: refresh cache, second: get updates
              setToolTip(i18n("Checking for updates ... %1%", refreshProgress));
            }
          });
}

//--------------------------------------------------------------------------------

void PkUpdates::refreshFinished(PackageKit::Transaction::Exit status, uint runtime)
{
  Q_UNUSED(runtime)
  Q_UNUSED(status)

  // don't stop on exit error; it could e.g. only be an error on one of the repos
  //if ( status != PackageKit::Transaction::ExitSuccess )
    //return;

  /////////////////
  // hmm ... does that do anything on openSuse ? seems not implemented there
  /*
  {
    PackageKit::Transaction *transaction = PackageKit::Daemon::getDistroUpgrades();

    connect(transaction, &PackageKit::Transaction::errorCode, this, &PkUpdates::transactionError);

    connect(transaction, &PackageKit::Transaction::distroUpgrade, this,
            [this](PackageKit::Transaction::DistroUpgrade type, const QString &name, const QString &description)
            {
              qDebug() << "distroUpgrade" << type << name << description;
            });
  }
  */
  /////////////////

  PackageKit::Transaction *transaction = PackageKit::Daemon::getUpdates();

  connect(transaction, &PackageKit::Transaction::package, this, &PkUpdates::package);
  connect(transaction, &PackageKit::Transaction::errorCode, this, &PkUpdates::transactionError);

  connect(transaction, &PackageKit::Transaction::finished, this,
          [this]()
          {
            if ( updateList )
              updateList->setPackages(packages);

            setRefreshProgress(100);
            createToolTip(true);
          });

  connect(transaction, &PackageKit::Transaction::percentageChanged, this,
          [this, transaction]()
          {
            if ( (transaction->percentage() <= 100) && (transaction->status() != PackageKit::Transaction::StatusFinished) )
            {
              setRefreshProgress(50 + (transaction->percentage() / 2));  // second step in checking for updates
              setToolTip(i18n("Checking for updates ... %1%", refreshProgress));
            }
          });
}

//--------------------------------------------------------------------------------

void PkUpdates::package(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary)
{
  PackageData pkg;
  pkg.id = packageID;
  pkg.summary = summary;

  packages.insert(info, pkg);
}

//--------------------------------------------------------------------------------

void PkUpdates::transactionError(PackageKit::Transaction::Error error, const QString &details)
{
  Q_UNUSED(error)

  setToolTip(i18n("Last check: %1\nError on checking for updates: %2",
                  locale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat), details));

  KNotification::event("update error", i18n("Software Update Error"), details,
                       QIcon::fromTheme("dialog-error").pixmap(32));

  setRefreshProgress(100);
}

//--------------------------------------------------------------------------------

void PkUpdates::createToolTip(bool notify)
{
  PackageKit::Transaction::Info info = PackageKit::Transaction::InfoUnknown;
  QString tooltip;

  int count = 0;
  QList<PackageData> list = packages.values(PackageKit::Transaction::InfoSecurity);

  if ( list.count() )
  {
    info = PackageKit::Transaction::InfoSecurity;

    count += list.count();
    tooltip += i18np("%1 <b>security</b> update available", "%1 <b>security</b> updates available", list.count());
    addItems(tooltip, list);
  }

  list = packages.values(PackageKit::Transaction::InfoImportant);

  if ( list.count() )
  {
    if ( info == PackageKit::Transaction::InfoUnknown )
      info = PackageKit::Transaction::InfoImportant;

    count += list.count();
    tooltip += i18np("%1 <i>important</i> update available", "%1 <i>important</i> updates available", list.count());
    addItems(tooltip, list);
  }

  list = packages.values(PackageKit::Transaction::InfoBugfix);

  if ( list.count() )
  {
    if ( info == PackageKit::Transaction::InfoUnknown )
      info = PackageKit::Transaction::InfoBugfix;

    count += list.count();
    tooltip += i18np("%1 bugfix update available", "%1 bugfix updates available", list.count());
    addItems(tooltip, list);
  }

  int others = packages.count() - count;

  if ( tooltip.isEmpty() )
  {
    if ( others )
    {
      setToolTip(i18np("Last check: %1\nNo important updates available\n%2 other",
                       "Last check: %1\nNo important updates available\n%2 others",
                       locale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat), others));

      if ( QIcon::hasThemeIcon("software-update-available") )
        setPixmap(currentPixmap = QIcon::fromTheme("software-update-available").pixmap(size()));
      else
        setPixmap(currentPixmap = QIcon::fromTheme("update-low").pixmap(size()));
    }
    else
    {
      setToolTip(i18n("Last check: %1\nNo updates available",
                      locale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat)));

      setPixmap(currentPixmap = QIcon::fromTheme("update-none").pixmap(size()));
    }
  }
  else
  {
    if ( others )
      tooltip += i18np("<br>%1 other", "<br>%1 others", others);

    tooltip = i18n("<html>Last check: %1<br>%2</html>",
                   locale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat), tooltip);
    setToolTip(tooltip);

    QString icon;
    switch ( info )
    {
      case PackageKit::Transaction::InfoSecurity:
      {
        if ( QIcon::hasThemeIcon("software-update-urgent") )
          icon = "software-update-urgent";
        else
          icon = "update-high";
        break;
      }
      case PackageKit::Transaction::InfoImportant:
      {
        if ( QIcon::hasThemeIcon("software-update-urgent") )
          icon = "software-update-urgent";
        else
          icon = "update-medium";
        break;
      }
      default: // PackageKit::Transaction::InfoBugfix:  // and others
      {
        if ( QIcon::hasThemeIcon("software-update-available") )
          icon = "software-update-available";
        else
          icon = "update-low";
        break;
      }
    }
    setPixmap(currentPixmap = QIcon::fromTheme(icon).pixmap(size()));

    if ( notify )
    {
      if ( notification )
        notification->close();  // close previous notification if it's still here

      notification = KNotification::event("updates available", i18n("Software Updates Available"), tooltip,
                                          QIcon::fromTheme(icon).pixmap(32));
    }
  }
}

//--------------------------------------------------------------------------------

void PkUpdates::addItems(QString &tooltip, const QList<PackageData> &list) const
{
  tooltip += "<ul>";

  int count = std::min(3, int(list.count()));
  if ( list.count() == 4 )  // if there's just one more, show it directly instead of "1 more"
    count++;

  for (int i = 0; i < count; i++)
    tooltip += "<li>" + list[i].summary + "</li>";

  if ( list.count() > 4 )
    tooltip += i18n("<li>%1 more ...</li>", list.count() - count);

  tooltip += "</ul>";
}

//--------------------------------------------------------------------------------

QWidget *PkUpdates::getDetailsList()
{
  if ( !updateList )
  {
    updateList = new PkUpdateList(this);
    updateList->setPackages(packages);
    updateList->setRefreshProgress(refreshProgress);
    connect(updateList, &PkUpdateList::refreshRequested, this, &PkUpdates::checkForUpdates);
    connect(updateList, &PkUpdateList::packageInstalled, this, &PkUpdates::packageInstalled);
    connect(updateList, &PkUpdateList::packageCountToInstall, this, &PkUpdates::packageCountToInstallChanged);
  }

  return updateList;
}

//--------------------------------------------------------------------------------

void PkUpdates::packageInstalled(const QString &id)
{
  for (PackageList::iterator it = packages.begin(); it != packages.end(); ++it)
  {
    if ( (*it).id == id )
    {
      packages.erase(it);
      break;
    }
  }
  createToolTip();
}

//--------------------------------------------------------------------------------

void PkUpdates::setRefreshProgress(int progress)
{
  refreshProgress = progress;

  if ( updateList )
    updateList->setRefreshProgress(refreshProgress);
}

//--------------------------------------------------------------------------------

void PkUpdates::packageCountToInstallChanged(int num)
{
  if ( num == 0 )
  {
    setPixmap(currentPixmap);
    return;
  }

  QPixmap pix = currentPixmap;
  QPainter painter(&pix);
  QFont f = font();
  f.setPixelSize(contentsRect().width() / 2);
  f.setBold(true);
  painter.setFont(f);

  painter.drawText(contentsRect(), Qt::AlignCenter, QString::number(num));
  painter.end();

  setPixmap(pix);
}

//--------------------------------------------------------------------------------
