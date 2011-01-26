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

#ifndef PREDICATE_ESCAPEDSTRING_H
#define PREDICATE_ESCAPEDSTRING_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QList>

#include <Predicate/predicate_export.h>

namespace Predicate
{

//! Specialized string for escaping.
//! In addition to byte array, contains "validity" flag that is transferred
//! when strings are concatenated or in general when any operation with invalid
//! escaped string is performed.
class PREDICATE_EXPORT EscapedString : protected QByteArray
{
public:
    inline EscapedString() : m_valid(true) {}

    explicit inline EscapedString(char ch)
        : QByteArray(&ch, 1), m_valid(true) {}

    explicit inline EscapedString(const char* string)
        : QByteArray(string), m_valid(true) {}

    explicit inline EscapedString(const QByteArray& string)
        : QByteArray(string), m_valid(true) {}

    explicit inline EscapedString(const QString& string)
        : QByteArray(string.toUtf8()), m_valid(true) {}

    inline EscapedString(const EscapedString& string)
        : QByteArray(string), m_valid(string.isValid()) {}

    //! @internal used to create invalid string
    explicit inline EscapedString(bool) : m_valid(false) {}

    inline ~EscapedString() {}

    //! @return invalid escaped string.
    static inline EscapedString invalid() { return EscapedString(false); }

    //! @return true is the string valid. Valid string means that the escaping process has finished
    //! successfully; it does not mean that the statement itself parses or executed without errors.
    inline bool isValid() const { return m_valid; }

    inline QByteArray toByteArray() const { return static_cast<const QByteArray&>(*this); }

    inline QString toString() const {
        return QString::fromUtf8(static_cast<const QByteArray&>(*this), length());
    }

    inline EscapedString &operator=(const EscapedString& string) {
        QByteArray::operator=(string);
        m_valid = string.isValid();
        return *this;
    }
    inline EscapedString &operator=(const QByteArray& string) {
        QByteArray::operator=(string);
        m_valid = true;
        return *this;
    }
    inline EscapedString &operator=(const char *string) {
        QByteArray::operator=(string);
        m_valid = true;
        return *this;
    }

    inline bool operator==(const EscapedString &other) const
    {
        return isValid() == other.isValid()
               && static_cast<const QByteArray&>(*this) == other.toByteArray();
    }
    //inline bool operator==(const QString &s2) const;

    inline int size() const { return QByteArray::size(); }
    inline bool isEmpty() const { return QByteArray::isEmpty(); }
    inline void resize(int size) { QByteArray::resize(size); }

    inline EscapedString &fill(char c, int size = -1) {
        m_valid = true;
        QByteArray::fill(c, size);
        return *this;
    }

    inline int capacity() const { return QByteArray::isEmpty(); }
    inline void reserve(int size) { QByteArray::reserve(size); }
    inline void squeeze() { QByteArray::squeeze(); }

    inline char *data() { return QByteArray::data(); }
    inline const char *data() const { return QByteArray::data(); }
    inline const char *constData() const { return QByteArray::constData(); }
    inline void clear() { m_valid = true; QByteArray::clear(); }

#ifdef Q_COMPILER_MANGLES_RETURN_TYPE
    inline const char at(int i) const { return QByteArray::at(i); }
    inline const char operator[](int i) const { return QByteArray::operator[](i); }
    inline const char operator[](uint i) const { return QByteArray::operator[](i); }
#else
    inline char at(int i) const { return QByteArray::at(i); }
    inline char operator[](int i) const { return QByteArray::operator[](i); }
    inline char operator[](uint i) const { return QByteArray::operator[](i); }
#endif
    inline QByteRef operator[](int i) { return QByteArray::operator[](i); }
    inline QByteRef operator[](uint i) { return QByteArray::operator[](i); }

    inline int indexOf(char c, int from = 0) const { return QByteArray::indexOf(c, from); }
    inline int indexOf(const char *c, int from = 0) const { return QByteArray::indexOf(c, from); }
    inline int indexOf(const QByteArray &a, int from = 0) const { return QByteArray::indexOf(a, from); }
    inline int indexOf(const EscapedString &s, int from = 0) const {
        return s.isValid() ? QByteArray::indexOf(s, from) : -1;
    }
    inline int lastIndexOf(char c, int from = -1) const { return QByteArray::lastIndexOf(c, from); }
    inline int lastIndexOf(const char *c, int from = -1) const { return QByteArray::lastIndexOf(c, from); }
    inline int lastIndexOf(const QByteArray &a, int from = -1) const { return QByteArray::lastIndexOf(a, from); }
    inline int lastIndexOf(const EscapedString &s, int from = 0) const {
        return s.isValid() ? QByteArray::lastIndexOf(s, from) : -1;
    }

    inline QBool contains(char c) const { return QByteArray::contains(c); }
    inline QBool contains(const char *a) const { return QByteArray::contains(a); }
    inline QBool contains(const QByteArray &a) const { return QByteArray::contains(a); }
    inline QBool contains(const EscapedString &s) const {
        return s.isValid() ? QByteArray::contains(s) : QBool(false);
    }
    inline int count(char c) const { return QByteArray::count(c); }
    inline int count(const char *a) const { return QByteArray::count(a); }
    inline int count(const QByteArray &a) const { return QByteArray::count(a); }
    inline int count(const EscapedString &s) const {
        return s.isValid() ? QByteArray::count(s) : -1;
    }

    inline EscapedString left(int len) const {
        return m_valid ? EscapedString(QByteArray::left(len)) : EscapedString::invalid();
    }
    inline EscapedString right(int len) const {
        return m_valid ? EscapedString(QByteArray::right(len)) : EscapedString::invalid();
    }
    inline EscapedString mid(int index, int len = -1) const {
        return m_valid ? EscapedString(QByteArray::mid(index, len)) : EscapedString::invalid();
    }

    inline bool startsWith(const EscapedString &s) const {
        return (m_valid && s.isValid()) ? QByteArray::startsWith(s) : false;
    }
    inline bool startsWith(const QByteArray &a) const {
        return m_valid ? QByteArray::startsWith(a) : false;
    }
    inline bool startsWith(char c) const {
        return m_valid ? QByteArray::startsWith(c) : false;
    }
    inline bool startsWith(const char *c) const {
        return m_valid ? QByteArray::startsWith(c) : false;
    }

    inline bool endsWith(const EscapedString &s) const {
        return (m_valid && s.isValid()) ? QByteArray::endsWith(s) : false;
    }
    inline bool endsWith(const QByteArray &a) const {
        return m_valid ? QByteArray::endsWith(a) : false;
    }
    inline bool endsWith(char c) const {
        return m_valid ? QByteArray::endsWith(c) : false;
    }
    inline bool endsWith(const char *c) const {
        return m_valid ? QByteArray::endsWith(c) : false;
    }

    inline void truncate(int pos) { QByteArray::truncate(pos); }
    inline void chop(int n) { QByteArray::chop(n); }

    inline EscapedString toLower() const {
        return m_valid ? EscapedString(QByteArray::toLower()) : EscapedString::invalid();
    }
    inline EscapedString toUpper() const {
        return m_valid ? EscapedString(QByteArray::toUpper()) : EscapedString::invalid();
    }

    inline EscapedString trimmed() const {
        return m_valid ? EscapedString(QByteArray::trimmed()) : EscapedString::invalid();
    }
    inline EscapedString simplified() const {
        return m_valid ? EscapedString(QByteArray::simplified()) : EscapedString::invalid();
    }
    inline EscapedString leftJustified(int width, char fill = ' ', bool truncate = false) const
    {
        return m_valid ? EscapedString(QByteArray::leftJustified(width, fill, truncate)) : EscapedString::invalid();
    }
    inline EscapedString rightJustified(int width, char fill = ' ', bool truncate = false) const
    {
        return m_valid ? EscapedString(QByteArray::rightJustified(width, fill, truncate)) : EscapedString::invalid();
    }

    inline EscapedString &prepend(char c) {
        if (m_valid)
            QByteArray::prepend(c);
        return *this;
    }
    inline EscapedString &prepend(const char *s) {
        if (m_valid)
            QByteArray::prepend(s);
        return *this;
    }
    inline EscapedString &prepend(const QByteArray &a) {
        if (m_valid)
            QByteArray::prepend(a);
        return *this;
    }
    EscapedString &prepend(const EscapedString &s);
    inline EscapedString &append(char c) {
        if (m_valid)
            QByteArray::append(c);
        return *this;
    }
    inline EscapedString &append(const char *s) {
        if (m_valid)
            QByteArray::append(s);
        return *this;
    }
    inline EscapedString &append(const char *s, int len) {
        if (m_valid)
            QByteArray::append(s, len);
        return *this;
    }
    inline EscapedString &append(const QByteArray &a) {
        if (m_valid)
            QByteArray::append(a);
        return *this;
    }
    inline EscapedString &append(const QString &a) {
        if (m_valid)
            QByteArray::append(a.toUtf8());
        return *this;
    }
    EscapedString &append(const EscapedString &s);
    inline EscapedString &insert(int i, char c) {
        if (m_valid)
            QByteArray::insert(i, c);
        return *this;
    }
    inline EscapedString &insert(int i, const char *s) {
        if (m_valid)
            QByteArray::insert(i, s);
        return *this;
    }
    inline EscapedString &insert(int i, const QByteArray &a) {
        if (m_valid)
            QByteArray::insert(i, a);
        return *this;
    }
    EscapedString &insert(int i, const EscapedString &s);
    inline EscapedString &remove(int index, int len) {
        if (m_valid)
            QByteArray::remove(index, len);
        return *this;
    }
    inline EscapedString &replace(int index, int len, const char *s) {
        if (m_valid)
            QByteArray::replace(index, len, s);
        return *this;
    }
    inline EscapedString &replace(int index, int len, const QByteArray &s) {
        if (m_valid)
            QByteArray::replace(index, len, s);
        return *this;
    }
    EscapedString &replace(int index, int len, const EscapedString &s);
    inline EscapedString &replace(char before, const char *after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    inline EscapedString &replace(char before, const QByteArray &after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    EscapedString &replace(char before, const EscapedString &after);
    inline EscapedString &replace(const char *before, const char *after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    inline EscapedString &replace(const char *before, int bsize, const char *after, int asize) {
        if (m_valid)
            QByteArray::replace(before, bsize, after, asize);
        return *this;
    }
    inline EscapedString &replace(const QByteArray &before, const QByteArray &after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    EscapedString &replace(const EscapedString &before, const QByteArray &after);
    EscapedString &replace(const QByteArray &before, const EscapedString &after);
    EscapedString &replace(const EscapedString &before, const EscapedString &after);
    inline EscapedString &replace(const QByteArray &before, const char *after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    inline EscapedString &replace(const char *before, const QByteArray &after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    inline EscapedString &replace(char before, char after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    inline EscapedString &operator+=(char c) { return append(c); }
    inline EscapedString &operator+=(const char *s) { return append(s); }
    inline EscapedString &operator+=(const QByteArray &a) { return append(a); }
    inline EscapedString &operator+=(const QString &a) { return append(a); }
    inline EscapedString &operator+=(const EscapedString &s) { return append(s); }

    //EscapedString operator+(const QVector<T> & other ) const

    QList<EscapedString> split(char sep) const;

    inline EscapedString repeated(int times) const {
        return m_valid ? EscapedString(QByteArray::repeated(times)) : EscapedString::invalid();
    }
    short toShort(bool *ok = 0, int base = 10) const;
    ushort toUShort(bool *ok = 0, int base = 10) const;
    int toInt(bool *ok = 0, int base = 10) const;
    uint toUInt(bool *ok = 0, int base = 10) const;
    long toLong(bool *ok = 0, int base = 10) const;
    ulong toULong(bool *ok = 0, int base = 10) const;
    qlonglong toLongLong(bool *ok = 0, int base = 10) const;
    qulonglong toULongLong(bool *ok = 0, int base = 10) const;
    float toFloat(bool *ok = 0) const;
    double toDouble(bool *ok = 0) const;
    inline EscapedString toBase64() const {
        return m_valid ? EscapedString(QByteArray::toBase64()) : EscapedString::invalid();
    }
    inline EscapedString toHex() const {
        return m_valid ? EscapedString(QByteArray::toHex()) : EscapedString::invalid();
    }
    inline EscapedString toPercentEncoding(const QByteArray &exclude = QByteArray(),
                                 const QByteArray &include = QByteArray(),
                                 char percent = '%') const
    {
        return m_valid ? EscapedString(QByteArray::toPercentEncoding(exclude, include))
                       : EscapedString::invalid();
    }

    inline EscapedString &setNum(short val, int base = 10) {
        m_valid = true;
        QByteArray::setNum(val, base);
        return *this;
    }
    inline EscapedString &setNum(ushort val, int base = 10) {
        m_valid = true;
        QByteArray::setNum(val, base);
        return *this;
    }
    inline EscapedString &setNum(int val, int base = 10) {
        m_valid = true;
        QByteArray::setNum(val, base);
        return *this;
    }
    inline EscapedString &setNum(uint val, int base = 10) {
        m_valid = true;
        QByteArray::setNum(val, base);
        return *this;
    }
    inline EscapedString &setNum(qlonglong val, int base = 10) {
        m_valid = true;
        QByteArray::setNum(val, base);
        return *this;
    }
    inline EscapedString &setNum(qulonglong val, int base = 10) {
        m_valid = true;
        QByteArray::setNum(val, base);
        return *this;
    }
    inline EscapedString &setNum(float val, char f = 'g', int prec = 6) {
        m_valid = true;
        QByteArray::setNum(val, f, prec);
        return *this;
    }
    inline EscapedString &setNum(double val, char f = 'g', int prec = 6) {
        m_valid = true;
        QByteArray::setNum(val, f, prec);
        return *this;
    }

    static inline EscapedString number(int val, int base = 10) {
        return EscapedString(QByteArray::number(val, base));
    }
    static inline EscapedString number(uint val, int base = 10) {
        return EscapedString(QByteArray::number(val, base));
    }
    static inline EscapedString number(qlonglong val, int base = 10) {
        return EscapedString(QByteArray::number(val, base));
    }
    static inline EscapedString number(qulonglong val, int base = 10) {
        return EscapedString(QByteArray::number(val, base));
    }
    static inline EscapedString number(double val, char f = 'g', int prec = 6) {
        return EscapedString(QByteArray::number(val, f, prec));
    }
    static inline EscapedString fromRawData(const char *s, int size) {
        return EscapedString(QByteArray::fromRawData(s, size));
    }
    static inline EscapedString fromBase64(const QByteArray &base64) {
        return EscapedString(QByteArray::fromBase64(base64));
    }
    static inline EscapedString fromBase64(const EscapedString &base64) {
        return base64.isValid() ? EscapedString(QByteArray::fromBase64(base64)) : EscapedString::invalid();
    }
    static inline EscapedString fromHex(const QByteArray &hexEncoded) {
        return EscapedString(QByteArray::fromHex(hexEncoded));
    }
    static inline EscapedString fromHex(const EscapedString &hexEncoded) {
        return hexEncoded.isValid() ? EscapedString(QByteArray::fromHex(hexEncoded))
                                    : EscapedString::invalid();
    }
    static inline EscapedString fromPercentEncoding(const QByteArray &pctEncoded, char percent = '%') {
        return EscapedString(QByteArray::fromPercentEncoding(pctEncoded, percent));
    }
    static inline EscapedString fromPercentEncoding(const EscapedString &pctEncoded, char percent = '%') {
        return pctEncoded.isValid() ? EscapedString(QByteArray::fromPercentEncoding(pctEncoded, percent))
                                    : EscapedString::invalid();
    }

    inline int count() const { return QByteArray::count(); }
    inline int length() const { return QByteArray::length(); }
    inline bool isNull() const { return QByteArray::isNull(); }

    EscapedString arg(const EscapedString &a1, const EscapedString &a2) const;
    EscapedString arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3) const;
    EscapedString arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3,
                      const EscapedString &a4) const;
    EscapedString arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3,
                      const EscapedString &a4, const EscapedString &a5) const;
    EscapedString arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3,
                      const EscapedString &a4, const EscapedString &a5, const EscapedString &a6) const;
    EscapedString arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3,
                      const EscapedString &a4, const EscapedString &a5, const EscapedString &a6,
                      const EscapedString &a7) const;
    EscapedString arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3,
                      const EscapedString &a4, const EscapedString &a5, const EscapedString &a6,
                      const EscapedString &a7, const EscapedString &a8) const;
    EscapedString arg(const EscapedString &a1, const EscapedString &a2, const EscapedString &a3,
                      const EscapedString &a4, const EscapedString &a5, const EscapedString &a6,
                      const EscapedString &a7, const EscapedString &a8, const EscapedString &a9) const;
    EscapedString arg(const EscapedString &a, int fieldWidth = 0, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(const QString &a, int fieldWidth = 0, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(const QByteArray &a, int fieldWidth = 0, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(int a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(uint a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(long a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(ulong a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(qlonglong a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(qulonglong a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(short a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(ushort a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(QChar a, int fieldWidth = 0, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(char a, int fieldWidth = 0, const QChar & fillChar = QLatin1Char( ' ' )) const;
    EscapedString arg(double a, int fieldWidth = 0, char format = 'g', int precision = -1, const QChar & fillChar = QLatin1Char( ' ' )) const;

    typedef QByteArray::DataPtr DataPtr;
    inline DataPtr &data_ptr() { return QByteArray::data_ptr(); }

private:
    //! Helper for methods having "bool* ok" argument
    inline bool checkValid(bool *ok) const {
        if (!m_valid) {
            if (ok)
                *ok = false;
            return false;
        }
        return true;
    }
    //! true if the string is valid; true by default
    bool m_valid;
};

#ifndef QT_NO_DATASTREAM
PREDICATE_EXPORT QDataStream &operator<<(QDataStream &stream, const EscapedString &string);
PREDICATE_EXPORT QDataStream &operator>>(QDataStream &stream, EscapedString &string);
#endif

inline EscapedString operator+(const EscapedString &a1, const EscapedString &a2)
{
    if (!a1.isValid() || !a2.isValid())
        return EscapedString::invalid();
    return EscapedString(a1.toByteArray() + a2.toByteArray());
}
inline EscapedString operator+(const EscapedString &a1, const QString &a2)
{
    if (!a1.isValid())
        return EscapedString::invalid();
    return EscapedString(a1.toByteArray() + a2.toUtf8());
}
inline EscapedString operator+(const EscapedString &a1, const QByteArray &a2)
{
    if (!a1.isValid())
        return EscapedString::invalid();
    return EscapedString(a1.toByteArray() + a2);
}
inline EscapedString operator+(const EscapedString &a1, const char* a2)
{
    if (!a1.isValid())
        return EscapedString::invalid();
    return EscapedString(a1.toByteArray() + a2);
}
inline EscapedString operator+(const EscapedString &a1, char a2)
{
    if (!a1.isValid())
        return EscapedString::invalid();
    return EscapedString(a1.toByteArray() + a2);
}
inline EscapedString operator+(const QString &a1, const EscapedString &a2)
{
    if (!a2.isValid())
        return EscapedString::invalid();
    return EscapedString(a1.toLatin1() + a2.toByteArray());
}
inline EscapedString operator+(const QByteArray &a1, const EscapedString &a2)
{
    if (!a2.isValid())
        return EscapedString::invalid();
    return EscapedString(a1 + a2.toByteArray());
}
inline EscapedString operator+(const char* a1, const EscapedString &a2)
{
    if (!a2.isValid())
        return EscapedString::invalid();
    return EscapedString(a1 + a2.toByteArray());
}
inline EscapedString operator+(char a1, const EscapedString &a2)
{
    if (!a2.isValid())
        return EscapedString::invalid();
    return EscapedString(a1 + a2.toByteArray());
}

inline bool operator==(const EscapedString &a1, const EscapedString &a2)
{
    return a1.isValid() == a2.isValid() && a1.toByteArray() == a2.toByteArray();
}

inline bool operator==(const EscapedString &a1, const QByteArray &a2)
{
    return a1.isValid() && a1.toByteArray() == a2;
}

inline bool operator==(const EscapedString &a1, const char *a2)
{
    return a1.isValid() && a1.toByteArray() == a2;
}

inline bool operator==(const QByteArray &a1, const EscapedString &a2)
{
    return a2.isValid() && a1 == a2.toByteArray();
}

inline bool operator==(const char *a1, const EscapedString &a2)
{
    return a2.isValid() && a1 == a2.toByteArray();
}

} //namespace

#if 0
(a1.size() == a2.size()) && (memcmp(a1.constData(), a2.constData(), a1.size())==0); }
inline bool operator==(const QByteArray &a1, const char *a2)
{ return a2 ? qstrcmp(a1,a2) == 0 : a1.isEmpty(); }
inline bool operator==(const char *a1, const QByteArray &a2)
{ return a1 ? qstrcmp(a1,a2) == 0 : a2.isEmpty(); }
inline bool operator!=(const QByteArray &a1, const QByteArray &a2)
{ return !(a1==a2); }
inline bool operator!=(const QByteArray &a1, const char *a2)
{ return a2 ? qstrcmp(a1,a2) != 0 : !a1.isEmpty(); }
inline bool operator!=(const char *a1, const QByteArray &a2)
{ return a1 ? qstrcmp(a1,a2) != 0 : !a2.isEmpty(); }
inline bool operator<(const QByteArray &a1, const QByteArray &a2)
{ return qstrcmp(a1, a2) < 0; }
 inline bool operator<(const QByteArray &a1, const char *a2)
{ return qstrcmp(a1, a2) < 0; }
inline bool operator<(const char *a1, const QByteArray &a2)
{ return qstrcmp(a1, a2) < 0; }
inline bool operator<=(const QByteArray &a1, const QByteArray &a2)
{ return qstrcmp(a1, a2) <= 0; }
inline bool operator<=(const QByteArray &a1, const char *a2)
{ return qstrcmp(a1, a2) <= 0; }
inline bool operator<=(const char *a1, const QByteArray &a2)
{ return qstrcmp(a1, a2) <= 0; }
inline bool operator>(const QByteArray &a1, const QByteArray &a2)
{ return qstrcmp(a1, a2) > 0; }
inline bool operator>(const QByteArray &a1, const char *a2)
{ return qstrcmp(a1, a2) > 0; }
inline bool operator>(const char *a1, const QByteArray &a2)
{ return qstrcmp(a1, a2) > 0; }
inline bool operator>=(const QByteArray &a1, const QByteArray &a2)
{ return qstrcmp(a1, a2) >= 0; }
inline bool operator>=(const QByteArray &a1, const char *a2)
{ return qstrcmp(a1, a2) >= 0; }
inline bool operator>=(const char *a1, const QByteArray &a2)
{ return qstrcmp(a1, a2) >= 0; }
#endif
#if 0
inline const QByteArray operator+(const QByteArray &a1, const QByteArray &a2)
{ return QByteArray(a1) += a2; }
inline const QByteArray operator+(const QByteArray &a1, const char *a2)
{ return QByteArray(a1) += a2; }
inline const QByteArray operator+(const QByteArray &a1, char a2)
{ return QByteArray(a1) += a2; }
inline const QByteArray operator+(const char *a1, const QByteArray &a2)
{ return QByteArray(a1) += a2; }
inline const QByteArray operator+(char a1, const QByteArray &a2)
{ return QByteArray(&a1, 1) += a2; }
#endif
Q_DECLARE_TYPEINFO(Predicate::EscapedString, Q_MOVABLE_TYPE);
//Q_DECLARE_SHARED(Predicate::EscapedString)

//! Sends escaped string @a string to debug output @a dbg.
PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Predicate::EscapedString& string);

#endif
