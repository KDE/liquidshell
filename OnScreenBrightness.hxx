/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _OnScreenBrightness_H_
#define _OnScreenBrightness_H_

#include <QProgressBar>
#include <QTimer>
class QDBusMessage;
class QDBusError;

class OnScreenBrightness : public QProgressBar
{
  Q_OBJECT

  public:
    OnScreenBrightness(QWidget *parent);

  private Q_SLOTS:
    void gotBrightnessMax(QDBusMessage msg);
    void brightnessMaxChanged(int value);
    void brightnessChanged(int value);

  private:
    QTimer hideTimer;
};

#endif
