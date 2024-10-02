/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _Battery_H_
#define _Battery_H_

#include <SysTrayItem.hxx>
#include <Solid/Device>
#include <QPointer>
#include <KCMultiDialog>
class QDBusMessage;

class Battery : public SysTrayItem
{
  Q_OBJECT

  public:
    Battery(QWidget *parent);

    static QIcon getStatusIcon(int charge, bool isCharging);

  protected:
    QWidget *getDetailsList() override;

  private Q_SLOTS:
    void onBatteryReply(const QDBusMessage &msg);
    void upowerPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidated);
    void changed();

  private:
    QString secsToHM(int secs) const;

  private:
    Solid::Device device;
    bool onBattery = false;
    QPointer<KCMultiDialog> dialog;
};

#endif
