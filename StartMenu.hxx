/*
  Copyright 2017 Martin Koller

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

#include <QPushButton>
#include <QMenu>

#include <KServiceGroup>

#include <DesktopPanel.hxx>

class StartMenu : public QPushButton
{
  Q_OBJECT

  Q_PROPERTY(QString themeIcon READ getThemeIcon WRITE setThemeIcon)

  public:
    StartMenu(DesktopPanel *parent);

    QString getThemeIcon() const { return themeIcon; }
    void setThemeIcon(const QString &icon);

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

  private slots:
    void fill();

  private:
    void fillFromGroup(QMenu *menu, KServiceGroup::Ptr group);

  private:
    QString themeIcon;
};

//--------------------------------------------------------------------------------

class PopupMenu : public QMenu
{
  Q_OBJECT

  public:
    PopupMenu(QWidget *parent) : QMenu(parent), dragStartPos(-1, -1) { }

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private:
    QPoint dragStartPos;
};

#endif
