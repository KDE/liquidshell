/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _PopupMenu_H_
#define _PopupMenu_H_

// A QMenu with drag functionality

#include <QMenu>

class PopupMenu : public QMenu
{
  Q_OBJECT

  public:
    PopupMenu(QWidget *parent) : QMenu(parent), dragStartPos(-1, -1) { }

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private:
    QPoint dragStartPos;
};

#endif
