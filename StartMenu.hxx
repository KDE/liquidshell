/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _StartMenu_H_
#define _StartMenu_H_

#include <QToolButton>
#include <QMenu>

#include <KServiceGroup>

#include <DesktopPanel.hxx>

class StartMenu : public QToolButton
{
  Q_OBJECT

  Q_PROPERTY(QString themeIcon READ getThemeIcon WRITE setThemeIcon)

  public:
    StartMenu(DesktopPanel *parent);

    QString getThemeIcon() const { return themeIcon; }
    void setThemeIcon(const QString &icon);

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

  private Q_SLOTS:
    void adjustIconSize();
    void fill();
    void showMenu();

  private:
    void fillFromGroup(QMenu *menu, KServiceGroup::Ptr group);

  private:
    QString themeIcon;
    QMenu *popup;
};

#endif
