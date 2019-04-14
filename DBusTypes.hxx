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

#ifndef DBusTypes_h
#define DBusTypes_h

// define types used in kf5_org.kde.StatusNotifierItem.xml

#include <QPixmap>
#include <QIcon>
#include <QString>
#include <QDBusArgument>
#include <QMetaType>

typedef QPixmap KDbusImageStruct;
typedef QIcon KDbusImageVector;

struct KDbusToolTipStruct
{
  QString icon;
  KDbusImageVector image;
  QString title;
  QString subTitle;
};

Q_DECLARE_METATYPE(KDbusToolTipStruct)

const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusImageStruct &pixmap);
const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusImageVector &icon);
const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusToolTipStruct &tip);
QDBusArgument &operator<<(QDBusArgument &argument, const KDbusImageStruct &);
QDBusArgument &operator<<(QDBusArgument &argument, const KDbusImageVector &);
QDBusArgument &operator<<(QDBusArgument &argument, const KDbusToolTipStruct &);

#endif
