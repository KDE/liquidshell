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

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDebug>

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

  //qDebug() << deviceList;
  for (const QString &device : deviceList)
    deviceAddedSlot(device);
}

//--------------------------------------------------------------------------------

void KdeConnect::deviceVisibilityChanged(const QString &dev, bool visible)
{
  if ( visible )
    deviceAddedSlot(dev);
  else
    deviceRemovedSlot(dev);
}

//--------------------------------------------------------------------------------

void KdeConnect::deviceAddedSlot(const QString &dev)
{
  if ( devices.contains(dev) )
    return;

  QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + dev,
                           "org.freedesktop.DBus.Properties");

  Device device;
  device->id = dev;

  QDBusReply<QVariant> reply;

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
    QDBusMessage::createMethodCall("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + dev,
                                   "org.kde.kdeconnect.device", "loadedPlugins");
  QDBusReply<QStringList> strings;
  strings = QDBusConnection::sessionBus().call(msg);
  device->plugins = strings.value();

  msg = QDBusMessage::createMethodCall("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + dev,
                                       "org.kde.kdeconnect.device.battery", "charge");
  QDBusReply<int> chargeReply = QDBusConnection::sessionBus().call(msg);
  if ( chargeReply.isValid() )
    device->charge = chargeReply.value();

  QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + dev,
                                        "org.kde.kdeconnect.device.battery", "chargeChanged",
                                        device.data(), SLOT(chargeChanged(int)));

  devices.insert(dev, device);

  emit deviceAdded(device);
}

//--------------------------------------------------------------------------------

void KdeConnect::deviceRemovedSlot(const QString &dev)
{
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
