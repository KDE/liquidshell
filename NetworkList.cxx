#include <NetworkList.hxx>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QToolButton>
#include <QDebug>

#include <KLocalizedString>
#include <KRun>

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/Utils>

//--------------------------------------------------------------------------------

NetworkButton::NetworkButton()
{
  setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  setAutoRaise(true);
  setCheckable(true);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

NetworkList::NetworkList(QWidget *parent)
  : QFrame(parent)
{
  setWindowFlag(Qt::Popup);
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
  connect(configure, &QToolButton::clicked, [this]() { hide(); KRun::runCommand("kcmshell5 kcm_networkmanagement", this); });
  hbox->addWidget(configure);

  // show connections

  NetworkManager::Connection::List allConnections = NetworkManager::listConnections();

  for (const NetworkManager::Connection::Ptr c : allConnections)
  {
    if ( !c->isValid() )
      continue;

    if ( (c->settings()->connectionType() == NetworkManager::ConnectionSettings::Wired) &&
         !c->uuid().isEmpty() )
    {
      QToolButton *net = new NetworkButton;
      net->setText(c->name());
      net->setIcon(QIcon::fromTheme("network-wired"));
      vbox->addWidget(net);

      for (const NetworkManager::ActiveConnection::Ptr &ac : NetworkManager::activeConnections())
      {
        if ( ac->uuid() == c->uuid() )
        {
          net->setChecked(true);
          break;
        }
      }

      connect(net, &QToolButton::toggled,
              [c](bool on)
              {
                if ( on )
                {
                  NetworkManager::activateConnection(c->path(), c->settings()->interfaceName(), QString());
                }
                else
                {
                  for (const NetworkManager::ActiveConnection::Ptr &ac : NetworkManager::activeConnections())
                  {
                    if ( ac->uuid() == c->uuid() )
                    {
                      NetworkManager::deactivateConnection(ac->path());
                      break;
                    }
                  }
                }
              }
             );
    }
    else if ( c->settings()->connectionType() == NetworkManager::ConnectionSettings::Vpn )
    {
      QToolButton *vpn = new NetworkButton;
      vpn->setText(c->name());
      vpn->setIcon(QIcon::fromTheme("security-high"));
      vbox->addWidget(vpn);

      for (const NetworkManager::ActiveConnection::Ptr &ac : NetworkManager::activeConnections())
      {
        if ( ac->uuid() == c->uuid() )
        {
          vpn->setChecked(true);
          break;
        }
      }

      connect(vpn, &QToolButton::toggled,
              [c](bool on)
              {
                if ( on )
                {
                  NetworkManager::ActiveConnection::Ptr conn(NetworkManager::primaryConnection());
                  if ( conn && !conn->devices().isEmpty() )
                    NetworkManager::activateConnection(c->path(), conn->devices()[0], QString());
                }
                else
                {
                  for (const NetworkManager::ActiveConnection::Ptr &ac : NetworkManager::activeConnections())
                  {
                    if ( ac->uuid() == c->uuid() )
                    {
                      NetworkManager::deactivateConnection(ac->path());
                      break;
                    }
                  }
                }
              }
             );
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
          QToolButton *net = new NetworkButton;
          net->setText(QString("%1 (%2%)").arg(network->ssid()).arg(network->signalStrength()));
          net->setIcon(QIcon::fromTheme("network-wireless"));
          vbox->addWidget(net);

          for (const NetworkManager::ActiveConnection::Ptr &ac : NetworkManager::activeConnections())
          {
            if ( ac->uuid() == conn->uuid() )
            {
              net->setChecked(true);
              break;
            }
          }

          connect(net, &QToolButton::toggled,
                  [conn, device](bool on)
                  {
                    if ( on )
                    {
                      NetworkManager::activateConnection(conn->path(), device->uni(), QString());
                    }
                    else
                    {
                      for (const NetworkManager::ActiveConnection::Ptr &ac : NetworkManager::activeConnections())
                      {
                        if ( ac->uuid() == conn->uuid() )
                        {
                          NetworkManager::deactivateConnection(ac->path());
                          break;
                        }
                      }
                    }
                  }
                 );
        }
        else
        {
          QToolButton *net = new NetworkButton;
          net->setText(QString("%1 (%2%)").arg(network->ssid()).arg(network->signalStrength()));
          net->setIcon(QIcon::fromTheme("network-wireless"));
          net->setEnabled(false);  // TODO: allow to add a new connection. See NetworkManager::addAndActivateConnection
          vbox->addWidget(net);
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

  vbox->addStretch();
}

//--------------------------------------------------------------------------------

void NetworkList::statusUpdate()
{
  network->setChecked(NetworkManager::isNetworkingEnabled());
  wireless->setChecked(NetworkManager::isWirelessEnabled());
}

//--------------------------------------------------------------------------------
