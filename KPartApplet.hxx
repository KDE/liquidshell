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

#ifndef _KPartApplet_H_
#define _KPartApplet_H_

#include <DesktopApplet.hxx>
#include <KPartAppletConfigureDialog.hxx>
#include <kparts/readonlypart.h>
#include <QLabel>
#include <QPointer>

class KPartApplet : public DesktopApplet
{
  Q_OBJECT

  public:
    KPartApplet(QWidget *parent, const QString &theId);

    void loadConfig() override;

    void setUrl(const QUrl &newUrl);
    const QUrl &getUrl() const { return url; }

    QSize sizeHint() const override { return QSize(600, 400); }

  private Q_SLOTS:
    void configure();

  private:
    void fill();

  private:
    KParts::ReadOnlyPart *part = nullptr;
    QUrl url;
    QLabel *label = nullptr;
    QPointer<KPartAppletConfigureDialog> dialog;
};

#endif
