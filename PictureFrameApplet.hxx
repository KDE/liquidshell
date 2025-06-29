/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _PictureFrameApplet_H_
#define _PictureFrameApplet_H_

#include <DesktopApplet.hxx>
#include <PictureFrameAppletConfigureDialog.hxx>
#include <QPointer>

class PictureFrameApplet : public DesktopApplet
{
  Q_OBJECT

  public:
    PictureFrameApplet(QWidget *parent, const QString &theId);

    void loadConfig() override;
    QSize sizeHint() const override;

    QString getImagePath() const { return imagePath; }
    void setImagePath(const QString &path) { imagePath = path; loadImage(); }

  public Q_SLOTS:
    void configure() override;

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

  private:
    void loadImage();

  private:
    QPixmap pixmap;
    QString imagePath;
    QPointer<PictureFrameAppletConfigureDialog> dialog;
};

#endif
