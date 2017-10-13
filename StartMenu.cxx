#include <StartMenu.hxx>

#include <KRun>
#include <KSycoca>
#include <KService>

//--------------------------------------------------------------------------------

StartMenu::StartMenu(QWidget *parent)
  : QPushButton(parent)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  setIconSize(QSize(48, 48));
  setThemeIcon("start-here-kde");

  setMenu(new QMenu(this));

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

    QMenu *submenu = menu->addMenu(groupEntry->caption());
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

    action->setToolTip(static_cast<KService *>(serviceEntry.data())->comment());

    connect(action, &QAction::triggered, [serviceEntry]() { KRun::runApplication(*serviceEntry, QList<QUrl>(), nullptr); });
  }
}

//--------------------------------------------------------------------------------
