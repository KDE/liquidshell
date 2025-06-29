/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <PictureFrameApplet.hxx>

#include <QAction>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>

//--------------------------------------------------------------------------------

PictureFrameApplet::PictureFrameApplet(QWidget *parent, const QString &theId)
  : DesktopApplet(parent, theId)
{
  setAutoFillBackground(true);
  setContentsMargins(2, 2, 2, 2);
}

//--------------------------------------------------------------------------------

QSize PictureFrameApplet::sizeHint() const
{
  return QSize(400, 400);
}

//--------------------------------------------------------------------------------

void PictureFrameApplet::resizeEvent(QResizeEvent *)
{
  loadImage();
}

//--------------------------------------------------------------------------------

void PictureFrameApplet::paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.drawPixmap(contentsRect().x() + (contentsRect().width() - pixmap.width()) / 2,
                     contentsRect().y() + (contentsRect().height() - pixmap.height()) / 2,
                     pixmap);

  const int frameWidth = contentsMargins().left();
  const int fw2 = frameWidth / 2;

  QPen pen(palette().color(foregroundRole()), frameWidth);
  pen.setJoinStyle(Qt::MiterJoin);
  painter.setPen(pen);
  painter.drawRect(rect().adjusted(fw2, fw2, -fw2, -fw2));
}

//--------------------------------------------------------------------------------

void PictureFrameApplet::loadConfig()
{
  KConfig config;
  KConfigGroup group = config.group(id);
  imagePath = group.readEntry("imagePath", QString());

  DesktopApplet::loadConfig();
  loadImage();
}

//--------------------------------------------------------------------------------

void PictureFrameApplet::loadImage()
{
  pixmap.load(imagePath);
  if ( !pixmap.isNull() )
  {
    pixmap = pixmap.scaled(contentsRect().size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    update();
  }
}

//--------------------------------------------------------------------------------

void PictureFrameApplet::configure()
{
  if ( dialog )
  {
    dialog->raise();
    dialog->activateWindow();
    return;
  }

  dialog = new PictureFrameAppletConfigureDialog(this);
  dialog->setWindowTitle(i18n("Configure PictureFrame Applet"));

  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();

  connect(dialog.data(), &QDialog::accepted, this,
          [this]()
          {
            saveConfig();

            KConfig config;
            KConfigGroup group = config.group(id);
            group.writeEntry("imagePath", imagePath);
          });
}

//--------------------------------------------------------------------------------
