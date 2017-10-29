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

#include <NotificationList.hxx>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QToolButton>
#include <QPointer>
#include <QTimer>
#include <QTime>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

#include <KRun>
#include <KLocalizedString>
#include <KWindowSystem>

static const Qt::WindowFlags POPUP_FLAGS = Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint;

//--------------------------------------------------------------------------------

NotifyItem::NotifyItem(QWidget *parent, uint theId, const QString &app,
                       const QString &summary, const QString &body, const QIcon &icon)
  : QFrame(parent, POPUP_FLAGS), id(theId), appName(app)
{
  setFrameShape(QFrame::StyledPanel);

  QVBoxLayout *vbox = new QVBoxLayout;
  timeLabel = new QLabel;
  iconLabel = new QLabel;
  vbox->addWidget(timeLabel, 0, Qt::AlignTop | Qt::AlignHCenter);
  vbox->addWidget(iconLabel, 0, Qt::AlignTop | Qt::AlignHCenter);

  QHBoxLayout *hbox = new QHBoxLayout(this);
  textLabel = new QLabel;
  QToolButton *closeButton = new QToolButton;
  closeButton->setAutoRaise(true);
  closeButton->setIcon(QIcon::fromTheme("window-close"));
  connect(closeButton, &QToolButton::clicked, this, &NotifyItem::deleteLater);

  textLabel->setWordWrap(true);
  textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
  textLabel->setOpenExternalLinks(true);
  textLabel->setMinimumWidth(300);
  textLabel->setMaximumWidth(600);
  textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  hbox->addLayout(vbox);
  hbox->addWidget(textLabel);
  hbox->addWidget(closeButton, 0, Qt::AlignTop);

  iconLabel->setFixedSize(32, 32);
  iconLabel->setPixmap(icon.pixmap(iconLabel->size()));

  QString title = (appName == summary) ? appName : (appName + ": " + summary);
  textLabel->setText(QString("<html><center><b>%1</b></center><br>%2</html>")
                     .arg(title)
                     .arg(body));

  timeLabel->setText(QTime::currentTime().toString(Qt::SystemLocaleShortDate));
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

NotificationList::NotificationList(QWidget *parent)
  : QScrollArea(parent)
{
  setWindowFlag(Qt::Tool);
  setWindowTitle(i18n("Notifications"));
  setWidgetResizable(true);

  QWidget *listWidget = new QWidget;
  listVbox = new QVBoxLayout(listWidget);
  listVbox->setContentsMargins(QMargins());
  listVbox->addStretch();

  setWidget(listWidget);

  resize(500, 300);
}

//--------------------------------------------------------------------------------

void NotificationList::addItem(uint id, const QString &appName, const QString &summary, const QString &body, const QIcon &icon)
{
  numItems++;
  emit itemsCountChanged();

  QPointer<NotifyItem> item = new NotifyItem(nullptr, id, appName, summary, body, icon);
  item->resize(500, item->sizeHint().height());
  KWindowSystem::setState(item->winId(), NET::SkipTaskbar | NET::SkipPager);

  QPoint point = parentWidget()->mapToGlobal(parentWidget()->pos());
  QRect screen = QApplication::desktop()->availableGeometry(this);
  point.setX(std::min(point.x(), screen.x() + screen.width() - item->sizeHint().width()));
  point.setY(screen.bottom() - item->sizeHint().height());
  item->move(point);
  item->show();

  connect(item.data(), &NotifyItem::destroyed,
          [this]()
          {
            if ( --numItems == 0 )
            {
              hide();
              emit listNowEmpty();
            }
            else
              emit itemsCountChanged();
          }
         );

  QTimer::singleShot(5000,
                     [item, appName, this]()
                     {
                       if ( item )
                       {
                         listVbox->insertWidget(listVbox->count() - 1, item);  // insert before stretch
                         item->show();

                         setWidgetResizable(true);  // to update scrollbars, else next line does not work
                         ensureWidgetVisible(item);

                         if ( !appTimeouts.contains(appName) )
                           appTimeouts.insert(appName, 10);  // default: 10 minutes lifetime

                         QTimer::singleShot(appTimeouts[appName] * 60 * 1000, item.data(), &NotifyItem::deleteLater);
                       }
                     }
                    );
}

//--------------------------------------------------------------------------------
