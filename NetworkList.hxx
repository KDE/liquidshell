/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _NetworkList_H_
#define _NetworkList_H_

#include <QFrame>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>

class NetworkList : public QFrame
{
  Q_OBJECT

  public:
    NetworkList(QWidget *parent);

    QSize sizeHint() const override;

  Q_SIGNALS:
    void changed();
    void configureDialogClicked();

  private Q_SLOTS:
    void statusUpdate();
    void fillConnections();

  private:
    QToolButton *network, *wireless;
    QVBoxLayout *connectionsVbox;
    QHBoxLayout *hbox;
    QScrollArea *scroll;
};

//--------------------------------------------------------------------------------

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/AccessPoint>
#include <IconButton.hxx>

class NetworkButton : public IconButton
{
  Q_OBJECT

  public:
    NetworkButton(NetworkManager::Connection::Ptr c = NetworkManager::Connection::Ptr(),
                  NetworkManager::Device::Ptr dev = NetworkManager::Device::Ptr(),
                  NetworkManager::AccessPoint::Ptr accessPoint = NetworkManager::AccessPoint::Ptr());

    static bool compare(const NetworkButton *left, const NetworkButton *right);

  private Q_SLOTS:
    void toggleNetworkStatus(bool on);

  private:
    NetworkManager::Connection::Ptr connection;
    NetworkManager::Device::Ptr device;
    QByteArray rawSsid;
    QString ssid;
    NetworkManager::AccessPoint::WpaFlags wpaFlags;
};

#endif
