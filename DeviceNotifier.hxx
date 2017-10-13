#ifndef _DeviceNotifier_H_
#define _DeviceNotifier_H_

#include <SysTrayItem.hxx>
class DeviceList;

class DeviceNotifier : public SysTrayItem
{
  Q_OBJECT

  public:
    DeviceNotifier(QWidget *parent);

  protected:
    QWidget *getDetailsList() override;

  private slots:
    void checkDeviceList();

  private:
    DeviceList *deviceList = nullptr;
};

#endif
