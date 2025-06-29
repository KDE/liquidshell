/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _QuickLaunch_H_
#define _QuickLaunch_H_

#include <Launcher.hxx>
#include <DesktopPanel.hxx>

#include <QGridLayout>

class QuickLaunch : public Launcher
{
  Q_OBJECT

  public:
    QuickLaunch(DesktopPanel *parent);

  private Q_SLOTS:
    void fill() override;

  private:
    QGridLayout *grid;
};

#endif
