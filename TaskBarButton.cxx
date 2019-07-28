// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017, 2018, 2019 Martin Koller, kollix@aon.at

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

#include <TaskBarButton.hxx>

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionButton>
#include <QPainter>
#include <QX11Info>
#include <QMenu>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDrag>
#include <QMimeData>
#include <QGuiApplication>
#include <QStyleHints>
#include <QPointer>

#include <KSqueezedTextLabel>
#include <KColorScheme>
#include <KLocalizedString>

#include <netwm.h>

//--------------------------------------------------------------------------------

TaskBarButton::TaskBarButton(WId theWid)
  : wid(theWid)
{
  setAutoFillBackground(true);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setAcceptDrops(true);
  dragDropTimer.setSingleShot(true);
  dragDropTimer.setInterval(1000);
  connect(&dragDropTimer, &QTimer::timeout,
          [this]() { KWindowSystem::raiseWindow(wid); KWindowSystem::forceActiveWindow(wid); });

  QHBoxLayout *hbox = new QHBoxLayout(this);
  hbox->setContentsMargins(QMargins(4, 2, 4, 2));

  iconLabel = new QLabel;
  iconLabel->setScaledContents(true);
  iconLabel->setFixedSize(32, 32);
  iconLabel->setContextMenuPolicy(Qt::PreventContextMenu);
  hbox->addWidget(iconLabel);

  textLabel = new KSqueezedTextLabel;
  textLabel->setTextElideMode(Qt::ElideRight);
  textLabel->setContextMenuPolicy(Qt::PreventContextMenu);
  hbox->addWidget(textLabel);

  fill();
  setBackground();

  connect(KWindowSystem::self(), SIGNAL(windowChanged(WId, NET::Properties, NET::Properties2)),
          this, SLOT(windowChanged(WId, NET::Properties, NET::Properties2)));

  connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged,
          this, &TaskBarButton::setBackground);
}

//--------------------------------------------------------------------------------

void TaskBarButton::setIconSize(int size)
{
  iconLabel->setFixedSize(size, size);
}

//--------------------------------------------------------------------------------

void TaskBarButton::fill()
{
  KWindowInfo win(wid, NET::WMName | NET::WMIcon);
  iconLabel->setPixmap(KWindowSystem::icon(wid, 32, 32, true));
  textLabel->setText(win.name());
  setToolTip(win.name());
}

//--------------------------------------------------------------------------------

void TaskBarButton::mousePressEvent(QMouseEvent *event)
{
  if ( event->button() == Qt::LeftButton )
  {
    KWindowSystem::setShowingDesktop(false);

    if ( wid == KWindowSystem::activeWindow() )
      KWindowSystem::minimizeWindow(wid);
    else
      KWindowSystem::forceActiveWindow(wid);

    dragStartPos = event->pos();
    event->accept();
  }
  else if ( event->button() == Qt::RightButton )
  {
    // context menu to close window etc.
    QPointer<QMenu> menu(new QMenu(this));

    if ( KWindowSystem::numberOfDesktops() > 1 )
    {
      QMenu *desktops = menu->addMenu(i18n("Move To Desktop"));
      desktops->addAction(i18n("All Desktops"), [this]() { KWindowSystem::setOnAllDesktops(wid, true); });
      desktops->addSeparator();

      for (int i = 1; i <= KWindowSystem::numberOfDesktops(); i++)
        desktops->addAction(KWindowSystem::desktopName(i), [this, i]() { KWindowSystem::setOnDesktop(wid, i); });
    }

    menu->addAction(QIcon::fromTheme("window-close"), i18n("Close"),
                    [this]()
                    {
                      NETRootInfo ri(QX11Info::connection(), NET::CloseWindow);
                      ri.closeWindowRequest(wid);
                    }
                   );

    menu->exec(event->globalPos());
    delete menu;
  }
}

//--------------------------------------------------------------------------------

void TaskBarButton::mouseMoveEvent(QMouseEvent *event)
{
  event->accept();

  if ( (event->buttons() == Qt::LeftButton) &&
       (event->pos() - dragStartPos).manhattanLength() > QGuiApplication::styleHints()->startDragDistance() )
  {
    QDrag *drag = new QDrag(parentWidget());
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-winId", QByteArray::number(static_cast<int>(wid)));
    drag->setMimeData(mimeData);
    drag->setPixmap(*(iconLabel->pixmap()));
    drag->exec();
  }
}

//--------------------------------------------------------------------------------

void TaskBarButton::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);

  QPainter painter(this);

  QStyleOptionButton option;
  initStyleOption(&option);

  style()->drawControl(QStyle::CE_PushButtonBevel, &option, &painter, this);
}

//--------------------------------------------------------------------------------

void TaskBarButton::windowChanged(WId id, NET::Properties props, NET::Properties2 props2)
{
  Q_UNUSED(id)
  Q_UNUSED(props2)

  //qDebug() << textLabel->text() << props << (props & (NET::WMVisibleName | NET::WMState |  NET::WMName));
  //qDebug() << this << id << props << "me" << (wid == id);
  //if ( (id != wid) || (props == 0) )
    //return;

  if ( props & (NET::WMState | NET::ActiveWindow) )
    setBackground();

  // WMVisibleName alone is not enough. WMName needed
  if ( (wid == id) && (props & (NET::WMIcon | NET::WMName)) )
    fill();
}

//--------------------------------------------------------------------------------

void TaskBarButton::setBackground()
{
  KColorScheme scheme(QPalette::Active, KColorScheme::Window);
  QPalette pal = palette();

  KWindowInfo win(wid, NET::WMState);

  if ( win.state() & NET::Hidden )
    pal.setBrush(foregroundRole(), scheme.foreground(KColorScheme::InactiveText));
  else
    pal.setBrush(foregroundRole(), scheme.foreground(KColorScheme::NormalText));

  QBrush brush;

  if ( win.state() & NET::DemandsAttention )
    brush = scheme.background(KColorScheme::ActiveBackground);
  else if ( wid == KWindowSystem::activeWindow() )
    brush = scheme.shade(KColorScheme::MidShade);
  else
    brush = scheme.background();

  pal.setBrush(backgroundRole(), brush);
  setPalette(pal);
}

//--------------------------------------------------------------------------------

void TaskBarButton::dragEnterEvent(QDragEnterEvent *event)
{
  event->accept();
  dragDropTimer.start();
}

//--------------------------------------------------------------------------------

void TaskBarButton::dragLeaveEvent(QDragLeaveEvent *event)
{
  event->accept();
  dragDropTimer.stop();
}

//--------------------------------------------------------------------------------

void TaskBarButton::dropEvent(QDropEvent *event)
{
  event->accept();
  dragDropTimer.stop();
}

//--------------------------------------------------------------------------------

void TaskBarButton::updateWMGeometry()
{
  NETWinInfo info(QX11Info::connection(), wid, QX11Info::appRootWindow(), 0, 0);
  NETRect rect;
  QPoint globalPos = mapToGlobal(QPoint(0, 0));
  rect.pos.x = globalPos.x();
  rect.pos.y = globalPos.y();
  rect.size.width = width();
  rect.size.height = height();

  info.setIconGeometry(rect);
}

//--------------------------------------------------------------------------------

QSize TaskBarButton::sizeHint() const
{
  QSize s = QPushButton::sizeHint();
  s.setHeight(s.height() - 2);
  return s;
}

//--------------------------------------------------------------------------------
