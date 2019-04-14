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

#include <NetworkList.hxx>

#include <QHBoxLayout>
#include <QCheckBox>
#include <QToolButton>
#include <QDebug>

#include <KLocalizedString>
#include <KRun>
#include <KService>

#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/Utils>

//--------------------------------------------------------------------------------

NetworkButton::NetworkButton(NetworkManager::Connection::Ptr c, NetworkManager::Device::Ptr dev)
  : connection(c), device(dev)
{
  setCheckable(true);

  if ( connection )
  {
    for (const NetworkManager::ActiveConnection::Ptr &ac : NetworkManager::activeConnections())
    {
      if ( ac->uuid() == c->uuid() )
      {
        setChecked(true);
        break;
      }
    }

    connect(this, &NetworkButton::toggled, this, &NetworkButton::toggleNetworkStatus);
  }
  else
    setEnabled(false);
}

//--------------------------------------------------------------------------------

void NetworkButton::toggleNetworkStatus(bool on)
{
  if ( on )
  {
    switch ( connection->settings()->connectionType() )
    {
      case NetworkManager::ConnectionSettings::Wired:
      {
        NetworkManager::activateConnection(connection->path(), connection->settings()->interfaceName(), QString());
        break;
      }

      case NetworkManager::ConnectionSettings::Wireless:
      {
        NetworkManager::activateConnection(connection->path(), device->uni(), QString());
        break;
      }

      case NetworkManager::ConnectionSettings::Vpn:
      {
        NetworkManager::ActiveConnection::Ptr conn(NetworkManager::primaryConnection());
        if ( conn && !conn->devices().isEmpty() )
          NetworkManager::activateConnection(connection->path(), conn->devices()[0], QString());
        break;
      }

      default: ; // TODO
    }
  }
  else
  {
    for (const NetworkManager::ActiveConnection::Ptr &ac : NetworkManager::activeConnections())
    {
      if ( ac->uuid() == connection->uuid() )
      {
        NetworkManager::deactivateConnection(ac->path());
        break;
      }
    }
  }
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

NetworkList::NetworkList(QWidget *parent)
  : QFrame(parent)
{
  setWindowFlags(windowFlags() | Qt::Popup);
  setFrameShape(QFrame::StyledPanel);

  QVBoxLayout *vbox = new QVBoxLayout(this);
  QHBoxLayout *hbox = new QHBoxLayout;
  vbox->addLayout(hbox);

  network = new QToolButton;
  network->setIcon(QIcon::fromTheme("network-wired"));
  network->setCheckable(true);
  connect(network, &QToolButton::toggled, [](bool on) { NetworkManager::setNetworkingEnabled(on); });
  connect(NetworkManager::notifier(), &NetworkManager::Notifier::networkingEnabledChanged, this, &NetworkList::statusUpdate);
  hbox->addWidget(network);

  wireless = new QToolButton;
  wireless->setIcon(QIcon::fromTheme("network-wireless"));
  wireless->setCheckable(true);
  connect(wireless, &QToolButton::toggled, [](bool on) { NetworkManager::setWirelessEnabled(on); });
  connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessEnabledChanged, this, &NetworkList::statusUpdate);
  hbox->addWidget(wireless);

  statusUpdate();

  hbox->addStretch();

  QToolButton *configure = new QToolButton;
  configure->setIcon(QIcon::fromTheme("configure"));
  connect(configure, &QToolButton::clicked, this, &NetworkList::openConfigureDialog);
  hbox->addWidget(configure);

  // show connections
  connectionsVbox = new QVBoxLayout;
  connectionsVbox->setContentsMargins(QMargins());
  vbox->addLayout(connectionsVbox);
  fillConnections();

  QTimer *checkConnectionsTimer = new QTimer(this);
  checkConnectionsTimer->setInterval(1000);
  connect(checkConnectionsTimer, &QTimer::timeout, this, &NetworkList::fillConnections);
  checkConnectionsTimer->start();
}

//--------------------------------------------------------------------------------

void NetworkList::openConfigureDialog()
{
  hide();

  // newer plasma has already a KCM
  KService::Ptr service = KService::serviceByDesktopName("kcm_networkmanagement");

  if ( service )
    KRun::runApplication(*service, QList<QUrl>(), this);
  else
    KRun::run("kde5-nm-connection-editor", QList<QUrl>(), this);
}

//--------------------------------------------------------------------------------

void NetworkList::statusUpdate()
{
  network->setChecked(NetworkManager::isNetworkingEnabled());
  wireless->setChecked(NetworkManager::isWirelessEnabled());
}

//--------------------------------------------------------------------------------

void NetworkList::fillConnections()
{
  if ( underMouse() )  // avoid to delete the widget we are possibly hovering over
    return;

  QLayoutItem *child;
  while ( (child = connectionsVbox->takeAt(0)) )
  {
    delete child->widget();
    delete child;
  }

  NetworkManager::Connection::List allConnections = NetworkManager::listConnections();

  for (const NetworkManager::Connection::Ptr c : allConnections)
  {
    if ( !c->isValid() )
      continue;

    if ( (c->settings()->connectionType() == NetworkManager::ConnectionSettings::Wired) &&
         !c->uuid().isEmpty() )
    {
      NetworkButton *net = new NetworkButton(c);
      net->setText(c->name());
      net->setIcon(QIcon::fromTheme("network-wired"));
      connectionsVbox->addWidget(net);
    }
    else if ( c->settings()->connectionType() == NetworkManager::ConnectionSettings::Vpn )
    {
      NetworkButton *vpn = new NetworkButton(c);
      vpn->setText(c->name());
      vpn->setIcon(QIcon::fromTheme("security-high"));
      connectionsVbox->addWidget(vpn);
    }
  }

  // show available wifi networks
  if ( NetworkManager::isWirelessEnabled() )
  {
    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces())
    {
      if ( device->type() != NetworkManager::Device::Wifi )
        continue;

      NetworkManager::WirelessDevice::Ptr wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();

      for (const NetworkManager::WirelessNetwork::Ptr &network : wifiDevice->networks())
      {
        NetworkManager::AccessPoint::Ptr accessPoint = network->referenceAccessPoint();

        if ( !accessPoint )
          continue;

        // check if we have a connection for this SSID
        bool haveConnection = false;
        NetworkManager::Connection::Ptr conn;
        for (const NetworkManager::Connection::Ptr c : allConnections)
        {
          if ( c->isValid() && (c->settings()->connectionType() == NetworkManager::ConnectionSettings::Wireless) )
          {
            NetworkManager::Setting::Ptr setting = c->settings()->setting(NetworkManager::Setting::Wireless);
            NetworkManager::WirelessSetting::Ptr s = setting.staticCast<NetworkManager::WirelessSetting>();

            if ( s->ssid() == network->ssid() )
            {
              haveConnection = true;
              conn = c;
              break;
            }
          }
        }

        if ( haveConnection )
        {
          NetworkButton *net = new NetworkButton(conn, device);
          net->setText(QString("%1 (%2%)").arg(network->ssid()).arg(network->signalStrength()));
          net->setIcon(QIcon::fromTheme("network-wireless"));
          connectionsVbox->addWidget(net);
        }
        else
        {
          NetworkButton *net = new NetworkButton;
          net->setText(QString("%1 (%2%)").arg(network->ssid()).arg(network->signalStrength()));
          net->setIcon(QIcon::fromTheme("network-wireless"));
          net->setEnabled(false);  // TODO: allow to add a new connection. See NetworkManager::addAndActivateConnection
          connectionsVbox->addWidget(net);
        }

        /*
        NetworkManager::WirelessSecurityType security =
            NetworkManager::findBestWirelessSecurity(wifiDevice->wirelessCapabilities(), true,
                                                     wifiDevice->mode() == NetworkManager::WirelessDevice::Adhoc,
                                                     accessPoint->capabilities(), accessPoint->wpaFlags(), accessPoint->rsnFlags());

        if ( (security != NetworkManager::UnknownSecurity) && (security != NetworkManager::NoneSecurity) )
          net->setIcon(QIcon::fromTheme("object-locked"));
        else
          net->setIcon(QIcon::fromTheme("object-unlocked"));
        */
      }
    }
  }

  connectionsVbox->addStretch();
  adjustSize();
  emit changed();
}

//--------------------------------------------------------------------------------
