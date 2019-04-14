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

  public Q_SLOTS:
    void goToday();

  private:
    QCalendarWidget *cal;
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

    QString timeFormat = QStringLiteral("HH:mm");
    QString dayFormat  = QStringLiteral("ddd");
    QString dateFormat = QStringLiteral("d.MMM yyyy");

    QVector<QByteArray> timeZoneIds;
};

#endif
