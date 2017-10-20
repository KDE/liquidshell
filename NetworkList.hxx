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

  signals:
    void changed();

  private slots:
    void statusUpdate();
    void fillConnections();

  private:
    QToolButton *network, *wireless;
    QVBoxLayout *connectionsVbox;
};

//--------------------------------------------------------------------------------

#include <NetworkManagerQt/Manager>

class NetworkButton : public QToolButton
{
  Q_OBJECT

  public:
    NetworkButton(NetworkManager::Connection::Ptr c = nullptr, NetworkManager::Device::Ptr dev = nullptr);

  private slots:
    void toggleNetworkStatus(bool on);

  private:
    NetworkManager::Connection::Ptr connection;
    NetworkManager::Device::Ptr device;
};

#endif
