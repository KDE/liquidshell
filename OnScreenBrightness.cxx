// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2020 Martin Koller, kollix@aon.at

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

#include <OnScreenBrightness.hxx>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QApplication>
#include <QScreen>
#include <QDebug>

#include <KWindowSystem>

//--------------------------------------------------------------------------------

OnScreenBrightness::OnScreenBrightness(QWidget *parent)
  : QProgressBar(parent)
{
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);

  setFixedSize(400, 40);
  hide();

  KWindowSystem::setState(winId(), NET::KeepAbove);
  KWindowSystem::setType(winId(), NET::Dock);
  KWindowSystem::setOnAllDesktops(winId(), true);

  hideTimer.setInterval(1000);
  hideTimer.setSingleShot(true);
  connect(&hideTimer, &QTimer::timeout, this, &QWidget::hide);

  QDBusConnection::sessionBus()
      .connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement/Actions/BrightnessControl",
               "org.kde.Solid.PowerManagement.Actions.BrightnessControl", "brightnessChanged",
               this, SLOT(brightnessChanged(int)));

  QDBusMessage msg =
      QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                     "/org/kde/Solid/PowerManagement/Actions/BrightnessControl",
                                     "org.kde.Solid.PowerManagement.Actions.BrightnessControl",
                                     "brightnessMax");

  QDBusConnection::sessionBus().callWithCallback(msg, this, SLOT(gotBrightnessMax(QDBusMessage)), nullptr);
}

//--------------------------------------------------------------------------------

void OnScreenBrightness::gotBrightnessMax(QDBusMessage msg)
{
  QDBusReply<int> reply = msg;

  if ( !reply.isValid() )
    return;

  int max = reply.value();

  setMaximum(max);
}

//--------------------------------------------------------------------------------

void OnScreenBrightness::brightnessChanged(int value)
{
  setValue(value);

  move((QApplication::primaryScreen()->size().width() - width()) / 2,
        QApplication::primaryScreen()->size().height() * 0.8);

  show();
  hideTimer.start();
}

//--------------------------------------------------------------------------------
