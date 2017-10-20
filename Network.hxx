#ifndef _Network_H_
#define _Network_H_

// network management (status display, activate, switch, ...)

#include <SysTrayItem.hxx>
#include <NetworkList.hxx>
#include <QTimer>
#include <QPixmap>

class Network : public SysTrayItem
{
  Q_OBJECT

  public:
    Network(QWidget *parent);

  protected:
    QWidget *getDetailsList() override;

  private slots:
    void checkState();

  private:
    QTimer blinkTimer;
    bool blinkState = false;
    QPixmap origPixmap;
    QPointer<NetworkList> networkList;
};

#endif
