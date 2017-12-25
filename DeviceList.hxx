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

#ifndef _DeviceList_H_
#define _DeviceList_H_

#include <QFrame>
#include <QVBoxLayout>
#include <QMap>

#include <Solid/Device>
#include <Solid/Predicate>
#include <Solid/SolidNamespace>
#include <QLabel>
#include <QTimer>
#include <QToolButton>
#include <QPointer>

#include <KCMultiDialog>
#include <KServiceAction>

#include <KdeConnect.hxx>

//--------------------------------------------------------------------------------

struct DeviceAction
{
  DeviceAction() { }
  DeviceAction(const QString &filePath, Solid::Predicate p, KServiceAction a)
    : path(filePath), predicate(p), action(a) { }

  QString path;
  Solid::Predicate predicate;
  KServiceAction action;
};

//--------------------------------------------------------------------------------

class DeviceItem : public QFrame
{
  Q_OBJECT

  public:
    DeviceItem(Solid::Device dev, const QVector<DeviceAction> &deviceActions);
    DeviceItem(const KdeConnect::Device &dev);

  private:
    static QString errorToString(Solid::ErrorType error);
    void fillData();

    enum Action { Mount, Unmount };
    void mountDone(Action action, Solid::ErrorType error, QVariant errorData, const QString &udi);

  private Q_SLOTS:
    void teardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi);
    void setupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

  private:
    Solid::Device device;
    QToolButton *mountButton = nullptr;
    QLabel *textLabel = nullptr, *statusLabel = nullptr;
    QTimer statusTimer;
    QPointer<KCMultiDialog> dialog;
};

//--------------------------------------------------------------------------------

class DeviceList : public QFrame
{
  Q_OBJECT

  public:
    DeviceList(QWidget *parent);

    bool isEmpty() const { return items.isEmpty(); }

  Q_SIGNALS:
    void deviceWasAdded();
    void deviceWasRemoved();

  private Q_SLOTS:
    void deviceAdded(const QString &dev);
    void deviceRemoved(const QString &dev);
    void kdeConnectDeviceAdded(const KdeConnect::Device &dev);

  private:
    void loadActions();
    void addDevice(Solid::Device device);

  private:
    QVBoxLayout *vbox;
    QMap<QString, DeviceItem *> items;
    Solid::Predicate predicate;

    QVector<DeviceAction> actions;
    KdeConnect kdeConnect;
};

#endif
