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
    QFileInfoList entries = dir.entryInfoList();
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
      else if ( !info.isDir() )
      {
        QMimeDatabase db;
        icon = QIcon::fromTheme(db.mimeTypeForFile(info.absoluteFilePath()).iconName());
      }
      else
        continue;

      QToolButton *button = new QToolButton(this);
      button->setAutoRaise(true);
      button->setIcon(icon);
      button->setToolTip(name);
      button->setIconSize(QSize(22, 22));
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
    QUrl url = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    connect(button, &QToolButton::clicked, [url]() { new KRun(url, nullptr); });

    grid->addWidget(button, 0, 0, Qt::AlignCenter);

    button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setIcon(QIcon::fromTheme("internet-web-browser"));
    button->setIconSize(QSize(22, 22));
    connect(button, &QToolButton::clicked, []() { new KRun(QUrl("www.kde.org"), nullptr); });

    if ( MAX_ROWS == 1 )
      grid->addWidget(button, 0, 1, Qt::AlignCenter);
    else
      grid->addWidget(button, 1, 0, Qt::AlignCenter);
  }
}

//--------------------------------------------------------------------------------
