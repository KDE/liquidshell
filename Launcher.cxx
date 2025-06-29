/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <Launcher.hxx>

#include <QVBoxLayout>
#include <QFileDialog>
#include <QAction>
#include <QCursor>
#include <QMenu>
#include <QIcon>
#include <QDebug>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <KIO/OpenUrlJob>
#include <KIO/JobUiDelegateFactory>

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
    connect(action, &QAction::triggered, [this]()
            {
              KIO::OpenUrlJob *job = new KIO::OpenUrlJob(QUrl::fromLocalFile(dirPath));
              job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
              job->start();
            });
  }

  menu.exec(QCursor::pos());
}

//--------------------------------------------------------------------------------
