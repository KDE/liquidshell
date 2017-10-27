/*
  Copyright 2017 Martin Koller

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

#include <StartMenu.hxx>

#include <QAction>
#include <QContextMenuEvent>
#include <QDrag>
#include <QMimeData>
#include <QGuiApplication>
#include <QStyleHints>

#include <KRun>
#include <KSycoca>
#include <KService>
#include <KLocalizedString>

//--------------------------------------------------------------------------------

StartMenu::StartMenu(DesktopPanel *parent)
  : QPushButton(parent)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  setIconSize((parent->getRows() == 1) ? QSize(22, 22) : QSize(48, 48));

  connect(parent, &DesktopPanel::rowsChanged,
          [this](int rows) { setIconSize((rows == 1) ? QSize(22, 22) : QSize(48, 48)); });

  setThemeIcon("start-here-kde");

  setMenu(new PopupMenu(this));

  fill();

  connect(KSycoca::self(), SIGNAL(databaseChanged()), this, SLOT(fill()));
}

//--------------------------------------------------------------------------------

void StartMenu::setThemeIcon(const QString &icon)
{
  themeIcon = icon;
  setIcon(QIcon::fromTheme(themeIcon));
}

//--------------------------------------------------------------------------------

void StartMenu::fill()
{
  menu()->clear();

  fillFromGroup(menu(), KServiceGroup::root());

  QAction *action = menu()->addAction(QIcon::fromTheme("system-switch-user"), i18n("Switch User"));
  connect(action, &QAction::triggered,
          []()
          {
            std::system("dbus-send --type=method_call --dest=org.kde.ksmserver "
                        "/KSMServer org.kde.KSMServerInterface.openSwitchUserDialog");
          });
}

//--------------------------------------------------------------------------------

void StartMenu::fillFromGroup(QMenu *menu, KServiceGroup::Ptr group)
{
  if ( !group || !group->isValid() )
    return;

  QList<KServiceGroup::Ptr> groupEntries =
      group->groupEntries(static_cast<KServiceGroup::EntriesOptions>(KServiceGroup::SortEntries |
                                                                     KServiceGroup::ExcludeNoDisplay));

  for (KServiceGroup::Ptr groupEntry : groupEntries)
  {
    if ( groupEntry->childCount() == 0 )
      continue;

    QMenu *submenu = new PopupMenu(menu);
    menu->addMenu(submenu);
    submenu->setTitle(groupEntry->caption());
    submenu->setIcon(QIcon::fromTheme(groupEntry->icon()));
    fillFromGroup(submenu, groupEntry);
  }

  KService::List serviceEntries =
      group->serviceEntries(static_cast<KServiceGroup::EntriesOptions>(KServiceGroup::SortEntries |
                                                                       KServiceGroup::ExcludeNoDisplay));

  for (KService::Ptr serviceEntry : serviceEntries)
  {
    if ( !serviceEntry->isType(KST_KService) )
      continue;

    QIcon icon = QIcon::fromTheme(serviceEntry->icon());
    QString text = serviceEntry->name();
    if ( !serviceEntry->genericName().isEmpty() && (serviceEntry->genericName() != serviceEntry->name()) )
      text += QString(" (%1)").arg(serviceEntry->genericName());

    QAction *action = menu->addAction(icon, text);
    action->setData(serviceEntry->entryPath());

    action->setToolTip(static_cast<const KService *>(serviceEntry.data())->comment());

    connect(action, &QAction::triggered, [serviceEntry]() { KRun::runApplication(*serviceEntry, QList<QUrl>(), nullptr); });
  }
}

//--------------------------------------------------------------------------------

void StartMenu::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu menu;

  QAction *action = menu.addAction(QIcon::fromTheme("configure"), i18n("Configure Menu..."));
  connect(action, &QAction::triggered, []() { KRun::runCommand(QString("kmenuedit"), nullptr); });

  menu.exec(event->globalPos());
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

void PopupMenu::mousePressEvent(QMouseEvent *event)
{
  if ( event->button() == Qt::LeftButton )
    dragStartPos = event->pos();

  QMenu::mousePressEvent(event);
}

//--------------------------------------------------------------------------------

void PopupMenu::mouseMoveEvent(QMouseEvent *event)
{
  if ( (event->buttons() == Qt::LeftButton) && (dragStartPos != QPoint(-1, -1)) &&
       (event->pos() - dragStartPos).manhattanLength() > QGuiApplication::styleHints()->startDragDistance() )
  {
    dragStartPos = QPoint(-1, -1);
    QAction *action = actionAt(event->pos());
    if ( action && !action->menu() && action->data().isValid() )
    {
      event->accept();

      QDrag *drag = new QDrag(this);
      QMimeData *mimeData = new QMimeData;
      mimeData->setUrls(QList<QUrl>() << QUrl::fromLocalFile(action->data().toString()));
      drag->setMimeData(mimeData);
      drag->setPixmap(action->icon().pixmap(32, 32));
      drag->exec(Qt::CopyAction);
    }
  }
  else
    QMenu::mouseMoveEvent(event);
}

//--------------------------------------------------------------------------------
