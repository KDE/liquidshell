// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 - 2023 Martin Koller, martin@kollix.at

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
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDebug>
#include <QImage>
#include <QPixmap>

#include <KLocalizedString>
#include <KService>
#include <KIconLoader>

//--------------------------------------------------------------------------------
// https://specifications.freedesktop.org/notification-spec/latest/icons-and-images.html
// image-data / icon_data: (iiibiiay) raw RGB(A), not a QIcon

static QIcon iconFromImageHint(const QVariant &hint)
{
  if ( !hint.canConvert<QDBusArgument>() )
    return QIcon();

  const QDBusArgument arg = hint.value<QDBusArgument>();
  if ( arg.currentType() != QDBusArgument::StructureType )
    return QIcon();

  int width = 0, height = 0, rowStride = 0, bitsPerSample = 0, channels = 0;
  bool hasAlpha = false;
  QByteArray pixels;

  arg.beginStructure();
  arg >> width >> height >> rowStride >> hasAlpha >> bitsPerSample >> channels >> pixels;
  arg.endStructure();

  if ( (width <= 0) || (height <= 0) || (width >= 2048) || (height >= 2048) || (rowStride <= 0) )
    return QIcon();
  if ( (bitsPerSample != 8) || ((channels != 3) && (channels != 4)) )
    return QIcon();
  if ( hasAlpha != (channels == 4) )
    return QIcon();
  if ( rowStride < width * channels )
    return QIcon();

  QImage image(width, height, hasAlpha ? QImage::Format_ARGB32 : QImage::Format_RGB32);
  const char *ptr = pixels.constData();
  const char *end = ptr + pixels.size();

  for (int y = 0; y < height; ++y, ptr += rowStride)
  {
    if ( ptr + channels * width > end )
      break;

    QRgb *dst = reinterpret_cast<QRgb *>(image.scanLine(y));
    if ( hasAlpha )
    {
      for (int x = 0; x < width; ++x)
        dst[x] = qRgba(ptr[x * 4], ptr[x * 4 + 1], ptr[x * 4 + 2], ptr[x * 4 + 3]);
    }
    else
    {
      for (int x = 0; x < width; ++x)
        dst[x] = qRgb(ptr[x * 3], ptr[x * 3 + 1], ptr[x * 3 + 2]);
    }
  }

  return QIcon(QPixmap::fromImage(image));
}

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
    icon = iconFromImageHint(hints["image-data"]);
  else if ( hints.contains("image_data") )  // spec 1.1
    icon = iconFromImageHint(hints["image_data"]);

  if ( icon.isNull() && hints.contains("image-path") )
    icon = QIcon(hints["image-path"].toString());
  else if ( icon.isNull() && hints.contains("image_path") )
    icon = QIcon(hints["image_path"].toString());

  if ( icon.isNull() && !app_icon.isEmpty() )
    icon = QIcon::fromTheme(app_icon);

  if ( icon.isNull() && hints.contains("icon_data") )
    icon = iconFromImageHint(hints["icon_data"]);

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
