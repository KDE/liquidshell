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

#ifndef _SysLoad_H_
#define _SysLoad_H_

#include <QFrame>
#include <QVector>
#include <QMap>
#include <QByteArray>
#include <QTimer>

class SysLoad : public QFrame
{
  Q_OBJECT

  Q_PROPERTY(QColor cpuUserColor   MEMBER cpuUserColor)
  Q_PROPERTY(QColor cpuSystemColor MEMBER cpuSystemColor)
  Q_PROPERTY(QColor cpuNiceColor   MEMBER cpuNiceColor)
  Q_PROPERTY(bool   cpuSummaryBar  MEMBER cpuSummaryBar)

  Q_PROPERTY(QColor memUsedColor   MEMBER memUsedColor)
  Q_PROPERTY(QColor memCachedColor MEMBER memCachedColor)
  Q_PROPERTY(QColor memSwapColor   MEMBER memSwapColor)

  Q_PROPERTY(QColor netReceivedColor MEMBER netReceivedColor)
  Q_PROPERTY(QColor netSentColor     MEMBER netSentColor)

  public:
    SysLoad(QWidget *parent);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

  private Q_SLOTS:
    void fetch();

  private:
    struct CpuData
    {
      int userCPU = 0, niceCPU = 0, systemCPU = 0;
      double userPercent = 0, nicePercent = 0, systemPercent = 0;
      double MHz = 0;
    };

    struct MemoryData
    {
      double memPercent = 0;
      double memCachedPercent = 0;
      double swapPercent = 0;
    } memData;

    struct NetworkData
    {
      size_t prevReceived = 0, prevSent = 0;
      size_t received = 0, sent = 0;
      bool valid = false;
    };

    QVector<CpuData> cpus;

    QMap<QByteArray, NetworkData> netDevs;
    size_t maxScale = 0;
    size_t maxBytes = 0;
    size_t sumSent = 0, sumReceived = 0;

    QTimer timeoutTimer;
    QTimer netLoadTimer;

    QColor cpuUserColor   = "#881f1f";
    QColor cpuSystemColor = Qt::darkGreen;
    QColor cpuNiceColor   = Qt::yellow;
    bool cpuSummaryBar    = false;

    QColor memUsedColor   = Qt::blue;
    QColor memCachedColor = Qt::darkGreen;
    QColor memSwapColor   = Qt::cyan;

    QColor netReceivedColor = Qt::green;
    QColor netSentColor     = Qt::red;
};

#endif
