#ifndef _SysTrayItem_H_
#define _SysTrayItem_H_

#include <QLabel>

class SysTrayItem : public QLabel
{
  Q_OBJECT

  public:
    SysTrayItem(QWidget *parent);

  protected slots:
    void toggleDetailsList();
    void showDetailsList();

  protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual QWidget *getDetailsList() { return nullptr; }
};

#endif
