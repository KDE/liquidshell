// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 - 2020 Martin Koller, kollix@aon.at

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
    new NotificationServer(this),
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
    if ( item->isVisible() )
      visibleItems++;

  const int MAX_COLUMNS = static_cast<int>(std::ceil(visibleItems / float(appsRows.count())));
  int row = 0;
  foreach (SysTrayNotifyItem *item, items)
  {
    if ( item->isVisible() )
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
