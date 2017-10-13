#ifndef _ClockWidget_H_
#define _ClockWidget_H_

#include <QTimer>
#include <QLabel>
#include <QFrame>
#include <QCalendarWidget>

class CalendarPopup : public QFrame
{
  Q_OBJECT

  public:
    CalendarPopup(QWidget *parent);

  public slots:
    void goToday();

  private:
    QCalendarWidget *cal;
};

//--------------------------------------------------------------------------------

class ClockWidget : public QFrame
{
  Q_OBJECT

  public:
    ClockWidget(QWidget *parent);

  protected:
    void mousePressEvent(QMouseEvent *event) override;

  private slots:
    void tick();

  private:
    QTimer *timer;
    QLabel *timeLabel, *dayLabel, *dateLabel;
    CalendarPopup *calendar;
};

#endif
