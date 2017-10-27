/*
  Copyright 2017 Martin Koller

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
#include <QDebug>

#include <Solid/Power>
#include <Solid/Battery>

#include <KLocalizedString>

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
    connect(Solid::Power::self(), &Solid::Power::acPluggedChanged, this, &Battery::changed);
    connect(device.as<Solid::Battery>(), &Solid::Battery::chargePercentChanged, this, &Battery::changed);
    connect(device.as<Solid::Battery>(), &Solid::Battery::chargeStateChanged, this, &Battery::changed);
    connect(device.as<Solid::Battery>(), &Solid::Battery::timeToFullChanged, this, &Battery::changed);
    connect(device.as<Solid::Battery>(), &Solid::Battery::timeToEmptyChanged, this, &Battery::changed);

    changed();
  }
}

//--------------------------------------------------------------------------------

void Battery::changed()
{
  Solid::Battery *battery = device.as<Solid::Battery>();

  if ( battery->remainingTime() == -1 )  // when on AC
  {
    hide();
    return;
  }

  show();

  QString icon, tip;

  switch ( battery->chargeState() )
  {
    case Solid::Battery::NoCharge: icon = "battery"; tip = i18n("Not Charging"); break;
    case Solid::Battery::FullyCharged: icon = "battery-100"; tip = i18n("Fully Charged"); break;

    case Solid::Battery::Charging:
    case Solid::Battery::Discharging:
    {
      int p = std::round(battery->chargePercent() / 20.0) * 20;

      if ( battery->chargeState() == Solid::Battery::Charging )
      {
        tip = i18n("Charging at %1%").arg(battery->chargePercent());

        int min = battery->timeToFull() / 60;
        tip += '\n' + i18n("Time until full: %1 minutes").arg(min);
      }
      else
      {
        tip = i18n("Discharging at %1%").arg(battery->chargePercent());

        int min = battery->timeToEmpty() / 60;
        tip += '\n' + i18n("Remaining Time: %1 minutes").arg(min);
      }

      if ( p < 20 )
        icon = "battery-caution";
      else if ( p < 40 )
        icon = "battery-low";
      else
      {
        icon = QString("battery%1-%2")
                      .arg((battery->chargeState() == Solid::Battery::Charging) ? "-charging" : "")
                      .arg(p, 3, 10, QLatin1Char('0'));
      }

      break;
    }
  }

  setPixmap(QIcon::fromTheme(icon).pixmap(size()));
  setToolTip(tip);
}

//--------------------------------------------------------------------------------
