/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _ClockWidgetConfigureDialog_H_
#define _ClockWidgetConfigureDialog_H_

#include <QDialog>
class QTreeWidget;

class ClockWidgetConfigureDialog : public QDialog
{
  Q_OBJECT

  public:
    ClockWidgetConfigureDialog(QWidget *parent, const QVector<QByteArray> &timeZoneIds);

    QVector<QByteArray> getSelectedTimeZoneIds() const;

  private:
    QTreeWidget *tree;
};

#endif
