#ifndef _SysLoad_H_
#define _SysLoad_H_

#include <QFrame>
#include <QVector>
#include <QMap>
#include <QByteArray>
#include <QTimer>

class SysLoad : public QFrame
{
  Q_OBJECT

  public:
    SysLoad(QWidget *parent);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

  private slots:
    void fetch();

  private:
    struct CpuData
    {
      int userCPU = 0, niceCPU = 0, systemCPU = 0;
      double userPercent = 0, nicePercent = 0, systemPercent = 0;
      double MHz = 0;
    };

    struct MemoryData
    {
      double memPercent = 0;
      double memCachedPercent = 0;
      double swapPercent = 0;
    } memData;

    struct NetworkData
    {
      size_t prevReceived = 0, prevSent = 0;
      size_t received = 0, sent = 0;
      bool valid = false;
    };

    QVector<CpuData> cpus;

    QMap<QByteArray, NetworkData> netDevs;
    size_t maxBytes = 100;
    size_t sumSent = 0, sumReceived = 0;

    QTimer timeoutTimer;
};

#endif
