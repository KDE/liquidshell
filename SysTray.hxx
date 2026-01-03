/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _SysTray_H_
#define _SysTray_H_

#include <QFrame>
#include <QMap>
#include <QVector>
#include <QPointer>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <DesktopPanel.hxx>
#include <SysTrayNotifyItem.hxx>
class NotificationServer;

class SysTray : public QFrame
{
  Q_OBJECT

  public:
    SysTray(DesktopPanel *parent);

  private Q_SLOTS:
    void fill();
    void itemRegistered(QString service);
    void itemUnregistered(QString service);
    void itemInitialized(SysTrayNotifyItem *item, SysTrayNotifyItem::Result res);

  private:
    void registerWatcher();
    void arrangeNotifyItems();
    void contextMenuEvent(QContextMenuEvent *event) override;

  private:
    QVBoxLayout *vbox, *appsVbox;
    QVector<QHBoxLayout *> appsRows;
    QString serviceName;
    QMap<QString, QPointer<SysTrayNotifyItem>> items;
    NotificationServer *notificationServer = nullptr;
};

#endif
