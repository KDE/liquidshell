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

#ifndef _ConfigureDesktopDialog_H_
#define _ConfigureDesktopDialog_H_

#include <QDialog>
#include <QButtonGroup>
#include <DesktopWidget.hxx>

#include <ui_ConfigureDesktopDialog.h>

class ConfigureDesktopDialog : public QDialog
{
  Q_OBJECT

  public:
    ConfigureDesktopDialog(QWidget *parent, const DesktopWidget::Wallpaper &wp);

    const DesktopWidget::Wallpaper &getWallpaper() const { return wallpaper; }

  Q_SIGNALS:
    void changed();

  private Q_SLOTS:
    void returnPressed(const QString &text);
    void buttonClicked(QAbstractButton *button);

  private:
    void showImages();

  private:
    Ui::ConfigureDesktopDialog ui;
    QButtonGroup buttonGroup;
    DesktopWidget::Wallpaper wallpaper;
};

#endif
