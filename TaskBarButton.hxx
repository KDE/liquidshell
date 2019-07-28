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

#ifndef _TaskBarButton_H_
#define _TaskBarButton_H_

#include <QPushButton>
#include <QTimer>
class QLabel;
class KSqueezedTextLabel;

#include <kwindowsystem.h>

class TaskBarButton : public QPushButton
{
  Q_OBJECT

  public:
    TaskBarButton(WId wid);

    void setIconSize(int size);

    QSize sizeHint() const override;

  Q_SIGNALS:
    void clicked();

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void moveEvent(QMoveEvent *) override { updateWMGeometry(); }
    void resizeEvent(QResizeEvent *) override { updateWMGeometry(); }

  private Q_SLOTS:
    void fill();
    void setBackground();
    void windowChanged(WId id, NET::Properties props, NET::Properties2 props2);

  private:
    void updateWMGeometry();

  private:
    WId wid;
    QLabel *iconLabel;
    KSqueezedTextLabel *textLabel;
    QTimer dragDropTimer;
    QPoint dragStartPos;
};

#endif
