#include <QApplication>

#include <DesktopWidget.hxx>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  DesktopWidget desktop;
  desktop.show();

  return app.exec();
}
