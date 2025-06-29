/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <OnScreenBrightness.hxx>
#include <KWinCompat.hxx>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QApplication>
#include <QScreen>
#include <QDebug>

//--------------------------------------------------------------------------------

OnScreenBrightness::OnScreenBrightness(QWidget *parent)
  : QProgressBar(parent)
{
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);

  setFixedSize(400, 40);
  hide();

  KWinCompat::setState(winId(), NET::KeepAbove);
  KWinCompat::setType(winId(), NET::Dock);
  KWinCompat::setOnAllDesktops(winId(), true);

  hideTimer.setInterval(1000);
  hideTimer.setSingleShot(true);
  connect(&hideTimer, &QTimer::timeout, this, &QWidget::hide);

  QDBusConnection::sessionBus()
      .connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement/Actions/BrightnessControl",
               "org.kde.Solid.PowerManagement.Actions.BrightnessControl", "brightnessChanged",
               this, SLOT(brightnessChanged(int)));

  QDBusConnection::sessionBus()
      .connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement/Actions/BrightnessControl",
               "org.kde.Solid.PowerManagement.Actions.BrightnessControl", "brightnessMaxChanged",
               this, SLOT(brightnessMaxChanged(int)));

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

void OnScreenBrightness::brightnessMaxChanged(int value)
{
  setMaximum(value);
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
