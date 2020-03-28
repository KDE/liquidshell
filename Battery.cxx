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

#include <Battery.hxx>

#include <QIcon>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QDebug>

#include <Solid/Battery>

#include <KLocalizedString>
#include <KIconLoader>

#include <cmath>

//--------------------------------------------------------------------------------

Battery::Battery(QWidget *parent)
  : SysTrayItem(parent)
{
  QList<Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::Battery);

  for (Solid::Device dev : devices)
  {
    if ( dev.is<Solid::Battery>() && (dev.as<Solid::Battery>()->type() == Solid::Battery::PrimaryBattery) )
    {
      device = dev;
      break;
    }
  }

  if ( !device.isValid() )
    hide();
  else
  {
    QDBusConnection::systemBus()
      .connect("org.freedesktop.UPower",
               "/org/freedesktop/UPower",
               "org.freedesktop.DBus.Properties",
               "PropertiesChanged",
               this,
               SLOT(upowerPropertiesChanged(QString, QVariantMap, QStringList)));

    QDBusMessage msg =
      QDBusMessage::createMethodCall("org.freedesktop.UPower",
                                     "/org/freedesktop/UPower",
                                     "org.freedesktop.DBus.Properties",
                                     "Get");
    msg << QLatin1String("org.freedesktop.UPower") << QLatin1String("OnBattery");
    QDBusConnection::systemBus().callWithCallback(msg, this, SLOT(onBatteryReply(QDBusMessage)), nullptr);

    connect(device.as<Solid::Battery>(), &Solid::Battery::chargePercentChanged, this, &Battery::changed);
    connect(device.as<Solid::Battery>(), &Solid::Battery::chargeStateChanged, this, &Battery::changed);
    connect(device.as<Solid::Battery>(), &Solid::Battery::timeToFullChanged, this, &Battery::changed);
    connect(device.as<Solid::Battery>(), &Solid::Battery::timeToEmptyChanged, this, &Battery::changed);

    connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, &Battery::changed);

    changed();
  }
}

//--------------------------------------------------------------------------------

QString Battery::secsToHM(int secs) const
{
  int h = secs / 3600;
  int m = (secs % 3600) / 60;

  QString hStr = i18np("%1 hour", "%1 hours", h);
  QString mStr = i18np("%1 minute", "%1 minutes", m);

  QString result;

  if ( h )
    result = hStr;

  if ( m )
  {
    if ( h )
      result += ", ";

    result += mStr;
  }

  return result;
}

//--------------------------------------------------------------------------------

void Battery::onBatteryReply(const QDBusMessage &msg)
{
  onBattery = msg.arguments()[0].value<QDBusVariant>().variant().toBool();
  changed();
}

//--------------------------------------------------------------------------------

void Battery::upowerPropertiesChanged(const QString &interface,
                                      const QVariantMap &properties,
                                      const QStringList &invalidated)
{
  Q_UNUSED(interface)
  Q_UNUSED(invalidated)

  if ( properties.contains("OnBattery") )
  {
    onBattery = properties.value("OnBattery").toBool();
    changed();
  }
}

//--------------------------------------------------------------------------------

QIcon Battery::getStatusIcon(int charge, bool isCharging)
{
  QString iconName;

  int p = qRound(charge / 20.0) * 20;

  if ( p < 20 )
    iconName = "caution";
  else if ( p < 40 )
    iconName = "low";
  else
    iconName = QString("%1").arg(p, 3, 10, QLatin1Char('0'));

  iconName = QString("battery%1-%2").arg(isCharging ? "-charging" : "").arg(iconName);

  return QIcon::fromTheme(iconName);
}

//--------------------------------------------------------------------------------

void Battery::changed()
{
  Solid::Battery *battery = device.as<Solid::Battery>();

  QString tip;

  switch ( battery->chargeState() )
  {
    case Solid::Battery::NoCharge: tip = i18n("Not Charging"); break;
    case Solid::Battery::FullyCharged: tip = i18n("Fully Charged"); break;

    case Solid::Battery::Charging:
    case Solid::Battery::Discharging:
    {
      if ( battery->chargeState() == Solid::Battery::Charging )
      {
        tip = i18n("Charging at %1%", battery->chargePercent());
        if ( battery->timeToFull() )  // it can be 0, so we don't know
          tip += '\n' + i18n("Time until full: ") + secsToHM(battery->timeToFull());
      }
      else
      {
        tip = i18n("Discharging at %1%", battery->chargePercent());
        if ( battery->timeToEmpty() )  // it can be 0, so we don't know
          tip += '\n' + i18n("Remaining Time: ") + secsToHM(battery->timeToEmpty());
      }

      break;
    }
  }

  setPixmap(getStatusIcon(battery->chargePercent(),
                          battery->chargeState() == Solid::Battery::Charging).pixmap(size()));
  setToolTip(tip);

  setVisible(onBattery || (battery->chargeState() != Solid::Battery::FullyCharged));
}

//--------------------------------------------------------------------------------

QWidget *Battery::getDetailsList()
{
  if ( !dialog )
  {
    dialog = new KCMultiDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->addModule("powerdevilglobalconfig");
    dialog->addModule("powerdevilprofilesconfig");
    dialog->adjustSize();
    dialog->setWindowTitle(i18n("Power Management"));
  }

  return dialog;
}

//--------------------------------------------------------------------------------
