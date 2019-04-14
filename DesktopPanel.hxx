// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 Martin Koller, kollix@aon.at

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

#ifndef _DesktopPanel_H_
#define _DesktopPanel_H_

#include <QFrame>

class DesktopPanel : public QFrame
{
  Q_OBJECT

  public:
    DesktopPanel(QWidget *parent);

    int getRows() const { return rows; }

  Q_SIGNALS:
    void resized();
    void rowsChanged(int rows);

  protected:
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private Q_SLOTS:
    void updateRowCount();

  private:
    int rows = 2;
};

#endif
