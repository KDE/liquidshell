/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef DESKTOP_WIDGET_H
#define DESKTOP_WIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QVector>
class DesktopPanel;
class DesktopApplet;
class ConfigureDesktopDialog;

class DesktopWidget : public QWidget
{
  Q_OBJECT

  public:
    DesktopWidget();

    struct Wallpaper
    {
      QString fileName, mode;
      QColor color;
      QVector<QPixmap> pixmaps;  // per screen
    };

    // since on X11 QScreen:available* methods do not deliver the true
    // values (according to Qt doc: This is a limitation in X11 window manager specification.)
    static QRect availableGeometry();
    static QSize availableSize();

  protected:
    void paintEvent(QPaintEvent *event) override;

  private Q_SLOTS:
    void loadSettings();
    void placePanel();
    void desktopChanged();
    void configureWallpaper();
    void configureDisplay();
    void addApplet(const QString &type);
    void removeApplet(DesktopApplet *applet);

  private:
    void saveAppletsList();

  private:
    DesktopPanel *panel = nullptr;
    ConfigureDesktopDialog *dialog = nullptr;

    QVector<Wallpaper> wallpapers;
    int currentDesktop = -1;

    QVector<DesktopApplet *> applets;
    static int appletNum;
    static DesktopWidget *instance;
};

#endif
