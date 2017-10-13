#ifndef _Bluetooth_H_
#define _Bluetooth_H_

#include <SysTrayItem.hxx>

#include <BluezQt/Manager>
#include <QPointer>
#include <KCMultiDialog>

class Bluetooth : public SysTrayItem
{
  Q_OBJECT

  public:
    Bluetooth(QWidget *parent);

  protected:
    QWidget *getDetailsList() override;

  private slots:
    void changed();

  private:
    BluezQt::Manager *manager;
    QPointer<KCMultiDialog> dialog = nullptr;
};

#endif
