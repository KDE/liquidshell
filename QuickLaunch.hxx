// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 Martin Koller, kollix@aon.at

  This file is part of liquidshell.

  liquidshell is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  liquidshell is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with liquidshell.  If not, see <http://www.gnu.org/licenses/>.
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
