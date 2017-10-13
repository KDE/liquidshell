#include <WindowList.hxx>

#include <QIcon>
#include <QMenu>

#include <KWindowSystem>

//--------------------------------------------------------------------------------

WindowList::WindowList(QWidget *parent)
  : QPushButton(parent)
{
  setIcon(QIcon::fromTheme("arrow-up"));
  setMaximumWidth(22);
  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

  setMenu(new QMenu(this));
  connect(menu(), &QMenu::aboutToShow, this, &WindowList::fillMenu);
}

//--------------------------------------------------------------------------------

void WindowList::fillMenu()
{
  menu()->clear();

  QList<WId> windows = KWindowSystem::windows();

  for (int i = 1; i <= KWindowSystem::numberOfDesktops(); i++)
  {
    menu()->addSection(KWindowSystem::desktopName(i).isEmpty() ?
                       QString::number(i) : KWindowSystem::desktopName(i));

    for (WId wid : windows)
    {
      KWindowInfo win(wid, NET::WMDesktop | NET::WMWindowType | NET::WMState | NET::WMName | NET::WMIcon);
      if ( win.valid(true) && win.isOnDesktop(i) &&
          (win.windowType(NET::DesktopMask) != NET::Desktop) &&
          (win.windowType(NET::DockMask) != NET::Dock) &&
           !(win.state() & NET::SkipTaskbar) )
      {
        menu()->addAction(KWindowSystem::icon(wid, 22, 22, true), win.name(),
                          [wid]() { KWindowSystem::forceActiveWindow(wid); });
      }
    }
  }
}

//--------------------------------------------------------------------------------
