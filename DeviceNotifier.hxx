/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _DeviceNotifier_H_
#define _DeviceNotifier_H_

#include <SysTrayItem.hxx>
#include <QTimer>
class DeviceList;

class DeviceNotifier : public SysTrayItem
{
  Q_OBJECT

  public:
    DeviceNotifier(QWidget *parent);

  protected:
    QWidget *getDetailsList() override;
    bool eventFilter(QObject *watched, QEvent *event) override;

  private Q_SLOTS:
    void checkDeviceList();

  private:
    DeviceList *deviceList = nullptr;
    QTimer timer;
};

#endif
