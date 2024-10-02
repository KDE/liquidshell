/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _LockLogout_H_
#define _LockLogout_H_

#include <QWidget>
#include <QToolButton>

#include <DesktopPanel.hxx>

class LockLogout : public QWidget
{
  Q_OBJECT

  public:
    LockLogout(DesktopPanel *parent);

  private Q_SLOTS:
    void fill(int rows);

  private:
    QToolButton *lock, *logout;
};

#endif
