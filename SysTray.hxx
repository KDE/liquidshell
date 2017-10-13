#ifndef _SysTray_H_
#define _SysTray_H_

#include <QFrame>
#include <QMap>
#include <QVector>
#include <QPointer>
class QHBoxLayout;

#include <SysTrayNotifyItem.hxx>

class SysTray : public QFrame
{
  Q_OBJECT

  public:
    SysTray(QWidget *parent);

  private slots:
    void itemRegistered(QString service);
    void itemUnregistered(QString service);
    void itemInitialized(SysTrayNotifyItem *item);

  private:
    void registerWatcher();

  private:
    QVector<QHBoxLayout *> appsRows;
    QString serviceName;
    QMap<QString, QPointer<SysTrayNotifyItem>> items;
};

#endif
