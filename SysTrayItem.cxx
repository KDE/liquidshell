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

#include <SysTrayItem.hxx>

#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>

#include <KWindowSystem>

//--------------------------------------------------------------------------------

SysTrayItem::SysTrayItem(QWidget *parent)
  : QLabel(parent)
{
  setFixedSize(QSize(22, 22));
}

//--------------------------------------------------------------------------------

void SysTrayItem::mousePressEvent(QMouseEvent *event)
{
  if ( event->button() != Qt::LeftButton )
    return;

  toggleDetailsList();
}

//--------------------------------------------------------------------------------

void SysTrayItem::showDetailsList()
{
  QWidget *detailsList = getDetailsList();

  if ( !detailsList )
    return;

  QPoint point = mapToGlobal(pos());
  QRect screen = QApplication::desktop()->availableGeometry(this);
  point.setX(std::min(point.x(), screen.x() + screen.width() - detailsList->size().width()));
  point.setY(screen.bottom() - detailsList->size().height());
  detailsList->move(point);
  detailsList->show();
  KWindowSystem::raiseWindow(detailsList->winId());
}

//--------------------------------------------------------------------------------

void SysTrayItem::toggleDetailsList()
{
  QWidget *detailsList = getDetailsList();

  if ( !detailsList )
    return;

  if ( detailsList->isVisible() )
    detailsList->close();
  else
    showDetailsList();
}

//--------------------------------------------------------------------------------
