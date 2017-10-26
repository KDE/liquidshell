#ifndef _DesktopPanel_H_
#define _DesktopPanel_H_

#include <QFrame>

class DesktopPanel : public QFrame
{
  Q_OBJECT

  public:
    DesktopPanel(QWidget *parent);

    int getRows() const { return rows; }

  signals:
    void resized();
    void rowsChanged(int rows);

  protected:
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private slots:
    void updateRowCount();

  private:
    int rows = 1;
};

#endif
