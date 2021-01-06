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

  const QString devicePath = "/modules/kdeconnect/devices/" + dev;

  QDBusInterface interface("org.kde.kdeconnect", devicePath, "org.kde.kdeconnect.device");

  if ( !interface.property("isTrusted").toBool() ||
       !interface.property("isReachable").toBool() )
    return;  // only show paired, reachable devices

  Device device;
  device->id = dev;

  device->name = interface.property("name").toString();

  QDBusConnection::sessionBus().connect("org.kde.kdeconnect", devicePath,
                                        "org.kde.kdeconnect.device", "nameChanged",
                                        device.data(), SLOT(nameChangedSlot(const QString &)));

  QDBusConnection::sessionBus().connect("org.kde.kdeconnect", devicePath,
                                        "org.kde.kdeconnect.device", "pluginsChanged",
                                        device.data(), SLOT(updatePlugins()));

  QString type = interface.property("type").toString();

  if ( type == "smartphone" )
    device->icon = QIcon::fromTheme("smartphone");
  else if ( type == "tablet" )
    device->icon = QIcon::fromTheme("tablet");
  else
    device->icon = QIcon::fromTheme(interface.property("statusIconName").toString());

  device->updatePlugins();

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

void KdeConnectDevice::chargeChangedSlot()
{
  charge = batteryInterface->property("charge").toInt();
  isCharging = batteryInterface->property("isCharging").toBool();

  //qDebug() << __FUNCTION__ << name << charge << isCharging;

  if ( charge < 0 )
    return;

  chargeIcon = Battery::getStatusIcon(charge, isCharging);
  emit changed();

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

void KdeConnectDevice::nameChangedSlot(const QString &newName)
{
  //qDebug() << __FUNCTION__ << newName;
  name = newName;
  emit changed();
}

//--------------------------------------------------------------------------------

void KdeConnectDevice::updatePlugins()
{
  const QString devicePath = "/modules/kdeconnect/devices/" + id;
  QDBusInterface interface("org.kde.kdeconnect", devicePath, "org.kde.kdeconnect.device");

  QDBusReply<QStringList> strings = interface.call("loadedPlugins");
  plugins = strings.value();

  if ( plugins.contains("kdeconnect_battery") )
  {
    if ( !batteryInterface )
    {
      batteryInterface = new QDBusInterface("org.kde.kdeconnect", devicePath + "/battery",
                             "org.kde.kdeconnect.device.battery", QDBusConnection::sessionBus(),
                             this);

      chargeChangedSlot();

      connect(batteryInterface, SIGNAL(refreshed()), this, SLOT(chargeChangedSlot()));
    }
  }
  else
  {
    delete batteryInterface;
    batteryInterface = nullptr;
    charge = -1;
  }

  emit changed();
}

//--------------------------------------------------------------------------------
