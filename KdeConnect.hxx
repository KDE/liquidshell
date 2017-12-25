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

#ifndef _KdeConnect_H_
#define _KdeConnect_H_

#include <QObject>
#include <QMap>
#include <QIcon>
#include <QSharedPointer>
class QDBusMessage;

//--------------------------------------------------------------------------------

class KdeConnectDevice : public QObject
{
  Q_OBJECT

  Q_SIGNALS:
    void chargeChanged(int charge);

  public:
    void ringPhone();

    QString id;
    QString name;
    QIcon icon;
    QStringList plugins;
    int charge = -1;
    
  private Q_SLOTS:
    void chargeChangedSlot(int c)
    {
      charge = c;
      emit chargeChanged(charge);
    }
};

//--------------------------------------------------------------------------------

class KdeConnect : public QObject
{
  Q_OBJECT

  public:
    KdeConnect();

    struct Device : public QSharedPointer<KdeConnectDevice>
    {
      Device() : QSharedPointer(new KdeConnectDevice) {}
    };

  Q_SIGNALS:
    void deviceAdded(const Device &device);
    void deviceRemoved(const QString &devId);

  private Q_SLOTS:
    void getDevices();
    void gotDevices(const QDBusMessage &msg);
    void deviceAddedSlot(const QString &dev);
    void deviceRemovedSlot(const QString &dev);
    void deviceVisibilityChanged(const QString &dev, bool visible);

  private:
    QMap<QString, Device> devices;
};

#endif
