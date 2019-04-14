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

      if ( static_cast<QMetaType::Type>(action->data().type()) == QMetaType::QUrl )
      {
        mimeData->setUrls(QList<QUrl>() << action->data().toUrl());
        dropAction = Qt::CopyAction;
      }
      else if ( static_cast<QMetaType::Type>(action->data().type()) == QMetaType::Int )
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
