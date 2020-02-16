// SPDX-License-Identifier: GPL-3.0-or-later
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

#ifndef _PkUpdates_H_
#define _PkUpdates_H_

// software updates via PackageKit

#include <SysTrayItem.hxx>

#include <PackageKit/Daemon>

#include <QMultiMap>
#include <QTimer>
#include <QDateTime>
class PkUpdateList;

class PkUpdates : public SysTrayItem
{
  Q_OBJECT

  public:
    PkUpdates(QWidget *parent);

    struct PackageData
    {
      QString id;
      QString summary;
    };
    typedef QMultiMap<PackageKit::Transaction::Info, PackageData> PackageList;

  protected:
    QWidget *getDetailsList() override;

  private Q_SLOTS:
    void checkForUpdatesReached();
    void checkForUpdates();
    void refreshFinished(PackageKit::Transaction::Exit status, uint runtime);
    void package(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary);
    void transactionError(PackageKit::Transaction::Error error, const QString &details);
    void packageInstalled(const QString &id);
    void packageCountToInstallChanged(int num);

  private:
    void addItems(QString &tooltip, const QList<PackageData> &list) const;
    void createToolTip(bool notify = false);
    void setRefreshProgress(int progress);

  private:
    PackageList packages;
    QTimer updateTimer;
    QDateTime nextCheck;
    PkUpdateList *updateList = nullptr;
    int refreshProgress = 100;
    QPixmap currentPixmap;
};

#endif
