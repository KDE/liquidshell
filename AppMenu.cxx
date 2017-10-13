#include <AppMenu.hxx>

#include <QVBoxLayout>
#include <QToolButton>
#include <QDir>
#include <QMimeDatabase>
#include <QDebug>

#include <KFileItem>
#include <KDesktopFile>
#include <KIOWidgets/KRun>

//--------------------------------------------------------------------------------

Menu::Menu(QWidget *parent)
  : QFrame(parent, Qt::Popup)
{
  setFrameShape(QFrame::StyledPanel);
  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(QMargins());
}

//--------------------------------------------------------------------------------

void Menu::hideEvent(QHideEvent *event)
{
  Q_UNUSED(event);
  eventLoop.exit();
}

//--------------------------------------------------------------------------------

void Menu::exec()
{
  adjustSize();
  QPoint p = parentWidget()->mapToGlobal(QPoint(0, 0));
  move(p.x(), p.y() - height());
  show();
  eventLoop.exec();
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

AppMenu::AppMenu(QWidget *parent, const QString &dirPath)
  : QPushButton(parent)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  setIconSize(QSize(48, 48));

  popup = new Menu(this);
  connect(this, &QPushButton::pressed, this, &AppMenu::showMenu);

  setDir(dirPath);
}

//--------------------------------------------------------------------------------

void AppMenu::setDir(const QString &theDirPath)
{
  dirPath = theDirPath;
  fill();

  dirWatcher.addPath(dirPath);
  connect(&dirWatcher, &QFileSystemWatcher::directoryChanged, [this]() { fill(); });
}

//--------------------------------------------------------------------------------

void AppMenu::fill()
{
  KDesktopFile desktopFile(dirPath + "/.directory");

  if ( !desktopFile.readIcon().isEmpty() )
    setIcon(QIcon::fromTheme(desktopFile.readIcon()));
  else if ( dirPath == QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) )
    setIcon(QIcon::fromTheme("user-desktop"));
  else // fallback
    setIcon(QIcon::fromTheme("start-here"));

  QLayoutItem *child;
  while ( (child = popup->layout()->takeAt(0)) )
    delete child;

  QDir dir(dirPath);
  QFileInfoList entries = dir.entryInfoList();

  foreach (const QFileInfo &info, entries)
  {
    QUrl url(QUrl::fromLocalFile(info.absoluteFilePath()));
    QString name;
    QIcon icon;

    KFileItem item(url);

    if ( item.isDesktopFile() )
    {
      KDesktopFile desktopFile(info.absoluteFilePath());
      if ( desktopFile.noDisplay() )
        continue;

      name = desktopFile.readName();
      if ( name.isEmpty() )
        name = desktopFile.readGenericName();

      icon = QIcon::fromTheme(desktopFile.readIcon());
    }
    else if ( info.isDir() )
    {
      if ( info.fileName() == ".." )
        continue;
      else if ( info.fileName() == "." )
      {
        name = info.dir().dirName();
        icon = this->icon();
      }
    }
    else
    {
      QMimeDatabase db;
      icon = QIcon::fromTheme(db.mimeTypeForFile(info.absoluteFilePath()).iconName());
    }

    if ( name.isEmpty() )
      name = info.fileName();

    QToolButton *button = new QToolButton;
    button->setAutoRaise(true);
    button->setIcon(icon);
    button->setText(name);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    button->setIconSize(QSize(32, 32));
    connect(button, &QToolButton::clicked, [this, url]() { popup->close(); new KRun(url, nullptr); });
    popup->layout()->addWidget(button);
  }
}

//--------------------------------------------------------------------------------

void AppMenu::showMenu()
{
  popup->exec();
  setDown(false);
}

//--------------------------------------------------------------------------------
