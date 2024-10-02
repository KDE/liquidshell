/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <DiskUsageApplet.hxx>

#include <QLabel>
#include <QProgressBar>
#include <QGridLayout>
#include <QAction>
#include <QFileInfo>
#include <QStorageInfo>
#include <QDebug>

#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/StorageVolume>
#include <Solid/StorageAccess>
#include <KLocalizedString>
#include <KConfig>
#include <KConfigGroup>
#include <KIO/Global>

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

  // beside cyclic update, react immediately when a device is added/removed
  connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded,
          this, &DiskUsageApplet::fill);

  connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved,
          this, &DiskUsageApplet::fill);
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
    if ( !storage )
      continue;

    if ( !storage->isAccessible() )
    {
      connect(storage, &Solid::StorageAccess::setupDone, this, &DiskUsageApplet::fill, Qt::UniqueConnection);
      continue;
    }

    QProgressBar *progress;
    QLabel *sizeLabel;

    QStorageInfo info = QStorageInfo(storage->filePath());

    //qDebug() << "mount" << info.rootPath() << "path" << storage->filePath()
    //         << "valid" << info.isValid() << "bytesTotal" << info.bytesTotal()
    //         << "available" << info.bytesAvailable() << "free" << info.bytesFree()
    //         << "readable" << QFileInfo(storage->filePath()).isReadable();

    if ( !info.isValid() || (info.bytesTotal() == 0) || !info.isReady() || !QFileInfo(storage->filePath()).isReadable() )
      continue;

    QString key = storage->filePath();

    if ( !partitionMap.contains(key) )
    {
      progress = new QProgressBar(this);

      QLabel *label = new QLabel(storage->filePath(), this);
      grid->addWidget(label, row, 0);
      grid->addWidget(progress, row, 1);
      grid->addWidget(sizeLabel = new QLabel(this), row, 2);

      row++;

      SizeInfo sizeInfo;
      sizeInfo.label = label;
      sizeInfo.progress = progress;
      sizeInfo.sizeLabel = sizeLabel;
      sizeInfo.used = true;
      partitionMap.insert(key, sizeInfo);

      // workaround Qt bug
      label->setPalette(palette());
      sizeLabel->setPalette(palette());
      progress->setPalette(palette());

      connect(storage, &Solid::StorageAccess::teardownDone, this, &DiskUsageApplet::fill, Qt::UniqueConnection);
    }
    else
    {
      partitionMap[key].used = true;
      SizeInfo sizeInfo = partitionMap[key];
      progress = sizeInfo.progress;
      sizeLabel = sizeInfo.sizeLabel;
    }

    progress->setValue(std::round(double(info.bytesTotal() - info.bytesAvailable()) / double(info.bytesTotal()) * 100.0));

    sizeLabel->setText(i18n("%1 free / %2",
                            KIO::convertSize(info.bytesAvailable()),
                            KIO::convertSize(info.bytesTotal())));
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

void DiskUsageApplet::loadConfig()
{
  DesktopApplet::loadConfig();

  KConfig config;
  KConfigGroup group = config.group(id);

  QColor barTextCol = group.readEntry("barTextCol", palette().color(QPalette::HighlightedText));
  QColor barBackCol = group.readEntry("barBackCol", palette().color(QPalette::Highlight));

  QPalette pal = palette();
  pal.setColor(QPalette::HighlightedText, barTextCol);
  pal.setColor(QPalette::Highlight, barBackCol);
  setPalette(pal);
}

//--------------------------------------------------------------------------------

void DiskUsageApplet::saveConfig()
{
  DesktopApplet::saveConfig();

  KConfig config;
  KConfigGroup group = config.group(id);
  group.writeEntry("barTextCol", palette().color(QPalette::HighlightedText));
  group.writeEntry("barBackCol", palette().color(QPalette::Highlight));
}

//--------------------------------------------------------------------------------
