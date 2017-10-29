/*
  Copyright 2017 Martin Koller

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

#include <DesktopWidget.hxx>
#include <DesktopPanel.hxx>
#include <ConfigureDesktopDialog.hxx>

#include <KWindowSystem>

#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QAction>
#include <QIcon>
#include <QScreen>
#include <QDebug>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KCMultiDialog>

//--------------------------------------------------------------------------------

DesktopWidget::DesktopWidget()
  : QWidget(nullptr, Qt::WindowDoesNotAcceptFocus)
{
  setAttribute(Qt::WA_AlwaysShowToolTips);
  setAutoFillBackground(true);
  setFixedSize(QApplication::desktop()->size());
  setWindowIcon(QIcon::fromTheme("kde"));

  KWindowSystem::setType(winId(), NET::Desktop);
  KWindowSystem::setState(winId(), NET::KeepBelow);

  panel = new DesktopPanel(nullptr);
  panel->show();
  connect(panel, &DesktopPanel::resized, this, &DesktopWidget::placePanel);
  placePanel();

  connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, &DesktopWidget::desktopChanged);
  connect(KWindowSystem::self(), &KWindowSystem::numberOfDesktopsChanged, this, &DesktopWidget::loadSettings);

  setContextMenuPolicy(Qt::ActionsContextMenu);
  QAction *action = new QAction(QIcon::fromTheme("configure"), i18n("Configure Wallpaper..."), this);
  connect(action, &QAction::triggered, this, &DesktopWidget::configureWallpaper);
  addAction(action);

  action = new QAction(QIcon::fromTheme("preferences-desktop-display"), i18n("Configure Display..."), this);
  connect(action, &QAction::triggered, this, &DesktopWidget::configureDisplay);
  addAction(action);

  loadSettings();

  connect(qApp->primaryScreen(), &QScreen::geometryChanged,
          [this](const QRect &rect) { setFixedSize(rect.size()); placePanel(); desktopChanged(); });
}

//--------------------------------------------------------------------------------

void DesktopWidget::loadSettings()
{
  // simple check for default files matching current desktop size
  QStringList dirNames = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                   "wallpapers", QStandardPaths::LocateDirectory);

  QStringList defaultFiles;
  const QString wantedPrefix = QString("%1x%2").arg(width()).arg(height());
  for (const QString &dirName : dirNames)
  {
    for (const QString &subdir : QDir(dirName).entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable))
    {
      QDir dir(dirName + '/' + subdir + "/contents/images");
      for (const QString &fileName : dir.entryList(QDir::Files | QDir::Readable))
      {
        if ( fileName.startsWith(wantedPrefix) )
          defaultFiles.append(dir.absoluteFilePath(fileName));
      }
    }
  }

  KConfig config;
  for (int i = wallpapers.count() + 1; i <= KWindowSystem::numberOfDesktops(); i++)
  {
    KConfigGroup group = config.group(QString("Desktop_%1").arg(i));

    Wallpaper wallpaper;

    wallpaper.color = group.readEntry("Color", QColor(Qt::black));
    wallpaper.mode = group.readEntry("WallpaperMode", QString());

    int idx = defaultFiles.count() ? ((i - 1) % defaultFiles.count()) : -1;
    wallpaper.fileName = group.readEntry(QString("Wallpaper"), (i != -1) ? defaultFiles[idx] : QString());

    wallpapers.append(wallpaper);
  }

  wallpapers.resize(KWindowSystem::numberOfDesktops());  // if we had more before, reduce size
  wallpapers.squeeze();

  desktopChanged();
}

//--------------------------------------------------------------------------------

void DesktopWidget::configureWallpaper()
{
  bool showingDesktop = KWindowSystem::showingDesktop();
  KWindowSystem::setShowingDesktop(true);

  Wallpaper origWallpaper = wallpapers[currentDesktop];

  ConfigureDesktopDialog dialog(this, wallpapers[currentDesktop]);

  connect(&dialog, &ConfigureDesktopDialog::changed,
          [this, &dialog]() { wallpapers[currentDesktop] = dialog.getWallpaper(); desktopChanged(); });

  if ( dialog.exec() == QDialog::Rejected )
  {
    wallpapers[currentDesktop] = origWallpaper;
    desktopChanged();
  }
  else  // store in config file
  {
    Wallpaper &wallpaper = wallpapers[currentDesktop];
    KConfig config;
    KConfigGroup group = config.group(QString("Desktop_%1").arg(currentDesktop + 1));
    group.writeEntry("Color", wallpaper.color);
    group.writeEntry(QString("Wallpaper"), wallpaper.fileName);
    group.writeEntry("WallpaperMode", wallpaper.mode);
  }
  KWindowSystem::setShowingDesktop(showingDesktop);  // restore
}

//--------------------------------------------------------------------------------

void DesktopWidget::configureDisplay()
{
  KCMultiDialog *dialog = new KCMultiDialog(this);
  dialog->addModule("kcm_kscreen");
  dialog->adjustSize();
  dialog->setWindowTitle(i18n("Configure Display"));
  dialog->exec();
  delete dialog;
}

//--------------------------------------------------------------------------------

void DesktopWidget::placePanel()
{
  int panelHeight = panel->sizeHint().height();
  panel->setGeometry(0, height() - panelHeight, width(), panelHeight);
  KWindowSystem::setStrut(panel->winId(), 0, 0, 0, panelHeight);
}

//--------------------------------------------------------------------------------

void DesktopWidget::desktopChanged()
{
  // free memory in previous shown desktop
  if ( (currentDesktop >= 0) && (currentDesktop < wallpapers.count()) )
    wallpapers[currentDesktop].pixmap = QPixmap();

  currentDesktop = KWindowSystem::currentDesktop() - 1;  // num to index

  if ( currentDesktop >= wallpapers.count() )
    return;

  Wallpaper &wallpaper = wallpapers[currentDesktop];

  if ( wallpaper.color.isValid() )
  {
    QPalette pal = palette();
    pal.setColor(backgroundRole(), wallpaper.color);
    setPalette(pal);
  }

  if ( !wallpaper.fileName.isEmpty() )
  {
    QPixmap pixmap;
    pixmap.load(wallpaper.fileName);

    if ( !pixmap.isNull() )
    {
      if ( wallpaper.mode == "Scaled" )
        pixmap = pixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      else if ( wallpaper.mode == "ScaledKeepRatio" )
        pixmap = pixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
      else if ( wallpaper.mode == "ScaledKeepRatioExpand" )
        pixmap = pixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

      wallpaper.pixmap = pixmap;
    }
  }

  update();
}

//--------------------------------------------------------------------------------

void DesktopWidget::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event)

  if ( currentDesktop < wallpapers.count() )
  {
    Wallpaper &wallpaper = wallpapers[currentDesktop];

    if ( !wallpaper.pixmap.isNull() )
    {
      QPainter painter(this);
      painter.drawPixmap(x() + (width() - wallpaper.pixmap.width()) / 2,
                         y() + (height() - wallpaper.pixmap.height()) / 2,
                         wallpaper.pixmap);
    }
  }
}

//--------------------------------------------------------------------------------
