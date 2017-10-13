#include <SysTrayItem.hxx>

#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>

//--------------------------------------------------------------------------------

SysTrayItem::SysTrayItem(QWidget *parent)
  : QLabel(parent)
{
  setFixedSize(QSize(22, 22));
}

//--------------------------------------------------------------------------------

void SysTrayItem::mousePressEvent(QMouseEvent *event)
{
  if ( event->button() != Qt::LeftButton )
    return;

  toggleDetailsList();
}

//--------------------------------------------------------------------------------

void SysTrayItem::showDetailsList()
{
  QWidget *detailsList = getDetailsList();

  if ( !detailsList )
    return;

  QPoint point = mapToGlobal(pos());
  QRect screen = QApplication::desktop()->availableGeometry(this);
  point.setX(std::min(point.x(), screen.x() + screen.width() - detailsList->size().width()));
  point.setY(screen.bottom() - detailsList->size().height());
  detailsList->move(point);
  detailsList->show();
  detailsList->raise();
}

//--------------------------------------------------------------------------------

void SysTrayItem::toggleDetailsList()
{
  QWidget *detailsList = getDetailsList();

  if ( !detailsList )
    return;

  if ( detailsList->isVisible() )
    detailsList->close();
  else
    showDetailsList();
}

//--------------------------------------------------------------------------------
