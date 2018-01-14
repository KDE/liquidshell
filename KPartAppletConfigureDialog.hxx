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

#ifndef _KPartAppletConfigureDialog_H_
#define _KPartAppletConfigureDialog_H_

#include <QDialog>
#include <ui_KPartAppletConfigureDialog.h>
class KPartApplet;

class KPartAppletConfigureDialog : public QDialog
{
  Q_OBJECT

  public:
    KPartAppletConfigureDialog(KPartApplet *parent);

  private Q_SLOTS:
    void accept() override;

  private:
    KPartApplet *applet;
    Ui::KPartAppletConfigureDialog ui;
};

#endif
