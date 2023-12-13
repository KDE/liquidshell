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

#ifndef _NotificationList_H_
#define _NotificationList_H_

#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QIcon>
#include <QMap>
class QVBoxLayout;
class QProgressBar;
class NotificationServer;

class NotifyItem : public QFrame
{
  Q_OBJECT

  public:
    NotifyItem(QWidget *parent, NotificationServer *server, uint theid, const QString &app,
               const QString &summary, const QString &body, const QIcon &icon,
               const QStringList &actions);

    void destroySysResources();

    void setTimeout(int milliSecs);

    uint id;
    QString appName, summary, body;
    QStringList actions;
    QLabel *timeLabel, *iconLabel, *textLabel;
    QProgressBar *timeoutBar;
};

//--------------------------------------------------------------------------------

class NotificationList : public QWidget
{
  Q_OBJECT

  public:
    NotificationList(NotificationServer *parent);
    ~NotificationList() override;

    void addItem(uint id, const QString &appName, const QString &summary, const QString &body,
                 const QIcon &icon, const QStringList &actions, const QVariantMap &hints, int timeout);

    void closeItem(uint id);

    int itemCount() const { return items.count(); }
    QVector<NotifyItem *> getItems() const { return items; }

    void setAvoidPopup(bool on);
    bool getAvoidPopup() const { return avoidPopup; }

  Q_SIGNALS:
    void itemsCountChanged();
    void listNowEmpty();

  private Q_SLOTS:
    void itemDestroyed(QObject *item);

  private:
    void placeItems();

  private:
    QScrollArea *scrollArea;
    QVBoxLayout *listVbox;
    QMap<QString, int> appTimeouts;  // appName, timeout (minutes)
    QVector<NotifyItem *> items;
    NotificationServer *server;
    bool avoidPopup = false;
};

#endif
