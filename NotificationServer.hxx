/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _NotificationServer_H_
#define _NotificationServer_H_

// https://developer.gnome.org/notification-spec/
// https://specifications.freedesktop.org/notification-spec

#include <SysTrayItem.hxx>
class NotificationList;

class NotificationServer : public SysTrayItem
{
  Q_OBJECT

  public:
    NotificationServer(QWidget *parent);

    void CloseNotification(uint id);

    QStringList GetCapabilities();

    QString GetServerInformation(QString &vendor, QString &version, QString &spec_version);

    uint Notify(const QString &app_name, uint replaces_id, const QString &app_icon,
                const QString &summary, const QString &body, const QStringList &actions,
                const QVariantMap &hints, int timeout);

    enum CloseReason
    {
      Expired = 1,
      Dismissed = 2,
      Closed = 3,
      Undefined = 4
    };

    void setAvoidPopup(bool avoid);
    bool getAvoidPopup() const;

  protected:
    QWidget *getDetailsList() override;

  Q_SIGNALS:
    void ActionInvoked(uint id, const QString &action_key);
    void NotificationClosed(uint id, uint reason);

  private:
    QString makeToolTip() const;

  private:
    uint notifyId = 1;
    NotificationList *notificationList;
};

#endif
