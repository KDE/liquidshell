/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <DesktopPanel.hxx>
#include <StartMenu.hxx>
#include <QuickLaunch.hxx>
#include <AppMenu.hxx>
#include <Pager.hxx>
#include <TaskBar.hxx>
#include <LockLogout.hxx>
#include <SysLoad.hxx>
#include <SysTray.hxx>
#include <ClockWidget.hxx>
#include <WindowList.hxx>
#include <KWinCompat.hxx>

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QDBusConnection>
#include <QDebug>

//--------------------------------------------------------------------------------

DesktopPanel::DesktopPanel(QWidget *parent)
  : QFrame(parent, Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus)
{
  setAttribute(Qt::WA_AlwaysShowToolTips);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setWindowIcon(QIcon::fromTheme("liquidshell"));

  KWinCompat::setState(winId(), NET::KeepAbove);
  KWinCompat::setOnAllDesktops(winId(), true);
  KWinCompat::setType(winId(), NET::Dock);

  setFrameShape(QFrame::StyledPanel);

  QHBoxLayout *hboxLayout = new QHBoxLayout(this);
  hboxLayout->setContentsMargins(QMargins());
  hboxLayout->setSpacing(2);

  hboxLayout->addWidget(new StartMenu(this));
  hboxLayout->addWidget(new QuickLaunch(this));
  hboxLayout->addWidget(new AppMenu(this));
  hboxLayout->addWidget(new Pager(this));
  hboxLayout->addWidget(new WindowList(this));
  hboxLayout->addWidget(new TaskBar(this));
  hboxLayout->addWidget(new LockLogout(this));
  hboxLayout->addWidget(new SysLoad(this));
  hboxLayout->addWidget(new SysTray(this));
  hboxLayout->addWidget(new ClockWidget(this));

  // to get notified about num-of-rows changed
  updateRowCount();
  QDBusConnection dbus = QDBusConnection::sessionBus();
  dbus.connect(QString(), "/KWin", "org.kde.KWin", "reloadConfig", this, SLOT(updateRowCount()));
  connect(KWinCompat::self(), &KWinCompat::numberOfDesktopsChanged, this, &DesktopPanel::updateRowCount);
}

//--------------------------------------------------------------------------------

void DesktopPanel::updateRowCount()
{
  NETRootInfo ri(qApp->nativeInterface<QNativeInterface::QX11Application>()->connection(), NET::Property(), NET::WM2DesktopLayout);

  int newRows = std::max(1, ri.desktopLayoutColumnsRows().height());

  if ( newRows != rows )
  {
    rows = newRows;
    emit rowsChanged(rows);
  }
}

//--------------------------------------------------------------------------------

bool DesktopPanel::event(QEvent *ev)
{
  bool ret = QFrame::event(ev);

  if ( ev->type() == QEvent::LayoutRequest )
  {
    if ( sizeHint().height() != height() )
      emit resized();
  }

  return ret;
}

//--------------------------------------------------------------------------------

void DesktopPanel::resizeEvent(QResizeEvent *event)
{
  Q_UNUSED(event);

  emit resized();
}

//--------------------------------------------------------------------------------
