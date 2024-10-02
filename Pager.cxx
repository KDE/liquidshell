/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <Pager.hxx>
#include <PagerButton.hxx>
#include <DesktopPanel.hxx>
#include <KWinCompat.hxx>

#include <QGridLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QAction>
#include <QWheelEvent>
#include <QDebug>

#include <KLocalizedString>
#include <KCMultiDialog>
#include <KConfig>
#include <KConfigGroup>
#include <KPluginMetaData>

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

            KPluginMetaData data("plasma/kcms/systemsettings/kcm_kwin_virtualdesktops");

            dialog->addModule(data);
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

  NETRootInfo ri(qApp->nativeInterface<QNativeInterface::QX11Application>()->connection(), NET::Property(), NET::WM2DesktopLayout);

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
