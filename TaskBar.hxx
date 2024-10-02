/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _TaskBar_H_
#define _TaskBar_H_

#include <DesktopPanel.hxx>
#include <QGridLayout>

#include <kwindowinfo.h>

class TaskBar : public QWidget
{
  Q_OBJECT

  public:
    TaskBar(DesktopPanel *parent);

  private Q_SLOTS:
    void fill();
    void windowChanged(WId wid, NET::Properties props, NET::Properties2 props2);

  private:
    QGridLayout *grid;
};

#endif
