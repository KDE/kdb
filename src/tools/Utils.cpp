/* This file is part of the KDE project
   Copyright (C) 2003-2007 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "Utils.h"

#include <QRegExp>
#include <QPainter>
#include <QImage>
#include <QIcon>
#include <QMetaProperty>
#include <QBitmap>
#include <QFocusEvent>
#include <QFile>
#include <QStyle>
#include <QtDebug>
#include <QCoreApplication>

#include <Predicate/Global.h>
#include "predicate_global.h"

using namespace Predicate::Utils;

void Predicate::Utils::serializeMap(const QMap<QString, QString>& map, QByteArray& array)
{
    QDataStream ds(&array, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds << map;
}

void Predicate::Utils::serializeMap(const QMap<QString, QString>& map, QString& string)
{
    QByteArray array;
    QDataStream ds(&array, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds << map;
    PreDbg << array[3] << array[4] << array[5];
    const uint size = array.size();
    string.clear();
    string.reserve(size);
    for (uint i = 0; i < size; i++) {
        string[i] = QChar(ushort(array[i]) + 1);
    }
}

QMap<QString, QString> Predicate::Utils::deserializeMap(const QByteArray& array)
{
    QMap<QString, QString> map;
    QByteArray ba(array);
    QDataStream ds(&ba, QIODevice::ReadOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds >> map;
    return map;
}

QMap<QString, QString> Predicate::Utils::deserializeMap(const QString& string)
{
    QByteArray array;
    const uint size = string.length();
    array.resize(size);
    for (uint i = 0; i < size; i++) {
        array[i] = char(string[i].unicode() - 1);
    }
    QMap<QString, QString> map;
    QDataStream ds(&array, QIODevice::ReadOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds >> map;
    return map;
}

QString Predicate::Utils::stringToFileName(const QString& string)
{
    QString _string(string);
    _string.replace(QRegExp(QLatin1String("[\\\\/:\\*?\"<>|]")), QLatin1String(" "));
    return _string.simplified();
}

void Predicate::Utils::simpleCrypt(QString& string)
{
    for (int i = 0; i < string.length(); i++)
        string[i] = QChar(string[i].unicode() + 47 + i);
}

void Predicate::Utils::simpleDecrypt(QString& string)
{
    for (int i = 0; i < string.length(); i++)
        string[i] = QChar(string[i].unicode() - 47 - i);
}

QString Predicate::Utils::ptrToStringInternal(void* ptr, uint size)
{
    QString str;
    unsigned char* cstr_ptr = (unsigned char*) & ptr;
    for (uint i = 0; i < size; i++) {
        QString s;
        s.sprintf("%2.2x", cstr_ptr[i]);
        str.append(s);
    }
    return str;
}

void* Predicate::Utils::stringToPtrInternal(const QString& str, uint size)
{
    if ((str.length() / 2) < (int)size)
        return 0;
    QByteArray array;
    array.resize(size);
    bool ok;
    for (uint i = 0; i < size; i++) {
        array[i] = (unsigned char)(str.mid(i * 2, 2).toUInt(&ok, 16));
        if (!ok)
            return 0;
    }
    return *(void**)(array.data());
}

//---------

//! @internal
class StaticSetOfStrings::Private
{
public:
    Private() : array(0), set(0) {}
    ~Private() {
        delete set;
    }
    const char** array;
    QSet<QByteArray> *set;
};

StaticSetOfStrings::StaticSetOfStrings()
        : d(new Private)
{
}

StaticSetOfStrings::StaticSetOfStrings(const char* array[])
        : d(new Private)
{
    setStrings(array);
}

StaticSetOfStrings::~StaticSetOfStrings()
{
    delete d;
}

void StaticSetOfStrings::setStrings(const char* array[])
{
    delete d->set;
    d->set = 0;
    d->array = array;
}

bool StaticSetOfStrings::isEmpty() const
{
    return d->array == 0;
}

bool StaticSetOfStrings::contains(const QByteArray& string) const
{
    if (!d->set) {
        d->set = new QSet<QByteArray>();
        for (const char ** p = d->array;*p;p++)
            d->set->insert(QByteArray::fromRawData(*p, qstrlen(*p)));
    }
    return d->set->contains(string);
}
