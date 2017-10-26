#ifndef _LockLogout_H_
#define _LockLogout_H_

#include <QWidget>
#include <QToolButton>

#include <DesktopPanel.hxx>

class LockLogout : public QWidget
{
  Q_OBJECT

  public:
    LockLogout(DesktopPanel *parent);

  private slots:
    void fill(int rows);

  private:
    QToolButton *lock, *logout;
};

#endif
