/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
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
