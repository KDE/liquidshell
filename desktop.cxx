/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QApplication>
#include <QCommandLineParser>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCall>

#include <DesktopWidget.hxx>

#include <KCrash>
#include <KLocalizedString>
#include <KAboutData>
#include <KDBusService>

int main(int argc, char **argv)
{
  QCoreApplication::setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles);

  QApplication app(argc, argv);

  KLocalizedString::setApplicationDomain("liquidshell");

  KAboutData aboutData("liquidshell", i18n("Liquidshell"), "1.10",
                       i18n("A QtWidgets based basic desktop shell"),
                       KAboutLicense::GPL_V3,
                       i18n("Copyright 2017 - 2024 Martin Koller"), QString(),
                       "https://apps.kde.org/de/liquidshell"); // homepage
                       //"https://www.linux-apps.com/p/1205621"); // homepage

  aboutData.addAuthor("Martin Koller", "", "kollix@aon.at");

  KAboutData::setApplicationData(aboutData);

  QCommandLineParser parser;
  aboutData.setupCommandLine(&parser);
  parser.process(app);
  aboutData.processCommandLine(&parser);

  KCrash::setFlags(KCrash::AutoRestart);
  KDBusService programDBusService(KDBusService::Unique | KDBusService::NoExitOnFailure);

  DesktopWidget desktop;
  desktop.show();

  QDBusMessage ksplashProgressMessage =
      QDBusMessage::createMethodCall(QStringLiteral("org.kde.KSplash"),
                                     QStringLiteral("/KSplash"),
                                     QStringLiteral("org.kde.KSplash"),
                                     QStringLiteral("setStage"));
  ksplashProgressMessage.setArguments(QList<QVariant>() << QStringLiteral("desktop"));
  QDBusConnection::sessionBus().asyncCall(ksplashProgressMessage);

  return app.exec();
}
