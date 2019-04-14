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
  qDebug() << dbus->service();
  qDebug() << "att pixmap file:" << dbus->attentionIconName();
  qDebug() << "pixmap file:" << dbus->iconName();
  qDebug() << "overlay pixmap file:" << dbus->overlayIconName();
  */

  QPixmap attentionPixmap = dbus->attentionIconPixmap().pixmap(size());
  if ( attentionPixmap.isNull() )
    attentionPixmap = QIcon::fromTheme(dbus->attentionIconName(), QIcon()).pixmap(size());

  QPixmap pixmap = dbus->iconPixmap().pixmap(size());

  if ( pixmap.isNull() )
    pixmap = QIcon::fromTheme(dbus->iconName(), QIcon()).pixmap(size());

  QPixmap overlay = dbus->overlayIconPixmap().pixmap(size());
  if ( overlay.isNull() )
    overlay = QIcon::fromTheme(dbus->overlayIconName(), QIcon()).pixmap(size());

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
    dbus->Activate(event->globalPos().x(), event->globalPos().y());
    WId wid = dbus->windowId();
    KWindowSystem::raiseWindow(wid);
    KWindowSystem::forceActiveWindow(wid);
  }
  else if ( event->button() == Qt::RightButton )
  {
    dbus->ContextMenu(event->globalPos().x(), event->globalPos().y());
  }
}

//--------------------------------------------------------------------------------
