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

#include "EscapedString.h"

#include <QDataStream>
#include <QtDebug>

using namespace Predicate;

EscapedString &EscapedString::prepend(const EscapedString &s)
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

EscapedString &EscapedString::append(const EscapedString &s)
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

EscapedString &EscapedString::insert(int i, const EscapedString &s)
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

EscapedString &EscapedString::replace(int index, int len, const EscapedString &s)
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

EscapedString &EscapedString::replace(char before, const EscapedString &after)
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

EscapedString &EscapedString::replace(const EscapedString &before, const QByteArray &after)
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

EscapedString &EscapedString::replace(const QByteArray &before, const EscapedString &after)
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

EscapedString &EscapedString::replace(const EscapedString &before, const EscapedString &after)
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

QList<EscapedString> EscapedString::split(char sep) const
{
    QList<EscapedString> result;
    foreach(const QByteArray& ba, QByteArray::split(sep))
        result.append(EscapedString(ba));
    return result;
}

#ifndef QT_NO_DATASTREAM
PREDICATE_EXPORT QDataStream& operator<<(QDataStream &stream, const EscapedString &string)
{
    stream << string.isValid();
    if (string.isValid())
        stream << string.toByteArray();
    return stream;
}

PREDICATE_EXPORT QDataStream& operator>>(QDataStream &stream, EscapedString &string)
{
    bool valid;
    stream >> valid;
    if (valid) {
        QByteArray ba;
        stream >> ba;
        string = ba;
    }
    else {
        string = EscapedString::invalid();
    }
    return stream;
}
#endif

short EscapedString::toShort(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toShort(ok, base);
}

ushort EscapedString::toUShort(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toUShort(ok, base);
}

int EscapedString::toInt(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toInt(ok, base);
}

uint EscapedString::toUInt(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toUInt(ok, base);
}

long EscapedString::toLong(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toLong(ok, base);
}

ulong EscapedString::toULong(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toULong(ok, base);
}

qlonglong EscapedString::toLongLong(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toLongLong(ok, base);
}

qulonglong EscapedString::toULongLong(bool *ok, int base) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toULongLong(ok, base);
}

float EscapedString::toFloat(bool *ok) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toFloat(ok);
}

double EscapedString::toDouble(bool *ok) const
{
    if (!checkValid(ok))
        return 0;
    return QByteArray::toDouble(ok);
}

EscapedString EscapedString::arg(const EscapedString &a1, const EscapedString &a2) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid())
        return EscapedString::invalid();
    return EscapedString(toString().arg(a1.toString(), a2.toString()));
}

EscapedString EscapedString::arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid())
        return EscapedString::invalid();
    return EscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString()));
}

EscapedString EscapedString::arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3,
                    const EscapedString &a4) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid() || !a4.isValid())
        return EscapedString::invalid();
    return EscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString(), a4.toString()));
}

EscapedString EscapedString::arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3,
                    const EscapedString &a4, const EscapedString &a5) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid() || !a4.isValid() || !a5.isValid())
        return EscapedString::invalid();
    return EscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString(), a4.toString(),
                         a5.toString()));
}

EscapedString EscapedString::arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3,
                    const EscapedString &a4, const EscapedString &a5, const EscapedString &a6) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid() || !a4.isValid() || !a5.isValid()
         || !a6.isValid())
        return EscapedString::invalid();
    return EscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString(), a4.toString(),
                         a5.toString(), a6.toString()));
}

EscapedString EscapedString::arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3,
                    const EscapedString &a4, const EscapedString &a5, const EscapedString &a6,
                    const EscapedString &a7) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid() || !a4.isValid() || !a5.isValid()
         || !a6.isValid() || !a7.isValid())
        return EscapedString::invalid();
    return EscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString(), a4.toString(),
                         a5.toString(), a6.toString(), a7.toString()));
}

EscapedString EscapedString::arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3,
                    const EscapedString &a4, const EscapedString &a5, const EscapedString &a6,
                    const EscapedString &a7, const EscapedString &a8) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid() || !a4.isValid() || !a5.isValid()
         || !a6.isValid() || !a7.isValid() || !a8.isValid())
        return EscapedString::invalid();
    return EscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString(), a4.toString(),
                         a5.toString(), a6.toString(), a7.toString(), a8.toString()));
}

EscapedString EscapedString::arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3,
                    const EscapedString &a4, const EscapedString &a5, const EscapedString &a6,
                    const EscapedString &a7, const EscapedString &a8, const EscapedString &a9) const
{
    if (!m_valid || !a1.isValid() || !a2.isValid() || !a3.isValid() || !a4.isValid() || !a5.isValid()
         || !a6.isValid() || !a7.isValid() || !a8.isValid() || !a9.isValid())
        return EscapedString::invalid();
    return EscapedString(toString().arg(a1.toString(), a2.toString(), a3.toString(), a4.toString(),
                         a5.toString(), a6.toString(), a7.toString(), a8.toString(), a9.toString()));
}

EscapedString EscapedString::arg(const EscapedString &a, int fieldWidth, const QChar & fillChar) const
{
    if (!m_valid || !a.isValid())
        return EscapedString::invalid();
    return EscapedString(toString().arg(a.toString(), fieldWidth, fillChar));
}

EscapedString EscapedString::arg(const QString &a, int fieldWidth, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(a, fieldWidth, fillChar));
}

EscapedString EscapedString::arg(const QByteArray &a, int fieldWidth, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(QString(a), fieldWidth, fillChar));
}

EscapedString EscapedString::arg(int a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

EscapedString EscapedString::arg(uint a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

EscapedString EscapedString::arg(long a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

EscapedString EscapedString::arg(ulong a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

EscapedString EscapedString::arg(qlonglong a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

EscapedString EscapedString::arg(qulonglong a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

EscapedString EscapedString::arg(short a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

EscapedString EscapedString::arg(ushort a, int fieldWidth, int base, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(a, fieldWidth, base, fillChar));
}

EscapedString EscapedString::arg(QChar a, int fieldWidth, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(a, fieldWidth, fillChar));
}

EscapedString EscapedString::arg(char a, int fieldWidth, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(a, fieldWidth, fillChar));
}

EscapedString EscapedString::arg(double a, int fieldWidth, char format, int precision, const QChar & fillChar) const
{
    if (!m_valid)
        return EscapedString::invalid();
    return EscapedString(toString().arg(a, fieldWidth, format, precision, fillChar));
}

PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Predicate::EscapedString& string)
{
    if (string.isValid())
        dbg.nospace() << "Predicate::EscapedString:" << string.toByteArray();
    else
        dbg.nospace() << "Predicate::EscapedString(INVALID)";
    return dbg.space();
}
