/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _SysTrayItem_H_
#define _SysTrayItem_H_

#include <QLabel>

class SysTrayItem : public QLabel
{
  Q_OBJECT

  public:
    SysTrayItem(QWidget *parent, const QString &icon = QString());

  protected Q_SLOTS:
    void toggleDetailsList();
    void showDetailsList();

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    virtual QWidget *getDetailsList() { return nullptr; }

  protected:
    QString iconName;
};

#endif
