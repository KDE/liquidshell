#include <SysLoad.hxx>

#include <QFile>
#include <QPainter>
#include <QToolTip>
#include <QMouseEvent>
#include <QDebug>

#include <KRun>
#include <KLocalizedString>

const int INTERVAL = 800; // TODO sometimes creates > 100% CPU. check what is defined for /proc/stat

//--------------------------------------------------------------------------------

SysLoad::SysLoad(QWidget *parent)
  : QFrame(parent)
{
  setFrameShape(QFrame::StyledPanel);

  timeoutTimer.setInterval(INTERVAL);
  timeoutTimer.start();
  connect(&timeoutTimer, &QTimer::timeout, this, &SysLoad::fetch);

  setFixedWidth(60);
}

//--------------------------------------------------------------------------------

void SysLoad::fetch()
{
  QFile f("/proc/stat");
  if ( !f.open(QIODevice::ReadOnly) )
    return;

  int num = 0;
  bool first = cpus.isEmpty();
  while ( true )
  {
    QByteArray line = f.readLine();
    if ( line.isEmpty() )
      break;

    if ( line.startsWith("cpu") && !line.startsWith("cpu ") )
    {
      CpuData data;
      sscanf(line.constData(), "%*s %d %d %d", &data.userCPU, &data.niceCPU, &data.systemCPU);

      if ( first )
        cpus.append(data);
      else
      {
        cpus[num].userPercent = (data.userCPU - cpus[num].userCPU) / (INTERVAL / 1000.0);
        cpus[num].nicePercent = (data.niceCPU - cpus[num].niceCPU) / (INTERVAL / 1000.0);
        cpus[num].systemPercent = (data.systemCPU - cpus[num].systemCPU) / (INTERVAL / 1000.0);
        cpus[num].userCPU = data.userCPU;
        cpus[num].niceCPU = data.niceCPU;
        cpus[num].systemCPU = data.systemCPU;
        num++;
      }
    }
  }
  f.close();

  // get memory information
  size_t memTotal = 0, memFree = 0, swapTotal = 0, swapFree = 0, cached = 0;
  f.setFileName("/proc/meminfo");
  if ( f.open(QIODevice::ReadOnly) )
  {
    while ( true )
    {
      QByteArray line = f.readLine();
      if ( line.isEmpty() )
        break;

      if (      line.startsWith("MemTotal:")  ) sscanf(line, "%*s %zd kB", &memTotal);
      else if ( line.startsWith("MemFree:")   ) sscanf(line, "%*s %zd kB", &memFree);
      else if ( line.startsWith("SwapTotal:") ) sscanf(line, "%*s %zd kB", &swapTotal);
      else if ( line.startsWith("SwapFree:")  ) sscanf(line, "%*s %zd kB", &swapFree);
      else if ( line.startsWith("Cached:")    ) sscanf(line, "%*s %zd kB", &cached);
    }
    memData.memPercent = memTotal ? (double(memTotal - memFree) / double(memTotal)) * 100.0 : 0.0;
    memData.memCachedPercent = memTotal ? (double(cached) / double(memTotal)) * 100.0 : 0.0;
    memData.swapPercent = swapTotal ? (double(swapTotal - swapFree) / double(swapTotal)) * 100.0 : 0.0;
    f.close();
  }

  f.setFileName("/proc/cpuinfo");  // get speed of cores
  if ( f.open(QIODevice::ReadOnly) )
  {
    int num = 0;
    while ( true )
    {
      QByteArray line = f.readLine();
      if ( line.isEmpty() )
        break;

      if ( line.startsWith("cpu MHz")  )
      {
        if ( num < cpus.count() )
          sscanf(line, "cpu MHz %*s %lf", &cpus[num++].MHz);
      }
    }
    f.close();
  }

  f.setFileName("/proc/net/dev");
  sumSent = sumReceived = 0;
  if ( f.open(QIODevice::ReadOnly) )
  {
    while ( true )
    {
      QByteArray line = f.readLine();
      if ( line.isEmpty() )
        break;

      int colon = line.indexOf(':');
      if ( colon > 1 )
      {
        QByteArray device = line.left(colon).trimmed();
        size_t received = 0, sent = 0;
        sscanf(line.data() + colon + 1, "%zd %*d %*d %*d %*d %*d %*d %*d %zd", &received, &sent);
        if ( !netDevs.contains(device) )
        {
          NetworkData data;
          data.prevReceived = received;
          data.prevSent = sent;
          data.valid = false;  // first scan not valid since we count differences
          netDevs.insert(device, data);
        }
        else
        {
          NetworkData &data = netDevs[device];
          data.received = received - data.prevReceived;
          data.sent = sent - data.prevSent;
          data.prevReceived = received;
          data.prevSent = sent;
          data.valid = true;

          if ( !device.startsWith("tun") ) // TODO: correct ?
          {
            sumReceived += data.received;
            sumSent += data.sent;
          }
        }
      }
    }
    f.close();
  }

  update();

  QString tip;
  for (int i = 0; i < cpus.count(); i++)
  {
    if ( i ) tip += '\n';
    tip += QString("CPU %1: %2% (%3 MHz)")
                   .arg(i)
                   .arg(cpus[i].userPercent + cpus[i].nicePercent + cpus[i].systemPercent)
                   .arg(cpus[i].MHz, 0, 'f', 2);
  }
  tip += '\n';
  memFree += cached;  // show also the cached memory as free (for user applications)
  size_t memUsed = memTotal - memFree;
  size_t swapUsed = swapTotal - swapFree;
  tip += i18n("Memory Total: %1 MB (%2 GB)\n").arg(memTotal  / 1024).arg(memTotal  / 1024.0 / 1024.0, 0, 'f', 2);
  tip += i18n("Memory Used: %1 MB (%2 GB)\n" ).arg(memUsed   / 1024).arg(memUsed   / 1024.0 / 1024.0, 0, 'f', 2);
  tip += i18n("Memory Free: %1 MB (%2 GB)\n" ).arg(memFree   / 1024).arg(memFree   / 1024.0 / 1024.0, 0, 'f', 2);
  tip += i18n("Swap Total: %1 MB (%2 GB)\n"  ).arg(swapTotal / 1024).arg(swapTotal / 1024.0 / 1024.0, 0, 'f', 2);
  tip += i18n("Swap Used: %1 MB (%2 GB)\n"   ).arg(swapUsed  / 1024).arg(swapUsed  / 1024.0 / 1024.0, 0, 'f', 2);
  tip += i18n("Swap Free: %1 MB (%2 GB)"     ).arg(swapFree  / 1024).arg(swapFree  / 1024.0 / 1024.0, 0, 'f', 2);

  tip += '\n';
  tip += i18n("Net send/receive: %1/%2 KB/sec")
              .arg((sumSent / 1024.0) / (INTERVAL / 1000.0), 0, 'f', 2)
              .arg((sumReceived / 1024.0) / (INTERVAL / 1000.0), 0, 'f', 2);

  if ( underMouse() )
    QToolTip::showText(QCursor::pos(), tip, this, rect());
}

//--------------------------------------------------------------------------------

void SysLoad::paintEvent(QPaintEvent *event)
{
  QFrame::paintEvent(event);

  int const barWidth = contentsRect().width() / (cpus.count() + 2 + 1);  // mem usage 2 bars + netSum
  int x = contentsRect().x(), y = contentsRect().y() + contentsRect().height();

  QPainter painter(this);
  foreach (const CpuData &data, cpus)
  {
    int h = (contentsRect().height() * (data.userPercent / 100.0));
    painter.fillRect(x, y - h, barWidth, h, Qt::blue);
    y -= h;
    h = (contentsRect().height() * (data.systemPercent / 100.0));
    painter.fillRect(x, y - h, barWidth, h, Qt::darkGreen);
    y -= h;
    h = (contentsRect().height() * (data.nicePercent / 100.0));
    painter.fillRect(x, y - h, barWidth, h, Qt::yellow);

    x += barWidth;
    y = contentsRect().y() + contentsRect().height();
  }

  int h = (contentsRect().height() * (memData.memPercent / 100.0));
  painter.fillRect(x, y - h, barWidth, h, Qt::blue);
  y -= h;
  h = (contentsRect().height() * (memData.memCachedPercent / 100.0));
  painter.fillRect(x, y, barWidth, h, Qt::darkGreen);

  x += barWidth;
  y = contentsRect().y() + contentsRect().height();
  h = (contentsRect().height() * (memData.swapPercent / 100.0));
  painter.fillRect(x, y - h, barWidth, h, Qt::cyan);

  // net
  maxBytes = std::max(maxBytes, (sumReceived + sumSent));

  x += barWidth;
  y = contentsRect().y() + contentsRect().height();
  h = (contentsRect().height() * (double(sumReceived) / maxBytes));
  painter.fillRect(x, y - h, barWidth, h, Qt::green);
  y -= h;
  h = (contentsRect().height() * (double(sumSent) / maxBytes));
  painter.fillRect(x, y - h, barWidth, h, Qt::red);
}

//--------------------------------------------------------------------------------

void SysLoad::mousePressEvent(QMouseEvent *event)
{
  if ( event->button() == Qt::LeftButton )
  {
    KRun::runCommand("ksysguard", this);
  }
}

//--------------------------------------------------------------------------------
