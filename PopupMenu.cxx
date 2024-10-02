/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <PopupMenu.hxx>

#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QGuiApplication>
#include <QStyleHints>
#include <QDebug>

//--------------------------------------------------------------------------------

void PopupMenu::mousePressEvent(QMouseEvent *event)
{
  if ( event->button() == Qt::LeftButton )
    dragStartPos = event->pos();

  QMenu::mousePressEvent(event);
}

//--------------------------------------------------------------------------------

void PopupMenu::mouseMoveEvent(QMouseEvent *event)
{
  if ( (event->buttons() == Qt::LeftButton) && (dragStartPos != QPoint(-1, -1)) &&
       (event->pos() - dragStartPos).manhattanLength() > QGuiApplication::styleHints()->startDragDistance() )
  {
    dragStartPos = QPoint(-1, -1);
    QAction *action = actionAt(event->pos());
    if ( action && !action->menu() && action->data().isValid() )
    {
      event->accept();

      QDrag *drag = new QDrag(this);
      QMimeData *mimeData = new QMimeData;
      Qt::DropActions dropAction;

      if ( action->data().typeId() == QMetaType::QUrl )
      {
        mimeData->setUrls(QList<QUrl>() << action->data().toUrl());
        dropAction = Qt::CopyAction;
      }
      else if ( action->data().typeId() == QMetaType::Int )
      {
        mimeData->setData("application/x-winId", QByteArray::number(action->data().toInt()));
        dropAction = Qt::MoveAction;
      }
      else
        qDebug() << "illegal data in PopupMenu action";  // should never come here

      drag->setMimeData(mimeData);
      drag->setPixmap(action->icon().pixmap(32, 32));
      drag->exec(dropAction);
    }
  }
  else
    QMenu::mouseMoveEvent(event);
}

//--------------------------------------------------------------------------------
