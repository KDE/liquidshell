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

#include <TaskBar.hxx>
#include <TaskBarButton.hxx>

#include <KWindowSystem>

#include <QVariant>
#include <QSpacerItem>
#include <QDebug>

#include <cmath>

//--------------------------------------------------------------------------------

TaskBar::TaskBar(DesktopPanel *parent)
  : QWidget(parent)
{
  grid = new QGridLayout(this);
  grid->setSpacing(2);
  grid->setContentsMargins(QMargins());

  fill();

  connect(parent, &DesktopPanel::rowsChanged, this, &TaskBar::fill);

  connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, &TaskBar::fill);
  connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &TaskBar::fill);
  connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, &TaskBar::fill);

  connect(KWindowSystem::self(), SIGNAL(windowChanged(WId, NET::Properties, NET::Properties2)),
          this, SLOT(windowChanged(WId, NET::Properties, NET::Properties2)));
}

//--------------------------------------------------------------------------------

void TaskBar::fill()
{
  QLayoutItem *child;
  while ( (child = grid->takeAt(0)) )
  {
    delete child->widget();
    delete child;
  }

  QList<WId> windowsToShow;
  const int MAX_ROWS = qobject_cast<DesktopPanel *>(parentWidget())->getRows();

  for (WId wid : KWindowSystem::windows())
  {
    KWindowInfo win(wid, NET::WMDesktop | NET::WMWindowType | NET::WMState);
    if ( win.valid(true) && win.isOnCurrentDesktop() &&
        (win.windowType(NET::DesktopMask) != NET::Desktop) &&
        (win.windowType(NET::DockMask) != NET::Dock) &&
        (win.windowType(NET::PopupMenuMask) != NET::PopupMenu) &&
        (win.windowType(NET::UtilityMask) != NET::Utility) &&
         !(win.state() & NET::SkipTaskbar) )
      windowsToShow.append(wid);
  }

  if ( windowsToShow.count() == 0 )
  {
    grid->addItem(new QSpacerItem(0, 0,  QSizePolicy::Expanding), 0, 0);
    return;
  }

  int row = 0, col = 0, actualRows;
  const int MAX_COLUMNS = std::max(2, static_cast<int>(std::ceil(windowsToShow.count() / float(MAX_ROWS))));
  actualRows = static_cast<int>(std::ceil(windowsToShow.count() / float(MAX_COLUMNS)));
  const int ICON_SIZE = height() / actualRows >= 36 ? 32 : 22;

  for (WId wid : windowsToShow)
  {
    TaskBarButton *b = new TaskBarButton(wid);
    b->setIconSize(ICON_SIZE);
    grid->addWidget(b, row, col);

    col = (col + 1) % MAX_COLUMNS;
    if ( col == 0 ) row++;
  }
}

//--------------------------------------------------------------------------------

void TaskBar::windowChanged(WId wid, NET::Properties props, NET::Properties2 props2)
{
  Q_UNUSED(wid)
  Q_UNUSED(props2)

  if ( props & NET::WMDesktop )
    fill();
}

//--------------------------------------------------------------------------------
