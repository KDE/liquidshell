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

#include <WeatherAppletConfigureDialog.hxx>
#include <WeatherApplet.hxx>

#include <QUrl>
#include <QStandardPaths>
#include <QFile>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QElapsedTimer>
#include <QTimer>
#include <QDir>

#include <KRun>
#include <KFilterDev>

//--------------------------------------------------------------------------------

WeatherAppletConfigureDialog::WeatherAppletConfigureDialog(WeatherApplet *parent)
  : QDialog(parent), applet(parent)
{
  ui.setupUi(this);
  connect(ui.city, &QLineEdit::textEdited, [this](const QString &txt)
          { auto found = ui.cityList->findItems(txt, Qt::MatchStartsWith);
            if ( found.count() ) ui.cityList->setCurrentItem(found[0]); });

  connect(ui.cityList, &QTreeWidget::itemClicked,
          [this](QTreeWidgetItem *current) { ui.city->setText(current->text(0)); });

  ui.apiKey->setText(applet->apiKey);
  if ( applet->units == "metric" )
    ui.metric->setChecked(true);
  else
    ui.imperial->setChecked(true);

  connect(ui.getApiKey, &QPushButton::clicked, []() { new KRun(QUrl("http://openweathermap.org/appid"), nullptr); });

  ui.textColor->setColor(applet->palette().color(applet->foregroundRole()));
  ui.backgroundColor->setColor(applet->palette().color(applet->backgroundRole()));

  // for city selection, we need the json file
  QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
                     "/api.openweathermap.org/";

  QDir dir;
  dir.mkpath(cacheDir);
  QString filePath = cacheDir + "city.list.json.gz";

  if ( QFile::exists(filePath) )
    QTimer::singleShot(0, [this, filePath]() { readJsonFile(filePath); });
  else
  {
    KIO::CopyJob *job = KIO::copy(QUrl("http://bulk.openweathermap.org/sample/city.list.json.gz"),
                                  QUrl::fromLocalFile(filePath));

    connect(job, &KIO::Job::result, this, &WeatherAppletConfigureDialog::gotJsonFile);

    ui.city->setText(i18n("Downloading city list..."));
    ui.city->setReadOnly(true);
  }
}

//--------------------------------------------------------------------------------

void WeatherAppletConfigureDialog::gotJsonFile(KJob *job)
{
  ui.city->setText(QString());
  ui.city->setReadOnly(false);

  if ( job->error() )
  {
    QMessageBox::warning(this, i18n("Download Error"),
                         i18n("Error on downloading city list: %1").arg(job->errorString()));
    return;
  }
  readJsonFile(static_cast<KIO::CopyJob *>(job)->destUrl().toLocalFile());
}

//--------------------------------------------------------------------------------

void WeatherAppletConfigureDialog::readJsonFile(const QString &filePath)
{
  ui.city->setFocus();

  KFilterDev jsonFile(filePath);
  if ( !jsonFile.open(QIODevice::ReadOnly) )
    return;

  QJsonArray array = QJsonDocument::fromJson(jsonFile.readAll()).array();

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  int i = 0;
  for (const QJsonValue &value : array)
  {
    if ( value.isObject() )
    {
      QJsonObject obj = value.toObject();
      QString city = obj["name"].toString();
      QString country = obj["country"].toString();

      if ( !city.isEmpty() && (city != "-") && !country.isEmpty() )
      {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui.cityList);
        item->setText(0, city);
        item->setText(1, country);
        item->setData(0, Qt::UserRole, obj["id"].toInt());
      }
    }
    if ( (i++ % 40000) == 0 )
      QApplication::processEvents();
  }

  ui.cityList->setSortingEnabled(true);
  ui.cityList->sortByColumn(0, Qt::AscendingOrder);
  ui.cityList->header()->resizeSections(QHeaderView::ResizeToContents);

  for (int i = 0; i < ui.cityList->topLevelItemCount(); i++)
  {
    if ( ui.cityList->topLevelItem(i)->data(0, Qt::UserRole).toString() == applet->cityId )
    {
      ui.cityList->setCurrentItem(ui.cityList->topLevelItem(i));
      ui.city->setText(ui.cityList->topLevelItem(i)->text(0));
      break;
    }
  }

   QApplication::restoreOverrideCursor();
}

//--------------------------------------------------------------------------------

void WeatherAppletConfigureDialog::accept()
{
  applet->apiKey = ui.apiKey->text();

  auto selected = ui.cityList->selectedItems();
  if ( selected.count() )
    applet->cityId = selected[0]->data(0, Qt::UserRole).toString();

  if ( ui.metric->isChecked() )
    applet->units = "metric";
  else
    applet->units = "imperial";

  QPalette pal = applet->palette();
  pal.setColor(applet->foregroundRole(), ui.textColor->color());
  pal.setColor(applet->backgroundRole(), ui.backgroundColor->color());
  applet->setPalette(pal);

  QDialog::accept();
}

//--------------------------------------------------------------------------------
