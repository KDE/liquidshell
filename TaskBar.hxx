#ifndef _TaskBar_H_
#define _TaskBar_H_

#include <QWidget>
#include <QGridLayout>

#include <kwindowinfo.h>

class TaskBar : public QWidget
{
  Q_OBJECT

  public:
    TaskBar(QWidget *parent);

  private slots:
    void fill();
    void currentDesktopChanged(int desktop);
    void windowChanged(WId wid, NET::Properties props, NET::Properties2 props2);

  private:
    QGridLayout *grid;
};

#endif
