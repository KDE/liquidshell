// SPDX-License-Identifier: GPL-3.0-or-later
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

#include <Launcher.hxx>

#include <QVBoxLayout>
#include <QFileDialog>
#include <QAction>
#include <QCursor>
#include <QMenu>
#include <QIcon>
#include <QDebug>

#include <KRun>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

//--------------------------------------------------------------------------------

Launcher::Launcher(QWidget *parent, const QString &theId)
  : QWidget(parent), id(theId)
{
  new QVBoxLayout(this);
  layout()->setContentsMargins(QMargins());
}

//--------------------------------------------------------------------------------

void Launcher::loadConfig(const QString &defaultDir)
{
  KConfig config;
  KConfigGroup group = config.group(id);
  setDir(group.readEntry(QString("dirPath"), defaultDir));
}

//--------------------------------------------------------------------------------

void Launcher::setDir(const QString &theDirPath)
{
  if ( !dirWatcher.directories().isEmpty() )
    dirWatcher.removePaths(dirWatcher.directories());

  dirPath = theDirPath;
  fill();

  if ( !dirPath.isEmpty() )
  {
    dirWatcher.addPath(dirPath);
    connect(&dirWatcher, &QFileSystemWatcher::directoryChanged, this, &Launcher::fill);
  }
}

//--------------------------------------------------------------------------------

void Launcher::contextMenuEvent(QContextMenuEvent *event)
{
  Q_UNUSED(event)

  QMenu menu;

  QAction *action = menu.addAction(QIcon::fromTheme("configure"), i18n("Configure..."));
  connect(action, &QAction::triggered,
          [this]()
          {
            QString path = QFileDialog::getExistingDirectory(this, QString(), dirPath);
            if ( !path.isEmpty() )
            {
              KConfig config;
              KConfigGroup group = config.group(id);
              group.writeEntry(QString("dirPath"), path);
              setDir(path);
            }
          }
         );

  if ( !dirPath.isEmpty() )
  {
    action = menu.addAction(QIcon::fromTheme("folder"), i18n("Open Folder"));
    connect(action, &QAction::triggered, [this]() { new KRun(QUrl::fromLocalFile(dirPath), nullptr); });
  }

  menu.exec(QCursor::pos());
}

//--------------------------------------------------------------------------------
