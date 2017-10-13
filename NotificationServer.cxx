#include <NotificationServer.hxx>
#include <NotificationList.hxx>

#include <notificationsadaptor.h>

#include <QIcon>
#include <QDBusConnection>
#include <QDebug>

#include <KLocalizedString>

//--------------------------------------------------------------------------------

NotificationServer::NotificationServer(QWidget *parent)
  : SysTrayItem(parent)
{
  new NotificationsAdaptor(this);

  setPixmap(QIcon::fromTheme("preferences-desktop-notification").pixmap(size()));

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
            setToolTip(i18np("%1 notification", "%1 notifications", notificationList->itemCount()));
          }
         );

  hide();
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
           ;
}

//--------------------------------------------------------------------------------

void NotificationServer::CloseNotification(uint id)
{
  qDebug() << "close notif" << id;
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
  //qDebug() << "app" << app_name << "summary" << summary << "body" << theBody << "timeout" << timeout << "replaceId" << replaces_id;
  QString body(theBody);
  body.replace("\n", "<br>");
  notificationList->addItem(notifyId, app_name, summary, body, QIcon::fromTheme(app_icon));
  show();

  return notifyId++;
}

//--------------------------------------------------------------------------------

QWidget *NotificationServer::getDetailsList()
{
  return notificationList;
}

//--------------------------------------------------------------------------------
