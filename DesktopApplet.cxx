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

#include <DesktopApplet.hxx>

#include <QAction>
#include <QIcon>
#include <QAbstractButton>
#include <QMessageBox>
#include <QDebug>

#include <KLocalizedString>
#include <KConfig>
#include <KConfigGroup>

//--------------------------------------------------------------------------------

DesktopApplet::DesktopApplet(QWidget *parent, const QString &theId)
  : QFrame(parent), id(theId)
{
  setFrameShape(QFrame::StyledPanel);
  setContextMenuPolicy(Qt::ActionsContextMenu);

  QAction *action = new QAction(this);
  action->setIcon(QIcon::fromTheme("preferences-system-windows-move"));
  action->setText(i18n("Change Size && Position"));
  addAction(action);
  connect(action, &QAction::triggered, this, &DesktopApplet::startGeometryChange);

  action = new QAction(this);
  action->setIcon(QIcon::fromTheme("window-close"));
  action->setText(i18n("Remove this Applet"));
  addAction(action);
  connect(action, &QAction::triggered, this, &DesktopApplet::removeThisApplet);

  buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  buttons->adjustSize();
  buttons->hide();
  connect(buttons, &QDialogButtonBox::clicked, this, &DesktopApplet::finishGeometryChange);
}

//--------------------------------------------------------------------------------

void DesktopApplet::loadConfig()
{
  KConfig config;
  KConfigGroup group = config.group(id);
  setGeometry(group.readEntry("rect", QRect(30, 30, 600, 400)));
  onDesktop = group.readEntry("onDesktop", int(NET::OnAllDesktops));
}

//--------------------------------------------------------------------------------

void DesktopApplet::saveConfig()
{
  KConfig config;
  KConfigGroup group = config.group(id);
  group.writeEntry("rect", geometry());
  group.writeEntry("onDesktop", onDesktop);
}

//--------------------------------------------------------------------------------

void DesktopApplet::startGeometryChange()
{
  buttons->raise();
  buttons->show();

  oldRect = geometry();
  setWindowFlags(Qt::Window);
  setWindowTitle(i18n("%1: Change Size & Position").arg(id));

  if ( onDesktop == NET::OnAllDesktops )
    KWindowSystem::setOnAllDesktops(winId(), true);

  show();
  setGeometry(oldRect);
}

//--------------------------------------------------------------------------------

void DesktopApplet::finishGeometryChange(QAbstractButton *clicked)
{
  KWindowInfo info(winId(), NET::WMDesktop);
  if ( info.valid() )
    onDesktop = info.desktop();

  buttons->hide();
  QRect rect = geometry();
  setWindowFlags(Qt::Widget);
  show();

  if ( buttons->buttonRole(clicked) == QDialogButtonBox::AcceptRole )
  {
    setGeometry(rect);
    KConfig config;
    KConfigGroup group = config.group(id);
    group.writeEntry("rect", rect);
    group.writeEntry("onDesktop", onDesktop);
  }
  else
  {
    setGeometry(oldRect);
  }
}

//--------------------------------------------------------------------------------

void DesktopApplet::removeThisApplet()
{
  if ( QMessageBox::question(this, i18n("Remove Applet"),
                             i18n("Really remove this applet?")) == QMessageBox::Yes )
  {
    KConfig config;
    config.deleteGroup(id);
    emit removeThis(this);
  }
}

//--------------------------------------------------------------------------------
