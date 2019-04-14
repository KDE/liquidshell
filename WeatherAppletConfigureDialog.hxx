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

#ifndef _WeatherAppletConfigureDialog_H_
#define _WeatherAppletConfigureDialog_H_

#include <QDialog>
#include <ui_WeatherAppletConfigureDialog.h>
#include <KIO/CopyJob>
class WeatherApplet;

class WeatherAppletConfigureDialog : public QDialog
{
  Q_OBJECT

  public:
    WeatherAppletConfigureDialog(WeatherApplet *parent);

  public Q_SLOTS:
    void accept() override;

  private Q_SLOTS:
    void gotJsonFile(KJob *job);
    void readJsonFile(const QString &filePath);

  private:
    WeatherApplet *applet;
    Ui::WeatherAppletConfigureDialog ui;
    QString cityId;
};

#endif
