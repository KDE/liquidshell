#ifndef _PagerButton_H_
#define _PagerButton_H_

#include <QPushButton>
#include <QTimer>

#include <kwindowsystem.h>

class PagerButton : public QPushButton
{
  Q_OBJECT

  public:
    PagerButton(int num);

    int getDesktop() const { return desktop; }

  protected:
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

  private slots:
    void createPixmap();
    void windowChanged(WId id, NET::Properties props, NET::Properties2 props2);

  private:
    int desktop;
    QPixmap firstPixmap;
    QTimer dragDropTimer;
};

#endif
