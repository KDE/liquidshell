/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
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
