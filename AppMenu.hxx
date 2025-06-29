/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _AppMenu_H_
#define _AppMenu_H_

#include <Launcher.hxx>
#include <DesktopPanel.hxx>

#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QEventLoop>

//--------------------------------------------------------------------------------

class AppMenu : public Launcher
{
  Q_OBJECT

  public:
    AppMenu(DesktopPanel *parent);

  private Q_SLOTS:
    void adjustIconSize();
    void fill() override;
    void showMenu();

  private:
    QToolButton *button;
};

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

class Menu : public QMenu
{
  Q_OBJECT

  public:
    Menu(QWidget *parent);
    void exec();

  protected:
    void hideEvent(QHideEvent *event) override;

  private:
    QEventLoop eventLoop;
};

//--------------------------------------------------------------------------------

#endif
