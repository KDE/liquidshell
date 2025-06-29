/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
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
    QPixmap moon;
    QLabel *cityLabel = nullptr;
    QLabel *moonLabel = nullptr;
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
