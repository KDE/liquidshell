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
    void volumeChanged(QDBusMessage reply);

  private:
    QTimer hideTimer, retryTimer;
    QString masterMixer;
    QString masterControl;
    bool requestPending = false;
};

#endif
