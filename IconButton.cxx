// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2018 Martin Koller, kollix@aon.at

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

#include <IconButton.hxx>

#include <QHBoxLayout>

//--------------------------------------------------------------------------------

IconButton::IconButton(QWidget *parent)
  : QToolButton(parent)
{
  setAutoRaise(true);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

  QHBoxLayout *hbox = new QHBoxLayout(this);

  iconLabel = new QLabel;
  iconLabel->setFixedSize(iconSize());
  iconLabel->setContextMenuPolicy(Qt::PreventContextMenu);
  hbox->addWidget(iconLabel);

  textLabel = new QLabel;
  hbox->addWidget(textLabel);
}

//--------------------------------------------------------------------------------

IconButton::IconButton(QWidget *parent, const QIcon &icon, int theIconSize, const QString &name)
  : IconButton(parent)
{
  if ( theIconSize != -1 )
    setIconSize(QSize(theIconSize, theIconSize));

  iconLabel->setFixedSize(iconSize());
  iconLabel->setPixmap(icon.pixmap(iconSize()));

  textLabel->setText(name);
}

//--------------------------------------------------------------------------------

void IconButton::setText(const QString &txt)
{
  textLabel->setText(txt);
}

//--------------------------------------------------------------------------------

void IconButton::setIcon(const QIcon &icon)
{
  iconLabel->setPixmap(icon.pixmap(iconSize()));
}

//--------------------------------------------------------------------------------

QSize IconButton::sizeHint() const
{
  return layout()->sizeHint();
}

//--------------------------------------------------------------------------------
