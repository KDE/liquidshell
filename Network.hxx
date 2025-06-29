/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _Network_H_
#define _Network_H_

// network management (status display, activate, switch, ...)

#include <SysTrayItem.hxx>
#include <NetworkList.hxx>
#include <QPointer>
#include <QTimer>
#include <QPixmap>
#include <KCMultiDialog>

class Network : public SysTrayItem
{
  Q_OBJECT

  public:
    Network(QWidget *parent);

  protected:
    QWidget *getDetailsList() override;

  private Q_SLOTS:
    void checkState();
    void openConfigureDialog();

  private:
    QTimer blinkTimer;
    bool blinkState = false;
    QPixmap origPixmap;
    QPointer<NetworkList> networkList;
    QPointer<KCMultiDialog> dialog;
};

#endif
