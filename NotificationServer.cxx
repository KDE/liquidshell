/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <NotificationServer.hxx>
#include <NotificationList.hxx>

#include <notificationsadaptor.h>

#include <QIcon>
#include <QDBusConnection>
#include <QDebug>

#include <KLocalizedString>
#include <KService>
#include <KIconLoader>

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

  setAvoidPopup(getAvoidPopup());  // adjust icon for current state

  connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this,
          [this]() { setAvoidPopup(getAvoidPopup()); });
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
  //qDebug() << "CloseNotification" << id;
  notificationList->closeItem(id);
  emit NotificationClosed(id, CloseReason::Closed);
}

//--------------------------------------------------------------------------------

QString NotificationServer::GetServerInformation(QString &vendor, QString &version, QString &spec_version)
{
  vendor = "KDE";
  version = "1.0";
  spec_version = "1.2";
  return "liquidshell";
}

//--------------------------------------------------------------------------------

uint NotificationServer::Notify(const QString &app_name, uint replaces_id, const QString &app_icon,
                                const QString &summary, const QString &theBody, const QStringList &actions,
                                const QVariantMap &hints, int timeout)
{
  //qDebug() << "app" << app_name << "summary" << summary << "body" << theBody << "timeout" << timeout
           //<< "replaceId" << replaces_id << "hints" << hints << "actions" << actions << "app_icon" << app_icon;

  uint newId;

  if ( replaces_id != 0 )
    notificationList->closeItem(newId = replaces_id);  // reuse id
  else
    newId = notifyId++;

  QString body(theBody);
  body.replace("\n", "<br>");

  // icon preference order: https://specifications.freedesktop.org/notification-spec/latest/ar01s05.html
  QIcon icon;
  if ( hints.contains("image-data") )
    icon = hints["image-data"].value<QIcon>();

  if ( icon.isNull() && hints.contains("image-path") )
    icon = QIcon(hints["image-path"].toString());

  if ( icon.isNull() && !app_icon.isEmpty() )
    icon = QIcon::fromTheme(app_icon);

  if ( icon.isNull() && hints.contains("icon_data") )
    icon = hints["icon_data"].value<QIcon>();

  QString appName = app_name;
  if ( appName.isEmpty() && hints.contains("desktop-entry") )
  {
    KService::Ptr service = KService::serviceByDesktopName(hints["desktop-entry"].toString().toLower());
    if ( service )
      appName = service->name();
  }

  notificationList->addItem(newId, appName, summary, body, icon, actions, hints, timeout);

  return newId;
}

//--------------------------------------------------------------------------------

QWidget *NotificationServer::getDetailsList()
{
  return notificationList;
}

//--------------------------------------------------------------------------------

void NotificationServer::setAvoidPopup(bool avoid)
{
  notificationList->setAvoidPopup(avoid);

  if ( avoid )
    setPixmap(QIcon::fromTheme(iconName).pixmap(size(), QIcon::Disabled));
  else
    setPixmap(QIcon::fromTheme(iconName).pixmap(size()));
}

//--------------------------------------------------------------------------------

bool NotificationServer::getAvoidPopup() const
{
  return notificationList->getAvoidPopup();
}

//--------------------------------------------------------------------------------
