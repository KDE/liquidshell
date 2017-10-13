#include <SysTray.hxx>
#include <DBusTypes.hxx>
#include <NotificationServer.hxx>
#include <Network.hxx>
#include <DeviceNotifier.hxx>
#include <Battery.hxx>
#include <Bluetooth.hxx>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QDBusMetaType>
#include <QDebug>

const int MAX_ROWS = 2;

//--------------------------------------------------------------------------------

static const QLatin1String WATCHER_SERVICE("org.kde.StatusNotifierWatcher");

//--------------------------------------------------------------------------------

SysTray::SysTray(QWidget *parent)
  : QFrame(parent)
{
  qRegisterMetaType<KDbusImageStruct>("KDbusImageStruct");
  qRegisterMetaType<KDbusImageVector>("KDbusImageVector");

  qDBusRegisterMetaType<KDbusImageStruct>();
  qDBusRegisterMetaType<KDbusImageVector>();
  qDBusRegisterMetaType<KDbusToolTipStruct>();

  setFrameShape(QFrame::StyledPanel);

  QHBoxLayout *hbox = new QHBoxLayout(this);

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->setContentsMargins(QMargins());
  vbox->setSpacing(4);

  QHBoxLayout *rowsLayout[MAX_ROWS];
  for (int i = 0; i < MAX_ROWS; i++)
  {
    rowsLayout[i] = new QHBoxLayout;
    rowsLayout[i]->setContentsMargins(QMargins());
    rowsLayout[i]->setSpacing(4);
    vbox->addLayout(rowsLayout[i]);
  }

  QFrame *separator = new QFrame;
  separator->setFrameStyle(QFrame::Plain);
  separator->setFrameShape(QFrame::VLine);

  QVBoxLayout *appsVbox = new QVBoxLayout;
  appsVbox->setContentsMargins(QMargins());
  appsVbox->setSpacing(4);

  appsRows.resize(MAX_ROWS);
  for (int i = 0; i < MAX_ROWS; i++)
  {
    appsRows[i] = new QHBoxLayout;
    appsRows[i]->setContentsMargins(QMargins());
    appsRows[i]->setSpacing(4);
    appsVbox->addLayout(appsRows[i]);
  }

  hbox->addLayout(vbox);
  hbox->addWidget(separator);
  hbox->addLayout(appsVbox);

  if ( QDBusConnection::sessionBus().isConnected() )
  {
    serviceName = QString("org.kde.StatusNotifierHost-%1").arg(QApplication::applicationPid());
    QDBusConnection::sessionBus().registerService(serviceName);

    registerWatcher();
  }

  rowsLayout[0]->addWidget(new NotificationServer(this), 0, Qt::AlignLeft);
  rowsLayout[1]->addWidget(new Network(this), 0, Qt::AlignLeft);
  rowsLayout[0]->addWidget(new DeviceNotifier(this), 0, Qt::AlignLeft);
  rowsLayout[1]->addWidget(new Battery(this), 0, Qt::AlignLeft);
  rowsLayout[0]->addWidget(new Bluetooth(this), 0, Qt::AlignLeft);
}

//--------------------------------------------------------------------------------

void SysTray::registerWatcher()
{
  QDBusMessage msg = 
      QDBusMessage::createMethodCall(WATCHER_SERVICE, "/StatusNotifierWatcher",
                                     "org.kde.StatusNotifierWatcher",
                                     "RegisterStatusNotifierHost");

  msg << serviceName;

  QDBusConnection::sessionBus().send(msg);

  // get list of currently existing items
  msg = QDBusMessage::createMethodCall(WATCHER_SERVICE, "/StatusNotifierWatcher",
                                       "org.freedesktop.DBus.Properties",
                                       "Get");

  msg << "org.kde.StatusNotifierWatcher" << "RegisteredStatusNotifierItems";
  QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
  QDBusPendingCallWatcher *pendingCallWatcher = new QDBusPendingCallWatcher(call, this);
  connect(pendingCallWatcher, &QDBusPendingCallWatcher::finished, this,
          [this](QDBusPendingCallWatcher *w)
          {
            w->deleteLater();
            QDBusPendingReply<QDBusVariant> reply = *w;
            QStringList items = reply.value().variant().toStringList();
            for (const QString &item : items)
              itemRegistered(item);
          }
         );

  // connect for new items
  QDBusConnection::sessionBus().
      connect(WATCHER_SERVICE, "/StatusNotifierWatcher",
              "org.kde.StatusNotifierWatcher",
              "StatusNotifierItemRegistered",
              this, SLOT(itemRegistered(QString)));

  // connect for removed items
  QDBusConnection::sessionBus().
      connect(WATCHER_SERVICE, "/StatusNotifierWatcher",
              "org.kde.StatusNotifierWatcher",
              "StatusNotifierItemUnregistered",
              this, SLOT(itemUnregistered(QString)));
}

//--------------------------------------------------------------------------------

void SysTray::itemRegistered(QString item)
{
  //qDebug() << "itemRegistered" << item;

  int slash = item.indexOf('/');
  if ( slash < 1 )
    return;

  QString service = item.left(slash);
  QString path = item.mid(slash);

  // create item but insert it into the layout just when it was initialized
  // since it might be an invalid item which has no StatusNotifierItem path
  // and will delete itself on error
  SysTrayNotifyItem *sysItem = new SysTrayNotifyItem(this, service, path);
  sysItem->setObjectName(item);
  sysItem->setFixedSize(QSize(22, 22));

  connect(sysItem, &SysTrayNotifyItem::initialized, this, &SysTray::itemInitialized);
}

//--------------------------------------------------------------------------------

void SysTray::itemInitialized(SysTrayNotifyItem *item)
{
  int lowestCount = 0;
  int lowestCountAt = 0;
  for (int i = 0; i < appsRows.count(); i++)
  {
    if ( i == 0 )
      lowestCount = appsRows[i]->count();
    else if ( appsRows[i]->count() < lowestCount )
    {
      lowestCount = appsRows[i]->count();
      lowestCountAt = i;
    }
  }

  appsRows[lowestCountAt]->addWidget(item, 0, Qt::AlignLeft);

  if ( !items.contains(item->objectName()) )
  {
    items.insert(item->objectName(), item);
    disconnect(item, &SysTrayNotifyItem::initialized, this, &SysTray::itemInitialized);
  }
}

//--------------------------------------------------------------------------------

void SysTray::itemUnregistered(QString item)
{
  //qDebug() << "itemUnregistered" << item;

  if ( items.contains(item) )
    delete items.take(item);
}

//--------------------------------------------------------------------------------
