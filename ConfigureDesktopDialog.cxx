// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 - 2019 Martin Koller, kollix@aon.at

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

#include <ConfigureDesktopDialog.hxx>

#include <QStandardPaths>
#include <QMimeDatabase>
#include <QImageReader>
#include <QApplication>
#include <QScreen>
#include <QDebug>

#include <KNS3/DownloadDialog>

//--------------------------------------------------------------------------------

ConfigureDesktopDialog::ConfigureDesktopDialog(QWidget *parent, const DesktopWidget::Wallpaper &wp)
  : QDialog(parent), wallpaper(wp)
{
  ui.setupUi(this);
  ui.iconView->setIconSize(QSize(200, 200));
  connect(ui.iconView, &QListWidget::itemClicked,
          [this](QListWidgetItem *item)
          {
            ui.kurlrequester->setUrl(QUrl::fromLocalFile(item->data(Qt::UserRole).toString()));
            wallpaper.fileName = ui.kurlrequester->url().toLocalFile();
            emit changed();
          });

  showImages();

  QPushButton *newstuff = ui.buttonBox->addButton(i18n("Get New Wallpapers..."), QDialogButtonBox::ActionRole);
  newstuff->setIcon(QIcon::fromTheme("get-hot-new-stuff"));
  connect(newstuff, &QPushButton::clicked,
          [this]()
          {
            KNS3::DownloadDialog dialog("wallpaper.knsrc", this);
            dialog.setTitle(i18n("Download Wallpapers"));
            dialog.exec();
            if ( dialog.changedEntries().count() )
              showImages();
          });

  ui.kcolorcombo->setColor(wallpaper.color);
  ui.kurlrequester->setUrl(QUrl::fromLocalFile(wallpaper.fileName));

  connect(ui.kcolorcombo, &KColorCombo::activated,
          [this](const QColor &col) { wallpaper.color = col; emit changed(); });

  connect(ui.kurlrequester, &KUrlRequester::urlSelected,
          [this](const QUrl &url) { wallpaper.fileName = url.toLocalFile(); emit changed(); });

  // older compiler can't use this
  //connect(ui.kurlrequester, QOverload<const QString &>::of(&KUrlRequester::returnPressed),
  //        [this](const QString &text) { wallpaper.fileName = text; emit changed(); });
  connect(ui.kurlrequester, SIGNAL(returnPressed(QString)), this, SLOT(returnPressed(QString)));

  if ( wallpaper.mode == "Scaled" )
    ui.scaledIgnoreRatioButton->setChecked(true);
  else if ( wallpaper.mode == "ScaledKeepRatio" )
    ui.scaledKeepRatioButton->setChecked(true);
  else if ( wallpaper.mode == "ScaledKeepRatioExpand" )
    ui.scaledKeepRatioClipButton->setChecked(true);
  else
    ui.origSizeButton->setChecked(true);

  buttonGroup.addButton(ui.origSizeButton);
  buttonGroup.addButton(ui.scaledIgnoreRatioButton);
  buttonGroup.addButton(ui.scaledKeepRatioButton);
  buttonGroup.addButton(ui.scaledKeepRatioClipButton);

  connect(&buttonGroup, SIGNAL(buttonClicked(QAbstractButton *)),
          this, SLOT(buttonClicked(QAbstractButton *)));
}

//--------------------------------------------------------------------------------

void ConfigureDesktopDialog::returnPressed(const QString &text)
{
  wallpaper.fileName = text;
  emit changed();
}

//--------------------------------------------------------------------------------

void ConfigureDesktopDialog::buttonClicked(QAbstractButton *button)
{
  if ( button == ui.origSizeButton )
    wallpaper.mode = "";
  else if ( button == ui.scaledIgnoreRatioButton )
    wallpaper.mode = "Scaled";
  else if ( button == ui.scaledKeepRatioButton )
    wallpaper.mode = "ScaledKeepRatio";
  else if ( button == ui.scaledKeepRatioClipButton )
    wallpaper.mode = "ScaledKeepRatioExpand";

  emit changed();
}

//--------------------------------------------------------------------------------

void ConfigureDesktopDialog::showImages()
{
  // create filter (list of patterns) for image files we can read
  QMimeDatabase db;
  QStringList filterList;
  foreach (const QByteArray &type, QImageReader::supportedMimeTypes())
  {
    QMimeType mime(db.mimeTypeForName(QString::fromLatin1(type)));
    if ( mime.isValid() )
    {
      foreach (const QString &pattern, mime.globPatterns())
        filterList << pattern;
    }
  }

  ui.iconView->clear();

  QStringList dirNames = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                   "wallpapers", QStandardPaths::LocateDirectory);

  const QString geometryString = QString("%1x%2")
                                         .arg(QApplication::primaryScreen()->size().width())
                                         .arg(QApplication::primaryScreen()->size().height());

  for (const QString &dirName : dirNames)
  {
    // check for file directly in this folder
    QStringList fileNames = QDir(dirName).entryList(filterList, QDir::Files | QDir::Readable);
    for (const QString &fileName : fileNames)
    {
      QPixmap pixmap(dirName + '/' + fileName);
      QListWidgetItem *item = new QListWidgetItem(pixmap, fileName, ui.iconView);
      item->setData(Qt::UserRole, dirName + '/' + fileName);
      item->setToolTip(QString("%1 (%2x%3)").arg(fileName).arg(pixmap.width()).arg(pixmap.height()));
    }

    // check for files in the special subdirs
    for (const QString &subdir : QDir(dirName).entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable))
    {
      QDir dir(dirName + '/' + subdir + "/contents/images");
      QString chosenFilePath;
      for (const QString &fileName : dir.entryList(filterList, QDir::Files | QDir::Readable))
      {
        chosenFilePath = dir.absoluteFilePath(fileName);
        if ( fileName.startsWith(geometryString) )
          break; // just take one
      }

      if ( !chosenFilePath.isEmpty() )
      {
        QPixmap pixmap(chosenFilePath);
        QListWidgetItem *item = new QListWidgetItem(pixmap, subdir, ui.iconView);
        item->setData(Qt::UserRole, chosenFilePath);
        item->setToolTip(QString("%1 (%2x%3)").arg(subdir).arg(pixmap.width()).arg(pixmap.height()));
      }
    }
  }
}

//--------------------------------------------------------------------------------
