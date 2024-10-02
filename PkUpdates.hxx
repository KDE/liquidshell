/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _PkUpdates_H_
#define _PkUpdates_H_

// software updates via PackageKit

#include <SysTrayItem.hxx>

#include <PackageKit/Daemon>
#include <KNotification>

#include <QMultiMap>
#include <QTimer>
#include <QDateTime>
#include <QPointer>
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
    QPointer<KNotification> notification;
};

#endif
