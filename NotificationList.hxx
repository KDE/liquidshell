/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
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
               const QStringList &actions, bool isResident);

    void destroySysResources();

    void setTimeout(int milliSecs);

    uint id;
    QString appName, summary, body;
    QStringList actions;
    QLabel *timeLabel, *iconLabel, *textLabel;
    QProgressBar *timeoutBar;
    bool resident = false;  // keep after clicking an action
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
