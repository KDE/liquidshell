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

#ifndef _NotificationList_H_
#define _NotificationList_H_

#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QIcon>
#include <QMap>
class QVBoxLayout;

class NotifyItem : public QFrame
{
  Q_OBJECT

  public:
    NotifyItem(QWidget *parent, uint theid, const QString &app,
               const QString &summary, const QString &body, const QIcon &icon);

  private:
    QLabel *timeLabel, *iconLabel, *textLabel;
    uint id;
    QString appName;
};

//--------------------------------------------------------------------------------

class NotificationList : public QScrollArea
{
  Q_OBJECT

  public:
    NotificationList(QWidget *parent);

    void addItem(uint id, const QString &appName, const QString &summary, const QString &body, const QIcon &icon);

    int itemCount() const { return numItems; }

  signals:
    void itemsCountChanged();
    void listNowEmpty();

  private:
    QVBoxLayout *listVbox;
    QMap<QString, int> appTimeouts;  // appName, timeout (minutes)
    int numItems = 0;
};

#endif
