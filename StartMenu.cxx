/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <StartMenu.hxx>
#include <PopupMenu.hxx>

#include <QAction>
#include <QContextMenuEvent>
#include <QDBusConnection>
#include <QDBusMessage>

#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>
#include <KIO/JobUiDelegateFactory>

#include <KSycoca>
#include <KService>
#include <KLocalizedString>
#include <KIconLoader>

//--------------------------------------------------------------------------------

StartMenu::StartMenu(DesktopPanel *parent)
  : QToolButton(parent)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  setThemeIcon("liquidshell");

  popup = new PopupMenu(this);
  connect(this, &QToolButton::pressed, this, &StartMenu::showMenu);

  fill();
  adjustIconSize();

  connect(KSycoca::self(), SIGNAL(databaseChanged()), this, SLOT(fill()));
  connect(parent, &DesktopPanel::rowsChanged, this, &StartMenu::adjustIconSize);
  connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, &StartMenu::adjustIconSize);
}

//--------------------------------------------------------------------------------

void StartMenu::adjustIconSize()
{
  const int MAX_ROWS = qobject_cast<DesktopPanel *>(parentWidget())->getRows();

  if ( MAX_ROWS > 1 )
    setIconSize(QSize(48, 48));
  else
  {
    int size = KIconLoader::global()->currentSize(KIconLoader::Panel);
    setIconSize(QSize(size, size));
  }
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
  popup->clear();

  fillFromGroup(popup, KServiceGroup::root());

  popup->addSeparator();

  // add important actions
  QAction *action = popup->addAction(QIcon::fromTheme("system-switch-user"), i18n("Switch User"));
  connect(action, &QAction::triggered,
          []()
          {
            QDBusConnection::sessionBus().send(
                QDBusMessage::createMethodCall("org.kde.ksmserver", "/KSMServer",
                                               "org.kde.KSMServerInterface", "openSwitchUserDialog"));
          });

  KService::Ptr sysSettings = KService::serviceByDesktopName("systemsettings");
  if ( sysSettings )
  {
    action = popup->addAction(QIcon::fromTheme(sysSettings->icon()), sysSettings->name());
    connect(action, &QAction::triggered,
            [this, sysSettings]()
            {
              KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(sysSettings);
              job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
              job->start();
            });
  }

  action = popup->addAction(QIcon::fromTheme("system-run"), i18n("Run Command..."));
  connect(action, &QAction::triggered,
          []()
          {
            QDBusConnection::sessionBus().send(
                QDBusMessage::createMethodCall("org.kde.krunner", "/App",
                                               "org.kde.krunner.App", "display"));
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
    action->setData(QUrl::fromLocalFile(serviceEntry->entryPath()));

    action->setToolTip(static_cast<const KService *>(serviceEntry.data())->comment());

    connect(action, &QAction::triggered,
            [this, serviceEntry]()
            {
              KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(serviceEntry);
              job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
              job->start();
            });
  }
}

//--------------------------------------------------------------------------------

void StartMenu::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu menu;

  QAction *action = menu.addAction(QIcon::fromTheme("configure"), i18n("Configure Menu..."));
  connect(action, &QAction::triggered,
          [this]()
          {
            KIO::CommandLauncherJob *job = new KIO::CommandLauncherJob("kmenuedit");
            job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
            job->start();
          });

  menu.exec(event->globalPos());
}

//--------------------------------------------------------------------------------

void StartMenu::showMenu()
{
  popup->adjustSize();
  QPoint p = mapToGlobal(QPoint(0, 0));
  popup->move(p.x(), p.y() - popup->sizeHint().height());
  popup->exec();
  setDown(false);
}

//--------------------------------------------------------------------------------
