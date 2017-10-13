#ifndef _DesktopPanel_H_
#define _DesktopPanel_H_

#include <QFrame>

class DesktopPanel : public QFrame
{
  Q_OBJECT

  public:
    DesktopPanel(QWidget *parent);

  signals:
    void resized();

  protected:
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
};

#endif
