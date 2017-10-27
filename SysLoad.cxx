/*
  Copyright 2017 Martin Koller

  This file is part of liquidshell.

  liquidshell is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  liquidshell is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with liquidshell.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <SysLoad.hxx>

#include <QFile>
#include <QPainter>
#include <QToolTip>
#include <QMouseEvent>
#include <QDebug>

#include <KRun>
#include <KLocalizedString>

#include <NetworkManagerQt/Manager>

//--------------------------------------------------------------------------------

const int INTERVAL_MS = 800;

//--------------------------------------------------------------------------------

SysLoad::SysLoad(QWidget *parent)
  : QFrame(parent)
{
  setFrameShape(QFrame::StyledPanel);

  timeoutTimer.setInterval(INTERVAL_MS);
  timeoutTimer.start();
  connect(&timeoutTimer, &QTimer::timeout, this, &SysLoad::fetch);

  connect(NetworkManager::notifier(), &NetworkManager::Notifier::primaryConnectionChanged,
          [this]() { maxBytes = 100; });  // reset since we're changing network (which might be slower)

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
      // time is in 1/100s units https://www.kernel.org/doc/Documentation/filesystems/proc.txt
      CpuData data;
      sscanf(line.constData(), "%*s %d %d %d", &data.userCPU, &data.niceCPU, &data.systemCPU);

      if ( first )
        cpus.append(data);
      else
      {
        cpus[num].userPercent = (data.userCPU - cpus[num].userCPU) * 1000.0 / INTERVAL_MS;
        cpus[num].nicePercent = (data.niceCPU - cpus[num].niceCPU) * 1000.0 / INTERVAL_MS;
        cpus[num].systemPercent = (data.systemCPU - cpus[num].systemCPU) * 1000.0 / INTERVAL_MS;
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
    if ( i ) tip += "<br>";
    tip += QString("CPU %1: %2% (%3 MHz)")
                   .arg(i)
                   .arg(std::min(100.0, cpus[i].userPercent + cpus[i].nicePercent + cpus[i].systemPercent))
                   .arg(static_cast<int>(cpus[i].MHz));
  }
  tip += "<hr>";
  memFree += cached;  // show also the cached memory as free (for user applications)
  size_t memUsed = memTotal - memFree;
  size_t swapUsed = swapTotal - swapFree;
  int memUsedPercent = memUsed * 100 / memTotal;
  int swapUsedPercent = swapUsed * 100 / swapTotal;
  tip += i18n("Memory Total: %1 MB (%2 GB)"   ).arg(memTotal  / 1024).arg(memTotal  / 1024.0 / 1024.0, 0, 'f', 2);
  tip += "<br>";
  tip += i18n("Memory Used: %1 MB (%2 GB) %3%").arg(memUsed   / 1024).arg(memUsed   / 1024.0 / 1024.0, 0, 'f', 2).arg(memUsedPercent);
  tip += "<br>";
  tip += i18n("Memory Free: %1 MB (%2 GB)"    ).arg(memFree   / 1024).arg(memFree   / 1024.0 / 1024.0, 0, 'f', 2);
  tip += "<hr>";
  tip += i18n("Swap Total: %1 MB (%2 GB)"     ).arg(swapTotal / 1024).arg(swapTotal / 1024.0 / 1024.0, 0, 'f', 2);
  tip += "<br>";
  tip += i18n("Swap Used: %1 MB (%2 GB) %3%"  ).arg(swapUsed  / 1024).arg(swapUsed  / 1024.0 / 1024.0, 0, 'f', 2).arg(swapUsedPercent);
  tip += "<br>";
  tip += i18n("Swap Free: %1 MB (%2 GB)"        ).arg(swapFree  / 1024).arg(swapFree  / 1024.0 / 1024.0, 0, 'f', 2);

  tip += "<hr>";
  tip += i18n("Net send/receive: %1/%2 KB/sec")
              .arg((sumSent / 1024.0) / (INTERVAL_MS / 1000.0), 0, 'f', 2)
              .arg((sumReceived / 1024.0) / (INTERVAL_MS / 1000.0), 0, 'f', 2);
  tip += "<br>";
  tip += i18n("Net max used: %1 KB/sec").arg((maxBytes / 1024) / (INTERVAL_MS / 1000.0));

  if ( underMouse() )
    QToolTip::showText(QCursor::pos(), QLatin1String("<html>") + tip + QLatin1String("</html>"), this, rect());
}

//--------------------------------------------------------------------------------

void SysLoad::paintEvent(QPaintEvent *event)
{
  QFrame::paintEvent(event);

  int const barWidth = contentsRect().width() / (cpus.count() + 2 + 1);  // mem usage 2 bars + netSum
  int x = contentsRect().x(), y = contentsRect().y() + contentsRect().height();

  QPainter painter(this);
  for (const CpuData &data : cpus)
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
