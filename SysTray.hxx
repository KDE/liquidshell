#ifndef _SysTray_H_
#define _SysTray_H_

#include <QFrame>
#include <QMap>
#include <QVector>
#include <QPointer>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <DesktopPanel.hxx>
#include <SysTrayNotifyItem.hxx>

class SysTray : public QFrame
{
  Q_OBJECT

  public:
    SysTray(DesktopPanel *parent);

  private slots:
    void fill();
    void itemRegistered(QString service);
    void itemUnregistered(QString service);
    void itemInitialized(SysTrayNotifyItem *item);

  private:
    void registerWatcher();

  private:
    QVBoxLayout *vbox, *appsVbox;
    QVector<QHBoxLayout *> appsRows;
    QString serviceName;
    QMap<QString, QPointer<SysTrayNotifyItem>> items;
};

#endif
