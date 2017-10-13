#ifndef _NotificationServer_H_
#define _NotificationServer_H_

// https://developer.gnome.org/notification-spec/

#include <SysTrayItem.hxx>
class NotificationList;

class NotificationServer : public SysTrayItem
{
  Q_OBJECT

  public:
    NotificationServer(QWidget *parent);

    void CloseNotification(uint id);

    QStringList GetCapabilities();

    QString GetServerInformation(QString &vendor, QString &version, QString &spec_version);

    uint Notify(const QString &app_name, uint replaces_id, const QString &app_icon,
                const QString &summary, const QString &body, const QStringList &actions,
                const QVariantMap &hints, int timeout);

  protected:
    QWidget *getDetailsList() override;

  signals:
    void ActionInvoked(uint id, const QString &action_key);
    void NotificationClosed(uint id, uint reason);

  private:
    uint notifyId = 1;
    NotificationList *notificationList;
};

#endif
