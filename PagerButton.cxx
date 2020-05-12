// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017, 2018 Martin Koller, kollix@aon.at

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

#include <PagerButton.hxx>
#include <DesktopPanel.hxx>

#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMenu>
#include <QDebug>

#include <KWindowSystem>
#include <KIconLoader>
#include <KIconEffect>

//--------------------------------------------------------------------------------

PagerButton::PagerButton(int num, DesktopPanel *p, bool doShowIcon)
  : desktop(num), panel(p), showIcon(doShowIcon)
{
  setText(KWindowSystem::desktopName(desktop).isEmpty() ?
          QString::number(desktop) : KWindowSystem::desktopName(desktop));

  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  setAcceptDrops(true);
  dragDropTimer.setSingleShot(true);
  dragDropTimer.setInterval(1000);
  connect(&dragDropTimer, &QTimer::timeout, this, [this]() { emit clicked(true); });

  if ( showIcon )
  {
    createPixmap();

    connect(KWindowSystem::self(), &KWindowSystem::windowAdded,
            this, &PagerButton::createPixmap);

    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved,
            this, &PagerButton::createPixmap);

    connect(KWindowSystem::self(), &KWindowSystem::stackingOrderChanged,
            this, &PagerButton::createPixmap);

    connect(KWindowSystem::self(), SIGNAL(windowChanged(WId, NET::Properties, NET::Properties2)),
            this, SLOT(windowChanged(WId, NET::Properties, NET::Properties2)));

    // when an application changes its icon very often very fast (windowChanged called)
    // (e.g. davmail when no network connection available)
    // let's avoid that liquidshell uses 100% CPU
    pixmapTimer.setSingleShot(true);
    pixmapTimer.setInterval(300);
    connect(&pixmapTimer, &QTimer::timeout, this, &PagerButton::createPixmap);

    connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, [this]() { updateGeometry(); });
  }

  connect(KWindowSystem::self(), &KWindowSystem::desktopNamesChanged, this,
          [this]()
          {
            setText(KWindowSystem::desktopName(desktop).isEmpty() ?
                    QString::number(desktop) : KWindowSystem::desktopName(desktop));
          });
}

//--------------------------------------------------------------------------------

QSize PagerButton::sizeHint() const
{
  QSize s = fontMetrics().size(0, text());
  s.setWidth(std::max(45, s.width() + 10));
  s.setHeight(QPushButton::sizeHint().height() - 2);

  if ( panel->getRows() == 1 )
    s.setHeight(std::max(s.height(), KIconLoader::global()->currentSize(KIconLoader::Panel)));

  return s;
}

//--------------------------------------------------------------------------------

void PagerButton::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);

  QPainter painter(this);

  QStyleOptionButton option;
  initStyleOption(&option);

  style()->drawControl(QStyle::CE_PushButtonBevel, &option, &painter, this);

  if ( !firstPixmap.isNull() )
    painter.drawPixmap((width() - firstPixmap.width()) / 2, (height() - firstPixmap.height()) / 2, firstPixmap);

  style()->drawControl(QStyle::CE_PushButtonLabel, &option, &painter, this);
}

//--------------------------------------------------------------------------------

void PagerButton::createPixmap()
{
  firstPixmap = QPixmap();
  QList<WId> windows = KWindowSystem::stackingOrder();

  // from top to bottom
  for (int i = windows.count() - 1; i >= 0; i--)
  {
    WId wid = windows[i];
    KWindowInfo win(wid, NET::WMDesktop | NET::WMWindowType | NET::WMState | NET::WMIcon);

    if ( win.valid(true) && win.isOnDesktop(desktop) &&
        (win.windowType(NET::DesktopMask) != NET::Desktop) &&
        (win.windowType(NET::DockMask) != NET::Dock) &&
         !(win.state() & NET::SkipTaskbar) )
    {
      firstPixmap = KWindowSystem::icon(wid, 22, 22, true);

      KIconEffect effect;
      firstPixmap = effect.apply(firstPixmap, KIconEffect::DeSaturate, 0, QColor(), true);

      break;
    }
  }

  update();
}

//--------------------------------------------------------------------------------

void PagerButton::windowChanged(WId id, NET::Properties props, NET::Properties2 props2)
{
  Q_UNUSED(id)
  Q_UNUSED(props2)

  if ( props & (NET::WMIcon | NET::WMDesktop) )
    pixmapTimer.start();
}

//--------------------------------------------------------------------------------

void PagerButton::dragEnterEvent(QDragEnterEvent *event)
{
  event->accept();
  dragDropTimer.start();
}

//--------------------------------------------------------------------------------

void PagerButton::dragLeaveEvent(QDragLeaveEvent *event)
{
  event->accept();
  dragDropTimer.stop();
}

//--------------------------------------------------------------------------------

void PagerButton::dropEvent(QDropEvent *event)
{
  dragDropTimer.stop();

  if ( !event->mimeData()->hasFormat("application/x-winId") )
  {
    event->ignore();
    return;
  }

  event->acceptProposedAction();
  WId wid = static_cast<WId>(event->mimeData()->data("application/x-winId").toInt());

  KWindowSystem::setOnDesktop(wid, desktop);
}

//--------------------------------------------------------------------------------
