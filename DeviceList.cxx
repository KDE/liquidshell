// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 - 2020 Martin Koller, kollix@aon.at

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

#include <DeviceList.hxx>
#include <IconButton.hxx>
#include <Battery.hxx>

#include <Solid/DeviceNotifier>
#include <Solid/DeviceInterface>
#include <Solid/StorageAccess>
#include <Solid/StorageVolume>
#include <Solid/StorageDrive>
#include <Solid/Block>
#include <Solid/Battery>

#include <QHBoxLayout>
#include <QIcon>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QScrollBar>
#include <QDebug>

#include <KLocalizedString>
#include <KDesktopFile>
#include <KDesktopFileActions>
#include <KConfigGroup>
#include <KService>
#include <KRun>
#include <kio/global.h>

//--------------------------------------------------------------------------------

DeviceItem::DeviceItem(Solid::Device dev, const QVector<DeviceAction> &deviceActions)
  : device(dev)
{
  setFrameShape(QFrame::StyledPanel);

  QVBoxLayout *vbox = new QVBoxLayout(this);
  QHBoxLayout *hbox = new QHBoxLayout;
  vbox->addLayout(hbox);

  QLabel *iconLabel = new QLabel;
  iconLabel->setPixmap(QIcon::fromTheme(device.icon()).pixmap(32));
  iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  newFlagLabel = new QLabel;
  newFlagLabel->setPixmap(QIcon::fromTheme("emblem-important").pixmap(16));
  newFlagLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  newFlagLabel->hide();

  textLabel = new QLabel;
  textLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
  textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

  hbox->addWidget(iconLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
  hbox->addWidget(newFlagLabel, 0, Qt::AlignCenter);
  hbox->addWidget(textLabel, 0, Qt::AlignVCenter);

  Solid::StorageAccess *storage = device.as<Solid::StorageAccess>();

  if ( storage )
  {
    connect(storage, &Solid::StorageAccess::teardownDone, this, &DeviceItem::teardownDone);
    connect(storage, &Solid::StorageAccess::setupDone, this, &DeviceItem::setupDone);

    mountBusyTimer.setInterval(500);
    connect(&mountBusyTimer, &QTimer::timeout, this,
            [this]() { mountButton->setVisible(!mountButton->isVisible()); });

    mountButton = new QToolButton;
    mountButton->setIconSize(QSize(32, 32));

    connect(mountButton, &QToolButton::clicked, mountButton,
            [this]()
            {
              statusLabel->hide();
              mountButton->setEnabled(false);
              mountBusyTimer.start();

              Solid::StorageAccess *storage = device.as<Solid::StorageAccess>();

              if ( storage->isAccessible() )  // mounted -> unmount it
                storage->teardown();
              else
                storage->setup();
            }
           );


    hbox->addWidget(mountButton);
  }

  statusLabel = new QLabel;

  if ( device.is<Solid::Battery>() )
  {
    hbox->addWidget(statusLabel, 0, Qt::AlignVCenter);

    QLabel *chargeIcon = new QLabel;
    hbox->addWidget(chargeIcon, 0, Qt::AlignVCenter);

    Solid::Battery *battery = device.as<Solid::Battery>();

    if ( battery->chargePercent() >= 0 )
    {
      statusLabel->setText(QString::number(battery->chargePercent()) + '%');
      chargeIcon->setPixmap(Battery::getStatusIcon(battery->chargePercent(),
                            battery->chargeState() == Solid::Battery::Charging).pixmap(22));
    }

    connect(battery, &Solid::Battery::chargePercentChanged, this,
            [this, chargeIcon, battery]()
            {
              statusLabel->setText(QString::number(battery->chargePercent()) + '%');
              chargeIcon->setPixmap(Battery::getStatusIcon(battery->chargePercent(),
                                    battery->chargeState() == Solid::Battery::Charging).pixmap(22));
            });
  }
  else
  {
    statusLabel->setWordWrap(true);
    statusLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    statusLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    vbox->addWidget(statusLabel);
    statusLabel->hide();

    statusTimer.setSingleShot(true);
    statusTimer.setInterval(60000);
    connect(&statusTimer, &QTimer::timeout, statusLabel, &QLabel::hide);
  }

  fillData();

  // append actions
  for (const DeviceAction &action : deviceActions)
  {
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addSpacing(iconLabel->sizeHint().width());

    IconButton *button = new IconButton;
    button->setIcon(QIcon::fromTheme(action.action.icon()));
    button->setText(action.action.text() + " (" + QFileInfo(action.path).baseName() + ")");

    connect(button, &IconButton::clicked, button,
            [action, this]()
            {
              QString command = action.action.exec();

              if ( device.is<Solid::Block>() )
                command.replace("%d", device.as<Solid::Block>()->device());

              command.replace("%i", device.udi());

              Solid::StorageAccess *storage = device.as<Solid::StorageAccess>();
              if ( storage )
              {
                if ( !storage->isAccessible() )
                {
                  statusLabel->hide();
                  storage->setup();
                  pendingCommand = command;
                  return;
                }

                command.replace("%f", storage->filePath());
              }

              KRun::runCommand(command, this);
              window()->hide();
            }
           );

    hbox->addWidget(button);

    vbox->addLayout(hbox);
  }
}

//--------------------------------------------------------------------------------

DeviceItem::DeviceItem(const KdeConnect::Device &dev)
{
  setFrameShape(QFrame::StyledPanel);

  QVBoxLayout *vbox = new QVBoxLayout(this);
  QHBoxLayout *hbox = new QHBoxLayout;
  vbox->addLayout(hbox);

  QLabel *iconLabel = new QLabel;
  iconLabel->setPixmap(dev->icon.pixmap(32));
  iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  newFlagLabel = new QLabel;
  newFlagLabel->setPixmap(QIcon::fromTheme("emblem-important").pixmap(16));
  newFlagLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  newFlagLabel->hide();

  textLabel = new QLabel;
  textLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
  textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

  hbox->addWidget(iconLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
  hbox->addWidget(newFlagLabel, 0, Qt::AlignCenter);
  hbox->addWidget(textLabel, 0, Qt::AlignVCenter);

  statusLabel = new QLabel;
  hbox->addWidget(statusLabel, 0, Qt::AlignVCenter);

  QLabel *chargeIcon = new QLabel;
  hbox->addWidget(chargeIcon, 0, Qt::AlignVCenter);

  if ( dev->charge >= 0 )
  {
    statusLabel->setText(QString::number(dev->charge) + '%');
    chargeIcon->setPixmap(dev->chargeIcon.pixmap(22));
  }

  connect(dev.data(), &KdeConnectDevice::changed, this,
          [this, chargeIcon, dev]()
          {
            textLabel->setText(dev->name);

            if ( dev->charge >= 0 )
            {
              statusLabel->setText(QString::number(dev->charge) + '%');
              chargeIcon->setPixmap(dev->chargeIcon.pixmap(22));
            }
          });

  if ( dev->plugins.contains("kdeconnect_findmyphone") )
  {
    QToolButton *ringButton = new QToolButton;
    ringButton->setIcon(QIcon::fromTheme("preferences-desktop-notification-bell"));
    connect(ringButton, &QToolButton::clicked, dev.data(), [dev]() { dev->ringPhone(); });
    hbox->addWidget(ringButton, 0, Qt::AlignVCenter);
  }

  QToolButton *configure = new QToolButton;
  configure->setIcon(QIcon::fromTheme("configure"));
  connect(configure, &QToolButton::clicked, configure,
          [this]()
          {
            if ( !dialog )
            {
              dialog = new KCMultiDialog(nullptr);
              dialog->setAttribute(Qt::WA_DeleteOnClose);
              dialog->addModule("kcm_kdeconnect");
              dialog->setWindowTitle(i18n("KDE Connect"));
              dialog->adjustSize();
            }
            dialog->show();
          });

  hbox->addWidget(configure, 0, Qt::AlignVCenter);

  textLabel->setText(dev->name);

  if ( dev->plugins.contains("kdeconnect_sftp") )
  {
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addSpacing(iconLabel->sizeHint().width());

    IconButton *button = new IconButton;
    button->setIcon(QIcon::fromTheme("system-file-manager"));
    button->setText(i18n("Open with File Manager"));

    connect(button, &IconButton::clicked,
            [dev]() { new KRun(QUrl(QLatin1String("kdeconnect://") + dev->id), nullptr); });

    hbox->addWidget(button);

    vbox->addLayout(hbox);
  }
}

//--------------------------------------------------------------------------------

void DeviceItem::markAsNew()
{
  newFlagLabel->show();
  QTimer::singleShot(5000, newFlagLabel, &QLabel::hide);
}

//--------------------------------------------------------------------------------

void DeviceItem::fillData()
{
  Solid::StorageAccess *storage = device.as<Solid::StorageAccess>();
  QString text = device.description();

  if ( !device.product().isEmpty() && (device.product() != text) )
    text += " (" + device.product() + ")";
  else if ( !device.vendor().isEmpty() && (device.vendor() != text) )
    text += " (" + device.vendor() + ")";

  Solid::StorageVolume *volume = device.as<Solid::StorageVolume>();
  if ( volume )
    text += " " + KIO::convertSize(volume->size());

  if ( storage && !storage->filePath().isEmpty() )
    text += '\n' + storage->filePath();

  textLabel->setText(text);

  if ( mountButton )
  {
    if ( storage && storage->isAccessible() )
      mountButton->setIcon(QIcon::fromTheme("media-eject"));
    else
    {
      if ( device.emblems().count() )
        mountButton->setIcon(QIcon::fromTheme(device.emblems()[0]));
      else
        mountButton->setIcon(QIcon::fromTheme("emblem-unmounted"));
    }

    if ( storage )
    {
      mountButton->setToolTip(storage->isAccessible() ?
                              i18n("Device is mounted.\nClick to unmount/eject") :
                              i18n("Device is unmounted.\nClick to mount"));
    }
  }
}

//--------------------------------------------------------------------------------

QString DeviceItem::errorToString(Solid::ErrorType error)
{
  switch ( error )
  {
    case Solid::UnauthorizedOperation: return i18n("Unauthorized Operation");
    case Solid::DeviceBusy: return i18n("Device Busy");
    case Solid::OperationFailed: return i18n("Operation Failed");
    case Solid::UserCanceled: return i18n("User Canceled");
    case Solid::InvalidOption: return i18n("Invalid Option");
    case Solid::MissingDriver: return i18n("Missing Driver");

    default: return QString();
  }
}

//--------------------------------------------------------------------------------

void DeviceItem::mountDone(Action action, Solid::ErrorType error, QVariant errorData, const QString &udi)
{
  Q_UNUSED(udi)

  mountBusyTimer.stop();
  mountButton->setEnabled(true);
  mountButton->setVisible(true);

  if ( error == Solid::NoError )
  {
    fillData();

    if ( action == Unmount )
    {
      statusLabel->setText(i18n("The device can now be safely removed"));
      statusLabel->show();
      statusTimer.start();
    }
  }
  else
  {
    QString text = (action == Mount) ? i18n("Mount failed:") : i18n("Unmount failed:");
    statusLabel->setText("<b>" + text + "</b>" + errorToString(error) + "<br>" + errorData.toString());
    statusLabel->show();
    statusTimer.start();
  }
}

//--------------------------------------------------------------------------------

void DeviceItem::teardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi)
{
  mountDone(Unmount, error, errorData, udi);
}

//--------------------------------------------------------------------------------

void DeviceItem::setupDone(Solid::ErrorType error, QVariant errorData, const QString &udi)
{
  mountDone(Mount, error, errorData, udi);

  if ( !pendingCommand.isEmpty() )
  {
    if ( error == Solid::NoError )
    {
      Solid::StorageAccess *storage = device.as<Solid::StorageAccess>();
      if ( storage )  // should always be true. paranoid check
      {
        pendingCommand.replace("%f", storage->filePath());
        KRun::runCommand(pendingCommand, this);
        window()->hide();
      }
    }
    pendingCommand.clear();
  }
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

DeviceList::DeviceList(QWidget *parent)
  : QScrollArea(parent)
{
  setWindowFlags(windowFlags() | Qt::Tool);
  setFrameShape(QFrame::StyledPanel);
  setAttribute(Qt::WA_AlwaysShowToolTips);
  setWidgetResizable(true);

  loadActions();

  QWidget *widget = new QWidget;

  vbox = new QVBoxLayout(widget);
  vbox->setContentsMargins(QMargins());
  vbox->addStretch();
  vbox->setSizeConstraint(QLayout::SetMinAndMaxSize);
  setWidget(widget);

  predicate = Solid::Predicate(Solid::DeviceInterface::StorageAccess);
  predicate |= Solid::Predicate(Solid::DeviceInterface::StorageDrive);
  predicate |= Solid::Predicate(Solid::DeviceInterface::StorageVolume);
  predicate |= Solid::Predicate(Solid::DeviceInterface::OpticalDrive);
  predicate |= Solid::Predicate(Solid::DeviceInterface::OpticalDisc);
  predicate |= Solid::Predicate(Solid::DeviceInterface::PortableMediaPlayer);
  predicate |= Solid::Predicate(Solid::DeviceInterface::Camera);
  predicate |= Solid::Predicate(Solid::DeviceInterface::Battery);  // non-primary batteries, e.g. mouse

  QList<Solid::Device> devices = Solid::Device::listFromQuery(predicate);

  for (Solid::Device device : devices)
    addDevice(device);

  connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded,
          this, &DeviceList::deviceAdded);

  connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved,
          this, &DeviceList::deviceRemoved);

  connect(&kdeConnect, &KdeConnect::deviceAdded, this, &DeviceList::kdeConnectDeviceAdded);
  connect(&kdeConnect, &KdeConnect::deviceRemoved, this, &DeviceList::deviceRemoved);
}

//--------------------------------------------------------------------------------

QSize DeviceList::sizeHint() const
{
  // avoid horizontal scrollbar when the list is higher than 2/3 of the screen
  // where a vertical scrollbar will be shown, reducing the available width
  // leading to also getting a horizontal scrollbar
  QSize s = widget()->sizeHint() + QSize(2 * frameWidth(), 2 * frameWidth());
  s.setWidth(s.width() + verticalScrollBar()->sizeHint().width());
  return s;
}

//--------------------------------------------------------------------------------

void DeviceList::loadActions()
{
  actions.clear();

  const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "solid/actions", QStandardPaths::LocateDirectory);
  for (const QString &dirPath : dirs)
  {
    QDir dir(dirPath);

    for (const QString &file : dir.entryList(QStringList(QLatin1String("*.desktop")), QDir::Files))
    {
      QString path = dir.absoluteFilePath(file);
      KDesktopFile cfg(path);
      const QString predicateString = cfg.desktopGroup().readEntry("X-KDE-Solid-Predicate");

      QList<KServiceAction> actionList = KDesktopFileActions::userDefinedServices(path, true);

      if ( !actionList.isEmpty() && !predicateString.isEmpty() )
        actions.append(DeviceAction(path, Solid::Predicate::fromString(predicateString), actionList[0]));
    }
  }
}

//--------------------------------------------------------------------------------

DeviceItem *DeviceList::addDevice(Solid::Device device)
{
  if ( items.contains(device.udi()) )
  {
    //qDebug() << device.udi() << "already known";
    return nullptr;
  }

  QVector<DeviceAction> deviceActions;

  for (const DeviceAction &action : actions)
  {
    if ( action.predicate.matches(device) )
      deviceActions.append(action);
  }

  if ( device.is<Solid::Battery>() && (device.as<Solid::Battery>()->type() != Solid::Battery::PrimaryBattery) )
  {
    // show also non-primary batteries
  }
  else
  {
    Solid::StorageVolume *volume = device.as<Solid::StorageVolume>();
    Solid::StorageAccess *access = device.as<Solid::StorageAccess>();

    if ( !volume )  // volume can at least be mounted; others need some specific actions
    {
      if ( deviceActions.isEmpty() )
      {
        //qDebug() << device.udi() << "no action found";
        return nullptr;
      }
      if ( access && access->isAccessible() && !QFileInfo(access->filePath()).isReadable() )
      {
        //qDebug() << device.udi() << access->filePath() << "not readable";
        return nullptr;
      }
    }
    else if ( (volume->usage() != Solid::StorageVolume::FileSystem) || volume->isIgnored() )
    {
      //qDebug() << device.udi() << (access ? access->filePath() : QString()) << "no filesystem or ignored";
      return nullptr;
    }

    // show only removable devices
    bool isRemovable = (device.is<Solid::StorageDrive>() &&
                        device.as<Solid::StorageDrive>()->isRemovable()) ||
                       (device.parent().is<Solid::StorageDrive>() &&
                        device.parent().as<Solid::StorageDrive>()->isRemovable());

    if ( !isRemovable )
    {
      //qDebug() << device.udi() << "not Removable";
      return nullptr;
    }
  }

  DeviceItem *item = new DeviceItem(device, deviceActions);
  vbox->insertWidget(vbox->count() - 1, item);  // insert before stretch

  items.insert(device.udi(), item);
  return item;
}

//--------------------------------------------------------------------------------

void DeviceList::deviceAdded(const QString &dev)
{
  Solid::Device device(dev);

  if ( !predicate.matches(device) )
    return;

  DeviceItem *item = addDevice(device);

  // when we added a new device, make sure the DeviceNotifier shows and places this window
  if ( item )
  {
    item->markAsNew();
    item->show();
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());

    emit deviceWasAdded();
  }
}

//--------------------------------------------------------------------------------

void DeviceList::deviceRemoved(const QString &dev)
{
  if ( items.contains(dev) )
  {
    delete items.take(dev);
    emit deviceWasRemoved();
  }
}

//--------------------------------------------------------------------------------

void DeviceList::kdeConnectDeviceAdded(const KdeConnect::Device &device)
{
  if ( items.contains(device->id) )
  {
    //qDebug() << device->id << "already known";
    return;
  }

  DeviceItem *item = new DeviceItem(device);
  vbox->insertWidget(vbox->count() - 1, item);  // insert before stretch

  items.insert(device->id, item);
  item->markAsNew();
  item->show();
  verticalScrollBar()->setValue(verticalScrollBar()->maximum());

  // when we added a new device, make sure the DeviceNotifier shows and places this window
  emit deviceWasAdded();
}

//--------------------------------------------------------------------------------
