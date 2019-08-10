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

#include <WeatherApplet.hxx>
#include <WeatherAppletConfigureDialog.hxx>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QAction>
#include <QDebug>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <NetworkManagerQt/Manager>

//--------------------------------------------------------------------------------

QString WeatherApplet::apiKey;

//--------------------------------------------------------------------------------

WeatherApplet::WeatherApplet(QWidget *parent, const QString &theId)
  : DesktopApplet(parent, theId)
{
  setAutoFillBackground(true);

  timer.setInterval(600000); // 10min smallest update interval for free data
  connect(&timer, &QTimer::timeout, this, &WeatherApplet::fetchData);

  QVBoxLayout *vbox = new QVBoxLayout(this);
  cityLabel = new QLabel(this);
  cityLabel->setObjectName("city");
  cityLabel->setWordWrap(true);

  QFont f = font();
  f.setPointSizeF(fontInfo().pointSizeF() * 2);
  f.setBold(true);
  cityLabel->setFont(f);

  vbox->addWidget(cityLabel);

  QGridLayout *grid = new QGridLayout;
  vbox->addLayout(grid);

  grid->addWidget(new QLabel(i18n("Temperature:"), this), 0, 0);
  grid->addWidget(tempLabel = new QLabel, 0, 1);

  grid->addWidget(new QLabel(i18n("Pressure:"), this), 1, 0);
  grid->addWidget(pressureLabel = new QLabel, 1, 1);

  grid->addWidget(new QLabel(i18n("Humidity:"), this), 2, 0);
  grid->addWidget(humidityLabel = new QLabel, 2, 1);

  grid->addWidget(new QLabel(i18n("Wind Speed:"), this), 3, 0);
  grid->addWidget(windSpeedLabel = new QLabel, 3, 1);

  grid->addWidget(new QLabel(i18n("Wind Direction:"), this), 4, 0);
  grid->addWidget(windDirectionLabel = new QLabel, 4, 1);

  for (int i = 0; i < 4; i++)
  {
    shortForecast[i] = new ForecastWidget(this, false);
    grid->addWidget(shortForecast[i], 0, 2 + i, 5, 1, Qt::AlignCenter);
  }

  QHBoxLayout *hbox = new QHBoxLayout;
  vbox->addLayout(hbox);

  for (int i = 0; i < 5; i++)
  {
    forecast[i] = new ForecastWidget(this);
    hbox->addWidget(forecast[i]);

    if ( i < 4 )
      hbox->addStretch();
  }

  connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this,
          [this](NetworkManager::Connectivity connectivity)
          {
            if ( connectivity == NetworkManager::Full )
              fetchData();
          });
}

//--------------------------------------------------------------------------------

QSize WeatherApplet::sizeHint() const
{
  return QSize(700, 300);
}

//--------------------------------------------------------------------------------

void WeatherApplet::loadConfig()
{
  KConfig config;
  KConfigGroup group = config.group("Weather");
  apiKey = group.readEntry("apiKey", QString());
  group = config.group(id);
  cityId = group.readEntry("cityId", QString());
  units = group.readEntry("units", QString("metric"));

  DesktopApplet::loadConfig();

  if ( apiKey.isEmpty() || cityId.isEmpty() )
    cityLabel->setText(i18n("Not configured"));
}

//--------------------------------------------------------------------------------

void WeatherApplet::showEvent(QShowEvent *)
{
  // only query every 10 minutes, which is the limit for free data
  if ( !timer.isActive() )
  {
    timer.start();
    fetchData();
  }
}

//--------------------------------------------------------------------------------

void WeatherApplet::fetchData()
{
  if ( !isVisible() || apiKey.isEmpty() || cityId.isEmpty() )
    return;

  QString url = QString("http://api.openweathermap.org/data/2.5/weather?APPID=%1&units=%2&id=%3")
                        .arg(apiKey, units, cityId);

  KIO::StoredTransferJob *job = KIO::storedGet(QUrl(url), KIO::Reload, KIO::HideProgressInfo);
  connect(job, &KIO::Job::result, this, &WeatherApplet::gotData);

  url = QString("http://api.openweathermap.org/data/2.5/forecast?APPID=%1&units=%2&id=%3")
                .arg(apiKey, units, cityId);

  job = KIO::storedGet(QUrl(url), KIO::Reload, KIO::HideProgressInfo);
  connect(job, &KIO::Job::result, this, &WeatherApplet::gotData);
}

//--------------------------------------------------------------------------------

void WeatherApplet::gotData(KJob *job)
{
  if ( job->error() )
  {
    cityLabel->setText(job->errorString());
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(static_cast<KIO::StoredTransferJob *>(job)->data());
  if ( doc.isNull() || !doc.isObject() )
    return;

  QString tempUnit = i18n("°K");
  if ( units == "metric" ) tempUnit = i18n("°C");
  else if ( units == "imperial" ) tempUnit = i18n("°F");

  QJsonObject data = doc.object();

  if ( data.contains("city") && data["city"].isObject() )
    cityLabel->setText(data["city"].toObject()["name"].toString());

  // current
  if ( data.contains("main") && data["main"].isObject() )
  {
    QJsonObject mainData = data["main"].toObject();
    double temp = mainData["temp"].toDouble();

    tempLabel->setText(i18n("%1 %2", locale().toString(temp, 'f', 1), tempUnit));

    double pressure = mainData["pressure"].toDouble();
    pressureLabel->setText(i18n("%1 hPa", locale().toString(pressure, 'f', 1)));

    double humidity = mainData["humidity"].toDouble();
    humidityLabel->setText(i18n("%1 %", locale().toString(humidity, 'f', 1)));
  }

  if ( data.contains("wind") && data["wind"].isObject() )
  {
    QJsonObject windData = data["wind"].toObject();

    QString speedUnit = "m/s";
    if ( units == "imperial" ) speedUnit = "mi/h";

    double speed = windData["speed"].toDouble();
    windSpeedLabel->setText(i18n("%1 %2", locale().toString(speed, 'f', 0), speedUnit));

    double deg = windData["deg"].toDouble();
    windDirectionLabel->setText(i18n("%1 °", locale().toString(deg, 'f', 0)));
  }

  if ( data.contains("weather") && data["weather"].isArray() )
  {
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(qint64(data["dt"].toInt()) * 1000);
    shortForecast[0]->day->setText(dt.time().toString(Qt::SystemLocaleShortDate));
    setIcon(shortForecast[0]->icon, data["weather"].toArray()[0].toObject()["icon"].toString());
  }

  // forecast
  if ( data.contains("list") && data["list"].isArray() )
  {
    for (int i = 0; i < 5; i++)
      forecast[i]->hide();

    QJsonArray array = data["list"].toArray();

    // 3 hours short forecast
    for (int i = 0; i < 3; i++)
    {
      setIcon(shortForecast[1 + i]->icon, array[i].toObject()["weather"].toArray()[0].toObject()["icon"].toString());
      QDateTime dt = QDateTime::fromMSecsSinceEpoch(qint64(array[i].toObject()["dt"].toInt()) * 1000);
      shortForecast[1 + i]->day->setText(dt.time().toString(Qt::SystemLocaleShortDate));
      shortForecast[1 + i]->show();
    }

    QHash<int, double> minTemp, maxTemp; // key = day

    for (QJsonValue value : array)
    {
      QJsonObject obj = value.toObject();

      int day = QDateTime::fromMSecsSinceEpoch(qint64(obj["dt"].toInt()) * 1000).date().dayOfWeek();
      double temp = obj["main"].toObject()["temp"].toDouble();

      if ( !minTemp.contains(day) )
      {
        minTemp.insert(day, temp);
        maxTemp.insert(day, temp);
      }
      else
      {
        if ( temp < minTemp[day] ) minTemp[day] = temp;
        if ( temp > maxTemp[day] ) maxTemp[day] = temp;
      }
    }

    int idx = 0;
    for (QJsonValue value : array)
    {
      QJsonObject obj = value.toObject();

      if ( obj["dt_txt"].toString().contains("12:00") )
      {
        QString icon = obj["weather"].toArray()[0].toObject()["icon"].toString();
        setIcon(forecast[idx]->icon, icon);

        int day = QDateTime::fromMSecsSinceEpoch(qint64(obj["dt"].toInt()) * 1000).date().dayOfWeek();
        forecast[idx]->day->setText(locale().dayName(day, QLocale::ShortFormat));
        forecast[idx]->min->setText(i18n("%1 %2", locale().toString(minTemp[day], 'f', 1), tempUnit));
        forecast[idx]->max->setText(i18n("%1 %2", locale().toString(maxTemp[day], 'f', 1), tempUnit));
        forecast[idx]->show();
        idx++;
        if ( idx == 5 ) break;
      }
    }
  }

  timer.start();  // after showEvent make sure to wait another full timeout phase
}

//--------------------------------------------------------------------------------

void WeatherApplet::setIcon(QLabel *label, const QString &icon)
{
  QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
                     "/api.openweathermap.org/";
  QDir dir;
  dir.mkpath(cacheDir);
  QString filePath = cacheDir + icon + ".png";

  if ( QFile::exists(filePath) )
  {
    QPixmap pixmap(filePath);
    if ( !pixmap.isNull() )
      label->setPixmap(pixmap);
  }
  else
  {
    KIO::StoredTransferJob *job =
        KIO::storedGet(QUrl("http://api.openweathermap.org/img/w/" + icon), KIO::Reload, KIO::HideProgressInfo);

    connect(job, &KIO::Job::result, this,
            [label, filePath](KJob *job)
            {
              if ( job->error() )
                return;

              QPixmap pixmap;
              pixmap.loadFromData(static_cast<KIO::StoredTransferJob *>(job)->data());
              if ( !pixmap.isNull() )
              {
                label->setPixmap(pixmap);
                pixmap.save(filePath, "PNG");
              }
            });
  }
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

ForecastWidget::ForecastWidget(QWidget *parent, bool showMinMax)
  : QWidget(parent)
{
  QGridLayout *grid = new QGridLayout(this);

  if ( showMinMax )
  {
    min = new QLabel(this);
    max = new QLabel(this);

    min->setAlignment(Qt::AlignRight);
    max->setAlignment(Qt::AlignRight);

    grid->addWidget(max, 0, 1);
    grid->addWidget(min, 1, 1);
  }

  day = new QLabel(this);
  icon = new QLabel(this);

  day->setAlignment(Qt::AlignCenter);

  icon->setFixedSize(64, 64);
  icon->setScaledContents(true);

  grid->addWidget(day, 2, 0, 1, 2);
  grid->addWidget(icon, 0, 0, 2, 1);

  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

//--------------------------------------------------------------------------------

void WeatherApplet::configure()
{
  if ( dialog )
  {
    dialog->raise();
    dialog->activateWindow();
    return;
  }

  dialog = new WeatherAppletConfigureDialog(this);
  dialog->setWindowTitle(i18n("Configure Weather Applet"));

  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();

  connect(dialog.data(), &QDialog::accepted, this,
          [this]()
          {
            saveConfig();

            KConfig config;
            KConfigGroup group = config.group("Weather");
            group.writeEntry("apiKey", apiKey);
            group = config.group(id);
            group.writeEntry("cityId", cityId);
            group.writeEntry("units", units);

            if ( !apiKey.isEmpty() && !cityId.isEmpty() )
            {
              fetchData();
              timer.start();
            }
            else
            {
              cityLabel->setText(i18n("Not configured"));
            }
          });
}

//--------------------------------------------------------------------------------
