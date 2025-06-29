/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _WindowList_H_
#define _WindowList_H_

#include <QPushButton>

class WindowList : public QPushButton
{
  Q_OBJECT

  public:
    WindowList(QWidget *parent);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private Q_SLOTS:
    void fillMenu();
};

#endif
