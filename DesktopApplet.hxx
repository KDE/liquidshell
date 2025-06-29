/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _DesktopApplet_H_
#define _DesktopApplet_H_

#include <QFrame>
#include <QDialogButtonBox>
#include <KWinCompat.hxx>

class DesktopApplet : public QFrame
{
  Q_OBJECT

  public:
    DesktopApplet(QWidget *parent, const QString &theId, bool addConfigureAction = true);

    virtual void loadConfig();

    bool isConfiguring() const { return buttons->isVisible(); }

    const QString &getId() const { return id; }
    int isOnDesktop(int d) const { return (onDesktop == NET::OnAllDesktops) || (onDesktop == d); }

  public Q_SLOTS:
    virtual void configure() { }
    virtual void saveConfig();

  Q_SIGNALS:
    void removeThis(DesktopApplet *);

  protected:
    QString id;

  private Q_SLOTS:
    void startGeometryChange();
    void finishGeometryChange(QAbstractButton *clicked);
    void removeThisApplet();

  private:
    QDialogButtonBox *buttons = nullptr;
    QRect oldRect;
    int onDesktop = NET::OnAllDesktops;
};

#endif
