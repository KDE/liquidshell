/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
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
