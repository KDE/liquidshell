/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <TaskBarButton.hxx>
#include <KWinCompat.hxx>

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionButton>
#include <QPainter>
#include <QMenu>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDrag>
#include <QMimeData>
#include <QGuiApplication>
#include <QStyleHints>
#include <QPointer>

#include <KSqueezedTextLabel>
#include <KColorScheme>
#include <KLocalizedString>
#include <KWindowInfo>

//--------------------------------------------------------------------------------

TaskBarButton::TaskBarButton(WId theWid)
  : wid(theWid)
{
  setAutoFillBackground(true);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setAcceptDrops(true);
  dragDropTimer.setSingleShot(true);
  dragDropTimer.setInterval(1000);
  connect(&dragDropTimer, &QTimer::timeout, [this]() { KWinCompat::forceActiveWindow(wid); });

  QHBoxLayout *hbox = new QHBoxLayout(this);
  hbox->setContentsMargins(QMargins(4, 2, 4, 2));

  iconLabel = new QLabel;
  iconLabel->setScaledContents(true);
  iconLabel->setFixedSize(32, 32);
  iconLabel->setContextMenuPolicy(Qt::PreventContextMenu);
  hbox->addWidget(iconLabel);

  textLabel = new KSqueezedTextLabel;
  textLabel->setTextElideMode(Qt::ElideRight);
  textLabel->setContextMenuPolicy(Qt::PreventContextMenu);
  hbox->addWidget(textLabel);

  fill();
  setBackground();

  connect(KWinCompat::self(), SIGNAL(windowChanged(WId, NET::Properties, NET::Properties2)),
          this, SLOT(windowChanged(WId, NET::Properties, NET::Properties2)));

  connect(KWinCompat::self(), &KWinCompat::activeWindowChanged,
          this, &TaskBarButton::setBackground);
}

//--------------------------------------------------------------------------------

void TaskBarButton::setIconSize(int size)
{
  iconLabel->setFixedSize(size, size);
}

//--------------------------------------------------------------------------------

void TaskBarButton::fill()
{
  KWindowInfo win(wid, NET::WMName | NET::WMIcon);
  iconLabel->setPixmap(KWinCompat::icon(wid, 32, 32, true));
  textLabel->setText(win.name());
  setToolTip(win.name());
}

//--------------------------------------------------------------------------------

void TaskBarButton::mousePressEvent(QMouseEvent *event)
{
  if ( event->button() == Qt::LeftButton )
  {
    KWindowSystem::setShowingDesktop(false);

    if ( wid == KWinCompat::activeWindow() )
      KWinCompat::minimizeWindow(wid);
    else
      KWinCompat::forceActiveWindow(wid);

    dragStartPos = event->pos();
    event->accept();
  }
  else if ( event->button() == Qt::RightButton )
  {
    // context menu to close window etc.
    QPointer<QMenu> menu(new QMenu(this));

    if ( KWinCompat::numberOfDesktops() > 1 )
    {
      QMenu *desktops = menu->addMenu(i18n("Move To Desktop"));
      desktops->addAction(i18n("All Desktops"), [this]() { KWinCompat::setOnAllDesktops(wid, true); });
      desktops->addSeparator();

      for (int i = 1; i <= KWinCompat::numberOfDesktops(); i++)
        desktops->addAction(KWinCompat::desktopName(i), [this, i]() { KWinCompat::setOnDesktop(wid, i); });
    }

    menu->addAction(QIcon::fromTheme("window-close"), i18n("Close"),
                    [this]()
                    {
                      NETRootInfo ri(qApp->nativeInterface<QNativeInterface::QX11Application>()->connection(), NET::CloseWindow);
                      ri.closeWindowRequest(wid);
                    }
                   );

    menu->exec(event->globalPosition().toPoint());
    delete menu;
  }
}

//--------------------------------------------------------------------------------

void TaskBarButton::mouseMoveEvent(QMouseEvent *event)
{
  event->accept();

  if ( (event->buttons() == Qt::LeftButton) &&
       (event->pos() - dragStartPos).manhattanLength() > QGuiApplication::styleHints()->startDragDistance() )
  {
    QDrag *drag = new QDrag(parentWidget());
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-winId", QByteArray::number(static_cast<int>(wid)));
    drag->setMimeData(mimeData);
    drag->setPixmap(iconLabel->pixmap());
    drag->exec();
  }
}

//--------------------------------------------------------------------------------

void TaskBarButton::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);

  QPainter painter(this);

  QStyleOptionButton option;
  initStyleOption(&option);

  style()->drawControl(QStyle::CE_PushButtonBevel, &option, &painter, this);
}

//--------------------------------------------------------------------------------

void TaskBarButton::windowChanged(WId id, NET::Properties props, NET::Properties2 props2)
{
  Q_UNUSED(id)
  Q_UNUSED(props2)

  //qDebug() << textLabel->text() << props << (props & (NET::WMVisibleName | NET::WMState |  NET::WMName));
  //qDebug() << this << id << props << "me" << (wid == id);
  //if ( (id != wid) || (props == 0) )
    //return;

  if ( props & (NET::WMState | NET::ActiveWindow) )
    setBackground();

  // WMVisibleName alone is not enough. WMName needed
  if ( (wid == id) && (props & (NET::WMIcon | NET::WMName)) )
    fill();
}

//--------------------------------------------------------------------------------

void TaskBarButton::setBackground()
{
  KColorScheme scheme(QPalette::Active, KColorScheme::Window);
  QPalette pal = palette();

  KWindowInfo win(wid, NET::WMState);

  if ( win.state() & NET::Hidden )
    pal.setBrush(foregroundRole(), scheme.foreground(KColorScheme::InactiveText));
  else
    pal.setBrush(foregroundRole(), scheme.foreground(KColorScheme::NormalText));

  QBrush brush;

  if ( win.state() & NET::DemandsAttention )
    brush = scheme.background(KColorScheme::ActiveBackground);
  else if ( wid == KWinCompat::activeWindow() )
    brush = scheme.shade(KColorScheme::MidShade);
  else
    brush = scheme.background();

  pal.setBrush(backgroundRole(), brush);
  setPalette(pal);
}

//--------------------------------------------------------------------------------

void TaskBarButton::dragEnterEvent(QDragEnterEvent *event)
{
  event->accept();
  dragDropTimer.start();
}

//--------------------------------------------------------------------------------

void TaskBarButton::dragLeaveEvent(QDragLeaveEvent *event)
{
  event->accept();
  dragDropTimer.stop();
}

//--------------------------------------------------------------------------------

void TaskBarButton::dropEvent(QDropEvent *event)
{
  event->accept();
  dragDropTimer.stop();
}

//--------------------------------------------------------------------------------

void TaskBarButton::updateWMGeometry()
{
  NETWinInfo info(qApp->nativeInterface<QNativeInterface::QX11Application>()->connection(), wid,
                  //QX11Info::appRootWindow(),
                  NETRootInfo(qApp->nativeInterface<QNativeInterface::QX11Application>()->connection(),
                              NET::Properties()).rootWindow(),
                  NET::Properties(), NET::Properties2());

  NETRect rect;
  QPoint globalPos = mapToGlobal(QPoint(0, 0));
  rect.pos.x = globalPos.x();
  rect.pos.y = globalPos.y();
  rect.size.width = width();
  rect.size.height = height();

  info.setIconGeometry(rect);
}

//--------------------------------------------------------------------------------

QSize TaskBarButton::sizeHint() const
{
  QSize s = QPushButton::sizeHint();
  s.setHeight(s.height() - 2);
  return s;
}

//--------------------------------------------------------------------------------
