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

#include <SysTrayItem.hxx>
#include <DesktopWidget.hxx>

#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QIcon>

#include <KWindowSystem>
#include <KIconLoader>

//--------------------------------------------------------------------------------

SysTrayItem::SysTrayItem(QWidget *parent, const QString &icon)
  : QLabel(parent), iconName(icon)
{
  setFixedSize(QSize(22, 22));

  if ( !iconName.isEmpty() )
  {
    setPixmap(QIcon::fromTheme(iconName).pixmap(size()));

    connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this,
            [this]() { setPixmap(QIcon::fromTheme(iconName).pixmap(size())); });
  }
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
  QRect screen = DesktopWidget::availableGeometry();
  QSize size = detailsList->frameSize();
  point.setX(std::min(point.x(), screen.x() + screen.width() - size.width()));
  point.setY(screen.bottom() - size.height());
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
