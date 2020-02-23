// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2018 - 2020 Martin Koller, kollix@aon.at

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

#ifndef _IconButton_H_
#define _IconButton_H_

#include <QToolButton>
#include <QLabel>

// button class to ensure the positioning of the icon and text independent of the style

class IconButton : public QToolButton
{
  Q_OBJECT

  public:
    IconButton(QWidget *parent = nullptr);
    IconButton(QWidget *parent, const QIcon &icon, int iconSize, const QString &name);

    void setText(const QString &txt);
    void setIcon(const QIcon &icon);
    void setIcon2(const QIcon &icon);

    QSize sizeHint() const override;

  private:
    QLabel *iconLabel = nullptr;
    QLabel *icon2Label = nullptr;
    QLabel *textLabel = nullptr;
};

#endif
