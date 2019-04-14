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

#ifndef _NetworkList_H_
#define _NetworkList_H_

#include <QFrame>
#include <QToolButton>
#include <QVBoxLayout>

class NetworkList : public QFrame
{
  Q_OBJECT

  public:
    NetworkList(QWidget *parent);

  Q_SIGNALS:
    void changed();

  private Q_SLOTS:
    void statusUpdate();
    void fillConnections();
    void openConfigureDialog();

  private:
    QToolButton *network, *wireless;
    QVBoxLayout *connectionsVbox;
};

//--------------------------------------------------------------------------------

#include <NetworkManagerQt/Manager>
#include <IconButton.hxx>

class NetworkButton : public IconButton
{
  Q_OBJECT

  public:
    NetworkButton(NetworkManager::Connection::Ptr c = NetworkManager::Connection::Ptr(),
                  NetworkManager::Device::Ptr dev = NetworkManager::Device::Ptr());

  private Q_SLOTS:
    void toggleNetworkStatus(bool on);

  private:
    NetworkManager::Connection::Ptr connection;
    NetworkManager::Device::Ptr device;
};

#endif
