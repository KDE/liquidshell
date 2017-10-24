#ifndef _SysTrayNotifyItem_H_
#define _SysTrayNotifyItem_H_

#include <QLabel>
#include <QTimer>
class OrgKdeStatusNotifierItem;
class QDBusPendingCallWatcher;

class SysTrayNotifyItem : public QLabel
{
  Q_OBJECT

  public:
    SysTrayNotifyItem(QWidget *parent, const QString &service, const QString &path);

  signals:
    void initialized(SysTrayNotifyItem *);

  protected:
    void wheelEvent(QWheelEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private slots:
    void startTimer();
    void fetchData();
    void fetchDataReply(QDBusPendingCallWatcher *w);

  private:
    QPixmap applyOverlay(const QPixmap &pixmap, const QPixmap &overlay);

  private:
    QTimer fetchTimer;
    OrgKdeStatusNotifierItem *dbus;
};

#endif
