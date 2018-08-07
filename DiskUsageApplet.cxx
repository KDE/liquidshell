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

#include <DiskUsageApplet.hxx>

#include <QLabel>
#include <QProgressBar>
#include <QGridLayout>
#include <QAction>

#include <Solid/Device>
#include <Solid/StorageVolume>
#include <Solid/StorageAccess>
#include <KDiskFreeSpaceInfo>
#include <KLocalizedString>
#include <KConfig>
#include <KConfigGroup>

#include <cmath>

//--------------------------------------------------------------------------------

DiskUsageApplet::DiskUsageApplet(QWidget *parent, const QString &theId)
  : DesktopApplet(parent, theId)
{
  setAutoFillBackground(true);

  connect(&timer, &QTimer::timeout, this, &DiskUsageApplet::fill);
  timer.setInterval(10000);
  timer.start();

  new QGridLayout(this);
  fill();

  QAction *action = new QAction(this);
  action->setText(i18n("Configure..."));
  action->setIcon(QIcon::fromTheme("configure"));
  insertAction(actions()[0], action);
  connect(action, &QAction::triggered, this, &DiskUsageApplet::configure);
}

//--------------------------------------------------------------------------------

void DiskUsageApplet::fill()
{
  QGridLayout *grid = static_cast<QGridLayout *>(layout());

  for (SizeInfo &info : partitionMap)
    info.used = false;

  QList<Solid::Device> partitions = Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume);

  int row = grid->rowCount();
  for (const Solid::Device &partition : partitions)
  {
    const Solid::StorageVolume *volume = partition.as<Solid::StorageVolume>();
    if ( !volume || volume->isIgnored() ||
         (volume->usage() != Solid::StorageVolume::FileSystem) )
      continue;

    const Solid::StorageAccess *storage = partition.as<Solid::StorageAccess>();
    if ( !storage || !storage->isAccessible() )
      continue;

    QProgressBar *progress;
    QLabel *sizeLabel;

    KDiskFreeSpaceInfo info = KDiskFreeSpaceInfo::freeSpaceInfo(storage->filePath());

    if ( !partitionMap.contains(info.mountPoint()) )
    {
      progress = new QProgressBar(this);

      QLabel *label = new QLabel(info.mountPoint(), this);
      grid->addWidget(label, row, 0);
      grid->addWidget(progress, row, 1);
      grid->addWidget(sizeLabel = new QLabel(this), row, 2);

      row++;

      SizeInfo sizeInfo;
      sizeInfo.label = label;
      sizeInfo.progress = progress;
      sizeInfo.sizeLabel = sizeLabel;
      sizeInfo.used = true;
      partitionMap.insert(storage->filePath(), sizeInfo);
    }
    else
    {
      partitionMap[info.mountPoint()].used = true;
      SizeInfo sizeInfo = partitionMap[info.mountPoint()];
      progress = sizeInfo.progress;
      sizeLabel = sizeInfo.sizeLabel;
    }

    progress->setValue(std::round(double(info.used()) / double(info.size()) * 100.0));

    sizeLabel->setText(i18n("%1 free / %2",
                            KIO::convertSize(info.available()),
                            KIO::convertSize(info.size())));
  }

  // remove entries which are no longer used
  QMutableMapIterator<QString, SizeInfo> iter(partitionMap);
  while ( iter.hasNext() )
  {
    iter.next();
    if ( !iter.value().used )
    {
      delete iter.value().label;
      delete iter.value().progress;
      delete iter.value().sizeLabel;

      iter.remove();
    }
  }
}

//--------------------------------------------------------------------------------

void DiskUsageApplet::configure()
{
  if ( dialog )
  {
    dialog->raise();
    dialog->activateWindow();
    return;
  }

  dialog = new DiskUsageAppletConfigureDialog(this);
  dialog->setWindowTitle(i18n("Configure DiskUsage Applet"));

  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();

  connect(dialog.data(), &QDialog::accepted, this, &DiskUsageApplet::saveConfig);
}

//--------------------------------------------------------------------------------
