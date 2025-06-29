/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <Bluetooth.hxx>

#include <QIcon>

#include <BluezQt/Adapter>
#include <BluezQt/InitManagerJob>

#include <KLocalizedString>
#include <KIconLoader>
#include <KPluginMetaData>

//--------------------------------------------------------------------------------

Bluetooth::Bluetooth(QWidget *parent)
  : SysTrayItem(parent)
{
  manager = new BluezQt::Manager(this);
  job = manager->init();
  job->start();
  connect(job, &BluezQt::InitManagerJob::result, this, &Bluetooth::changed);

  connect(manager, &BluezQt::Manager::adapterAdded, this, &Bluetooth::changed);
  connect(manager, &BluezQt::Manager::adapterRemoved, this, &Bluetooth::changed);
  connect(manager, &BluezQt::Manager::allAdaptersRemoved, this, &Bluetooth::changed);
  connect(manager, &BluezQt::Manager::bluetoothOperationalChanged, this, &Bluetooth::changed);
  connect(manager, &BluezQt::Manager::operationalChanged, this, &Bluetooth::changed);

  connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, &Bluetooth::changed);
}

//--------------------------------------------------------------------------------

Bluetooth::~Bluetooth()
{
  if ( job )
    job->kill();
}

//--------------------------------------------------------------------------------

void Bluetooth::changed()
{
  job = nullptr;

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

    dialog->addModule(KPluginMetaData("plasma/kcms/systemsettings/kcm_bluetooth"));

    dialog->adjustSize();
    dialog->setWindowTitle(i18n("Bluetooth"));
  }

  return dialog;
}

//--------------------------------------------------------------------------------
