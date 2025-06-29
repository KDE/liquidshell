/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <DBusTypes.hxx>

#include <QtEndian>

//--------------------------------------------------------------------------------

const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusImageStruct &pixmap)
{
  pixmap = QPixmap();

  if ( argument.currentType() == QDBusArgument::StructureType )
  {
    qint32 w = 0, h = 0;
    QByteArray data;

    argument.beginStructure();
    argument >> w;
    argument >> h;
    argument >> data;
    argument.endStructure();

    // convert data to QPixmap
    if ( w && h && !data.isEmpty() )
    {
      if ( QSysInfo::ByteOrder == QSysInfo::LittleEndian )
      {
        quint32 *uintBuf = reinterpret_cast<quint32 *>(data.data());
        for (uint i = 0; i < data.size() / sizeof(quint32); ++i)
        {
          *uintBuf = qToBigEndian(*uintBuf);
          ++uintBuf;
        }
      }
      pixmap = QPixmap::fromImage(QImage(reinterpret_cast<const uchar *>(data.constData()), w, h, QImage::Format_ARGB32));
    }
  }

  return argument;
}

//--------------------------------------------------------------------------------

const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusImageVector &icon)
{
  icon = QIcon();

  if ( argument.currentType() == QDBusArgument::ArrayType )
  {
    argument.beginArray();

    while ( !argument.atEnd() )
    {
      KDbusImageStruct pixmap;
      argument >> pixmap;
      if ( !pixmap.isNull() )
        icon.addPixmap(pixmap);
    }

    argument.endArray();
  }
  return argument;
}

//--------------------------------------------------------------------------------

const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusToolTipStruct &tip)
{
  tip = KDbusToolTipStruct();

  if ( argument.currentType() == QDBusArgument::StructureType )
  {
    argument.beginStructure();
    argument >> tip.icon;
    argument >> tip.image;
    argument >> tip.title;
    argument >> tip.subTitle;
    argument.endStructure();
  }

  return argument;
}

//--------------------------------------------------------------------------------
// add "dummy" operators we never need since we'll never send icons, but qDBusRegisterMetaType needs them
// But we must correctly create the structure information

QDBusArgument &operator<<(QDBusArgument &argument, const KDbusImageStruct &pixmap)
{
  argument.beginStructure();
  argument << qint32(pixmap.width());
  argument << qint32(pixmap.height());
  argument << QByteArray();
  argument.endStructure();
  return argument;
}

//--------------------------------------------------------------------------------

QDBusArgument &operator<<(QDBusArgument &argument, const KDbusImageVector &)
{
  argument.beginArray(qMetaTypeId<KDbusImageStruct>());
  argument.endArray();
  return argument;
}

//--------------------------------------------------------------------------------

QDBusArgument &operator<<(QDBusArgument &argument, const KDbusToolTipStruct &tip)
{
  argument.beginStructure();
  argument << tip.icon;
  argument << tip.image;
  argument << tip.title;
  argument << tip.subTitle;
  argument.endStructure();
  return argument;
}

//--------------------------------------------------------------------------------

const QDBusArgument &operator<<(QDBusArgument &arg, const DBusMenuLayoutItem &item)
{
  arg.beginStructure();
  arg << item.m_id << item.m_properties;
  arg.beginArray(qMetaTypeId<QDBusVariant>());
  for (const DBusMenuLayoutItem &child : item.m_children)
    arg << QDBusVariant(QVariant::fromValue<DBusMenuLayoutItem>(child));
  arg.endArray();
  arg.endStructure();
  return arg;
}

//--------------------------------------------------------------------------------

const QDBusArgument &operator>>(const QDBusArgument &arg, DBusMenuLayoutItem &item)
{
  arg.beginStructure();
  arg >> item.m_id >> item.m_properties;
  arg.beginArray();
  while ( !arg.atEnd() )
  {
    QDBusVariant dbusVariant;
    arg >> dbusVariant;
    QDBusArgument childArgument = qvariant_cast<QDBusArgument>(dbusVariant.variant());

    DBusMenuLayoutItem child;
    childArgument >> child;
    item.m_children.append(child);
  }
  arg.endArray();
  arg.endStructure();
  return arg;
}

//--------------------------------------------------------------------------------
