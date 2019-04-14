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

#include <LockLogout.hxx>

#include <QGridLayout>
#include <QIcon>
#include <QDBusConnection>
#include <QDBusMessage>

#include <KLocalizedString>

//--------------------------------------------------------------------------------

LockLogout::LockLogout(DesktopPanel *parent)
  : QWidget(parent)
{
  setObjectName("LockLogout");

  QGridLayout *grid = new QGridLayout(this);
  grid->setContentsMargins(QMargins());
  grid->setSpacing(2);

  lock   = new QToolButton;
  logout = new QToolButton;

  lock->setIcon(QIcon::fromTheme("system-lock-screen"));
  logout->setIcon(QIcon::fromTheme("system-shutdown"));

  lock->setIconSize(QSize(22, 22));
  logout->setIconSize(QSize(22, 22));

  lock->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  logout->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  connect(lock, &QToolButton::clicked,
          []()
          {
            QDBusConnection::sessionBus().send(
                QDBusMessage::createMethodCall("org.freedesktop.ScreenSaver", "/ScreenSaver",
                                               "org.freedesktop.ScreenSaver", "Lock"));
          });

  connect(logout, &QToolButton::clicked,
          []()
          {
            QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.ksmserver", "/KSMServer",
                                                              "org.kde.KSMServerInterface", "logout");
            msg << -1 << 0 << 0;  // plasma-workspace/libkworkspace/kworkspace.h

            QDBusConnection::sessionBus().send(msg);
          });

  fill(parent->getRows());
  connect(parent, &DesktopPanel::rowsChanged, this, &LockLogout::fill);
}

//--------------------------------------------------------------------------------

void LockLogout::fill(int rows)
{
  QGridLayout *grid = static_cast<QGridLayout *>(layout());
  delete grid->takeAt(0);
  delete grid->takeAt(1);

  if ( rows == 1 )
  {
    grid->addWidget(lock, 0, 0);
    grid->addWidget(logout, 0, 1);
  }
  else
  {
    grid->addWidget(lock, 0, 0);
    grid->addWidget(logout, 1, 0);
  }
}

//--------------------------------------------------------------------------------
