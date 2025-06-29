/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
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
