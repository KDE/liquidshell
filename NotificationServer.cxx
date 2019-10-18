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

#include <NotificationServer.hxx>
#include <NotificationList.hxx>

#include <notificationsadaptor.h>

#include <QIcon>
#include <QDBusConnection>
#include <QDebug>

#include <KLocalizedString>
#include <KService>

//--------------------------------------------------------------------------------

NotificationServer::NotificationServer(QWidget *parent)
  : SysTrayItem(parent, "preferences-desktop-notification")
{
  new NotificationsAdaptor(this);

  QDBusConnection dbus = QDBusConnection::sessionBus();
  if ( dbus.registerService("org.freedesktop.Notifications") )
  {
    if ( !dbus.registerObject("/org/freedesktop/Notifications", this) )
      dbus.unregisterService("org.freedesktop.Notifications");
  }
  notificationList = new NotificationList(this);
  connect(notificationList, &NotificationList::listNowEmpty, this, &NotificationServer::hide);

  connect(notificationList, &NotificationList::itemsCountChanged,
          [this]()
          {
            show();
            setToolTip(makeToolTip());
          }
         );

  hide();
}

//--------------------------------------------------------------------------------

QString NotificationServer::makeToolTip() const
{
  QString tip = "<html>";
  tip += i18np("%1 notification", "%1 notifications", notificationList->itemCount());

  if ( notificationList->itemCount() < 4 )
  {
    for (const NotifyItem *item : notificationList->getItems())
    {
      tip += "<hr>";
      tip += item->timeLabel->text() + " ";
      QString title = (item->appName == item->summary) ? item->appName : (item->appName + ": " + item->summary);
      tip += "<b>" + title + "</b>";
      tip += "<br>" + item->body;
    }
  }

  tip += "</html>";
  return tip;
}

//--------------------------------------------------------------------------------

QStringList NotificationServer::GetCapabilities()
{
  return QStringList()
           << "body"
           << "body-hyperlinks"
           << "body-images"
           << "body-markup"
           << "icon-static"
           << "persistence"
           << "actions"
           ;
}

//--------------------------------------------------------------------------------

void NotificationServer::CloseNotification(uint id)
{
  notificationList->closeItem(id);
}

//--------------------------------------------------------------------------------

QString NotificationServer::GetServerInformation(QString &vendor, QString &version, QString &spec_version)
{
  vendor = "kollix";
  version = "1.0";
  spec_version = "1.2";
  return vendor;
}

//--------------------------------------------------------------------------------

uint NotificationServer::Notify(const QString &app_name, uint replaces_id, const QString &app_icon,
                                const QString &summary, const QString &theBody, const QStringList &actions,
                                const QVariantMap &hints, int timeout)
{
  //qDebug() << "app" << app_name << "summary" << summary << "body" << theBody << "timeout" << timeout << "replaceId" << replaces_id
  //         << "hints" << hints << "actions" << actions;

  QString body(theBody);
  body.replace("\n", "<br>");

  QIcon icon;
  if ( !app_icon.isEmpty() )
    icon = QIcon::fromTheme(app_icon);
  else if ( hints.contains("image-path") )
    icon = QIcon(hints["image-path"].toString());

  QString appName = app_name;
  if ( appName.isEmpty() && hints.contains("desktop-entry") )
  {
    KService::Ptr service = KService::serviceByDesktopName(hints["desktop-entry"].toString().toLower());
    if ( service )
      appName = service->name();
  }

  notificationList->addItem(notifyId, appName, summary, body, icon, actions, hints, timeout);

  if ( replaces_id != 0 )
    notificationList->closeItem(replaces_id);

  return notifyId++;
}

//--------------------------------------------------------------------------------

QWidget *NotificationServer::getDetailsList()
{
  return notificationList;
}

//--------------------------------------------------------------------------------
