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

#ifndef _PagerButton_H_
#define _PagerButton_H_

#include <QPushButton>
#include <QTimer>

#include <kwindowsystem.h>

class PagerButton : public QPushButton
{
  Q_OBJECT

  public:
    PagerButton(int num);

    int getDesktop() const { return desktop; }

  protected:
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

  private slots:
    void createPixmap();
    void windowChanged(WId id, NET::Properties props, NET::Properties2 props2);

  private:
    int desktop;
    QPixmap firstPixmap;
    QTimer dragDropTimer;
};

#endif
