/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _PictureFrameAppletConfigureDialog_H_
#define _PictureFrameAppletConfigureDialog_H_

#include <QDialog>
#include <ui_PictureFrameAppletConfigureDialog.h>
class PictureFrameApplet;

class PictureFrameAppletConfigureDialog : public QDialog
{
  Q_OBJECT

  public:
    PictureFrameAppletConfigureDialog(PictureFrameApplet *parent);

  private Q_SLOTS:
    void accept() override;

  private:
    PictureFrameApplet *applet;
    Ui::PictureFrameAppletConfigureDialog ui;
};

#endif
