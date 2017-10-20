#include <QApplication>

#include <DesktopWidget.hxx>

#include <KCrash>

int main(int argc, char **argv)
{
  QCoreApplication::setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles);
  QApplication app(argc, argv);

  DesktopWidget desktop;
  desktop.show();

  KCrash::setFlags(KCrash::AutoRestart);

  return app.exec();
}
