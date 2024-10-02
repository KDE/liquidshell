/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _Launcher_H_
#define _Launcher_H_

#include <QWidget>
#include <QFileSystemWatcher>

class Launcher : public QWidget
{
  Q_OBJECT

  public:
    Launcher(QWidget *parent, const QString &theId);

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void loadConfig(const QString &defaultDir = QString());
    void setDir(const QString &theDirPath);

  private Q_SLOTS:
    virtual void fill() = 0;

  protected:
    QString id;
    QString dirPath;
    QFileSystemWatcher dirWatcher;
};

#endif
