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
