#ifndef _DiskUsageApplet_H_
#define _DiskUsageApplet_H_

#include <DesktopApplet.hxx>
#include <DiskUsageAppletConfigureDialog.hxx>
#include <QTimer>
#include <QMap>
#include <QPointer>
class QProgressBar;
class QLabel;

class DiskUsageApplet : public DesktopApplet
{
  Q_OBJECT

  public:
    DiskUsageApplet(QWidget *parent, const QString &theId);

  private Q_SLOTS:
    void fill();
    void configure();

  private:
    QTimer timer;

    struct SizeInfo
    {
      QLabel *label;
      QProgressBar *progress;
      QLabel *sizeLabel;
      bool used;
    };

    QMap<QString, SizeInfo> partitionMap;
    QPointer<DiskUsageAppletConfigureDialog> dialog;
};

#endif
