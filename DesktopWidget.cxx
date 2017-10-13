#include <DesktopWidget.hxx>
#include <DesktopPanel.hxx>

#include <KWindowSystem>

#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QDebug>

#include <KConfig>
#include <KConfigGroup>

//--------------------------------------------------------------------------------

DesktopWidget::DesktopWidget()
  : QWidget(nullptr, Qt::WindowDoesNotAcceptFocus)
{
  setAttribute(Qt::WA_AlwaysShowToolTips);
  setAutoFillBackground(true);
  setFixedSize(QApplication::desktop()->size());

  KWindowSystem::setType(winId(), NET::Desktop);
  KWindowSystem::setState(winId(), NET::KeepBelow);

  panel = new DesktopPanel(nullptr);
  panel->show();
  connect(panel, &DesktopPanel::resized, this, &DesktopWidget::placePanel);
  placePanel();

  KConfig config;
  for (int i = 1; i <= KWindowSystem::numberOfDesktops(); i++)
  {
    KConfigGroup group = config.group(QString("Desktop_%1").arg(i));

    Wallpaper wallpaper;

    wallpaper.color = group.readEntry("Color", QColor(Qt::black));
    wallpaper.fileName = group.readEntry(QString("Wallpaper"), QString());
    wallpaper.mode = group.readEntry("WallpaperMode", QString());

    wallpapers.append(wallpaper);
  }

  connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, &DesktopWidget::desktopChanged);

  desktopChanged();
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
  if ( currentDesktop != -1 )
    wallpapers[currentDesktop].pixmap = QPixmap();  // free memory

  currentDesktop = KWindowSystem::currentDesktop() - 1;

  if ( currentDesktop >= wallpapers.count() )  // paranoia
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
