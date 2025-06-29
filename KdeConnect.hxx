/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _KdeConnect_H_
#define _KdeConnect_H_

#include <QObject>
#include <QMap>
#include <QIcon>
#include <QSharedPointer>
#include <QPointer>
#include <QDBusInterface>

#include <KNotification>

class QDBusMessage;

//--------------------------------------------------------------------------------

class KdeConnectDevice : public QObject
{
  Q_OBJECT

  Q_SIGNALS:
    void changed();

  public Q_SLOTS:
    void updatePlugins();

  public:
    void ringPhone();

    QString id;
    QString name;
    QIcon icon, chargeIcon;
    QStringList plugins;
    int charge = -1;
    bool isCharging = false;
    bool warned = false;
    QDBusInterface *batteryInterface = nullptr;

  private Q_SLOTS:
    void nameChangedSlot(const QString &newName);
    void chargeChangedSlot();

  private:
    QPointer<KNotification> notif;
};

//--------------------------------------------------------------------------------

class KdeConnect : public QObject
{
  Q_OBJECT

  public:
    KdeConnect();

    struct Device : public QSharedPointer<KdeConnectDevice>
    {
      Device() : QSharedPointer(new KdeConnectDevice) {}
    };

  Q_SIGNALS:
    void deviceAdded(const Device &device);
    void deviceRemoved(const QString &devId);

  private Q_SLOTS:
    void getDevices();
    void gotDevices(const QDBusMessage &msg);
    void deviceAddedSlot(const QString &dev);
    void deviceRemovedSlot(const QString &dev);
    void deviceVisibilityChanged(const QString &dev, bool visible);

  private:
    QMap<QString, Device> devices;
};

#endif
