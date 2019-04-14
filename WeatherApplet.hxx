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

#ifndef _WeatherApplet_H_
#define _WeatherApplet_H_

#include <DesktopApplet.hxx>
#include <WeatherAppletConfigureDialog.hxx>

#include <QTimer>
#include <QLabel>
#include <QPointer>

#include <KIO/StoredTransferJob>

// applet using data from http://api.openweathermap.org

class WeatherApplet : public DesktopApplet
{
  Q_OBJECT

  public:
    WeatherApplet(QWidget *parent, const QString &theId);

    void loadConfig() override;
    QSize sizeHint() const override;

  public Q_SLOTS:
    void configure() override;

  protected:
    void showEvent(QShowEvent *event) override;

  private Q_SLOTS:
    void fetchData();
    void gotData(KJob *job);

  private:  // methods
    void setIcon(QLabel *label, const QString &icon);

  private:  // members
    static QString apiKey; // see http://openweathermap.org/api
    QString cityId, units;
    QTimer timer;
    QLabel *cityLabel = nullptr;
    QLabel *tempLabel = nullptr;
    QLabel *pressureLabel = nullptr;
    QLabel *humidityLabel = nullptr;
    QLabel *windSpeedLabel = nullptr;
    QLabel *windDirectionLabel = nullptr;

    class ForecastWidget *shortForecast[4] = { nullptr };
    class ForecastWidget *forecast[5] = { nullptr };

    QPointer<WeatherAppletConfigureDialog> dialog;

    friend WeatherAppletConfigureDialog;
};

//--------------------------------------------------------------------------------

class ForecastWidget : public QWidget
{
  Q_OBJECT

  public:
    ForecastWidget(QWidget *parent, bool showMinMax = true);

    QLabel *min = nullptr;
    QLabel *max = nullptr;
    QLabel *day = nullptr;
    QLabel *icon = nullptr;
};

#endif
