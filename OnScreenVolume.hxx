/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _OnScreenVolume_H_
#define _OnScreenVolume_H_

#include <QProgressBar>
#include <QVariantMap>
#include <QTimer>
class QDBusMessage;
class QDBusError;

class OnScreenVolume : public QProgressBar
{
  Q_OBJECT

  public:
    OnScreenVolume(QWidget *parent);

  private Q_SLOTS:
    void gotMasterMixerError(QDBusError error, QDBusMessage msg);
    void getMasterMixer();
    void gotMasterMixer(QDBusMessage msg);
    void controlChanged();
    void gotCurrentControl(QDBusMessage reply);
    void volumeChanged(QDBusMessage reply);

  private:
    QTimer hideTimer, retryTimer;
    QString masterMixer;
};

#endif
