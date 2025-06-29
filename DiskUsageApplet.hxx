/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _DiskUsageApplet_H_
#define _DiskUsageApplet_H_

#include <DesktopApplet.hxx>
#include <DiskUsageAppletConfigureDialog.hxx>
#include <QTimer>
#include <QMap>
#include <QPointer>
class QProgressBar;
class QLabel;

class DiskUsageApplet : public DesktopApplet
{
  Q_OBJECT

  public:
    DiskUsageApplet(QWidget *parent, const QString &theId);

    void loadConfig() override;

  public Q_SLOTS:
    void configure() override;
    void saveConfig() override;

  private Q_SLOTS:
    void fill();

  private:
    QTimer timer;

    struct SizeInfo
    {
      QLabel *label;
      QProgressBar *progress;
      QLabel *sizeLabel;
      bool used;
    };

    QMap<QString, SizeInfo> partitionMap;
    QPointer<DiskUsageAppletConfigureDialog> dialog;
};

#endif
