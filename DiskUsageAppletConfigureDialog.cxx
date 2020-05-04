// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 - 2020 Martin Koller, kollix@aon.at

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

#include <DiskUsageAppletConfigureDialog.hxx>
#include <DiskUsageApplet.hxx>

//--------------------------------------------------------------------------------

DiskUsageAppletConfigureDialog::DiskUsageAppletConfigureDialog(DiskUsageApplet *parent)
  : QDialog(parent), applet(parent)
{
  ui.setupUi(this);
  ui.textColor->setColor(applet->palette().color(applet->foregroundRole()));
  ui.backgroundColor->setColor(applet->palette().color(applet->backgroundRole()));
  ui.barTextColor->setColor(applet->palette().color(QPalette::HighlightedText));
  ui.barBackgroundColor->setColor(applet->palette().color(QPalette::Highlight));
}

//--------------------------------------------------------------------------------

void DiskUsageAppletConfigureDialog::accept()
{
  QPalette pal = applet->palette();
  pal.setColor(applet->foregroundRole(), ui.textColor->color());
  pal.setColor(applet->backgroundRole(), ui.backgroundColor->color());
  pal.setColor(QPalette::HighlightedText, ui.barTextColor->color());
  pal.setColor(QPalette::Highlight, ui.barBackgroundColor->color());
  applet->setPalette(pal);

  QDialog::accept();
}

//--------------------------------------------------------------------------------
