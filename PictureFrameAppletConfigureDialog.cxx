// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2018 Martin Koller, kollix@aon.at

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

#include <PictureFrameAppletConfigureDialog.hxx>
#include <PictureFrameApplet.hxx>

//--------------------------------------------------------------------------------

PictureFrameAppletConfigureDialog::PictureFrameAppletConfigureDialog(PictureFrameApplet *parent)
  : QDialog(parent), applet(parent)
{
  ui.setupUi(this);
  ui.textColor->setColor(applet->palette().color(applet->foregroundRole()));
  ui.backgroundColor->setColor(applet->palette().color(applet->backgroundRole()));
  ui.imagePath->setUrl(QUrl::fromLocalFile(applet->getImagePath()));
}

//--------------------------------------------------------------------------------

void PictureFrameAppletConfigureDialog::accept()
{
  QPalette pal = applet->palette();
  pal.setColor(applet->foregroundRole(), ui.textColor->color());
  pal.setColor(applet->backgroundRole(), ui.backgroundColor->color());
  applet->setPalette(pal);
  applet->setImagePath(ui.imagePath->url().toLocalFile());

  QDialog::accept();
}

//--------------------------------------------------------------------------------
