/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <WindowList.hxx>
#include <PopupMenu.hxx>

#include <QIcon>
#include <QStyle>
#include <QStyleOptionButton>
#include <QPainter>

#include <KWinCompat.hxx>
#include <KWindowInfo>

//--------------------------------------------------------------------------------

WindowList::WindowList(QWidget *parent)
  : QPushButton(parent)
{
  setIcon(QIcon::fromTheme("arrow-up"));
  setMaximumWidth(22);
  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

  setMenu(new PopupMenu(this));
  connect(menu(), &PopupMenu::aboutToShow, this, &WindowList::fillMenu);
}

//--------------------------------------------------------------------------------

void WindowList::fillMenu()
{
  menu()->clear();

  QList<WId> windows = KWinCompat::windows();

  for (int i = 1; i <= KWinCompat::numberOfDesktops(); i++)
  {
    menu()->addSection(KWinCompat::desktopName(i).isEmpty() ?
                       QString::number(i) : KWinCompat::desktopName(i));

    for (WId wid : windows)
    {
      KWindowInfo win(wid, NET::WMDesktop | NET::WMWindowType | NET::WMState | NET::WMName | NET::WMIcon);
      if ( win.valid(true) && win.isOnDesktop(i) &&
          (win.windowType(NET::DesktopMask) != NET::Desktop) &&
          (win.windowType(NET::DockMask) != NET::Dock) &&
           !(win.state() & NET::SkipTaskbar) )
      {
        QAction *action =
            menu()->addAction(KWinCompat::icon(wid, 22, 22, true), win.name(),
                              [wid]() { KWinCompat::forceActiveWindow(wid); });

        action->setData(static_cast<int>(wid));
      }
    }
  }
}

//--------------------------------------------------------------------------------

void WindowList::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);

  QPainter painter(this);

  QStyleOptionButton option;
  initStyleOption(&option);
  option.features = QStyleOptionButton::None;  // avoid menu arrow

  style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
}

//--------------------------------------------------------------------------------
