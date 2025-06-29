/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <ClockWidget.hxx>
#include <ClockWidgetConfigureDialog.hxx>
#include <DesktopWidget.hxx>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QPushButton>
#include <QIcon>
#include <QAction>
#include <QTimeZone>

#include <KConfig>
#include <KConfigGroup>
#include <KCMultiDialog>
#include <KLocalizedString>
#include <KPluginMetaData>

//--------------------------------------------------------------------------------

CalendarPopup::CalendarPopup(QWidget *parent)
  : QFrame(parent)
{
  setWindowFlags(windowFlags() | Qt::Popup);
  setFrameShape(QFrame::StyledPanel);

  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(QMargins());

  timeLabel = new QLabel;
  timeLabel->setObjectName("calendar_time");
  QFont f = font();
  f.setPointSizeF(fontInfo().pointSizeF() * 1.5);
  timeLabel->setFont(f);
  timeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  dateLabel = new QLabel;
  dateLabel->setObjectName("calendar_date");
  dateLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  dateLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  timer.setInterval(1000);
  timer.start();

  connect(&timer, &QTimer::timeout, this, &CalendarPopup::tick);

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->setSpacing(20);
  hbox->addWidget(timeLabel);
  hbox->addWidget(dateLabel);
  vbox->addLayout(hbox);

  cal = new QCalendarWidget;
  vbox->addWidget(cal);

  QPushButton *today = new QPushButton(QIcon::fromTheme("go-jump-today"), QString());
  vbox->addWidget(today);
  connect(today, &QPushButton::clicked, this, &CalendarPopup::goToday);
}

//--------------------------------------------------------------------------------

void CalendarPopup::showEvent(QShowEvent *event)
{
  Q_UNUSED(event);

  timer.start();
  tick();
}

//--------------------------------------------------------------------------------

void CalendarPopup::hideEvent(QHideEvent *event)
{
  Q_UNUSED(event);

  timer.stop();  // avoid timer events we don't need
}

//--------------------------------------------------------------------------------

void CalendarPopup::tick()
{
  // show time and date in localized long form
  QDateTime dateTime = QDateTime::currentDateTime();
  timeLabel->setText(locale().toString(dateTime.time()));
  dateLabel->setText(locale().toString(dateTime.date()));
}

//--------------------------------------------------------------------------------

void CalendarPopup::goToday()
{
  cal->showToday();
  cal->setSelectedDate(QDate::currentDate());
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

ClockWidget::ClockWidget(DesktopPanel *parent)
  : QFrame(parent), calendar(nullptr)
{
  timeFormat = locale().timeFormat(QLocale::ShortFormat);

  ensurePolished();  // make sure we already have the css applied

  timer = new QTimer(this);

  // if seconds are shown, update every second, else less often
  if ( timeFormat.contains('s') )
    timer->setInterval(1000);
  else
    timer->setInterval(5000);

  timer->start();
  connect(timer, &QTimer::timeout, this, &ClockWidget::tick);

  connect(parent, &DesktopPanel::rowsChanged, this, &ClockWidget::fill);

  timeLabel = new QLabel(this);
  dayLabel = new QLabel(this);
  dateLabel = new QLabel(this);

  timeLabel->setObjectName("time");
  dayLabel->setObjectName("day");
  dateLabel->setObjectName("date");

  timeLabel->setTextFormat(Qt::PlainText);
  dayLabel->setTextFormat(Qt::PlainText);
  dateLabel->setTextFormat(Qt::PlainText);

  timeLabel->setAlignment(Qt::AlignCenter);
  dayLabel->setAlignment(Qt::AlignCenter);
  dateLabel->setAlignment(Qt::AlignCenter);

  QFont f = font();
  f.setPointSizeF(fontInfo().pointSizeF() * 1.5);
  f.setBold(true);
  timeLabel->setFont(f);

  fill();

  timeLabel->setVisible(!timeFormat.isEmpty());
  dayLabel->setVisible(!dayFormat.isEmpty());
  dateLabel->setVisible(!dateFormat.isEmpty());

  // context menu
  QAction *action = new QAction(this);
  action->setIcon(QIcon::fromTheme("configure"));
  action->setText(i18n("Select Timezones..."));
  addAction(action);
  connect(action, &QAction::triggered,
          [this]()
          {
            ClockWidgetConfigureDialog dialog(parentWidget(), timeZoneIds);
            dialog.setWindowTitle(i18n("Select Timezones"));
            dialog.resize(600, 400);
            if ( dialog.exec() == QDialog::Accepted )
            {
              timeZoneIds = dialog.getSelectedTimeZoneIds();
              KConfig config;
              KConfigGroup group = config.group("ClockWidget");
              QStringList list;
              for (const QByteArray &id : timeZoneIds)
                list.append(id);
              group.writeEntry("timeZoneIds", list);
              tick();  // update tooltip
            }
          }
         );

  action = new QAction(this);
  action->setIcon(QIcon::fromTheme("preferences-system-time"));
  action->setText(i18n("Configure Date & Time..."));
  addAction(action);
  connect(action, &QAction::triggered,
          [this]()
          {
            KCMultiDialog *dialog = new KCMultiDialog(parentWidget());
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->setWindowTitle(i18n("Date & Time"));

            dialog->addModule(KPluginMetaData("plasma/kcms/systemsettings_qwidgets/kcm_clock"));
            dialog->adjustSize();
            dialog->show();
          }
         );
  setContextMenuPolicy(Qt::ActionsContextMenu);

  // load config
  KConfig config;
  KConfigGroup group = config.group("ClockWidget");
  QStringList list;
  list = group.readEntry<QStringList>("timeZoneIds", QStringList());
  for (const QString &id : list)
    timeZoneIds.append(id.toLatin1());

  tick();

  timeLabel->setFixedHeight(timeLabel->fontMetrics().tightBoundingRect(timeLabel->text()).height() + 2);
  dayLabel->setFixedHeight(dayLabel->fontMetrics().tightBoundingRect(dayLabel->text()).height() + 4);
  dateLabel->setFixedHeight(dateLabel->fontMetrics().tightBoundingRect(dateLabel->text()).height() + 4);
}

//--------------------------------------------------------------------------------

void ClockWidget::fill()
{
  delete layout();

  const int MAX_ROWS = qobject_cast<DesktopPanel *>(parentWidget())->getRows();

  QBoxLayout *box;
  if ( MAX_ROWS >= 2 )
  {
    box = new QVBoxLayout(this);
    box->setSpacing(0);
  }
  else
  {
    box = new QHBoxLayout(this);
  }

  box->setContentsMargins(QMargins());

  box->addWidget(timeLabel);
  box->addWidget(dayLabel);
  box->addWidget(dateLabel);
}

//--------------------------------------------------------------------------------

void ClockWidget::tick()
{
  QDateTime dateTimeUtc = QDateTime::currentDateTimeUtc();
  QDateTime dateTime = dateTimeUtc.toLocalTime();

  timeLabel->setText(dateTime.time().toString(timeFormat));
  dayLabel->setText(dateTime.date().toString(dayFormat));
  dateLabel->setText(dateTime.date().toString(dateFormat));

  if ( !timeZoneIds.isEmpty() )
  {
    QString tip = "<html><table cellpadding=5 style='white-space:pre;'>";
    for (const QByteArray &id : timeZoneIds)
    {
      QTimeZone timeZone(id);
      QDateTime dt = dateTimeUtc;
      dateTime = dt.toTimeZone(timeZone);

      tip += QString("<tr><td>%1</td> <td>%2</td> <td>%3</td> <td>%4</td></tr>")
                     .arg(QLatin1String(id))
                     .arg(dateTime.time().toString(timeFormat))
                     .arg(dateTime.date().toString(dayFormat))
                     .arg(dateTime.date().toString(dateFormat));
    }
    tip += "</table></html>";
    setToolTip(tip);
  }
}

//--------------------------------------------------------------------------------

void ClockWidget::mousePressEvent(QMouseEvent *event)
{
  if ( event->button() == Qt::LeftButton )
  {
    if ( !calendar )
      calendar = new CalendarPopup(this);

    calendar->goToday();
    QPoint point = mapToGlobal(pos());
    QRect screen = DesktopWidget::availableGeometry();
    point.setX(std::min(point.x(), screen.x() + screen.width() - calendar->sizeHint().width()));
    point.setY(point.y() - calendar->sizeHint().height());
    calendar->move(point);
    calendar->show();
  }
}

//--------------------------------------------------------------------------------
