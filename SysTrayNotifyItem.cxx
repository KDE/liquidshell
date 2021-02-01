// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 - 2021 Martin Koller, kollix@aon.at

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

#include <SysTrayNotifyItem.hxx>

#include <statusnotifieritem_interface.h>

#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QIcon>
#include <QWheelEvent>
#include <QContextMenuEvent>
#include <QPainter>

#include <KWindowSystem>

//--------------------------------------------------------------------------------
// See https://www.freedesktop.org/wiki/Specifications/StatusNotifierItem/StatusNotifierItem

SysTrayNotifyItem::SysTrayNotifyItem(QWidget *parent, const QString &service, const QString &path)
  : QLabel("?", parent)
{
  // make sure several "New*" signals from dbus just trigger one fetchData() call
  fetchTimer.setSingleShot(true);
  fetchTimer.setInterval(100);
  connect(&fetchTimer, &QTimer::timeout, this, &SysTrayNotifyItem::fetchData);

  dbus = new OrgKdeStatusNotifierItem(service, path, QDBusConnection::sessionBus(), this);

  connect(dbus, &OrgKdeStatusNotifierItem::NewAttentionIcon, this, &SysTrayNotifyItem::startTimer);
  connect(dbus, &OrgKdeStatusNotifierItem::NewIcon,          this, &SysTrayNotifyItem::startTimer);
  connect(dbus, &OrgKdeStatusNotifierItem::NewOverlayIcon,   this, &SysTrayNotifyItem::startTimer);
  connect(dbus, &OrgKdeStatusNotifierItem::NewStatus,        this, &SysTrayNotifyItem::startTimer);
  connect(dbus, &OrgKdeStatusNotifierItem::NewTitle,         this, &SysTrayNotifyItem::startTimer);
  connect(dbus, &OrgKdeStatusNotifierItem::NewToolTip,       this, &SysTrayNotifyItem::startTimer);

  fetchData();
}

//--------------------------------------------------------------------------------

void SysTrayNotifyItem::startTimer()
{
  fetchTimer.start();
}

//--------------------------------------------------------------------------------

void SysTrayNotifyItem::fetchData()
{
  QDBusMessage msg =
      QDBusMessage::createMethodCall(dbus->service(), dbus->path(),
                                     "org.freedesktop.DBus.Properties",
                                     "GetAll");

  msg << dbus->interface();

  QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
  QDBusPendingCallWatcher *pendingCallWatcher = new QDBusPendingCallWatcher(call, this);
  connect(pendingCallWatcher, &QDBusPendingCallWatcher::finished, this, &SysTrayNotifyItem::fetchDataReply);
}

//--------------------------------------------------------------------------------

void SysTrayNotifyItem::fetchDataReply(QDBusPendingCallWatcher *w)
{
  w->deleteLater();
  QDBusPendingReply<QVariantMap> reply = *w;

  if ( reply.isError() )
  {
    //qDebug() << dbus->service() << reply.error();
    deleteLater();
    return;
  }

  /*
  qDebug() << dbus->title() << dbus->service() << dbus->path() << dbus->interface();
  qDebug() << "att pixmap file:" << dbus->attentionIconName();
  qDebug() << "pixmap file:" << dbus->iconName();
  qDebug() << "overlay pixmap file:" << dbus->overlayIconName();
  qDebug() << "IconThemePath" << dbus->iconThemePath();
  qDebug() << "ItemIsMenu" << dbus->itemIsMenu();
  qDebug() << "Menu" << dbus->menu().path();
  */

  menuPath = dbus->menu().path();

  QStringList origThemePaths = QIcon::themeSearchPaths();

  QString path = dbus->iconThemePath();
  if ( !path.isEmpty() )
  {
    QStringList paths = origThemePaths;
    paths.prepend(path);
    QIcon::setThemeSearchPaths(paths);
  }

  QPixmap attentionPixmap = dbus->attentionIconPixmap().pixmap(size());
  if ( attentionPixmap.isNull() )
    attentionPixmap = QIcon::fromTheme(dbus->attentionIconName(), QIcon()).pixmap(size());

  QPixmap pixmap = dbus->iconPixmap().pixmap(size());

  if ( pixmap.isNull() )
  {
    pixmap = QIcon::fromTheme(dbus->iconName(), QIcon()).pixmap(size());

    if ( pixmap.isNull() && !dbus->iconName().isEmpty() && !dbus->iconThemePath().isEmpty() )
    {
      // the file should be findable, but probably the iconThemePath does not contain the index.theme file
      pixmap = findPixmap(dbus->iconName(), dbus->iconThemePath());
    }
  }

  QPixmap overlay = dbus->overlayIconPixmap().pixmap(size());
  if ( overlay.isNull() )
    overlay = QIcon::fromTheme(dbus->overlayIconName(), QIcon()).pixmap(size());

  // reset to orig since this setting is application wide
  QIcon::setThemeSearchPaths(origThemePaths);

  QPixmap finalPixmap;

  if ( !attentionPixmap.isNull() && (dbus->status() == "NeedsAttention") )
    finalPixmap = applyOverlay(attentionPixmap, overlay);
  else if ( !pixmap.isNull() )
    finalPixmap = applyOverlay(pixmap, overlay);

  if ( (dbus->id() == "KMail") || (dbus->id() == "Akregator") )
  {
    // hack for the unwillingness of the kmail maintainer to show unread message count on icon
    QString text = dbus->toolTip().subTitle.left(dbus->toolTip().subTitle.indexOf(QChar(' ')));
    bool ok = false;
    int num = text.toInt(&ok);
    if ( ok && (num < 100) )
    {
      QPainter painter(&finalPixmap);
      painter.setPen(Qt::blue);
      painter.drawText(finalPixmap.rect(), Qt::AlignCenter, QString::number(num));
    }
  }

  if ( !finalPixmap.isNull() )
    setPixmap(finalPixmap);

  QString tip(dbus->title());

  if ( !dbus->toolTip().icon.isEmpty() ||
       !dbus->toolTip().image.isNull() ||
       !dbus->toolTip().title.isEmpty() ||
       !dbus->toolTip().subTitle.isEmpty() )
  {
    tip = QString("<html><center><b>%1</b><br>%2</center></html>")
                  .arg(dbus->toolTip().title)
                  .arg(dbus->toolTip().subTitle);
  }

  setToolTip(tip);
  show();

  if ( dbus->status() == "Passive" )
  {
    // TODO make it configurable
    if ( dbus->id() == "KMail" )
      hide();
  }

  emit initialized(this);
}

//--------------------------------------------------------------------------------

QPixmap SysTrayNotifyItem::findPixmap(const QString &name, const QString &path)
{
  QDir dir(path);
  QFileInfoList infoList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

  for (const QFileInfo &info : infoList)
  {
    if ( info.fileName().startsWith(name) )    // maybe check for xxx and xxx. to match xxx.png etc when only xxx given
      return QPixmap(info.absoluteFilePath()).scaled(size(), Qt::KeepAspectRatio);

    if ( info.isDir() )
      return findPixmap(name, info.absoluteFilePath());
  }

  return QPixmap();
}

//--------------------------------------------------------------------------------

QPixmap SysTrayNotifyItem::applyOverlay(const QPixmap &pixmap, const QPixmap &overlay)
{
  QPixmap result(pixmap);

  if ( !overlay.isNull() )
  {
    QPainter painter(&result);
    painter.drawPixmap(0, 0, overlay);
    painter.end();
  }

  return result;
}

//--------------------------------------------------------------------------------

void SysTrayNotifyItem::wheelEvent(QWheelEvent *event)
{
  int delta;
  QString orientation;

  if ( event->angleDelta().x() != 0 )
  {
    orientation = "horizontal";
    delta = event->angleDelta().x();
  }
  else
  {
    orientation = "vertical";
    delta = event->angleDelta().y();
  }

  dbus->Scroll(delta, orientation);
  event->accept();
}

//--------------------------------------------------------------------------------

void SysTrayNotifyItem::mouseReleaseEvent(QMouseEvent *event)
{
  // need to do this in the release event since otherwise the popup opened
  // by the other application would immediately close again (I assume since this widget
  // still grabs the mouse)

  if ( event->button() == Qt::LeftButton )
  {
    QDBusPendingReply<> reply = dbus->Activate(event->globalPos().x(), event->globalPos().y());
    reply.waitForFinished();

    if ( !reply.isError() && dbus->windowId() )
    {
      WId wid = dbus->windowId();
      KWindowSystem::raiseWindow(wid);
      KWindowSystem::forceActiveWindow(wid);
    }
  }
  else if ( event->button() == Qt::RightButton )
  {
    QDBusPendingReply<> reply = dbus->ContextMenu(event->globalPos().x(), event->globalPos().y());
    reply.waitForFinished();

    if ( reply.isError() && (reply.error().type() == QDBusError::UnknownMethod) &&
         !menuPath.isEmpty() )
    {
      QDBusMessage msg =
          QDBusMessage::createMethodCall(dbus->service(), menuPath,
                                         "com.canonical.dbusmenu",
                                         "GetLayout");

      msg << 0;  // root node id
      msg << -1; // all items below root
      msg << QStringList(); // all properties

      QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
      QDBusPendingCallWatcher *pendingCallWatcher = new QDBusPendingCallWatcher(call, this);
      connect(pendingCallWatcher, &QDBusPendingCallWatcher::finished, this, &SysTrayNotifyItem::menuLayoutReply);
    }
  }
}

//--------------------------------------------------------------------------------

void SysTrayNotifyItem::menuLayoutReply(QDBusPendingCallWatcher *w)
{
  w->deleteLater();

  QDBusMenuItem::registerDBusTypes();
  QDBusPendingReply<unsigned int, QDBusMenuLayoutItem> reply = *w;

  if ( reply.isError() )
  {
    //qDebug() << dbus->service() << reply.error();
    return;
  }

  QDBusMenuLayoutItem item = reply.argumentAt<1>();

  QMenu menu;
  fillMenu(menu, item);

  QAction *action = menu.exec(QCursor::pos());

  if ( !action )
    return;

  QDBusMessage msg =
      QDBusMessage::createMethodCall(dbus->service(), menuPath,
                                     "com.canonical.dbusmenu",
                                     "Event");

  msg << action->data().toInt();
  msg << "clicked";
  msg << QVariant::fromValue(QDBusVariant(0));
  msg << (unsigned)QDateTime::currentDateTime().toSecsSinceEpoch();

  QDBusConnection::sessionBus().asyncCall(msg);
}

//--------------------------------------------------------------------------------
// https://github.com/gnustep/libs-dbuskit/blob/master/Bundles/DBusMenu/com.canonical.dbusmenu.xml

void SysTrayNotifyItem::fillMenu(QMenu &menu, const QDBusMenuLayoutItem &item)
{
  QString type = item.m_properties.value("type", "standard").toString();

  if ( type == "separator" )
    menu.addSeparator();
  else if ( type == "standard" )
  {
    if ( item.m_properties.value("visible", true).toBool() )
    {
      QIcon icon;

      QString iconName = item.m_properties.value("icon-name").toString();
      if ( !iconName.isEmpty() )
      {
        QStringList origThemePaths = QIcon::themeSearchPaths();

        QString path = dbus->iconThemePath();
        if ( !path.isEmpty() )
        {
          QStringList paths = origThemePaths;
          paths.append(path);
          QIcon::setThemeSearchPaths(paths);
        }

        icon = QIcon::fromTheme(iconName);

        // reset to orig since this setting is application wide
        QIcon::setThemeSearchPaths(origThemePaths);
      }
      else
      {
        QByteArray pixmapData = item.m_properties.value("icon-data").toByteArray();
        if ( !pixmapData.isEmpty() )
        {
          QPixmap pixmap;
          pixmap.loadFromData(pixmapData, "PNG");
          icon = QIcon(pixmap);
        }
      }

      QString title, label = item.m_properties.value("label").toString();
      bool foundAccessKey = false;
      for (int i = 0; i < label.length(); i++)
      {
        if ( label[i] != '_' )
          title += label[i];
        else if ( (i < (label.length() - 1)) && (label[i + 1] == '_') )
        {
          title += '_';  // two consecutive underscore characters "__" are displayed as a single underscore
          i++;
        }
        else if ( i != (label.length() - 1) && !foundAccessKey )
        {
          foundAccessKey = true;
          title += QString('&') + label[i];
        }
      }

      if ( item.m_properties.value("children-display").toString() == "submenu" )
      {
        QMenu *submenu = title.isEmpty() ? &menu : menu.addMenu(icon, title);

        for (const QDBusMenuLayoutItem &subItem : item.m_children)
        {
          fillMenu(*submenu, subItem);

          if ( submenu != &menu )
            menu.addMenu(submenu);
        }
      }
      else
      {
        QAction *action = menu.addAction(icon, title);
        action->setEnabled(item.m_properties.value("enabled", true).toBool());

        if ( item.m_properties.value("toggle-type").toString() == "checkmark" )
        {
          action->setCheckable(true);
          int state = item.m_properties.value("toggle-state").toInt();
          if ( state == 1 )
            action->setChecked(true);
        }

        action->setData(item.m_id);
      }
    }
  }
}

//--------------------------------------------------------------------------------
