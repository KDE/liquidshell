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

#include <QuickLaunch.hxx>

#include <QFrame>
#include <QIcon>
#include <QToolButton>
#include <QMimeDatabase>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include <KDesktopFile>
#include <KRun>
#include <KFileItem>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KIconLoader>

//--------------------------------------------------------------------------------

QuickLaunch::QuickLaunch(DesktopPanel *parent)
  : Launcher(parent, "QuickLaunch")
{
  QFrame *frame = new QFrame;
  frame->setFrameShape(QFrame::StyledPanel);

  grid = new QGridLayout(frame);
  grid->setContentsMargins(QMargins());
  grid->setSpacing(2);

  layout()->addWidget(frame);
  loadConfig();

  connect(parent, &DesktopPanel::rowsChanged, this, &QuickLaunch::fill);
  connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, &QuickLaunch::fill);
}

//--------------------------------------------------------------------------------

void QuickLaunch::fill()
{
  const int MAX_ROWS = qobject_cast<DesktopPanel *>(parentWidget())->getRows();

  QLayoutItem *child;
  while ( (child = grid->takeAt(0)) )
  {
    delete child->widget();
    delete child;
  }

  if ( !dirPath.isEmpty() )
  {
    QDir dir(dirPath);
    QFileInfoList entries = dir.entryInfoList(QDir::Files);
    int row = 0, col = 0;

    for (const QFileInfo &info : entries)
    {
      QUrl url(QUrl::fromLocalFile(info.absoluteFilePath()));
      QIcon icon;
      QString name = info.fileName();

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
      else
      {
        QMimeDatabase db;
        icon = QIcon::fromTheme(db.mimeTypeForFile(info.absoluteFilePath()).iconName());
      }

      QToolButton *button = new QToolButton(this);
      button->setAutoRaise(true);
      button->setIcon(icon);
      button->setToolTip(name);

      if ( MAX_ROWS > 1 )
        button->setIconSize(QSize(22, 22));
      else
      {
        int size = KIconLoader::global()->currentSize(KIconLoader::Panel);
        button->setIconSize(QSize(size, size));
      }

      button->setFixedHeight(button->sizeHint().height() - 2);

      connect(button, &QToolButton::clicked, [url]() { new KRun(url, nullptr); });

      grid->addWidget(button, row, col, Qt::AlignCenter);

      row = (row + 1) % MAX_ROWS;
      if ( row == 0 ) col++;

      // limit width in case one selects a directory with too many items
      const int MAX_COLS = 15;
      if ( col > MAX_COLS )
        break;
    }
  }

  if ( grid->count() == 0 )
  {
    // add default entry
    QToolButton *button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setIcon(QIcon::fromTheme("user-home"));
    button->setIconSize(QSize(22, 22));
    button->setFixedHeight(button->sizeHint().height() - 2);
    button->setToolTip(QStandardPaths::displayName(QStandardPaths::HomeLocation));
    QUrl url = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    connect(button, &QToolButton::clicked, [url]() { new KRun(url, nullptr); });

    grid->addWidget(button, 0, 0, Qt::AlignCenter);

    button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setIcon(QIcon::fromTheme("internet-web-browser"));
    button->setIconSize(QSize(22, 22));
    button->setFixedHeight(button->sizeHint().height() - 2);
    button->setToolTip(i18n("Web Browser"));
    connect(button, &QToolButton::clicked, []() { new KRun(QUrl("http://www.kde.org"), nullptr); });

    if ( MAX_ROWS == 1 )
      grid->addWidget(button, 0, 1, Qt::AlignCenter);
    else
      grid->addWidget(button, 1, 0, Qt::AlignCenter);
  }
}

//--------------------------------------------------------------------------------
