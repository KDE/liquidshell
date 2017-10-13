#ifndef _Battery_H_
#define _Battery_H_

#include <SysTrayItem.hxx>
#include <Solid/Device>

class Battery : public SysTrayItem
{
  Q_OBJECT

  public:
    Battery(QWidget *parent);

  private slots:
    void changed();

  private:
    Solid::Device device;
};

#endif
