/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
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

//--------------------------------------------------------------------------------

struct DBusMenuLayoutItem
{
  int m_id;
  QVariantMap m_properties;
  QVector<DBusMenuLayoutItem> m_children;
};

Q_DECLARE_METATYPE(DBusMenuLayoutItem)

const QDBusArgument &operator<<(QDBusArgument &arg, const DBusMenuLayoutItem &item);
const QDBusArgument &operator>>(const QDBusArgument &arg, DBusMenuLayoutItem &item);

#endif
