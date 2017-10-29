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

#include <Pager.hxx>
#include <PagerButton.hxx>

#include <QGridLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QX11Info>
#include <QDBusConnection>
#include <QAction>
#include <QDebug>

#include <KLocalizedString>
#include <KCMultiDialog>
#include <KWindowSystem>
#include <netwm.h>

//--------------------------------------------------------------------------------

Pager::Pager(QWidget *parent)
  : QWidget(parent)
{
  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  group = new QButtonGroup(this);

  QGridLayout *grid = new QGridLayout(this);
  grid->setSpacing(2);
  grid->setContentsMargins(QMargins());

  connect(KWindowSystem::self(), &KWindowSystem::numberOfDesktopsChanged, this, &Pager::fill);

  connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged,
          [this]()
          {
            if ( KWindowSystem::currentDesktop() <= buttons.count() )
              buttons[KWindowSystem::currentDesktop() - 1]->setChecked(true);
          }
         );

  // to get notified about num-of-rows changed
  QDBusConnection dbus = QDBusConnection::sessionBus();
  dbus.connect(QString(), "/KWin", "org.kde.KWin", "reloadConfig", this, SLOT(fill()));

  fill();

  QAction *action = new QAction(this);
  action->setIcon(QIcon::fromTheme("configure"));
  action->setText(i18n("Configure Virtual Desktops..."));
  addAction(action);
  connect(action, &QAction::triggered,
          [this]()
          {
            auto dialog = new KCMultiDialog(parentWidget());
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->setWindowTitle(i18n("Configure Virtual Desktops"));
            dialog->addModule("desktop");
            dialog->adjustSize();
            dialog->show();
          }
         );
  setContextMenuPolicy(Qt::ActionsContextMenu);
}

//--------------------------------------------------------------------------------

void Pager::fill()
{
  qDeleteAll(buttons);
  buttons.clear();

  NETRootInfo ri(QX11Info::connection(), 0, NET::WM2DesktopLayout);

  int row = 0, col = 0;
  const int MAX_COLUMNS = std::max(1, ri.desktopLayoutColumnsRows().width());

  for (int i = 1; i <= KWindowSystem::numberOfDesktops(); i++)
  {
    PagerButton *b = new PagerButton(i);

    b->setCheckable(true);
    b->setFocusPolicy(Qt::NoFocus);
    b->setMaximumWidth(50);
    group->addButton(b);
    buttons.append(b);

    if ( i == KWindowSystem::currentDesktop() )
      b->setChecked(true);

    connect(b, &PagerButton::clicked, this, &Pager::changeDesktop);

    static_cast<QGridLayout *>(layout())->addWidget(b, row, col);
    col = (col + 1) % MAX_COLUMNS;
    if ( col == 0 ) row++;
  }
}

//--------------------------------------------------------------------------------

void Pager::changeDesktop(bool checked)
{
  if ( !checked )
    return;

  int desktopNum = qobject_cast<PagerButton *>(sender())->getDesktop();

  if ( KWindowSystem::currentDesktop() == desktopNum )
    KWindowSystem::setShowingDesktop(!KWindowSystem::showingDesktop());
  else
    KWindowSystem::setCurrentDesktop(desktopNum);
}

//--------------------------------------------------------------------------------
