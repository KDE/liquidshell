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

#include <NotificationList.hxx>
#include <DesktopWidget.hxx>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QToolButton>
#include <QPushButton>
#include <QPointer>
#include <QTimer>
#include <QTime>
#include <QApplication>
#include <QScreen>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>

#include <KRun>
#include <KLocalizedString>
#include <KWindowSystem>

static const Qt::WindowFlags POPUP_FLAGS = Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint;

//--------------------------------------------------------------------------------

NotifyItem::NotifyItem(QWidget *parent, uint theId, const QString &app,
                       const QString &theSummary, const QString &theBody, const QIcon &icon,
                       const QStringList &theActions)
  : QFrame(parent, POPUP_FLAGS), id(theId), appName(app), summary(theSummary), body(theBody), actions(theActions)
{
  setAttribute(Qt::WA_ShowWithoutActivating);  // avoid focus stealing

  setFrameShape(QFrame::StyledPanel);
  QMargins margins = contentsMargins();
  margins.setRight(0); // allow the close button to reach to the right screen border - easier to click
  setContentsMargins(margins);

  QVBoxLayout *vbox = new QVBoxLayout;
  timeLabel = new QLabel;
  iconLabel = new QLabel;
  vbox->addWidget(timeLabel, 0, Qt::AlignTop | Qt::AlignHCenter);
  vbox->addWidget(iconLabel, 0, Qt::AlignTop | Qt::AlignHCenter);

  QVBoxLayout *centerBox = new QVBoxLayout;

  QHBoxLayout *hbox = new QHBoxLayout(this);
  margins = hbox->contentsMargins();
  margins.setRight(0); // allow the close button to reach to the right screen border - easier to click
  hbox->setContentsMargins(margins);

  textLabel = new QLabel;
  QToolButton *closeButton = new QToolButton;
  // easier to click with larger size
  closeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  closeButton->setMinimumWidth(40);
  closeButton->setAutoRaise(true);
  closeButton->setIcon(QIcon::fromTheme("window-close"));
  connect(closeButton, &QToolButton::clicked, this, &NotifyItem::deleteLater);

  textLabel->setWordWrap(true);
  textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
  textLabel->setOpenExternalLinks(true);
  textLabel->setMinimumWidth(300);
  textLabel->setMaximumWidth(600);
  textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  centerBox->addWidget(textLabel);

  if ( actions.count() )
  {
    QHBoxLayout *actionBox = new QHBoxLayout;
    centerBox->addLayout(actionBox);

    for (int i = 0; i < actions.count(); i++)
    {
      if ( ((i % 2) != 0) && !actions[i].isEmpty() )  // id
      {
        QPushButton *button = new QPushButton;
        button->setText(actions[i]);
        actionBox->addWidget(button);
        QString key = actions[i - 1];

        connect(button, &QPushButton::clicked, this,
                [this, key]()
                {
                  QDBusMessage msg =
                      QDBusMessage::createSignal("/org/freedesktop/Notifications",
                                                 "org.freedesktop.Notifications",
                                                 "ActionInvoked");
                  msg << id << key;

                  QDBusConnection::sessionBus().send(msg);
                });
      }
    }
  }

  hbox->addLayout(vbox);
  hbox->addLayout(centerBox);
  hbox->addWidget(closeButton);

  iconLabel->setFixedSize(32, 32);
  iconLabel->setPixmap(icon.pixmap(iconLabel->size()));

  QString title = (appName == summary) ? appName : (appName + ": " + summary);
  textLabel->setText(QString("<html><center><b>%1</b></center><br>%2</html>")
                     .arg(title)
                     .arg(body));

  timeLabel->setText(QTime::currentTime().toString(Qt::SystemLocaleShortDate));
}

//--------------------------------------------------------------------------------

void NotifyItem::destroySysResources()
{
  destroy();
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

NotificationList::NotificationList(QWidget *parent)
  : QWidget(parent)
{
  setWindowFlags(windowFlags() | Qt::Tool);
  setWindowTitle(i18n("Notifications"));

  scrollArea = new QScrollArea;
  scrollArea->setWidgetResizable(true);

  QWidget *listWidget = new QWidget;
  listVbox = new QVBoxLayout(listWidget);
  listVbox->setContentsMargins(QMargins());
  listVbox->addStretch();

  scrollArea->setWidget(listWidget);

  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(QMargins());
  vbox->addWidget(scrollArea);

  QPushButton *clearButton = new QPushButton;
  clearButton->setIcon(QIcon::fromTheme("edit-clear-list"));
  connect(clearButton, &QPushButton::clicked, this, [this]() { for (NotifyItem *item : items) item->deleteLater(); });
  vbox->addWidget(clearButton);

  resize(500, 300);
}

//--------------------------------------------------------------------------------

NotificationList::~NotificationList()
{
  for (NotifyItem *item : items)
    item->disconnect();  // make sure destroyed() is no longer triggered
}

//--------------------------------------------------------------------------------

void NotificationList::addItem(uint id, const QString &appName, const QString &summary, const QString &body,
                               const QIcon &icon, const QStringList &actions, const QVariantMap &hints, int timeout)
{
  QPointer<NotifyItem> item = new NotifyItem(nullptr, id, appName, summary, body, icon, actions);
  item->resize(500, item->sizeHint().height());
  KWindowSystem::setState(item->winId(), NET::SkipTaskbar | NET::SkipPager);

  items.append(item.data());

  placeItems();

  int wordCount = body.splitRef(' ', QString::SkipEmptyParts).count();

  bool neverExpires = timeout == 0;  // according to spec

  if ( timeout <= 0 )
  {
    timeout = 4000 + 250 * wordCount;

    if ( actions.count() )
      timeout += 15000;  // give user more time to think ...
  }

  // 0=low, 1=normal, 2=critical
  if ( hints.contains("urgency") && (hints["urgency"].toInt() == 2) )
  {
    neverExpires = true;
    timeout = 20000;  // just show it longer since its urgent
  }

  bool transient = hints.contains("transient") ? hints["transient"].toBool() : false;

  // if there are actions, show it longer
  if ( actions.count() || neverExpires )
    transient = false;

  // exceptions ...
  if ( transient && hints.contains("x-kde-eventId") )
  {
    // I'd like to keep this much longer
    if ( hints["x-kde-eventId"].toString() == "new-email" )
      transient = false;
  }

  // I often get the same notification multiple times (e.g. KDE-connect or EWS akonadi resource)
  // but I don't want to fill up the screen with useless duplicate information. Therefore
  // whenever the same notification is received while another instance of it is still visible,
  // only put it in the list-view (it has a different id, therefore still store it), but don't show it as popup
  for (NotifyItem *it : items)
  {
    if ( (it != item) &&         // not the new item already in items
         !it->parentWidget() &&  // temporary item not in the list-view yet
         (appName == it->appName) && (summary == it->summary) &&
         (body == it->body) && (actions == it->actions) )
    {
      timeout = 0;
    }
  }

  numItems++;
  emit itemsCountChanged();

  connect(item.data(), &NotifyItem::destroyed, this,
          [this](QObject *obj)
          {
            items.removeOne(static_cast<NotifyItem *>(obj));
            placeItems();

            if ( --numItems == 0 )
            {
              hide();
              emit listNowEmpty();
            }
            else
              emit itemsCountChanged();
          }
         );

  QTimer::singleShot(timeout,
                     [=]()
                     {
                       if ( item )
                       {
                         // sometimes there were some leftover/half-functioning windows. This seems to avoid that
                         item->destroySysResources();

                         listVbox->insertWidget(listVbox->count() - 1, item);  // insert before stretch
                         item->show();

                         placeItems();  // reorder the remaining ones

                         scrollArea->setWidgetResizable(true);  // to update scrollbars, else next line does not work
                         scrollArea->ensureWidgetVisible(item);

                         if ( !neverExpires )
                         {
                           if ( !appTimeouts.contains(appName) )
                             appTimeouts.insert(appName, 10);  // default: 10 minutes lifetime

                           int expireTimeout;

                           if ( transient )
                             expireTimeout = 2 * 60 * 1000;  // still keep it a short time
                           else
                             expireTimeout = appTimeouts[appName] * 60 * 1000;

                           QTimer::singleShot(expireTimeout, item.data(), &NotifyItem::deleteLater);
                         }
                       }
                     }
                    );
}

//--------------------------------------------------------------------------------

void NotificationList::closeItem(uint id)
{
  for (NotifyItem *item : items)
  {
    if ( item->id == id )
    {
      item->deleteLater();
      break;
    }
  }
}

//--------------------------------------------------------------------------------

void NotificationList::placeItems()
{
  QRect screen = DesktopWidget::availableGeometry();
  QPoint point = parentWidget()->mapToGlobal(parentWidget()->pos());
  int x = point.x();
  int y = screen.bottom();

  for (NotifyItem *item : items)
  {
    if ( !item->parentWidget() )  // temporary item not in the list yet
    {
      y -= item->sizeHint().height();
      y -= 5; // a small space

      point.setX(std::min(x, screen.x() + screen.width() - item->sizeHint().width()));
      point.setY(y);
      item->move(point);
      item->show();
    }
  }
}

//--------------------------------------------------------------------------------
