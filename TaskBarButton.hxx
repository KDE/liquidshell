#ifndef _TaskBarButton_H_
#define _TaskBarButton_H_

#include <QPushButton>
#include <QTimer>
#include <QElapsedTimer>
class QLabel;
class KSqueezedTextLabel;

#include <kwindowsystem.h>

class TaskBarButton : public QPushButton
{
  Q_OBJECT

  public:
    TaskBarButton(WId wid);

  signals:
    void clicked();

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

  private slots:
    void fill();
    void setBackground();
    void windowChanged(WId id, NET::Properties props, NET::Properties2 props2);

  private:
    WId wid;
    QLabel *iconLabel;
    KSqueezedTextLabel *textLabel;
    QTimer dragDropTimer;
    QPoint dragStartPos;
    QElapsedTimer dragStartTimer;
};

#endif
