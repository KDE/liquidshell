/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _Pager_H_
#define _Pager_H_

#include <QWidget>
class QButtonGroup;
class PagerButton;
class DesktopPanel;

class Pager : public QWidget
{
  Q_OBJECT

  public:
    Pager(DesktopPanel *parent);

  protected:
    void wheelEvent(QWheelEvent *event) override;

  private Q_SLOTS:
    void fill();
    void changeDesktop(bool checked);

  private:
    QButtonGroup *group;
    QVector<PagerButton *> buttons;
    bool showIcons = true;
};

#endif
