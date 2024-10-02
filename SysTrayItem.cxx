/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <SysTrayItem.hxx>
#include <DesktopWidget.hxx>

#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QIcon>

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
  QSize size = detailsList->windowHandle() ? detailsList->size() : detailsList->sizeHint();
  point.setX(std::min(point.x(), screen.x() + screen.width() - size.width()));
  point.setY(screen.bottom() - size.height());
  detailsList->move(point);
  detailsList->show();
  detailsList->raise();
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
