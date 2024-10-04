/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
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
#include <KFileItem>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KIconLoader>

#include <KIO/OpenUrlJob>
#include <KIO/JobUiDelegateFactory>

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

  // create default folder
  QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/QuickLaunch");
  if ( !dir.exists() )
    dir.mkpath(dir.path());

  loadConfig(dir.path());

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

      connect(button, &QToolButton::clicked, [this, url]()
              {
                KIO::OpenUrlJob *job = new KIO::OpenUrlJob(url);
                job->setRunExecutables(true);
                job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
                job->start();
              });

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
    connect(button, &QToolButton::clicked, [this, url]()
            {
              KIO::OpenUrlJob *job = new KIO::OpenUrlJob(url);
              job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
              job->start();
            });

    grid->addWidget(button, 0, 0, Qt::AlignCenter);

    button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setIcon(QIcon::fromTheme("internet-web-browser"));
    button->setIconSize(QSize(22, 22));
    button->setFixedHeight(button->sizeHint().height() - 2);
    button->setToolTip(i18n("Web Browser"));
    connect(button, &QToolButton::clicked, [this]()
            {
              KIO::OpenUrlJob *job = new KIO::OpenUrlJob(QUrl("http://www.kde.org"));
              job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
              job->start();
            });

    if ( MAX_ROWS == 1 )
      grid->addWidget(button, 0, 1, Qt::AlignCenter);
    else
      grid->addWidget(button, 1, 0, Qt::AlignCenter);
  }
}

//--------------------------------------------------------------------------------
