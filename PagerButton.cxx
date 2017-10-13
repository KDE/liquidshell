#include <PagerButton.hxx>

#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMenu>
#include <QDebug>

#include <KWindowSystem>
#include <KLocalizedString>
#include <KCMultiDialog>

//--------------------------------------------------------------------------------

PagerButton::PagerButton(int num)
  : desktop(num)
{
  setText(KWindowSystem::desktopName(desktop).isEmpty() ?
          QString::number(desktop) : KWindowSystem::desktopName(desktop));

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setAcceptDrops(true);
  dragDropTimer.setSingleShot(true);
  dragDropTimer.setInterval(1000);
  connect(&dragDropTimer, &QTimer::timeout, [this]() { emit clicked(true); });

  createPixmap();

  connect(KWindowSystem::self(), &KWindowSystem::windowAdded,
          this, &PagerButton::createPixmap);

  connect(KWindowSystem::self(), &KWindowSystem::windowRemoved,
          this, &PagerButton::createPixmap);

  connect(KWindowSystem::self(), &KWindowSystem::desktopNamesChanged,
          [this]() { KWindowSystem::desktopName(desktop).isEmpty() ?
                     QString::number(desktop) : KWindowSystem::desktopName(desktop); });

  connect(KWindowSystem::self(), SIGNAL(windowChanged(WId, NET::Properties, NET::Properties2)),
          this, SLOT(windowChanged(WId, NET::Properties, NET::Properties2)));

  QAction *action = new QAction(this);
  action->setIcon(QIcon::fromTheme("configure"));
  action->setText(i18n("Configure..."));
  addAction(action);
  connect(action, &QAction::triggered,
          [this]()
          {
            auto dialog = new KCMultiDialog(parentWidget());
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->addModule("desktop");
            dialog->adjustSize();
            dialog->show();
          }
         );
  setContextMenuPolicy(Qt::ActionsContextMenu);
}

//--------------------------------------------------------------------------------

void PagerButton::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);

  QPainter painter(this);

  QStyleOptionButton option;
  initStyleOption(&option);

  style()->drawControl(QStyle::CE_PushButtonBevel, &option, &painter, this);

  if ( !firstPixmap.isNull() )
    painter.drawPixmap((width() - firstPixmap.width()) / 2, (height() - firstPixmap.height()) / 2, firstPixmap);

  style()->drawControl(QStyle::CE_PushButtonLabel, &option, &painter, this);
}

//--------------------------------------------------------------------------------

void PagerButton::createPixmap()
{
  firstPixmap = QPixmap();
  QList<WId> windows = KWindowSystem::windows();

  foreach (WId wid, windows)
  {
    KWindowInfo win(wid, NET::WMDesktop | NET::WMWindowType | NET::WMState | NET::WMIcon);

    if ( win.valid(true) && win.isOnDesktop(desktop) &&
        (win.windowType(NET::DesktopMask) != NET::Desktop) &&
        (win.windowType(NET::DockMask) != NET::Dock) &&
         !(win.state() & NET::SkipTaskbar) )
    {
      firstPixmap = KWindowSystem::icon(wid, 22, 22, true);
      break;
    }
  }

  update();
}

//--------------------------------------------------------------------------------

void PagerButton::windowChanged(WId id, NET::Properties props, NET::Properties2 props2)
{
  Q_UNUSED(id)
  Q_UNUSED(props2)

  if ( props & (NET::WMIcon | NET::WMDesktop) )
    createPixmap();
}

//--------------------------------------------------------------------------------

void PagerButton::dragEnterEvent(QDragEnterEvent *event)
{
  event->accept();
  dragDropTimer.start();
}

//--------------------------------------------------------------------------------

void PagerButton::dragLeaveEvent(QDragLeaveEvent *event)
{
  event->accept();
  dragDropTimer.stop();
}

//--------------------------------------------------------------------------------

void PagerButton::dropEvent(QDropEvent *event)
{
  dragDropTimer.stop();

  if ( !event->mimeData()->hasFormat("application/x-winId") )
  {
    event->ignore();
    return;
  }

  event->acceptProposedAction();
  WId wid = static_cast<WId>(event->mimeData()->data("application/x-winId").toInt());

  KWindowSystem::setOnDesktop(wid, desktop);
}

//--------------------------------------------------------------------------------
