/*
  Copyright 2017 Martin Koller, kollix@aon.at

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

AppMenu::AppMenu(DesktopPanel *parent)
  : Launcher(parent, "AppMenu")
{
  button = new QToolButton;  // QToolButton is smaller than QPushButton
  button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  button->setIconSize((parent->getRows() == 1) ? QSize(22, 22) : QSize(48, 48));

  connect(parent, &DesktopPanel::rowsChanged,
          [this](int rows) { button->setIconSize((rows == 1) ? QSize(22, 22) : QSize(48, 48)); });

  popup = new Menu(button);
  connect(button, &QToolButton::pressed, this, &AppMenu::showMenu);

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

    AppButton *entryButton = new AppButton(this, icon, name);
    entryButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(entryButton, &AppButton::clicked, [this, url]() { popup->close(); new KRun(url, nullptr); });
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
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

Menu::Menu(QWidget *parent)
  : QFrame(parent, Qt::Popup)
{
  setFrameShape(QFrame::StyledPanel);
  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(QMargins());
  vbox->setSpacing(0);
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
// helper class to ensure the positioning of the icon and text independent of the style

#include <QLabel>

AppButton::AppButton(QWidget *parent, const QIcon &icon, const QString &name)
  : QToolButton(parent)
{
  setAutoRaise(true);
  QHBoxLayout *hbox = new QHBoxLayout(this);

  QLabel *iconLabel = new QLabel;
  iconLabel->setContextMenuPolicy(Qt::PreventContextMenu);
  iconLabel->setFixedSize(32, 32);
  iconLabel->setPixmap(icon.pixmap(QSize(32, 32)));
  hbox->addWidget(iconLabel);

  QLabel *textLabel = new QLabel(name);
  hbox->addWidget(textLabel);
}

//--------------------------------------------------------------------------------

QSize AppButton::sizeHint() const
{
  return layout()->sizeHint();
}

//--------------------------------------------------------------------------------
