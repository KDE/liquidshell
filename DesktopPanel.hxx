/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _DesktopPanel_H_
#define _DesktopPanel_H_

#include <QFrame>

class DesktopPanel : public QFrame
{
  Q_OBJECT

  public:
    DesktopPanel(QWidget *parent);

    int getRows() const { return rows; }

  Q_SIGNALS:
    void resized();
    void rowsChanged(int rows);

  protected:
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private Q_SLOTS:
    void updateRowCount();

  private:
    int rows = 2;
};

#endif
