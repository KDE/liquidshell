#include <ClockWidget.hxx>

#include <QVBoxLayout>
#include <QDateTime>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QIcon>

//--------------------------------------------------------------------------------

CalendarPopup::CalendarPopup(QWidget *parent)
  : QFrame(parent)
{
  setWindowFlag(Qt::Popup);
  setFrameShape(QFrame::StyledPanel);

  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(QMargins());
  cal = new QCalendarWidget;
  vbox->addWidget(cal);

  QPushButton *today = new QPushButton(QIcon::fromTheme("go-home"), QString());
  vbox->addWidget(today);
  connect(today, &QPushButton::clicked, this, &CalendarPopup::goToday);
}

//--------------------------------------------------------------------------------

void CalendarPopup::goToday()
{
  cal->showToday();
  cal->setSelectedDate(QDate::currentDate());
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

ClockWidget::ClockWidget(QWidget *parent)
  : QFrame(parent), calendar(nullptr)
{
  timer = new QTimer(this);
  timer->setInterval(10000);
  timer->start();
  connect(timer, &QTimer::timeout, this, &ClockWidget::tick);

  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(QMargins());
  vbox->setSpacing(0);

  timeLabel = new QLabel(this);
  dayLabel = new QLabel(this);
  dateLabel = new QLabel(this);

  timeLabel->setObjectName("time");
  dayLabel->setObjectName("day");
  dateLabel->setObjectName("date");
  
  timeLabel->setAlignment(Qt::AlignCenter);
  dayLabel->setAlignment(Qt::AlignCenter);
  dateLabel->setAlignment(Qt::AlignCenter);

  QFont f = font();
  f.setBold(true);
  timeLabel->setFont(f);

  vbox->addWidget(timeLabel);
  vbox->addWidget(dayLabel);
  vbox->addWidget(dateLabel);

  tick();
}

//--------------------------------------------------------------------------------

void ClockWidget::tick()
{
  QDateTime dateTime = QDateTime::currentDateTime();

  timeLabel->setText(dateTime.time().toString(QLatin1String("HH:mm")));
  dayLabel->setText(dateTime.date().toString(QLatin1String("ddd")));
  dateLabel->setText(dateTime.date().toString(QLatin1String("d.MMM yyyy")));
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
    QRect screen = QApplication::desktop()->availableGeometry(this);
    point.setX(std::min(point.x(), screen.x() + screen.width() - calendar->sizeHint().width()));
    point.setY(point.y() - calendar->sizeHint().height());
    calendar->move(point);
    calendar->show();
  }
}

//--------------------------------------------------------------------------------
