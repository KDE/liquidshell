#include <DesktopPanel.hxx>
#include <StartMenu.hxx>
#include <QuickLaunch.hxx>
#include <AppMenu.hxx>
#include <Pager.hxx>
#include <TaskBar.hxx>
#include <LockLogout.hxx>
#include <SysLoad.hxx>
#include <SysTray.hxx>
#include <ClockWidget.hxx>
#include <WindowList.hxx>

#include <kwindowsystem.h>

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QDebug>

//--------------------------------------------------------------------------------

DesktopPanel::DesktopPanel(QWidget *parent)
  : QFrame(parent, Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus)
{
  setAttribute(Qt::WA_AlwaysShowToolTips);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  KWindowSystem::setState(winId(), NET::KeepAbove);
  KWindowSystem::setOnAllDesktops(winId(), true);
  KWindowSystem::setType(winId(), NET::Dock);

  setFrameShape(QFrame::StyledPanel);

  QHBoxLayout *hboxLayout = new QHBoxLayout(this);
  hboxLayout->setContentsMargins(QMargins());
  hboxLayout->setSpacing(2);

  hboxLayout->addWidget(new StartMenu(this));
  hboxLayout->addWidget(new QuickLaunch(this));
  hboxLayout->addWidget(new AppMenu(this));
  hboxLayout->addWidget(new Pager(this));
  hboxLayout->addWidget(new WindowList(this));
  hboxLayout->addWidget(new TaskBar(this));
  hboxLayout->addWidget(new LockLogout(this));
  hboxLayout->addWidget(new SysLoad(this));
  hboxLayout->addWidget(new SysTray(this));
  hboxLayout->addWidget(new ClockWidget(this));
}

//--------------------------------------------------------------------------------

bool DesktopPanel::event(QEvent *ev)
{
  bool ret = QFrame::event(ev);

  if ( ev->type() == QEvent::LayoutRequest )
  {
    if ( sizeHint().height() != height() )
      emit resized();
  }

  return ret;
}

//--------------------------------------------------------------------------------

void DesktopPanel::resizeEvent(QResizeEvent *event)
{
  Q_UNUSED(event);

  emit resized();
}

//--------------------------------------------------------------------------------
