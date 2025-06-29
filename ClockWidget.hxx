/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _ClockWidget_H_
#define _ClockWidget_H_

#include <QTimer>
#include <QLabel>
#include <QFrame>
#include <QCalendarWidget>

#include <DesktopPanel.hxx>

class CalendarPopup : public QFrame
{
  Q_OBJECT

  public:
    CalendarPopup(QWidget *parent);

  protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

  public Q_SLOTS:
    void goToday();

  private Q_SLOTS:
    void tick();

  private:
    QCalendarWidget *cal;
    QTimer timer;
    QLabel *timeLabel;
    QLabel *dateLabel;
};

//--------------------------------------------------------------------------------

class ClockWidget : public QFrame
{
  Q_OBJECT
  Q_PROPERTY(QString timeFormat MEMBER timeFormat)
  Q_PROPERTY(QString dayFormat  MEMBER dayFormat)
  Q_PROPERTY(QString dateFormat MEMBER dateFormat)

  public:
    ClockWidget(DesktopPanel *parent);

  protected:
    void mousePressEvent(QMouseEvent *event) override;

  private Q_SLOTS:
    void fill();
    void tick();

  private:
    QTimer *timer;
    QLabel *timeLabel, *dayLabel, *dateLabel;
    CalendarPopup *calendar;

    QString timeFormat;
    QString dayFormat  = QStringLiteral("ddd");
    QString dateFormat = QStringLiteral("d.MMM yyyy");

    QVector<QByteArray> timeZoneIds;
};

#endif
