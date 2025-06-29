/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _Bluetooth_H_
#define _Bluetooth_H_

#include <SysTrayItem.hxx>

#include <BluezQt/Manager>
#include <QPointer>
#include <KCMultiDialog>

class Bluetooth : public SysTrayItem
{
  Q_OBJECT

  public:
    Bluetooth(QWidget *parent);
    ~Bluetooth() override;

  protected:
    QWidget *getDetailsList() override;

  private Q_SLOTS:
    void changed();

  private:
    BluezQt::Manager *manager;
    BluezQt::InitManagerJob *job;
    QPointer<KCMultiDialog> dialog;
};

#endif
