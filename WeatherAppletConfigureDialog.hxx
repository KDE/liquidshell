/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
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
