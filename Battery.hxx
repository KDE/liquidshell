// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 -2020 Martin Koller, kollix@aon.at

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

#ifndef _Battery_H_
#define _Battery_H_

#include <SysTrayItem.hxx>
#include <Solid/Device>
#include <QPointer>
#include <KCMultiDialog>
class QDBusMessage;

class Battery : public SysTrayItem
{
  Q_OBJECT

  public:
    Battery(QWidget *parent);

    static QIcon getStatusIcon(int charge, bool isCharging);

  protected:
    QWidget *getDetailsList() override;

  private Q_SLOTS:
    void onBatteryReply(const QDBusMessage &msg);
    void upowerPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidated);
    void changed();

  private:
    QString secsToHM(int secs) const;

  private:
    Solid::Device device;
    bool onBattery = false;
    QPointer<KCMultiDialog> dialog;
};

#endif
