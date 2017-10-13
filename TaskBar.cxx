#include <TaskBar.hxx>
#include <TaskBarButton.hxx>

#include <KWindowSystem>

#include <QVariant>
#include <QSpacerItem>
#include <QDebug>

//--------------------------------------------------------------------------------

TaskBar::TaskBar(QWidget *parent)
  : QWidget(parent)
{
  grid = new QGridLayout(this);
  grid->setSpacing(2);
  grid->setContentsMargins(QMargins());

  fill();

  connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged,
          this, [this](){ fill(); });
          //this, &TaskBar::currentDesktopChanged);

  connect(KWindowSystem::self(), &KWindowSystem::windowAdded,
          this, [this](){ fill(); });

  connect(KWindowSystem::self(), &KWindowSystem::windowRemoved,
          this, [this](){ fill(); });

  connect(KWindowSystem::self(), SIGNAL(windowChanged(WId, NET::Properties, NET::Properties2)),
          this, SLOT(windowChanged(WId, NET::Properties, NET::Properties2)));
}

//--------------------------------------------------------------------------------

void TaskBar::fill()
{
  QLayoutItem *child;
  while ( (child = grid->takeAt(0)) )
  {
    delete child->widget();
    delete child;
  }

  QList<WId> windows = KWindowSystem::windows();

  const int MAX_ROWS = 2;

  int count = 0;
  for (WId wid : windows)
  {
    KWindowInfo win(wid, NET::WMDesktop | NET::WMWindowType | NET::WMState);
    if ( win.valid(true) && win.isOnCurrentDesktop() &&
        (win.windowType(NET::DesktopMask) != NET::Desktop) &&
        (win.windowType(NET::DockMask) != NET::Dock) &&
         !(win.state() & NET::SkipTaskbar) )
      count++;
  }

  if ( count == 0 )
  {
    grid->addItem(new QSpacerItem(0, 0,  QSizePolicy::Expanding), 0, 0);
    return;
  }

  int row = 0, col = 0;
  const int MAX_COLUMNS = count / MAX_ROWS + 1;

  for (WId wid : windows)
  {
    NET::Properties props = NET::WMDesktop | NET::WMWindowType | NET::WMVisibleName | NET::WMState;
    KWindowInfo win(wid, props);
    if ( win.valid(true) && win.isOnCurrentDesktop() &&
        (win.windowType(NET::DesktopMask) != NET::Desktop) &&
        (win.windowType(NET::DockMask) != NET::Dock) &&
        (win.windowType(NET::PopupMenuMask) != NET::PopupMenu) &&
         !(win.state() & NET::SkipTaskbar) )
    {
      TaskBarButton *b = new TaskBarButton(wid);
      grid->addWidget(b, row, col);

      col = (col + 1) % MAX_COLUMNS;
      if ( col == 0 ) row++;
    }
  }
}

//--------------------------------------------------------------------------------

void TaskBar::currentDesktopChanged(int desktop)
{
  Q_UNUSED(desktop)

  fill();
}

//--------------------------------------------------------------------------------

void TaskBar::windowChanged(WId wid, NET::Properties props, NET::Properties2 props2)
{
  Q_UNUSED(wid)
  Q_UNUSED(props2)

  if ( props & NET::WMDesktop )
    fill();
}

//--------------------------------------------------------------------------------
