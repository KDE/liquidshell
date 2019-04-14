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
