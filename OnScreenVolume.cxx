/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <OnScreenVolume.hxx>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QRegularExpression>
#include <QApplication>
#include <QScreen>
#include <QDebug>

#include <KWinCompat.hxx>
#include <KConfig>
#include <KConfigGroup>

//--------------------------------------------------------------------------------

OnScreenVolume::OnScreenVolume(QWidget *parent)
  : QProgressBar(parent)
{
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);

  setFixedSize(400, 40);
  hide();

  KWinCompat::setState(winId(), NET::KeepAbove);
  KWinCompat::setType(winId(), NET::Dock);
  KWinCompat::setOnAllDesktops(winId(), true);

  hideTimer.setInterval(1000);
  hideTimer.setSingleShot(true);
  connect(&hideTimer, &QTimer::timeout, this, &QWidget::hide);

  QDBusConnection::sessionBus()
      .connect("org.kde.kmix", "/Mixers",
               "org.kde.KMix.MixSet", "masterChanged",
               this, SLOT(getMasterMixer()));

  if ( !QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kmix") )
  {
    QDBusServiceWatcher *w = new QDBusServiceWatcher("org.kde.kmix", QDBusConnection::sessionBus(),
                                                     QDBusServiceWatcher::WatchForRegistration, this);
    connect(w, &QDBusServiceWatcher::serviceRegistered, this, &OnScreenVolume::getMasterMixer);
  }
  else
    getMasterMixer();
}

//--------------------------------------------------------------------------------

void OnScreenVolume::getMasterMixer()
{
  QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.kmix", "/Mixers",
                                                    "org.freedesktop.DBus.Properties",
                                                    "GetAll");
  msg << "org.kde.KMix.MixSet";

  QDBusConnection::sessionBus()
      .callWithCallback(msg, this,
                        SLOT(gotMasterMixer(QDBusMessage)),
                        SLOT(gotMasterMixerError(QDBusError, QDBusMessage)));
}

//--------------------------------------------------------------------------------

void OnScreenVolume::gotMasterMixerError(QDBusError error, QDBusMessage msg)
{
  Q_UNUSED(error)
  Q_UNUSED(msg)

  if ( retryTimer.interval() == 0 )
  {
    // the service could already be registered but no object yet
    retryTimer.setInterval(1000);
    retryTimer.setSingleShot(true);
    connect(&retryTimer, &QTimer::timeout, this, &OnScreenVolume::getMasterMixer);
  }
  retryTimer.start();
}

//--------------------------------------------------------------------------------

void OnScreenVolume::gotMasterMixer(QDBusMessage msg)
{
  QDBusReply<QVariantMap> reply = msg;

  if ( !reply.isValid() )
    return;

  if ( !masterMixer.isEmpty() )
  {
    // disconnect previous master mixer
    QDBusConnection::sessionBus()
        .disconnect("org.kde.kmix", "/Mixers/" + masterMixer,
                    "org.kde.KMix.Mixer", "controlChanged",
                    this, SLOT(controlChanged()));
  }

  masterMixer = reply.value()["currentMasterMixer"].toString();
  masterMixer.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");

  //qDebug() << __FUNCTION__ << masterMixer;

  QDBusConnection::sessionBus()
      .connect("org.kde.kmix", "/Mixers/" + masterMixer,
               "org.kde.KMix.Mixer", "controlChanged",
               this, SLOT(controlChanged()));
}

//--------------------------------------------------------------------------------
// called when e.g. volume changes but also if currentMasterControl changes
// e.g. bluetooth headset connect/disconnect

void OnScreenVolume::controlChanged()
{
  QDBusMessage msg =
      QDBusMessage::createMethodCall("org.kde.kmix", "/Mixers",
                                     "org.freedesktop.DBus.Properties",
                                     "Get");

  msg << "org.kde.KMix.MixSet" << "currentMasterControl";

  QDBusConnection::sessionBus().callWithCallback(msg, this, SLOT(gotCurrentControl(QDBusMessage)));
}

//--------------------------------------------------------------------------------

void OnScreenVolume::gotCurrentControl(QDBusMessage reply)
{
  if ( reply.type() == QDBusMessage::ErrorMessage )
    return;

  QString masterControl = reply.arguments()[0].value<QDBusVariant>().variant().toString();
  masterControl.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");

  //qDebug() << __FUNCTION__ << masterMixer << masterControl;

  QDBusMessage msg =
      QDBusMessage::createMethodCall("org.kde.kmix", "/Mixers/" + masterMixer + '/' + masterControl,
                                     "org.freedesktop.DBus.Properties",
                                     "Get");

  msg << "org.kde.KMix.Control" << "volume";

  QDBusConnection::sessionBus().callWithCallback(msg, this, SLOT(volumeChanged(QDBusMessage)));
}

//--------------------------------------------------------------------------------

void OnScreenVolume::volumeChanged(QDBusMessage reply)
{
  if ( reply.type() == QDBusMessage::ErrorMessage )
    return;

  if ( reply.arguments().count() && (reply.arguments()[0].value<QDBusVariant>().variant().toInt() != value()) )
  {
    setValue(reply.arguments()[0].value<QDBusVariant>().variant().toInt());

    if ( !isVisible() )  // just always once before showing
    {
      KConfig config("kmixrc");
      KConfigGroup group = config.group("Global");
      if ( !group.readEntry("showOSD", true) )
        return;
    }

    move((QApplication::primaryScreen()->size().width() - width()) / 2,
          QApplication::primaryScreen()->size().height() * 0.8);

    show();
    hideTimer.start();
  }
}

//--------------------------------------------------------------------------------
