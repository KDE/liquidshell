// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2018 Martin Koller, kollix@aon.at

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
