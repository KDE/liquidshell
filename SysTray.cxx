/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <SysTray.hxx>
#include <DBusTypes.hxx>
#include <NotificationServer.hxx>
#include <Network.hxx>
#include <DeviceNotifier.hxx>
#include <Battery.hxx>
#include <Bluetooth.hxx>

#ifdef WITH_PACKAGEKIT
#include <PkUpdates.hxx>
#endif

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QDBusMetaType>
#include <QDebug>

#include <KLocalizedString>

#include <cmath>

//--------------------------------------------------------------------------------

static const QLatin1String WATCHER_SERVICE("org.kde.StatusNotifierWatcher");

//--------------------------------------------------------------------------------

SysTray::SysTray(DesktopPanel *parent)
  : QFrame(parent)
{
  qRegisterMetaType<KDbusImageStruct>("KDbusImageStruct");
  qRegisterMetaType<KDbusImageVector>("KDbusImageVector");

  qDBusRegisterMetaType<KDbusImageStruct>();
  qDBusRegisterMetaType<KDbusImageVector>();
  qDBusRegisterMetaType<KDbusToolTipStruct>();

  setFrameShape(QFrame::StyledPanel);
  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

  connect(parent, &DesktopPanel::rowsChanged, this, &SysTray::fill);

  QHBoxLayout *hbox = new QHBoxLayout(this);
  hbox->setContentsMargins(QMargins(4, 0, 4, 0));

  vbox = new QVBoxLayout;
  vbox->setContentsMargins(QMargins());
  vbox->setSpacing(4);

  QFrame *separator = new QFrame;
  separator->setFrameStyle(QFrame::Plain);
  separator->setFrameShape(QFrame::VLine);

  appsVbox = new QVBoxLayout;
  appsVbox->setContentsMargins(QMargins());
  appsVbox->setSpacing(4);

  hbox->addLayout(vbox);
  hbox->addWidget(separator);
  hbox->addLayout(appsVbox);

  if ( QDBusConnection::sessionBus().isConnected() )
  {
    serviceName = QString("org.kde.StatusNotifierHost-%1").arg(QApplication::applicationPid());
    QDBusConnection::sessionBus().registerService(serviceName);

    registerWatcher();
  }

  fill();
}

//--------------------------------------------------------------------------------

void SysTray::fill()
{
  // delete all internal widgets
  QLayoutItem *child;
  while ( (child = vbox->takeAt(0)) )
  {
    if ( child->layout() )
    {
      while ( QLayoutItem *widgetItem = child->layout()->takeAt(0) )
        delete widgetItem->widget();
    }

    delete child;
  }

  const int MAX_ROWS = qobject_cast<DesktopPanel *>(parentWidget())->getRows();

  QVector<QHBoxLayout *> rowsLayout(MAX_ROWS);
  for (int i = 0; i < MAX_ROWS; i++)
  {
    rowsLayout[i] = new QHBoxLayout;
    rowsLayout[i]->setContentsMargins(QMargins());
    rowsLayout[i]->setSpacing(4);
    vbox->addLayout(rowsLayout[i]);
  }

  QVector<QWidget *> internalWidgets =
  {
    notificationServer = new NotificationServer(this),
    new Network(this),
    new DeviceNotifier(this),
    new Battery(this),
    new Bluetooth(this),
#ifdef WITH_PACKAGEKIT
    new PkUpdates(this)
#endif
  };

  for (int i = 0; i < internalWidgets.count(); i++)
    rowsLayout[i % MAX_ROWS]->addWidget(internalWidgets[i], 0, Qt::AlignLeft);

  for (int i = 0; i < MAX_ROWS; i++)
    rowsLayout[i]->addStretch();

  // notifier items

  qDeleteAll(appsRows);
  appsRows.clear();
  appsRows.resize(MAX_ROWS);
  for (int i = 0; i < MAX_ROWS; i++)
  {
    appsRows[i] = new QHBoxLayout;
    appsRows[i]->setContentsMargins(QMargins());
    appsRows[i]->setSpacing(4);
    appsVbox->addLayout(appsRows[i]);
  }

  arrangeNotifyItems();
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

  if ( items.contains(item) )  // we have this already (e.g. try kded5 --replace)
    return;

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
  sysItem->hide();  // until initialized

  connect(sysItem, &SysTrayNotifyItem::initialized, this, &SysTray::itemInitialized);
}

//--------------------------------------------------------------------------------

void SysTray::itemInitialized(SysTrayNotifyItem *item)
{
  if ( !items.contains(item->objectName()) )
    items.insert(item->objectName(), item);

  arrangeNotifyItems();  // show/hide icons
}

//--------------------------------------------------------------------------------

void SysTray::itemUnregistered(QString item)
{
  //qDebug() << "itemUnregistered" << item;

  if ( items.contains(item) )
  {
    delete items.take(item);
    arrangeNotifyItems();
  }
}

//--------------------------------------------------------------------------------

void SysTray::arrangeNotifyItems()
{
  // cut all items from all layouts
  foreach (QHBoxLayout *row, appsRows)
  {
    while ( QLayoutItem *item = row->itemAt(0) )
    {
      if ( item->widget() )
        row->removeWidget(item->widget());  // take widget out but do not delete it
      else
        delete row->takeAt(0);  // spacer items
    }
  }

  // rearrange visible items - "row first" layout
  int visibleItems = 0;
  foreach (SysTrayNotifyItem *item, items)
    if ( item && item->isVisible() )
      visibleItems++;

  const int MAX_COLUMNS = static_cast<int>(std::ceil(visibleItems / float(appsRows.count())));
  int row = 0;
  foreach (SysTrayNotifyItem *item, items)
  {
    if ( item && item->isVisible() )
    {
      appsRows[row]->addWidget(item, 0, Qt::AlignLeft);
      if ( appsRows[row]->count() == MAX_COLUMNS )
        row++;
    }
  }

  foreach (QHBoxLayout *row, appsRows)
    row->addStretch();
}

//--------------------------------------------------------------------------------

void SysTray::contextMenuEvent(QContextMenuEvent *event)
{
  Q_UNUSED(event)

  // allow to disable notification popups (e.g. during a presentation)
  if ( notificationServer )
  {
    QMenu menu;

    QAction *action = menu.addAction(i18n("Show New Notifications"));
    action->setCheckable(true);
    action->setChecked(!notificationServer->getAvoidPopup());

    connect(action, &QAction::triggered,
            [this](bool checked)
            {
              notificationServer->setAvoidPopup(!checked);
            });

    menu.exec(QCursor::pos());
  }
}

//--------------------------------------------------------------------------------
