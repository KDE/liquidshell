// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 Martin Koller, kollix@aon.at

  This file is part of liquidshell.

  liquidshell is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  liquidshell is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with liquidshell.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Bluetooth.hxx>

#include <QIcon>

#include <BluezQt/Adapter>
#include <BluezQt/InitManagerJob>

#include <KLocalizedString>
#include <KIconLoader>

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
    dialog->addModule("bluedevilglobal");
    dialog->addModule("bluedeviladapters");
    dialog->addModule("bluedevildevices");
    dialog->adjustSize();
    dialog->setWindowTitle(i18n("Bluetooth"));
  }

  return dialog;
}

//--------------------------------------------------------------------------------
