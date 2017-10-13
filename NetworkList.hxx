#ifndef _NetworkList_H_
#define _NetworkList_H_

#include <QFrame>
#include <QToolButton>

class NetworkList : public QFrame
{
  Q_OBJECT

  public:
    NetworkList(QWidget *parent);

  private slots:
    void statusUpdate();

  private:
    QToolButton *network, *wireless;
};

//--------------------------------------------------------------------------------

class NetworkButton : public QToolButton
{
  Q_OBJECT

  public:
    NetworkButton();
};

#endif
