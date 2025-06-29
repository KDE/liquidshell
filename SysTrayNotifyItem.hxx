/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _SysTrayNotifyItem_H_
#define _SysTrayNotifyItem_H_

#include <QLabel>
#include <QTimer>
#include <QMenu>

class OrgKdeStatusNotifierItem;
class QDBusPendingCallWatcher;
class DBusMenuLayoutItem;

class SysTrayNotifyItem : public QLabel
{
  Q_OBJECT

  public:
    SysTrayNotifyItem(QWidget *parent, const QString &service, const QString &path);

  Q_SIGNALS:
    void initialized(SysTrayNotifyItem *);

  protected:
    void wheelEvent(QWheelEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private Q_SLOTS:
    void startTimer();
    void fetchData();
    void fetchDataReply(QDBusPendingCallWatcher *w);
    void menuLayoutReply(QDBusPendingCallWatcher *w);

  private:
    QPixmap findPixmap(const QString &name, const QString &path);
    QPixmap applyOverlay(const QPixmap &pixmap, const QPixmap &overlay);
    void fillMenu(QMenu &menu, const DBusMenuLayoutItem &item);

  private:
    QTimer fetchTimer;
    OrgKdeStatusNotifierItem *dbus;
    QString menuPath;
};

#endif
