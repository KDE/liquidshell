/*
  Copyright 2017 Martin Koller, kollix@aon.at

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

#include <PkUpdates.hxx>
#include <PkUpdateList.hxx>

#include <QIcon>
#include <QDateTime>
#include <QDebug>

#include <KLocalizedString>
#include <KNotification>

//--------------------------------------------------------------------------------

PkUpdates::PkUpdates(QWidget *parent)
  : SysTrayItem(parent)
{
  setPixmap(QIcon::fromTheme("system-software-update").pixmap(size()));

  // check every hour if the next checkpoint was reached. This ensures that
  // we check for updates even when the computer was suspended for a while
  // and therefore a normal QTimer timeout for 1 day will not be reached in 1 day
  // TODO check again. start() was missing
  updateTimer.setInterval(3600 * 1000);
  updateTimer.start();
  connect(&updateTimer, &QTimer::timeout, this, &PkUpdates::checkForUpdatesReached);

  checkForUpdatesReached();
}

//--------------------------------------------------------------------------------

void PkUpdates::checkForUpdatesReached()
{
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
  setPixmap(QIcon::fromTheme("system-software-update").pixmap(size()));
  setToolTip(i18n("Checking for updates ..."));

  packages.clear();
  PackageKit::Transaction *transaction = PackageKit::Daemon::getUpdates();

  connect(transaction, &PackageKit::Transaction::package, this, &PkUpdates::package);
  connect(transaction, &PackageKit::Transaction::errorCode, this, &PkUpdates::transactionError);
  connect(transaction, &PackageKit::Transaction::finished, this, &PkUpdates::transactionFinished);

  connect(transaction, &PackageKit::Transaction::percentageChanged,
          [this, transaction]()
          {
            if ( transaction->percentage() <= 100 )
              setToolTip(i18n("Checking for updates ... %1%", transaction->percentage()));
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
  qDebug() << error << details;
  setToolTip(i18n("Last check: %1\nError on checking for updates: %2",
                  QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate), details));
}

//--------------------------------------------------------------------------------

void PkUpdates::transactionFinished(PackageKit::Transaction::Exit status, uint runtime)
{
  Q_UNUSED(status)
  Q_UNUSED(runtime)

  if ( updateList )
    updateList->setPackages(packages);

  createToolTip(true);
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

  if ( tooltip.isEmpty() )
  {
    int others = packages.count() - count;

    if ( others )
    {
      setToolTip(i18np("Last check: %1\nNo important updates available\n%2 other",
                       "Last check: %1\nNo important updates available\n%2 others",
                       QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate), others));
    }
    else
    {
      setToolTip(i18n("Last check: %1\nNo important updates available",
                      QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate)));
    }

    setPixmap(QIcon::fromTheme("update-none").pixmap(size()));
  }
  else
  {
    tooltip = i18n("<html>Last check: %1<br/>%2</html>",
                   QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate), tooltip);
    setToolTip(tooltip);

    QString icon;
    switch ( info )
    {
      case PackageKit::Transaction::InfoSecurity: icon = "update-high"; break;
      case PackageKit::Transaction::InfoImportant: icon = "update-medium"; break;
      case PackageKit::Transaction::InfoBugfix: icon = "update-low"; break;
      default: ;
    }
    setPixmap(QIcon::fromTheme(icon).pixmap(size()));

    if ( notify )
    {
      KNotification::event("updates available", i18n("Software Updates Available"), tooltip,
                           QIcon::fromTheme(icon).pixmap(32), this, KNotification::Persistent);
    }
  }
}

//--------------------------------------------------------------------------------

void PkUpdates::addItems(QString &tooltip, const QList<PackageData> &list) const
{
  tooltip += "<ul>";

  int count = std::min(3, list.count());
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
    connect(updateList, &PkUpdateList::refreshRequested, this, &PkUpdates::checkForUpdates);
    connect(updateList, &PkUpdateList::packageInstalled, this, &PkUpdates::packageInstalled);
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
