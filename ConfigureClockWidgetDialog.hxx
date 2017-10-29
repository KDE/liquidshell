#ifndef _ConfigureClockWidgetDialog_H_
#define _ConfigureClockWidgetDialog_H_

#include <QDialog>
class QTreeWidget;

class ConfigureClockWidgetDialog : public QDialog
{
  Q_OBJECT

  public:
    ConfigureClockWidgetDialog(QWidget *parent, const QVector<QByteArray> &timeZoneIds);

    QVector<QByteArray> getSelectedTimeZoneIds() const;

  private slots:

  private:
    QTreeWidget *tree;
};

#endif
