/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <DesktopWidget.hxx>
#include <DesktopPanel.hxx>
#include <WeatherApplet.hxx>
#include <DiskUsageApplet.hxx>
#include <PictureFrameApplet.hxx>
#include <ConfigureDesktopDialog.hxx>
#include <OnScreenVolume.hxx>
#include <OnScreenBrightness.hxx>
#include <KWinCompat.hxx>

#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QCursor>
#include <QDBusConnection>
#include <QDBusMessage>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KCMultiDialog>
#include <KHelpMenu>
#include <kcmutils_version.h>

//--------------------------------------------------------------------------------

int DesktopWidget::appletNum = 0;
DesktopWidget *DesktopWidget::instance = nullptr;

//--------------------------------------------------------------------------------

DesktopWidget::DesktopWidget()
  : QWidget(nullptr)
  // would love to add Qt::WindowDoesNotAcceptFocus, but this leads to global shortcuts
  // no longer working when last maximized firefox window is closed on an empty desktop
{
  instance = this;

  setAttribute(Qt::WA_AlwaysShowToolTips);
  setAutoFillBackground(true);
  setFixedSize(QApplication::primaryScreen()->virtualSize());
  setWindowIcon(QIcon::fromTheme("liquidshell"));

  if ( KWindowSystem::isPlatformX11() )
    KX11Extras::setType(winId(), NET::Desktop);

  panel = new DesktopPanel(nullptr);
  panel->show();
  connect(panel, &DesktopPanel::resized, this, &DesktopWidget::placePanel);
  placePanel();

  connect(KWinCompat::self(), &KWinCompat::currentDesktopChanged, this, &DesktopWidget::desktopChanged);
  connect(KWinCompat::self(), &KWinCompat::numberOfDesktopsChanged, this, &DesktopWidget::loadSettings);

  setContextMenuPolicy(Qt::ActionsContextMenu);
  QAction *action = new QAction(QIcon::fromTheme("configure"), i18n("Configure Wallpaper..."), this);
  connect(action, &QAction::triggered, this, &DesktopWidget::configureWallpaper);
  addAction(action);

  action = new QAction(i18n("Add Applet"), this);
  addAction(action);
  QMenu *menu = new QMenu;
  action->setMenu(menu);
  action = menu->addAction(QIcon::fromTheme("weather-clouds"), i18n("Weather"));
  connect(action, &QAction::triggered, [this]() { addApplet("Weather"); });

  action = menu->addAction(QIcon::fromTheme("drive-harddisk"), i18n("Disk Usage"));
  connect(action, &QAction::triggered, [this]() { addApplet("DiskUsage"); });

  action = menu->addAction(QIcon::fromTheme("image-x-generix"), i18n("Picture Frame"));
  connect(action, &QAction::triggered, [this]() { addApplet("PictureFrame"); });

  action = new QAction(QIcon::fromTheme("preferences-desktop-display"), i18n("Configure Display..."), this);
  connect(action, &QAction::triggered, this, &DesktopWidget::configureDisplay);
  addAction(action);

  action = new QAction(QIcon::fromTheme("system-run"), i18n("Run Command..."), this);
  connect(action, &QAction::triggered,
          []()
          {
            QDBusConnection::sessionBus().send(
                QDBusMessage::createMethodCall("org.kde.krunner", "/App",
                                               "org.kde.krunner.App", "display"));
          });
  addAction(action);

  action = new QAction(i18n("Help"), this);
  KHelpMenu *helpMenu = new KHelpMenu(this);
  helpMenu->action(KHelpMenu::menuHelpContents)->setVisible(false); // no handbook
  helpMenu->action(KHelpMenu::menuWhatsThis)->setVisible(false);
  action->setMenu(helpMenu->menu());
  addAction(action);

  // restore applets
  KConfig config;
  KConfigGroup group = config.group("DesktopApplets");
  QStringList appletNames = group.readEntry("applets", QStringList());
  for (const QString &appletName : appletNames)
  {
    DesktopApplet *applet = nullptr;

    if ( appletName.startsWith("Weather_") )
      applet = new WeatherApplet(this, appletName);
    else if ( appletName.startsWith("DiskUsage_") )
      applet = new DiskUsageApplet(this, appletName);
    else if ( appletName.startsWith("PictureFrame_") )
      applet = new PictureFrameApplet(this, appletName);

    if ( applet )
    {
      int num = appletName.mid(appletName.indexOf('_') + 1).toInt();
      if ( num > appletNum )
        appletNum = num;

      applet->loadConfig();
      applets.append(applet);
      connect(applet, &DesktopApplet::removeThis, this, &DesktopWidget::removeApplet);
    }
  }

  loadSettings();

  connect(qApp, &QApplication::primaryScreenChanged, this, &DesktopWidget::placePanel);

  connect(qApp, &QApplication::screenAdded,
          [this]() { setFixedSize(QApplication::primaryScreen()->virtualSize()); desktopChanged(); });

  connect(qApp, &QApplication::screenRemoved,
          [this]() { setFixedSize(QApplication::primaryScreen()->virtualSize()); desktopChanged(); });

  connect(QApplication::primaryScreen(), &QScreen::virtualGeometryChanged,
          [this]() { setFixedSize(QApplication::primaryScreen()->virtualSize()); placePanel(); desktopChanged(); });

  new OnScreenVolume(this);
  new OnScreenBrightness(this);
}

//--------------------------------------------------------------------------------

void DesktopWidget::loadSettings()
{
  // simple check for default files matching current desktop size
  QStringList dirNames = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                   "wallpapers", QStandardPaths::LocateDirectory);

  QStringList defaultFiles;
  const QString geometryString = QString("%1x%2").arg(width()).arg(height());
  for (const QString &dirName : dirNames)
  {
    for (const QString &subdir : QDir(dirName).entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable))
    {
      QDir dir(dirName + '/' + subdir + "/contents/images");
      for (const QString &fileName : dir.entryList(QDir::Files | QDir::Readable))
      {
        if ( fileName.startsWith(geometryString) )
          defaultFiles.append(dir.absoluteFilePath(fileName));
      }
    }
  }

  KConfig config;
  for (int i = wallpapers.count() + 1; i <= KWinCompat::numberOfDesktops(); i++)
  {
    KConfigGroup group = config.group(QString("Desktop_%1").arg(i));

    Wallpaper wallpaper;

    wallpaper.color = group.readEntry("color", QColor(Qt::black));
    wallpaper.mode = group.readEntry("wallpaperMode", QString());

    int idx = defaultFiles.count() ? ((i - 1) % defaultFiles.count()) : -1;
    wallpaper.fileName = group.readEntry("wallpaper", (idx != -1) ? defaultFiles[idx] : QString());

    wallpapers.append(wallpaper);
  }

  wallpapers.resize(KWinCompat::numberOfDesktops());  // if we had more before, reduce size
  wallpapers.squeeze();

  desktopChanged();
}

//--------------------------------------------------------------------------------

void DesktopWidget::configureWallpaper()
{
  if ( dialog )  // already open, do nothing
    return;

  bool showingDesktop = KWindowSystem::showingDesktop();
  KWindowSystem::setShowingDesktop(true);

  int desktopNum = currentDesktop;
  Wallpaper origWallpaper = wallpapers[desktopNum];

  dialog = new ConfigureDesktopDialog(nullptr, wallpapers[desktopNum]);

  connect(dialog, &ConfigureDesktopDialog::changed,
          [=]() { wallpapers[desktopNum] = dialog->getWallpaper(); desktopChanged(); });

  connect(dialog, &QDialog::finished,
          [=](int result)
          {
            if ( result == QDialog::Rejected )
            {
              wallpapers[desktopNum] = origWallpaper;
              desktopChanged();
            }
            else  // store in config file
            {
              const Wallpaper &wallpaper = wallpapers[desktopNum];
              KConfig config;
              KConfigGroup group = config.group(QString("Desktop_%1").arg(desktopNum + 1));
              group.writeEntry("color", wallpaper.color);
              group.writeEntry("wallpaper", wallpaper.fileName);
              group.writeEntry("wallpaperMode", wallpaper.mode);
            }
            KWindowSystem::setShowingDesktop(showingDesktop);  // restore
            dialog->deleteLater();
            dialog = nullptr;
          });

  dialog->show(); // not modal
}

//--------------------------------------------------------------------------------

void DesktopWidget::configureDisplay()
{
  KCMultiDialog *dialog = new KCMultiDialog(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  dialog->addModule(KPluginMetaData("plasma/kcms/systemsettings/kcm_kscreen"));

  dialog->adjustSize();
  dialog->setWindowTitle(i18n("Configure Display"));
  dialog->show();  // not modal
}

//--------------------------------------------------------------------------------

void DesktopWidget::placePanel()
{
  // hack workaround: for an unknown reason, opening KCMultiDialog the first time
  // changes the panel window type
  KWinCompat::setType(panel->winId(), NET::Dock);

  int panelHeight = qMin(panel->sizeHint().height(), panel->height());
  QRect r = QApplication::primaryScreen()->geometry();
  QSize vs = QApplication::primaryScreen()->virtualSize();
  panel->setGeometry(r.x(), r.y() + r.height() - panelHeight, r.width(), panelHeight);

  // struts are to the combined screen geoms, not the single screen
  KWinCompat::setStrut(panel->winId(), 0, 0, 0, panelHeight + vs.height() - r.bottom());
}

//--------------------------------------------------------------------------------

QRect DesktopWidget::availableGeometry()
{
  QRect geom = QApplication::primaryScreen()->geometry();
  geom.setHeight(geom.height() - instance->panel->height());
  return geom;
}

//--------------------------------------------------------------------------------

QSize DesktopWidget::availableSize()
{
  return availableGeometry().size();
}

//--------------------------------------------------------------------------------

void DesktopWidget::desktopChanged()
{
  // free memory in previous shown desktop
  if ( (currentDesktop >= 0) && (currentDesktop < wallpapers.count()) )
    wallpapers[currentDesktop].pixmaps.clear();

  currentDesktop = KWinCompat::currentDesktop() - 1;  // num to index

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
    QPixmap pixmap(wallpaper.fileName);

    if ( !pixmap.isNull() )
    {
      for (int i = 0; i < QApplication::screens().count(); i++)
      {
        QRect r = QApplication::screens().at(i)->geometry();
        QPixmap scaledPixmap = pixmap;

        if ( wallpaper.mode == "Scaled" )
          scaledPixmap = pixmap.scaled(r.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        else if ( wallpaper.mode == "ScaledKeepRatio" )
          scaledPixmap = pixmap.scaled(r.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        else if ( wallpaper.mode == "ScaledKeepRatioExpand" )
          scaledPixmap = pixmap.scaled(r.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        wallpaper.pixmaps.append(scaledPixmap);
      }
    }
  }

  update();

  // show applets for new desktop
  for (DesktopApplet *applet : applets)
  {
    if ( !applet->isConfiguring() )
      applet->setVisible(applet->isOnDesktop(currentDesktop + 1));
  }
}

//--------------------------------------------------------------------------------

void DesktopWidget::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event)

  if ( currentDesktop < wallpapers.count() )
  {
    Wallpaper &wallpaper = wallpapers[currentDesktop];

    QPainter painter(this);

    for (int i = 0; i < QApplication::screens().count(); i++)
    {
      if ( i < wallpaper.pixmaps.count() )
      {
        QRect r = QApplication::screens().at(i)->geometry();
        painter.setClipRect(r);

        painter.drawPixmap(r.x() + (r.width() - wallpaper.pixmaps[i].width()) / 2,
                           r.y() + (r.height() - wallpaper.pixmaps[i].height()) / 2,
                           wallpaper.pixmaps[i]);
      }
    }
  }
}

//--------------------------------------------------------------------------------

void DesktopWidget::addApplet(const QString &type)
{
  DesktopApplet *applet = nullptr;

  if ( type == "Weather" )
  {
    applet = new WeatherApplet(this, QString("%1_%2").arg(type).arg(++appletNum));
  }
  else if ( type == "DiskUsage" )
  {
    applet = new DiskUsageApplet(this, QString("%1_%2").arg(type).arg(++appletNum));
  }
  else if ( type == "PictureFrame" )
  {
    applet = new PictureFrameApplet(this, QString("%1_%2").arg(type).arg(++appletNum));
  }

  if ( applet )
  {
    applets.append(applet);
    applet->loadConfig();  // defaults + size
    applet->move(QCursor::pos());
    applet->saveConfig();
    applet->show();
    connect(applet, &DesktopApplet::removeThis, this, &DesktopWidget::removeApplet);
  }

  saveAppletsList();
}

//--------------------------------------------------------------------------------

void DesktopWidget::removeApplet(DesktopApplet *applet)
{
  applets.removeOne(applet);
  applet->deleteLater();
  saveAppletsList();
}

//--------------------------------------------------------------------------------

void DesktopWidget::saveAppletsList()
{
  KConfig config;
  KConfigGroup group = config.group("DesktopApplets");

  QStringList appletNames;
  for (DesktopApplet *applet : applets)
    appletNames.append(applet->getId());

  group.writeEntry("applets", appletNames);
}

//--------------------------------------------------------------------------------
