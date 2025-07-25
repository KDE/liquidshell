/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <AppMenu.hxx>
#include <IconButton.hxx>
#include <DesktopWidget.hxx>

#include <QGridLayout>
#include <QToolButton>
#include <QStandardPaths>
#include <QDir>
#include <QMimeDatabase>
#include <QApplication>
#include <QScreen>

#include <KFileItem>
#include <KDesktopFile>
#include <KIconLoader>

#include <KIO/OpenUrlJob>
#include <KIO/JobUiDelegateFactory>

//--------------------------------------------------------------------------------

AppMenu::AppMenu(DesktopPanel *parent)
  : Launcher(parent, "AppMenu")
{
  button = new QToolButton;  // QToolButton is smaller than QPushButton
  adjustIconSize();
  button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  connect(button, &QToolButton::pressed, this, &AppMenu::showMenu);

  layout()->addWidget(button);
  loadConfig(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

  connect(parent, &DesktopPanel::rowsChanged, this, &AppMenu::adjustIconSize);
  connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, &AppMenu::adjustIconSize);
  connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, &AppMenu::fill);
}

//--------------------------------------------------------------------------------

void AppMenu::adjustIconSize()
{
  const int MAX_ROWS = qobject_cast<DesktopPanel *>(parentWidget())->getRows();

  if ( MAX_ROWS > 1 )
    button->setIconSize(QSize(48, 48));
  else
  {
    int size = KIconLoader::global()->currentSize(KIconLoader::Panel);
    button->setIconSize(QSize(size, size));
  }
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
    button->setIcon(QIcon::fromTheme("folder"));
}

//--------------------------------------------------------------------------------

void AppMenu::showMenu()
{
  Menu *popup = new Menu(button);

  QDir dir(dirPath);
  QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotDot);

  int row = 0, col = 0, h = 0;
  const int maxHeight = DesktopWidget::availableSize().height();

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
      if ( info.fileName() == "." )
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

    IconButton *entryButton = new IconButton(this, icon, 32, name);
    connect(entryButton, &IconButton::clicked, [this, url, popup]()
            {
              popup->close();
              KIO::OpenUrlJob *job = new KIO::OpenUrlJob(url);
              job->setRunExecutables(true);
              job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
              job->start();
            });

    h += entryButton->sizeHint().height();

    if ( h >= maxHeight )
    {
      h = entryButton->sizeHint().height();
      col++;
      row = 0;
    }

    static_cast<QGridLayout *>(popup->layout())->addWidget(entryButton, row++, col);
  }

  popup->exec();
  button->setDown(false);
  delete popup;
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

Menu::Menu(QWidget *parent)
  : QMenu(parent)
{
  QGridLayout *grid = new QGridLayout(this);
  grid->setContentsMargins(QMargins());
  grid->setSpacing(0);
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
