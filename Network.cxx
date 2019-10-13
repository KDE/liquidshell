// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 - 2019 Martin Koller, kollix@aon.at

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

#include <Network.hxx>

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/VpnConnection>

#include <QIcon>
#include <QMouseEvent>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>

#include <KLocalizedString>
#include <KIconLoader>

//--------------------------------------------------------------------------------

Network::Network(QWidget *parent)
  : SysTrayItem(parent)
{
  blinkTimer.setInterval(500);
  connect(&blinkTimer, &QTimer::timeout, [this]() { blinkState = !blinkState; setPixmap(blinkState ? origPixmap : QPixmap()); });

  checkState();

  connect(NetworkManager::notifier(), &NetworkManager::Notifier::statusChanged, this, &Network::checkState);
  connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this, &Network::checkState);
  connect(NetworkManager::notifier(), &NetworkManager::Notifier::primaryConnectionChanged, this, &Network::checkState);
  connect(NetworkManager::notifier(), &NetworkManager::Notifier::activeConnectionRemoved, this, &Network::checkState);
  connect(NetworkManager::notifier(), &NetworkManager::Notifier::activeConnectionsChanged, this, &Network::checkState);

  connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, &Network::checkState);

  QDBusConnection::sessionBus().send(
      QDBusMessage::createMethodCall("org.kde.kded5", "/modules/networkmanagement",
                                     "org.kde.plasmanetworkmanagement", "init"));
}

//--------------------------------------------------------------------------------

void Network::checkState()
{
  if ( NetworkManager::status() == NetworkManager::Unknown )
  {
    // if the system does not have NM running, hide the icon
    hide();
    return;
  }

  show();

  if ( NetworkManager::status() == NetworkManager::Connecting )
    blinkTimer.start();
  else
    blinkTimer.stop();

  if ( !NetworkManager::primaryConnection() || !NetworkManager::primaryConnection()->connection() )
  {
    setPixmap(origPixmap = QIcon::fromTheme("network-disconnect").pixmap(size()));
    setToolTip(i18n("No Network Connection"));
    return;
  }

  NetworkManager::ActiveConnection::Ptr conn(NetworkManager::primaryConnection());
  //connect(conn.data(), &NetworkManager::ActiveConnection::vpnChanged, this, &Network::checkState);

  QString tip;

  if ( NetworkManager::connectivity() == NetworkManager::Full )
    tip = i18n("Full Network Connectivity (%1)", conn->connection()->name());
  else
    tip = i18n("Limited Network Connectivity (%1)", conn->connection()->name());

  NetworkManager::Device::Ptr device;

  if ( conn->devices().count() )
  {
    device = NetworkManager::findNetworkInterface(conn->devices()[0]);

    if ( device && device->ipV4Config().addresses().count() )
      tip += "\n" + i18n("IPv4 Address: %1", device->ipV4Config().addresses()[0].ip().toString());
  }

  QPixmap pixmap;

  if ( conn->type() == NetworkManager::ConnectionSettings::Wireless )
  {
    NetworkManager::WirelessDevice::Ptr dev = qobject_cast<NetworkManager::WirelessDevice::Ptr>(device);

    if ( dev )
    {
      NetworkManager::AccessPoint::Ptr accessPoint = dev->activeAccessPoint();
      int signalStrength = accessPoint.isNull() ? 0 : accessPoint->signalStrength();
      int x = qRound(signalStrength / 25.0) * 25;
      pixmap = QIcon::fromTheme(QString("network-wireless-connected-%1").arg(x)).pixmap(size());

      if ( !accessPoint.isNull() )
        tip += "\n" + i18n("SSID: %1", accessPoint->ssid());

      tip += "\n" + i18n("Signal Strength: %1", signalStrength);
    }
  }
  else
  {
    pixmap = QIcon::fromTheme("network-connect").pixmap(size());
  }

  //qDebug() << conn << "type" << conn->type() << "vpn" << conn->vpn();

  bool vpnActive = false;
  for (const NetworkManager::ActiveConnection::Ptr &ac : NetworkManager::activeConnections())
  {
    if ( ac->vpn() )
    {
      vpnActive = true;
      break;
    }
  }

  if ( vpnActive )
  {
    pixmap = QIcon::fromTheme("security-high").pixmap(size());
    tip += "\n" + i18n("VPN active");
  }

  setPixmap(origPixmap = pixmap);
  setToolTip(tip);
}

//--------------------------------------------------------------------------------

QWidget *Network::getDetailsList()
{
  if ( !networkList )
  {
    networkList = new NetworkList(this);
    networkList->setAttribute(Qt::WA_DeleteOnClose);
    connect(networkList.data(), &NetworkList::changed, this, &Network::showDetailsList);  // reposition
  }
  return networkList.data();
}

//--------------------------------------------------------------------------------
