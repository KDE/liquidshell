/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
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

  icon2Label = new QLabel;
  icon2Label->setFixedSize(iconSize());
  icon2Label->setContextMenuPolicy(Qt::PreventContextMenu);
  icon2Label->hide();  // until an icon is set
  hbox->addWidget(icon2Label);

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

void IconButton::setIcon2(const QIcon &icon)
{
  icon2Label->setPixmap(icon.pixmap(iconSize()));
  icon2Label->show();
}

//--------------------------------------------------------------------------------

QSize IconButton::sizeHint() const
{
  return layout()->sizeHint();
}

//--------------------------------------------------------------------------------
