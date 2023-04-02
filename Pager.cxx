// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017, 2018 Martin Koller, kollix@aon.at

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
#include <DesktopPanel.hxx>
#include <KWinCompat.hxx>

#include <QGridLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QX11Info>
#include <QAction>
#include <QWheelEvent>
#include <QDebug>

#include <KLocalizedString>
#include <KCMultiDialog>
#include <KConfig>
#include <KConfigGroup>
#include <KPluginMetaData>
#include <kcmutils_version.h>
#include <netwm.h>

//--------------------------------------------------------------------------------

Pager::Pager(DesktopPanel *parent)
  : QWidget(parent)
{
  group = new QButtonGroup(this);

  QGridLayout *grid = new QGridLayout(this);
  grid->setSpacing(2);
  grid->setContentsMargins(QMargins());

  connect(KWinCompat::self(), &KWinCompat::numberOfDesktopsChanged, this, &Pager::fill);

  connect(KWinCompat::self(), &KWinCompat::currentDesktopChanged,
          [this]()
          {
            if ( KWinCompat::currentDesktop() <= buttons.count() )
              buttons[KWinCompat::currentDesktop() - 1]->setChecked(true);
          }
         );

  connect(parent, &DesktopPanel::rowsChanged, this, &Pager::fill);

  KConfig config;
  KConfigGroup group = config.group("Pager");
  if ( !group.hasKey("showIcons") )  // create config entry so that one knows it exists
    group.writeEntry("showIcons", true);

  showIcons  = group.readEntry("showIcons", true);

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

#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 85, 0)
            KPluginMetaData data("plasma/kcms/systemsettings/kcm_kwin_virtualdesktops");

            if ( !data.isValid() )
              data = KPluginMetaData::findPluginById("kcms", "kcm_kwin_virtualdesktops");

            if ( data.isValid() )
              dialog->addModule(data);
#else
            {
              KCModuleInfo module("kcm_kwin_virtualdesktops");
              if ( module.service() )
                dialog->addModule("kcm_kwin_virtualdesktops");
              else
                dialog->addModule("desktop");  // in older KDE versions
            }
#endif

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

#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
  NETRootInfo ri(QX11Info::connection(), NET::Property(), NET::WM2DesktopLayout);
#else
  NETRootInfo ri(QX11Info::connection(), 0, NET::WM2DesktopLayout);
#endif

  int row = 0, col = 0;
  const int MAX_COLUMNS = std::max(1, ri.desktopLayoutColumnsRows().width());

  for (int i = 1; i <= KWinCompat::numberOfDesktops(); i++)
  {
    PagerButton *b = new PagerButton(i, qobject_cast<DesktopPanel *>(parentWidget()), showIcons);

    b->setCheckable(true);
    b->setFocusPolicy(Qt::NoFocus);
    group->addButton(b);
    buttons.append(b);

    if ( i == KWinCompat::currentDesktop() )
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

  if ( KWinCompat::currentDesktop() == desktopNum )
    KWindowSystem::setShowingDesktop(!KWindowSystem::showingDesktop());
  else
    KWinCompat::setCurrentDesktop(desktopNum);
}

//--------------------------------------------------------------------------------

void Pager::wheelEvent(QWheelEvent *event)
{
  int desktopNum = KWinCompat::currentDesktop() - 1 + KWinCompat::numberOfDesktops();

  if ( event->angleDelta().y() > 0 )
    desktopNum++;
  else
    desktopNum--;

  KWinCompat::setCurrentDesktop((desktopNum % KWinCompat::numberOfDesktops()) + 1);
}

//--------------------------------------------------------------------------------
