#include <Bluetooth.hxx>

#include <QIcon>

#include <BluezQt/Adapter>
#include <BluezQt/InitManagerJob>

#include <KLocalizedString>

//--------------------------------------------------------------------------------

Bluetooth::Bluetooth(QWidget *parent)
  : SysTrayItem(parent)
{
  manager = new BluezQt::Manager(this);
  BluezQt::InitManagerJob *job = manager->init();
  job->start();
  connect(job, &BluezQt::InitManagerJob::result, this, &Bluetooth::changed);

  connect(manager, &BluezQt::Manager::adapterAdded, this, &Bluetooth::changed);
  connect(manager, &BluezQt::Manager::adapterRemoved, this, &Bluetooth::changed);
  connect(manager, &BluezQt::Manager::allAdaptersRemoved, this, &Bluetooth::changed);
  connect(manager, &BluezQt::Manager::bluetoothOperationalChanged, this, &Bluetooth::changed);
  connect(manager, &BluezQt::Manager::operationalChanged, this, &Bluetooth::changed);
}

//--------------------------------------------------------------------------------

void Bluetooth::changed()
{
  if ( manager->adapters().isEmpty() || !manager->isOperational() )
  {
    hide();  // no BT
    return;
  }

  show();

  if ( manager->isBluetoothOperational() )
  {
    setPixmap(QIcon::fromTheme("preferences-system-bluetooth.png").pixmap(size()));
    setToolTip(i18n("Bluetooth is operational"));
  }
  else
  {
    setPixmap(QIcon::fromTheme("preferences-system-bluetooth-inactive.png").pixmap(size()));
    setToolTip(i18n("Bluetooth is not operational"));
  }
}

//--------------------------------------------------------------------------------

QWidget *Bluetooth::getDetailsList()
{
  if ( !dialog )
  {
    dialog = new KCMultiDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->addModule("bluedevilglobal");
    dialog->addModule("bluedeviladapters");
    dialog->addModule("bluedevildevices");
    dialog->adjustSize();
    dialog->setWindowTitle(i18n("Bluetooth"));
  }

  return dialog;
}

//--------------------------------------------------------------------------------
