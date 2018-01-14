/*
  Copyright 2018 Martin Koller, kollix@aon.at

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

#include <KPartApplet.hxx>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDebug>

#include <KMimeTypeTrader>
#include <KLocalizedString>
#include <KRun>
#include <KConfig>
#include <KConfigGroup>
#include <kparts/browserextension.h>

//--------------------------------------------------------------------------------

KPartApplet::KPartApplet(QWidget *parent, const QString &theId)
  : DesktopApplet(parent, theId)
{
  label = new QLabel(this);
  label->setWordWrap(true);
  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(QMargins());
  vbox->addWidget(label);  // for errors
  label->hide();

  QAction *action = new QAction(this);
  action->setText(i18n("Configure..."));
  action->setIcon(QIcon::fromTheme("configure"));
  insertAction(actions()[0], action);
  connect(action, &QAction::triggered, this, &KPartApplet::configure);
}

//--------------------------------------------------------------------------------

void KPartApplet::loadConfig()
{
  KConfig config;
  KConfigGroup group = config.group(id);
  url = group.readEntry("url", QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)));

  DesktopApplet::loadConfig();
  fill();
}

//--------------------------------------------------------------------------------

void KPartApplet::fill()
{
  QString error;
  QString mimeType;

  if ( url.scheme().startsWith("http") )
    mimeType = "text/html";
  else
  {
    QMimeDatabase db;
    mimeType = db.mimeTypeForUrl(url).name();
  }

  part = KMimeTypeTrader::createPartInstanceFromQuery<KParts::ReadOnlyPart>(mimeType, this, this);

  if ( !part )
  {
    error = i18n("Could not find any KDE-component for %1 to display.", url.toString());
  }
  else
  {
    part->widget()->setContextMenuPolicy(Qt::NoContextMenu);

    connect(part->browserExtension(), &KParts::BrowserExtension::openUrlRequestDelayed, this,
            [this](const QUrl &newUrl)//, const KParts::OpenUrlArguments &arguments, const KParts::BrowserArguments &browserArguments)
            {
              new KRun(newUrl, this);
            });

    layout()->addWidget(part->widget());
    part->widget()->show();
    part->openUrl(url);
  }

  if ( !error.isEmpty() )
  {
    label->setText(error);
    label->show();
  }
}

//--------------------------------------------------------------------------------

void KPartApplet::setUrl(const QUrl &newUrl)
{
  if ( newUrl == url )
    return;

  url = newUrl;
  delete part;
  part = nullptr;
  fill();
}

//--------------------------------------------------------------------------------

void KPartApplet::configure()
{
  if ( dialog )
  {
    dialog->raise();
    dialog->activateWindow();
    return;
  }

  dialog = new KPartAppletConfigureDialog(this);
  dialog->setWindowTitle(i18n("Configure KPart Applet"));

  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();

  connect(dialog.data(), &QDialog::accepted, this,
          [this]()
          {
            saveConfig();

            KConfig config;
            KConfigGroup group = config.group(id);
            group.writeEntry("url", url);
          });
}

//--------------------------------------------------------------------------------
