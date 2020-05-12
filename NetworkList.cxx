// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 - 2020 Martin Koller, kollix@aon.at

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

#include <QCheckBox>
#include <QToolButton>
#include <QScrollBar>
#include <QStyle>
#include <QTimer>
#include <QDebug>

#include <KLocalizedString>
#include <KRun>
#include <KService>

#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/Utils>
#include <networkmanagerqt_version.h>

#include <sys/types.h>
#include <pwd.h>
#include <algorithm>

//--------------------------------------------------------------------------------

NetworkButton::NetworkButton(NetworkManager::Connection::Ptr c, NetworkManager::Device::Ptr dev,
                             NetworkManager::AccessPoint::Ptr accessPoint)
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
  }

  if ( accessPoint )
  {
    ssid = accessPoint->ssid();
    rawSsid = accessPoint->rawSsid();
    wpaFlags = accessPoint->rsnFlags() ? accessPoint->rsnFlags() : accessPoint->wpaFlags();
  }

  connect(this, &NetworkButton::toggled, this, &NetworkButton::toggleNetworkStatus);
}

//--------------------------------------------------------------------------------

bool NetworkButton::compare(const NetworkButton *left, const NetworkButton *right)
{
  if ( left->wpaFlags && !right->wpaFlags )
    return true;

  return left->ssid.localeAwareCompare(right->ssid) < 0;
}

//--------------------------------------------------------------------------------

void NetworkButton::toggleNetworkStatus(bool on)
{
  if ( on )
  {
    if ( !connection )  // no connection yet -> create one
    {
      // the connMap content was "reverse-engineered" by using qdbusviewer and the result of getting
      // GetSettings of one of theSettings.Connection elements

      NMVariantMapMap connMap;
      QVariantMap map;
      map.insert("id", ssid);

      // ensure to not need root password by creating only for the current user
      struct passwd *pwd = getpwuid(geteuid());
      if ( pwd )
        map.insert("permissions", QStringList(QString("user:") + QString::fromUtf8(pwd->pw_name)));

      connMap.insert("connection", map);

      QVariantMap wirelessMap;
      wirelessMap.insert("ssid", rawSsid);

      if ( wpaFlags )
      {
        wirelessMap.insert("security", "802-11-wireless-security");

        QVariantMap security;
        if ( wpaFlags & NetworkManager::AccessPoint::KeyMgmtPsk )
          security.insert("key-mgmt", QString("wpa-psk"));
#if (NETWORKMANAGERQT_VERSION >= QT_VERSION_CHECK(5, 63, 0))
        else if ( wpaFlags & NetworkManager::AccessPoint::KeyMgmtSAE )
          security.insert("key-mgmt", QString("sae"));
#endif
        else
        {
          // TODO: other types - find value names
        }

        connMap.insert("802-11-wireless-security", security);
      }

      connMap.insert("802-11-wireless", wirelessMap);

      QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> call =
          NetworkManager::addAndActivateConnection(connMap, device->uni(), QString());

      /*
      QDBusPendingCallWatcher *pendingCallWatcher = new QDBusPendingCallWatcher(call, this);
      connect(pendingCallWatcher, &QDBusPendingCallWatcher::finished, this,
              [this](QDBusPendingCallWatcher *w)
              {
                w->deleteLater();
                QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> reply = *w;
                qDebug() << reply.error();
              }
             );
      */

      // without closing our popup, the user can not enter the password in the password dialog which appears
      window()->close();

      return;
    }

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
  else if ( connection )
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
  hbox = new QHBoxLayout;
  vbox->addLayout(hbox);

  network = new QToolButton;
  network->setIcon(QIcon::fromTheme("network-wired"));
  network->setIconSize(QSize(22, 22));
  network->setToolTip(i18n("Enable Networking"));
  network->setCheckable(true);
  connect(network, &QToolButton::clicked, [](bool on) { NetworkManager::setNetworkingEnabled(on); });
  connect(NetworkManager::notifier(), &NetworkManager::Notifier::networkingEnabledChanged, this, &NetworkList::statusUpdate);
  hbox->addWidget(network);

  wireless = new QToolButton;
  wireless->setIcon(QIcon::fromTheme("network-wireless"));
  wireless->setIconSize(QSize(22, 22));
  wireless->setToolTip(i18n("Enable Wireless Networking"));
  wireless->setCheckable(true);
  connect(wireless, &QToolButton::clicked, [](bool on) { NetworkManager::setWirelessEnabled(on); });
  connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessEnabledChanged, this, &NetworkList::statusUpdate);
  hbox->addWidget(wireless);

  statusUpdate();

  hbox->addStretch();

  QToolButton *configure = new QToolButton;
  configure->setIcon(QIcon::fromTheme("configure"));
  configure->setIconSize(QSize(22, 22));
  configure->setToolTip(i18n("Configure Network Connections"));
  connect(configure, &QToolButton::clicked, this, &NetworkList::openConfigureDialog);
  hbox->addWidget(configure);

  // show connections
  QWidget *widget = new QWidget;
  connectionsVbox = new QVBoxLayout(widget);
  connectionsVbox->setContentsMargins(QMargins());
  connectionsVbox->setSizeConstraint(QLayout::SetMinAndMaxSize);

  scroll = new QScrollArea;
  scroll->setWidgetResizable(true);
  scroll->setWidget(widget);

  vbox->addWidget(scroll);

  fillConnections();

  QTimer *checkConnectionsTimer = new QTimer(this);
  checkConnectionsTimer->setInterval(1000);
  connect(checkConnectionsTimer, &QTimer::timeout, this, &NetworkList::fillConnections);
  checkConnectionsTimer->start();
}

//--------------------------------------------------------------------------------

void NetworkList::openConfigureDialog()
{
  // newer plasma has already a KCM
  KService::Ptr service = KService::serviceByDesktopName("kcm_networkmanagement");

  if ( service )
    KRun::runApplication(*service, QList<QUrl>(), this);
  else
    KRun::run("kde5-nm-connection-editor", QList<QUrl>(), this);

  close();
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

  // show VPN networks on top
  for (const NetworkManager::Connection::Ptr c : allConnections)
  {
    if ( !c->isValid() )
      continue;

    if ( c->settings()->connectionType() == NetworkManager::ConnectionSettings::Vpn )
    {
      NetworkButton *vpn = new NetworkButton(c);
      vpn->setText(c->name());
      vpn->setIcon(QIcon::fromTheme("security-high"));
      connectionsVbox->addWidget(vpn);
      vpn->show();
    }
  }

  // wired networks
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
      net->show();
    }
  }

  // show available wifi networks
  if ( NetworkManager::isWirelessEnabled() )
  {
    QVector<NetworkButton *> entries;

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
        NetworkManager::Connection::Ptr conn;
        for (const NetworkManager::Connection::Ptr c : allConnections)
        {
          if ( c->isValid() && (c->settings()->connectionType() == NetworkManager::ConnectionSettings::Wireless) )
          {
            NetworkManager::Setting::Ptr setting = c->settings()->setting(NetworkManager::Setting::Wireless);
            NetworkManager::WirelessSetting::Ptr s = setting.staticCast<NetworkManager::WirelessSetting>();

            if ( s->ssid() == network->ssid() )
            {
              conn = c;
              break;
            }
          }
        }

        NetworkButton *net = new NetworkButton(conn, device, accessPoint);

        if ( conn )
          net->setText(QString("%1 (%2%)").arg(conn->name()).arg(network->signalStrength()));
        else
          net->setText(QString("%1 (%2%)").arg(network->ssid()).arg(network->signalStrength()));

        net->setIcon(QIcon::fromTheme("network-wireless"));

        if ( accessPoint->wpaFlags() || accessPoint->rsnFlags() )
          net->setIcon2(QIcon::fromTheme("object-locked"));
        else
        {
          // make it more obvious when an access point is not secured
          // by not showing any "lock" icon (also the unlocked icon is hardly different than the locked one,
          // so the user might easily miss the "insecure" icon)
          //net->setIcon2(QIcon::fromTheme("object-unlocked"));
        }

        entries.append(net);
      }
    }

    // sort entries: secure before insecure APs
    std::stable_sort(entries.begin(), entries.end(), &NetworkButton::compare);
    foreach (NetworkButton *entry, entries)
    {
      connectionsVbox->addWidget(entry);
      entry->show();
    }
  }

#if 0
  // TEST
  static int count = 15;
  for (int i = 0; i < count; i++)
  {
    NetworkButton *net = new NetworkButton();
    net->setText(QString("dummy %1").arg(i));
    net->setIcon(QIcon::fromTheme("network-wired"));
    connectionsVbox->addWidget(net);
    net->show();
  }
  count -= 3;
  if ( count <= 0 ) count = 15;
#endif

  connectionsVbox->addStretch();
  adjustSize();
  emit changed();
}

//--------------------------------------------------------------------------------

QSize NetworkList::sizeHint() const
{
  QWidget *w = scroll->widget();
  QSize s;

  s.setHeight(frameWidth() +
              contentsMargins().top() +
              layout()->contentsMargins().top() +
              hbox->sizeHint().height() +
              ((layout()->spacing() == -1) ? style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing) : layout()->spacing()) +
              scroll->frameWidth() +
              scroll->contentsMargins().top() +
              w->sizeHint().height() +
              scroll->contentsMargins().bottom() +
              scroll->frameWidth() +
              layout()->contentsMargins().bottom() +
              contentsMargins().bottom() +
              frameWidth()
             );

  s.setWidth(frameWidth() +
             contentsMargins().left() +
             layout()->contentsMargins().left() +
             scroll->frameWidth() +
             scroll->contentsMargins().left() +
             w->sizeHint().width() +
             scroll->verticalScrollBar()->sizeHint().width() +
             scroll->contentsMargins().right() +
             scroll->frameWidth() +
             layout()->contentsMargins().right() +
             contentsMargins().right() +
             frameWidth()
            );
  return s;
}

//--------------------------------------------------------------------------------
