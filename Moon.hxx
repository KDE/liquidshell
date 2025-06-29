// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2021 Martin Koller, martin@kollix.at

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

#include <QDate>

class Moon
{
  public:
    // returns the frame to be used from the moon_56frames.png
    static int phase(const QDate &date);

  private:
    static double sun_position(double j);
    static double moon_position(double j, double ls);
};
