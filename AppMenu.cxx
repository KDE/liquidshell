#include <AppMenu.hxx>

#include <QVBoxLayout>
#include <QToolButton>
#include <QStandardPaths>
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

AppMenu::AppMenu(QWidget *parent)
  : Launcher(parent, "AppMenu")
{
  button = new QPushButton;
  button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  button->setIconSize(QSize(48, 48));

  popup = new Menu(button);
  connect(button, &QPushButton::pressed, this, &AppMenu::showMenu);

  layout()->addWidget(button);
  loadConfig(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
}

//--------------------------------------------------------------------------------

void AppMenu::fill()
{
  KDesktopFile desktopFile(dirPath + "/.directory");

  if ( !desktopFile.readIcon().isEmpty() )
    button->setIcon(QIcon::fromTheme(desktopFile.readIcon()));
  else if ( dirPath == QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) )
    button->setIcon(QIcon::fromTheme("user-desktop"));
  else // fallback
    button->setIcon(QIcon::fromTheme("start-here"));

  QLayoutItem *child;
  while ( (child = popup->layout()->takeAt(0)) )
  {
    delete child->widget();
    delete child;
  }

  QDir dir(dirPath);
  QFileInfoList entries = dir.entryInfoList();

  for (const QFileInfo &info : entries)
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

      QString iconName = desktopFile.readIcon();
      icon = QIcon::fromTheme(iconName.isEmpty() ? name : iconName);
    }
    else if ( info.isDir() )
    {
      if ( info.fileName() == ".." )
        continue;
      else if ( info.fileName() == "." )
      {
        name = info.dir().dirName();
        icon = button->icon();
      }
      else
        icon = QIcon::fromTheme("folder");
    }
    else
    {
      QMimeDatabase db;
      icon = QIcon::fromTheme(db.mimeTypeForFile(info.absoluteFilePath()).iconName());
    }

    if ( name.isEmpty() )
      name = info.fileName();

    QToolButton *entryButton = new QToolButton(this);
    entryButton->setAutoRaise(true);
    entryButton->setIcon(icon);
    entryButton->setText(name);
    entryButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    entryButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    entryButton->setIconSize(QSize(32, 32));
    connect(entryButton, &QToolButton::clicked, [this, url]() { popup->close(); new KRun(url, nullptr); });
    popup->layout()->addWidget(entryButton);
  }
}

//--------------------------------------------------------------------------------

void AppMenu::showMenu()
{
  popup->exec();
  button->setDown(false);
}

//--------------------------------------------------------------------------------
