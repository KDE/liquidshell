#ifndef _WindowList_H_
#define _WindowList_H_

#include <QPushButton>

class WindowList : public QPushButton
{
  Q_OBJECT

  public:
    WindowList(QWidget *parent);

  private slots:
    void fillMenu();
};

#endif
