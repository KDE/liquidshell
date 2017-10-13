#include <DeviceNotifier.hxx>
#include <DeviceList.hxx>

#include <Solid/DeviceNotifier>

#include <QIcon>
#include <QMouseEvent>
#include <QDebug>

#include <KLocalizedString>

//--------------------------------------------------------------------------------

DeviceNotifier::DeviceNotifier(QWidget *parent)
  : SysTrayItem(parent)
{
  setPixmap(QIcon::fromTheme("device-notifier").pixmap(size()));
  setToolTip(i18n("Device Notifier"));

  deviceList = new DeviceList(this);
  deviceList->setWindowTitle(i18n("Device List"));

  if ( deviceList->isEmpty() )
    hide();

  connect(deviceList, &DeviceList::deviceWasAdded, this, &DeviceNotifier::showDetailsList);
  connect(deviceList, &DeviceList::deviceWasRemoved, this, &DeviceNotifier::checkDeviceList);
}

//--------------------------------------------------------------------------------

QWidget *DeviceNotifier::getDetailsList()
{
  deviceList->adjustSize();
  deviceList->resize(deviceList->size().expandedTo(QSize(300, 100)));
  return deviceList;
}

//--------------------------------------------------------------------------------

void DeviceNotifier::checkDeviceList()
{
  if ( deviceList->isEmpty() )
  {
    deviceList->hide();
    hide();
  }
  else if ( deviceList->isVisible() )
    showDetailsList();  // reposition
}

//--------------------------------------------------------------------------------
