/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _DeviceList_H_
#define _DeviceList_H_

#include <QScrollArea>
#include <QVBoxLayout>
#include <QMap>

#include <Solid/Device>
#include <Solid/Predicate>
#include <Solid/SolidNamespace>
#include <QLabel>
#include <QTimer>
#include <QToolButton>
#include <QPointer>

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

    void markAsNew();

  private:
    static QString errorToString(Solid::ErrorType error);
    void fillData();
    void kdeConnectDeviceChanged(const KdeConnect::Device &dev);

    enum Action { Mount, Unmount };
    void mountDone(Action action, Solid::ErrorType error, QVariant errorData, const QString &udi);

  private Q_SLOTS:
    void teardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi);
    void setupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

  private:
    Solid::Device device;
    QToolButton *mountButton = nullptr;
    QLabel *textLabel = nullptr, *statusLabel = nullptr, *newFlagLabel = nullptr;
    QLabel *chargeIcon = nullptr;
    QToolButton *ringButton = nullptr;
    QTimer statusTimer, mountBusyTimer;
    QString pendingCommand;  // used when click -> mount -> action
};

//--------------------------------------------------------------------------------

class DeviceList : public QScrollArea
{
  Q_OBJECT

  public:
    DeviceList(QWidget *parent);

    bool isEmpty() const { return items.isEmpty(); }

    QSize sizeHint() const override;

  Q_SIGNALS:
    void deviceWasAdded();
    void deviceWasRemoved();

  private Q_SLOTS:
    void deviceAdded(const QString &dev);
    void deviceRemoved(const QString &dev);
    void kdeConnectDeviceAdded(const KdeConnect::Device &dev);

  private:
    void loadActions();
    DeviceItem *addDevice(Solid::Device device);

  private:
    QVBoxLayout *vbox;
    QMap<QString, DeviceItem *> items;
    Solid::Predicate predicate;

    QVector<DeviceAction> actions;
    KdeConnect kdeConnect;
};

#endif
