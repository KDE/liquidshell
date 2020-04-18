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

#include <DeviceNotifier.hxx>
#include <DeviceList.hxx>

#include <QIcon>
#include <QMouseEvent>
#include <QDebug>

#include <KLocalizedString>

//--------------------------------------------------------------------------------

DeviceNotifier::DeviceNotifier(QWidget *parent)
  : SysTrayItem(parent, "device-notifier")
{
  setToolTip(i18n("Device Notifier"));

  deviceList = new DeviceList(this);
  deviceList->setWindowTitle(i18n("Device List"));

  if ( deviceList->isEmpty() )
    hide();

  connect(deviceList, &DeviceList::deviceWasRemoved, this, &DeviceNotifier::checkDeviceList);
  connect(deviceList, &DeviceList::deviceWasAdded,
          [this]()
          {
            if ( !deviceList->isVisible() )
              timer.start();  // auto-hide

            showDetailsList();
          });

  // if the user did not activate the device list window, auto-hide it
  // but keep it if the mouse is over it (e.g. the user wants to click)
  timer.setInterval(4000);
  timer.setSingleShot(true);

  connect(&timer, &QTimer::timeout, this,
          [this]()
          {
            if ( deviceList->underMouse() )
              timer.start();
            else
              deviceList->hide();
          });

  deviceList->installEventFilter(this);
}

//--------------------------------------------------------------------------------

QWidget *DeviceNotifier::getDetailsList()
{
  deviceList->adjustSize();
  deviceList->resize(deviceList->size().expandedTo(QSize(300, 100)));
  setVisible(!deviceList->isEmpty());
  return deviceList;
}

//--------------------------------------------------------------------------------

void DeviceNotifier::checkDeviceList()
{
  if ( deviceList->isEmpty() )
  {
    deviceList->hide();
    hide();
  }
  else if ( deviceList->isVisible() )
    showDetailsList();  // reposition
}

//--------------------------------------------------------------------------------

bool DeviceNotifier::eventFilter(QObject *watched, QEvent *event)
{
  Q_UNUSED(watched)

  if ( event->type() == QEvent::WindowActivate )
    timer.stop();

  return false;
}

//--------------------------------------------------------------------------------
