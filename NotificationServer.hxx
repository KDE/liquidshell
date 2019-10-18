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

#ifndef _NotificationServer_H_
#define _NotificationServer_H_

// https://developer.gnome.org/notification-spec/

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
