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

#include <KdeConnect.hxx>
#include <Battery.hxx>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDebug>

#include <KLocalizedString>

//--------------------------------------------------------------------------------

KdeConnect::KdeConnect()
{
  getDevices();

  QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect",
                                        "org.kde.kdeconnect.daemon", "deviceAdded",
                                        this, SLOT(deviceAddedSlot(QString)));

  QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect",
                                        "org.kde.kdeconnect.daemon", "deviceRemoved",
                                        this, SLOT(deviceRemovedSlot(QString)));

  QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect",
                                        "org.kde.kdeconnect.daemon", "deviceVisibilityChanged",
                                        this, SLOT(deviceVisibilityChanged(QString, bool)));
}

//--------------------------------------------------------------------------------

void KdeConnect::getDevices()
{
  QDBusMessage msg =
    QDBusMessage::createMethodCall("org.kde.kdeconnect", "/modules/kdeconnect",
                                   "org.kde.kdeconnect.daemon", "devices");
  msg << true << true;  // onlyReachable, onlyPaired
  QDBusConnection::sessionBus().callWithCallback(msg, this, SLOT(gotDevices(QDBusMessage)));
}

//--------------------------------------------------------------------------------

void KdeConnect::gotDevices(const QDBusMessage &msg)
{
  QStringList deviceList = msg.arguments()[0].toStringList();

  //qDebug() << __FUNCTION__ << deviceList;
  for (const QString &device : deviceList)
    deviceAddedSlot(device);
}

//--------------------------------------------------------------------------------

void KdeConnect::deviceVisibilityChanged(const QString &dev, bool visible)
{
  //qDebug() << __FUNCTION__ << dev << visible;
  if ( visible )
    deviceAddedSlot(dev);
  else
    deviceRemovedSlot(dev);
}

//--------------------------------------------------------------------------------

void KdeConnect::deviceAddedSlot(const QString &dev)
{
  //qDebug() << __FUNCTION__ << dev;
  if ( devices.contains(dev) )
    return;

  const QString devicePath =  "/modules/kdeconnect/devices/" + dev;

  QDBusInterface interface("org.kde.kdeconnect", devicePath, "org.freedesktop.DBus.Properties");

  QDBusReply<QVariant> reply;
  reply = interface.call("Get", "org.kde.kdeconnect.device", "isTrusted");
  if ( !reply.value().toBool() )
    return;  // only show paired devices

  Device device;
  device->id = dev;

  reply = interface.call("Get", "org.kde.kdeconnect.device", "name");
  device->name = reply.value().toString();

  reply = interface.call("Get", "org.kde.kdeconnect.device", "type");
  if ( reply.value().toString() == "smartphone" )
    device->icon = QIcon::fromTheme("smartphone");
  else if ( reply.value().toString() == "tablet" )
    device->icon = QIcon::fromTheme("tablet");
  else
  {
    reply = interface.call("Get", "org.kde.kdeconnect.device", "statusIconName");
    device->icon = QIcon::fromTheme(reply.value().toString());
  }

  QDBusMessage msg =
    QDBusMessage::createMethodCall("org.kde.kdeconnect", devicePath,
                                   "org.kde.kdeconnect.device", "loadedPlugins");
  QDBusReply<QStringList> strings;
  strings = QDBusConnection::sessionBus().call(msg);
  device->plugins = strings.value();

  msg = QDBusMessage::createMethodCall("org.kde.kdeconnect", devicePath,
                                       "org.kde.kdeconnect.device.battery", "charge");
  QDBusReply<int> chargeReply = QDBusConnection::sessionBus().call(msg);
  if ( chargeReply.isValid() )
    device->charge = chargeReply.value();

  msg = QDBusMessage::createMethodCall("org.kde.kdeconnect", devicePath,
                                       "org.kde.kdeconnect.device.battery", "isCharging");
  QDBusReply<bool> isChargingReply = QDBusConnection::sessionBus().call(msg);
  if ( isChargingReply.isValid() )
    device->isCharging = isChargingReply.value();

  device->chargeChangedSlot(device->charge);

  QDBusConnection::sessionBus().connect("org.kde.kdeconnect", devicePath,
                                        "org.kde.kdeconnect.device.battery", "chargeChanged",
                                        device.data(), SLOT(chargeChangedSlot(int)));

  QDBusConnection::sessionBus().connect("org.kde.kdeconnect", devicePath,
                                        "org.kde.kdeconnect.device.battery", "stateChanged",
                                        device.data(), SLOT(stateChangedSlot(bool)));

  QDBusConnection::sessionBus().connect("org.kde.kdeconnect", devicePath,
                                        "org.kde.kdeconnect.device.battery", "nameChanged",
                                        device.data(), SLOT(nameChangedSlot(const QString &)));

  devices.insert(dev, device);

  emit deviceAdded(device);
}

//--------------------------------------------------------------------------------

void KdeConnect::deviceRemovedSlot(const QString &dev)
{
  if ( !devices.contains(dev) )
    return;

  devices.remove(dev);
  emit deviceRemoved(dev);
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

void KdeConnectDevice::ringPhone()
{
  QDBusMessage msg =
    QDBusMessage::createMethodCall("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + id + "/findmyphone",
                                   "org.kde.kdeconnect.device.findmyphone", "ring");
  QDBusConnection::sessionBus().call(msg);
}

//--------------------------------------------------------------------------------

void KdeConnectDevice::chargeChangedSlot(int c)
{
  //qDebug() << __FUNCTION__ << name << c;
  charge = c;

  if ( charge < 0 )
    return;

  calcChargeIcon();

  if ( isCharging )
    return;

  const int LIMIT = 40;

  if ( charge < LIMIT )  // I want to keep charge above 40%
  {
    if ( !warned )
    {
      warned = true;
      notif = new KNotification("device needs charging", KNotification::Persistent);
      notif->setTitle(i18n("Device needs charging"));
      notif->setText(i18n("Device charge of '%1' is at %2%.\nYou should charge it.", name, charge));
      notif->setIconName("battery-040");
      notif->sendEvent();
    }
  }
  else
  {
    warned = false;
    if ( notif ) notif->close();
  }
}

//--------------------------------------------------------------------------------

void KdeConnectDevice::stateChangedSlot(bool c)
{
  //qDebug() << __FUNCTION__ << c;
  isCharging = c;
  calcChargeIcon();

  if ( isCharging && notif )
    notif->close();
}

//--------------------------------------------------------------------------------

void KdeConnectDevice::calcChargeIcon()
{
  chargeIcon = Battery::getStatusIcon(charge, isCharging);

  emit changed();
}

//--------------------------------------------------------------------------------

void KdeConnectDevice::nameChangedSlot(const QString &newName)
{
  //qDebug() << __FUNCTION__ << newName;
  name = newName;
  emit changed();
}

//--------------------------------------------------------------------------------
