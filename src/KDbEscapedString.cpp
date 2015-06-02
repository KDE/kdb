/* This file is part of the KDE project
   Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>

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

#include "KDbEscapedString.h"
#include "kdb_debug.h"

#include <QDataStream>

KDbEscapedString &KDbEscapedString::prepend(const KDbEscapedString &s)
{
    if (s.isValid()) {
        if (m_valid)
            QByteArray::prepend(s);
    }
    else {
        QByteArray::clear();
        m_valid = false;
    }
    return *this;
}

KDbEscapedString &KDbEscapedString::append(const KDbEscapedString &s)
{
    if (s.isValid()) {
        if (m_valid)
            QByteArray::append(s);
    }
    else {
        QByteArray::clear();
        m_valid = false;
    }
    return *this;
}

KDbEscapedString &KDbEscapedString::insert(int i, const KDbEscapedString &s)
{
    if (s.isValid()) {
        if (m_valid)
            QByteArray::insert(i, s);
    }
    else {
        QByteArray::clear();
        m_valid = false;
    }
    return *this;
}

KDbEscapedString &KDbEscapedString::replace(int index, int len, const KDbEscapedString &s)
{
    if (s.isValid()) {
        if (m_valid)
            QByteArray::replace(index, len, s);
    }
    else {
        QByteArray::clear();
        m_valid = false;
    }
    return *this;
}

KDbEscapedString &KDbEscapedString::replace(char before, const KDbEscapedString &after)
{
    if (after.isValid()) {
        if (m_valid)
            QByteArray::replace(before, after);
    }
    else {
        QByteArray::clear();
        m_valid = false;
    }
    return *this;
}

KDbEscapedString &KDbEscapedString::replace(const KDbEscapedString &before, const QByteArray &after)
{
    if (before.isValid()) {
        if (m_valid)
            QByteArray::replace(before, after);
    }
    else {
        QByteArray::clear();
        m_valid = false;
    }
    return *this;
}

KDbEscapedString &KDbEscapedString::replace(const QByteArray &before, const KDbEscapedString &after)
{
    if (after.isValid()) {
        if (m_valid)
            QByteArray::replace(before, after);
    }
    else {
        QByteArray::clear();
        m_valid = false;
    }
    return *this;
}

KDbEscapedString &KDbEscapedString::replace(const KDbEscapedString &before, const KDbEscapedString &after)
{
    if (before.isValid() && after.isValid()) {
        if (m_valid)
            QByteArray::replace(before, after);
    }
    else {
        QByteArray::clear();
        m_valid = false;
    }
    return *this;
}

QList<KDbEscapedString> KDbEscapedString::split(char sep) const
{
    QList<KDbEscapedString> result;
    foreach(const QByteArray& ba, QByteArray::split(sep))
        result.append(KDbEscapedString(ba));
    return result;
}

#ifndef QT_NO_DATASTREAM
KDB_EXPORT QDataStream& operator<<(QDataStream &stream, const KDbEscapedString &string)
{
    stream << string.isValid();
    if (string.isValid())
        stream << string.toByteArray();
    return stream;
}

KDB_EXPORT QDataStream& operator>>(QDataStream &stream, KDbEscapedString &string)
{
    bool valid;
    stream >> valid;
    if (valid) {
        QByteArray ba;
        stream >> ba;
        string = ba;
    }
    else {
        string = KDbEscapedString::invalid();
    }
    return stream;
}
#endif

short KDbEscapedString::toShort(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toShort(ok, base);
}

ushort KDbEscapedString::toUShort(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toUShort(ok, base);
}

int KDbEscapedString::toInt(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toInt(ok, base);
}

uint KDbEscapedString::toUInt(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toUInt(ok, base);
}

long KDbEscapedString::toLong(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toLong(ok, base);
}

ulong KDbEscapedString::toULong(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toULong(ok, base);
}

qlonglong KDbEscapedString::toLongLong(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toLongLong(ok, base);
}

qulonglong KDbEscapedString::toULongLong(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toULongLong(ok, base);
}

float KDbEscapedString::toFloat(bool *ok) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toFloat(ok);
}

double KDbEscapedString::toDouble(bool *ok) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toDouble(ok);
}

KDbEscapedString KDbEscapedString::arg(const KDbEscapedString &a1, const KDbEscapedString &a2) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a1.toString(), a2.toString()));
}

KDbEscapedString KDbEscapedString::arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString()));
}

KDbEscapedString KDbEscapedString::arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3,
                    const KDbEscapedString &a4) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid() || !a4.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString(), a4.toString()));
}

KDbEscapedString KDbEscapedString::arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3,
                    const KDbEscapedString &a4, const KDbEscapedString &a5) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid() || !a4.isValid() || !a5.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString(), a4.toString(),
                         a5.toString()));
}

KDbEscapedString KDbEscapedString::arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3,
                    const KDbEscapedString &a4, const KDbEscapedString &a5, const KDbEscapedString &a6) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid() || !a4.isValid() || !a5.isValid()
         || !a6.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString(), a4.toString(),
                         a5.toString(), a6.toString()));
}

KDbEscapedString KDbEscapedString::arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3,
                    const KDbEscapedString &a4, const KDbEscapedString &a5, const KDbEscapedString &a6,
                    const KDbEscapedString &a7) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid() || !a4.isValid() || !a5.isValid()
         || !a6.isValid() || !a7.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString(), a4.toString(),
                         a5.toString(), a6.toString(), a7.toString()));
}

KDbEscapedString KDbEscapedString::arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3,
                    const KDbEscapedString &a4, const KDbEscapedString &a5, const KDbEscapedString &a6,
                    const KDbEscapedString &a7, const KDbEscapedString &a8) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid() || !a4.isValid() || !a5.isValid()
         || !a6.isValid() || !a7.isValid() || !a8.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString(), a4.toString(),
                         a5.toString(), a6.toString(), a7.toString(), a8.toString()));
}

KDbEscapedString KDbEscapedString::arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3,
                    const KDbEscapedString &a4, const KDbEscapedString &a5, const KDbEscapedString &a6,
                    const KDbEscapedString &a7, const KDbEscapedString &a8, const KDbEscapedString &a9) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid() || !a4.isValid() || !a5.isValid()
         || !a6.isValid() || !a7.isValid() || !a8.isValid() || !a9.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString(), a4.toString(),
                         a5.toString(), a6.toString(), a7.toString(), a8.toString(), a9.toString()));
}

KDbEscapedString KDbEscapedString::arg(const KDbEscapedString &a, int fieldWidth, const QChar & fillChar) const
{
    if (!m_valid || !a.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a.toString(), fieldWidth, fillChar));
}

KDbEscapedString KDbEscapedString::arg(const QString &a, int fieldWidth, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a, fieldWidth, fillChar));
}

KDbEscapedString KDbEscapedString::arg(const QByteArray &a, int fieldWidth, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(QLatin1String(a), fieldWidth, fillChar));
}

KDbEscapedString KDbEscapedString::arg(int a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

KDbEscapedString KDbEscapedString::arg(uint a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

KDbEscapedString KDbEscapedString::arg(long a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

KDbEscapedString KDbEscapedString::arg(ulong a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

KDbEscapedString KDbEscapedString::arg(qlonglong a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

KDbEscapedString KDbEscapedString::arg(qulonglong a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

KDbEscapedString KDbEscapedString::arg(short a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

KDbEscapedString KDbEscapedString::arg(ushort a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

KDbEscapedString KDbEscapedString::arg(QChar a, int fieldWidth, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a, fieldWidth, fillChar));
}

KDbEscapedString KDbEscapedString::arg(char a, int fieldWidth, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a, fieldWidth, fillChar));
}

KDbEscapedString KDbEscapedString::arg(double a, int fieldWidth, char format, int precision, const QChar & fillChar) const
{
    if (!m_valid)
        return KDbEscapedString::invalid();
    return KDbEscapedString(toString().arg(a, fieldWidth, format, precision, fillChar));
}

KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbEscapedString& string)
{
    if (string.isValid())
        dbg.nospace() << "KDbEscapedString:" << string.toByteArray();
    else
        dbg.nospace() << "KDbEscapedString(INVALID)";
    return dbg.space();
}
