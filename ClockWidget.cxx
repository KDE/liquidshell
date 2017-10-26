#include <ClockWidget.hxx>

#include <QVBoxLayout>
#include <QHBoxLayout>
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

ClockWidget::ClockWidget(DesktopPanel *parent)
  : QFrame(parent), calendar(nullptr)
{
  timer = new QTimer(this);
  timer->setInterval(10000);
  timer->start();
  connect(timer, &QTimer::timeout, this, &ClockWidget::tick);

  connect(parent, &DesktopPanel::rowsChanged, this, &ClockWidget::fill);

  timeLabel = new QLabel(this);
  dayLabel = new QLabel(this);
  dateLabel = new QLabel(this);

  timeLabel->setObjectName("time");
  dayLabel->setObjectName("day");
  dateLabel->setObjectName("date");

  timeLabel->setContentsMargins(QMargins(0, -5, 0, -5));
  dayLabel->setContentsMargins(QMargins(0, -5, 0, -5));
  dateLabel->setContentsMargins(QMargins(0, -5, 0, -5));

  timeLabel->setAlignment(Qt::AlignCenter);
  dayLabel->setAlignment(Qt::AlignCenter);
  dateLabel->setAlignment(Qt::AlignCenter);

  QFont f = font();
  f.setPointSizeF(fontInfo().pointSizeF() * 1.5);
  f.setBold(true);
  timeLabel->setFont(f);

  fill();
  tick();
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
  QDateTime dateTime = QDateTime::currentDateTime();

  timeLabel->setText(dateTime.time().toString(timeFormat));
  dayLabel->setText(dateTime.date().toString(dayFormat));
  dateLabel->setText(dateTime.date().toString(dateFormat));
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
