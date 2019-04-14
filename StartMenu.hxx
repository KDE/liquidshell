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
