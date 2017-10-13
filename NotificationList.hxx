#ifndef _NotificationList_H_
#define _NotificationList_H_

#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QIcon>
#include <QMap>
class QVBoxLayout;

class NotifyItem : public QFrame
{
  Q_OBJECT

  public:
    NotifyItem(QWidget *parent, uint theid, const QString &app,
               const QString &summary, const QString &body, const QIcon &icon);

  private:
    QLabel *timeLabel, *iconLabel, *textLabel;
    uint id;
    QString appName;
};

//--------------------------------------------------------------------------------

class NotificationList : public QScrollArea
{
  Q_OBJECT

  public:
    NotificationList(QWidget *parent);

    void addItem(uint id, const QString &appName, const QString &summary, const QString &body, const QIcon &icon);

    int itemCount() const { return numItems; }

  signals:
    void itemsCountChanged();
    void listNowEmpty();

  private:
    QVBoxLayout *listVbox;
    QMap<QString, int> appTimeouts;  // appName, timeout (minutes)
    int numItems = 0;
};

#endif
