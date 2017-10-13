#ifndef _Pager_H_
#define _Pager_H_

#include <QWidget>
class QButtonGroup;
class PagerButton;

class Pager : public QWidget
{
  Q_OBJECT

  public:
    Pager(QWidget *parent);

  private slots:
    void fill();
    void changeDesktop(bool checked);

  private:
    QButtonGroup *group;
    QVector<PagerButton *> buttons;
};

#endif
