/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
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
            QDBusConnection::sessionBus().send(
                QDBusMessage::createMethodCall("org.kde.LogoutPrompt", "/LogoutPrompt",
                                               "org.kde.LogoutPrompt", "promptAll"));
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
