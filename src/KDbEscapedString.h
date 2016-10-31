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

#ifndef KDB_ESCAPEDSTRING_H
#define KDB_ESCAPEDSTRING_H

#include <QByteArray>
#include <QString>
#include <QList>

#include "kdb_export.h"

//! Specialized string for escaping.
//! In addition to byte array, contains "validity" flag that is transferred
//! when strings are concatenated or in general when any operation with invalid
//! escaped string is performed.
class KDB_EXPORT KDbEscapedString : protected QByteArray
{
public:
    inline KDbEscapedString() : m_valid(true) {}

    explicit inline KDbEscapedString(char ch)
        : QByteArray(1, ch), m_valid(true) {}

    explicit inline KDbEscapedString(QChar ch)
        : QByteArray(1, ch.toLatin1()), m_valid(true) {}

    explicit inline KDbEscapedString(const char* string)
        : QByteArray(string), m_valid(true) {}

    explicit inline KDbEscapedString(const QByteArray& string)
        : QByteArray(string), m_valid(true) {}

    explicit inline KDbEscapedString(const QString& string)
        : QByteArray(string.toUtf8()), m_valid(true) {}

    inline KDbEscapedString(const KDbEscapedString& string)
        : QByteArray(string), m_valid(string.isValid()) {}

    inline ~KDbEscapedString() {}

    //! @return invalid escaped string.
    static inline KDbEscapedString invalid() { return KDbEscapedString(false); }

    //! @return true if the string is valid. Valid string means that the escaping process
    //! has finished successfully. It does not mean that the statement itself parses
    //! or can be executed without errors.
    inline bool isValid() const { return m_valid; }

    inline QByteArray toByteArray() const { return static_cast<const QByteArray&>(*this); }

    inline QString toString() const {
        return QString::fromUtf8(static_cast<const QByteArray&>(*this).constData(), length());
    }

    inline KDbEscapedString &operator=(const KDbEscapedString& string) {
        QByteArray::operator=(string);
        m_valid = string.isValid();
        return *this;
    }
    inline KDbEscapedString &operator=(const QByteArray& string) {
        QByteArray::operator=(string);
        m_valid = true;
        return *this;
    }
    inline KDbEscapedString &operator=(const char *string) {
        QByteArray::operator=(string);
        m_valid = true;
        return *this;
    }

    inline bool operator==(const KDbEscapedString &other) const
    {
        return isValid() == other.isValid()
               && static_cast<const QByteArray&>(*this) == other.toByteArray();
    }
    inline int size() const { return QByteArray::size(); }
    inline bool isEmpty() const { return QByteArray::isEmpty(); }
    inline void resize(int size) { QByteArray::resize(size); }

    inline KDbEscapedString &fill(char c, int size = -1) {
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
    inline int indexOf(const KDbEscapedString &s, int from = 0) const {
        return s.isValid() ? QByteArray::indexOf(s, from) : -1;
    }
    inline int lastIndexOf(char c, int from = -1) const { return QByteArray::lastIndexOf(c, from); }
    inline int lastIndexOf(const char *c, int from = -1) const { return QByteArray::lastIndexOf(c, from); }
    inline int lastIndexOf(const QByteArray &a, int from = -1) const { return QByteArray::lastIndexOf(a, from); }
    inline int lastIndexOf(const KDbEscapedString &s, int from = 0) const {
        return s.isValid() ? QByteArray::lastIndexOf(s, from) : -1;
    }

    inline int count(char c) const { return QByteArray::count(c); }
    inline int count(const char *a) const { return QByteArray::count(a); }
    inline int count(const QByteArray &a) const { return QByteArray::count(a); }
    inline int count(const KDbEscapedString &s) const {
        return s.isValid() ? QByteArray::count(s) : -1;
    }

    inline KDbEscapedString left(int len) const {
        return m_valid ? KDbEscapedString(QByteArray::left(len)) : KDbEscapedString::invalid();
    }
    inline KDbEscapedString right(int len) const {
        return m_valid ? KDbEscapedString(QByteArray::right(len)) : KDbEscapedString::invalid();
    }
    inline KDbEscapedString mid(int index, int len = -1) const {
        return m_valid ? KDbEscapedString(QByteArray::mid(index, len)) : KDbEscapedString::invalid();
    }

    inline bool startsWith(const KDbEscapedString &s) const {
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

    inline bool endsWith(const KDbEscapedString &s) const {
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

    inline KDbEscapedString toLower() const {
        return m_valid ? KDbEscapedString(QByteArray::toLower()) : KDbEscapedString::invalid();
    }
    inline KDbEscapedString toUpper() const {
        return m_valid ? KDbEscapedString(QByteArray::toUpper()) : KDbEscapedString::invalid();
    }

    inline KDbEscapedString trimmed() const {
        return m_valid ? KDbEscapedString(QByteArray::trimmed()) : KDbEscapedString::invalid();
    }
    inline KDbEscapedString simplified() const {
        return m_valid ? KDbEscapedString(QByteArray::simplified()) : KDbEscapedString::invalid();
    }
    inline KDbEscapedString leftJustified(int width, char fill = ' ', bool truncate = false) const
    {
        return m_valid ? KDbEscapedString(QByteArray::leftJustified(width, fill, truncate)) : KDbEscapedString::invalid();
    }
    inline KDbEscapedString rightJustified(int width, char fill = ' ', bool truncate = false) const
    {
        return m_valid ? KDbEscapedString(QByteArray::rightJustified(width, fill, truncate)) : KDbEscapedString::invalid();
    }

    inline KDbEscapedString &prepend(char c) {
        if (m_valid)
            QByteArray::prepend(c);
        return *this;
    }
    inline KDbEscapedString &prepend(const char *s) {
        if (m_valid)
            QByteArray::prepend(s);
        return *this;
    }
    inline KDbEscapedString &prepend(const QByteArray &a) {
        if (m_valid)
            QByteArray::prepend(a);
        return *this;
    }
    KDbEscapedString &prepend(const KDbEscapedString &s);
    inline KDbEscapedString &append(char c) {
        if (m_valid)
            QByteArray::append(c);
        return *this;
    }
    inline KDbEscapedString &append(const char *s) {
        if (m_valid)
            QByteArray::append(s);
        return *this;
    }
    inline KDbEscapedString &append(const char *s, int len) {
        if (m_valid)
            QByteArray::append(s, len);
        return *this;
    }
    inline KDbEscapedString &append(const QByteArray &a) {
        if (m_valid)
            QByteArray::append(a);
        return *this;
    }
    inline KDbEscapedString &append(const QString &a) {
        if (m_valid)
            QByteArray::append(a.toUtf8());
        return *this;
    }
    KDbEscapedString &append(const KDbEscapedString &s);
    inline KDbEscapedString &insert(int i, char c) {
        if (m_valid)
            QByteArray::insert(i, c);
        return *this;
    }
    inline KDbEscapedString &insert(int i, const char *s) {
        if (m_valid)
            QByteArray::insert(i, s);
        return *this;
    }
    inline KDbEscapedString &insert(int i, const QByteArray &a) {
        if (m_valid)
            QByteArray::insert(i, a);
        return *this;
    }
    KDbEscapedString &insert(int i, const KDbEscapedString &s);
    inline KDbEscapedString &remove(int index, int len) {
        if (m_valid)
            QByteArray::remove(index, len);
        return *this;
    }
    inline KDbEscapedString &replace(int index, int len, const char *s) {
        if (m_valid)
            QByteArray::replace(index, len, s);
        return *this;
    }
    inline KDbEscapedString &replace(int index, int len, const QByteArray &s) {
        if (m_valid)
            QByteArray::replace(index, len, s);
        return *this;
    }
    KDbEscapedString &replace(int index, int len, const KDbEscapedString &s);
    inline KDbEscapedString &replace(char before, const char *after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    inline KDbEscapedString &replace(char before, const QByteArray &after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    KDbEscapedString &replace(char before, const KDbEscapedString &after);
    inline KDbEscapedString &replace(const char *before, const char *after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    inline KDbEscapedString &replace(const char *before, int bsize, const char *after, int asize) {
        if (m_valid)
            QByteArray::replace(before, bsize, after, asize);
        return *this;
    }
    inline KDbEscapedString &replace(const QByteArray &before, const QByteArray &after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    KDbEscapedString &replace(const KDbEscapedString &before, const QByteArray &after);
    KDbEscapedString &replace(const QByteArray &before, const KDbEscapedString &after);
    KDbEscapedString &replace(const KDbEscapedString &before, const KDbEscapedString &after);
    inline KDbEscapedString &replace(const QByteArray &before, const char *after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    inline KDbEscapedString &replace(const char *before, const QByteArray &after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    inline KDbEscapedString &replace(char before, char after) {
        if (m_valid)
            QByteArray::replace(before, after);
        return *this;
    }
    inline KDbEscapedString &operator+=(char c) { return append(c); }
    inline KDbEscapedString &operator+=(const char *s) { return append(s); }
    inline KDbEscapedString &operator+=(const QByteArray &a) { return append(a); }
    inline KDbEscapedString &operator+=(const QString &a) { return append(a); }
    inline KDbEscapedString &operator+=(const KDbEscapedString &s) { return append(s); }

    //KDbEscapedString operator+(const QVector<T> & other ) const

    QList<KDbEscapedString> split(char sep) const;

    inline KDbEscapedString repeated(int times) const {
        return m_valid ? KDbEscapedString(QByteArray::repeated(times)) : KDbEscapedString::invalid();
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
    inline KDbEscapedString toBase64() const {
        return m_valid ? KDbEscapedString(QByteArray::toBase64()) : KDbEscapedString::invalid();
    }
    inline KDbEscapedString toHex() const {
        return m_valid ? KDbEscapedString(QByteArray::toHex()) : KDbEscapedString::invalid();
    }
    inline KDbEscapedString toPercentEncoding(const QByteArray &exclude = QByteArray(),
                                 const QByteArray &include = QByteArray(),
                                 char percent = '%') const
    {
        Q_UNUSED(percent);
        return m_valid ? KDbEscapedString(QByteArray::toPercentEncoding(exclude, include))
                       : KDbEscapedString::invalid();
    }

    inline KDbEscapedString &setNum(short val, int base = 10) {
        m_valid = true;
        QByteArray::setNum(val, base);
        return *this;
    }
    inline KDbEscapedString &setNum(ushort val, int base = 10) {
        m_valid = true;
        QByteArray::setNum(val, base);
        return *this;
    }
    inline KDbEscapedString &setNum(int val, int base = 10) {
        m_valid = true;
        QByteArray::setNum(val, base);
        return *this;
    }
    inline KDbEscapedString &setNum(uint val, int base = 10) {
        m_valid = true;
        QByteArray::setNum(val, base);
        return *this;
    }
    inline KDbEscapedString &setNum(qlonglong val, int base = 10) {
        m_valid = true;
        QByteArray::setNum(val, base);
        return *this;
    }
    inline KDbEscapedString &setNum(qulonglong val, int base = 10) {
        m_valid = true;
        QByteArray::setNum(val, base);
        return *this;
    }
    inline KDbEscapedString &setNum(float val, char f = 'g', int prec = 6) {
        m_valid = true;
        QByteArray::setNum(val, f, prec);
        return *this;
    }
    inline KDbEscapedString &setNum(double val, char f = 'g', int prec = 6) {
        m_valid = true;
        QByteArray::setNum(val, f, prec);
        return *this;
    }

    static inline KDbEscapedString number(int val, int base = 10) {
        return KDbEscapedString(QByteArray::number(val, base));
    }
    static inline KDbEscapedString number(uint val, int base = 10) {
        return KDbEscapedString(QByteArray::number(val, base));
    }
    static inline KDbEscapedString number(qlonglong val, int base = 10) {
        return KDbEscapedString(QByteArray::number(val, base));
    }
    static inline KDbEscapedString number(qulonglong val, int base = 10) {
        return KDbEscapedString(QByteArray::number(val, base));
    }
    static inline KDbEscapedString number(double val, char f = 'g', int prec = 6) {
        return KDbEscapedString(QByteArray::number(val, f, prec));
    }
    static inline KDbEscapedString fromRawData(const char *s, int size) {
        return KDbEscapedString(QByteArray::fromRawData(s, size));
    }
    static inline KDbEscapedString fromBase64(const QByteArray &base64) {
        return KDbEscapedString(QByteArray::fromBase64(base64));
    }
    static inline KDbEscapedString fromBase64(const KDbEscapedString &base64) {
        return base64.isValid() ? KDbEscapedString(QByteArray::fromBase64(base64)) : KDbEscapedString::invalid();
    }
    static inline KDbEscapedString fromHex(const QByteArray &hexEncoded) {
        return KDbEscapedString(QByteArray::fromHex(hexEncoded));
    }
    static inline KDbEscapedString fromHex(const KDbEscapedString &hexEncoded) {
        return hexEncoded.isValid() ? KDbEscapedString(QByteArray::fromHex(hexEncoded))
                                    : KDbEscapedString::invalid();
    }
    static inline KDbEscapedString fromPercentEncoding(const QByteArray &pctEncoded, char percent = '%') {
        return KDbEscapedString(QByteArray::fromPercentEncoding(pctEncoded, percent));
    }
    static inline KDbEscapedString fromPercentEncoding(const KDbEscapedString &pctEncoded, char percent = '%') {
        return pctEncoded.isValid() ? KDbEscapedString(QByteArray::fromPercentEncoding(pctEncoded, percent))
                                    : KDbEscapedString::invalid();
    }

    inline int count() const { return QByteArray::count(); }
    inline int length() const { return QByteArray::length(); }
    inline bool isNull() const { return QByteArray::isNull(); }

    KDbEscapedString arg(const KDbEscapedString &a1, const KDbEscapedString &a2) const;
    KDbEscapedString arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3) const;
    KDbEscapedString arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3,
                      const KDbEscapedString &a4) const;
    KDbEscapedString arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3,
                      const KDbEscapedString &a4, const KDbEscapedString &a5) const;
    KDbEscapedString arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3,
                      const KDbEscapedString &a4, const KDbEscapedString &a5, const KDbEscapedString &a6) const;
    KDbEscapedString arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3,
                      const KDbEscapedString &a4, const KDbEscapedString &a5, const KDbEscapedString &a6,
                      const KDbEscapedString &a7) const;
    KDbEscapedString arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3,
                      const KDbEscapedString &a4, const KDbEscapedString &a5, const KDbEscapedString &a6,
                      const KDbEscapedString &a7, const KDbEscapedString &a8) const;
    KDbEscapedString arg(const KDbEscapedString &a1, const KDbEscapedString &a2, const KDbEscapedString &a3,
                      const KDbEscapedString &a4, const KDbEscapedString &a5, const KDbEscapedString &a6,
                      const KDbEscapedString &a7, const KDbEscapedString &a8, const KDbEscapedString &a9) const;
    KDbEscapedString arg(const KDbEscapedString &a, int fieldWidth = 0, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(const QString &a, int fieldWidth = 0, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(const QByteArray &a, int fieldWidth = 0, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(int a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(uint a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(long a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(ulong a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(qlonglong a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(qulonglong a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(short a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(ushort a, int fieldWidth = 0, int base = 10, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(QChar a, int fieldWidth = 0, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(char a, int fieldWidth = 0, const QChar & fillChar = QLatin1Char( ' ' )) const;
    KDbEscapedString arg(double a, int fieldWidth = 0, char format = 'g', int precision = -1, const QChar & fillChar = QLatin1Char( ' ' )) const;

    typedef QByteArray::DataPtr DataPtr;
    inline DataPtr &data_ptr() { return QByteArray::data_ptr(); }

private:
    //! Used to create invalid string
    explicit inline KDbEscapedString(bool) : m_valid(false) {}

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
KDB_EXPORT QDataStream &operator<<(QDataStream &stream, const KDbEscapedString &string);
KDB_EXPORT QDataStream &operator>>(QDataStream &stream, KDbEscapedString &string);
#endif

inline KDbEscapedString operator+(const KDbEscapedString &a1, const KDbEscapedString &a2)
{
    if (!a1.isValid() || !a2.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(a1.toByteArray() + a2.toByteArray());
}
inline KDbEscapedString operator+(const KDbEscapedString &a1, const QString &a2)
{
    if (!a1.isValid())
        return KDbEscapedString::invalid();
    return a1 + KDbEscapedString(a2);
}
inline KDbEscapedString operator+(const KDbEscapedString &a1, const QByteArray &a2)
{
    if (!a1.isValid())
        return KDbEscapedString::invalid();
    return a1 + KDbEscapedString(a2);
}
inline KDbEscapedString operator+(const KDbEscapedString &a1, const char* a2)
{
    if (!a1.isValid())
        return KDbEscapedString::invalid();
    return a1 + KDbEscapedString(a2);
}
inline KDbEscapedString operator+(const KDbEscapedString &a1, QChar a2)
{
    if (!a1.isValid())
        return KDbEscapedString::invalid();
    return a1 + KDbEscapedString(a2.toLatin1());
}
inline KDbEscapedString operator+(const KDbEscapedString &a1, char a2)
{
    if (!a1.isValid())
        return KDbEscapedString::invalid();
    return a1 + KDbEscapedString(a2);
}
inline KDbEscapedString operator+(const QString &a1, const KDbEscapedString &a2)
{
    if (!a2.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(a1) + a2;
}
inline KDbEscapedString operator+(const QByteArray &a1, const KDbEscapedString &a2)
{
    if (!a2.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(a1) + a2;
}
inline KDbEscapedString operator+(const char* a1, const KDbEscapedString &a2)
{
    if (!a2.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(a1) + a2;
}
inline KDbEscapedString operator+(QChar a1, const KDbEscapedString &a2)
{
    if (!a2.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(a1.toLatin1()) + a2;
}
inline KDbEscapedString operator+(char a1, const KDbEscapedString &a2)
{
    if (!a2.isValid())
        return KDbEscapedString::invalid();
    return KDbEscapedString(a1) + a2;
}

inline bool operator==(const KDbEscapedString &a1, const QByteArray &a2)
{
    return a1.isValid() && a1.toByteArray() == a2;
}

inline bool operator==(const KDbEscapedString &a1, const char *a2)
{
    return a1.isValid() && a1.toByteArray() == a2;
}

inline bool operator==(const QByteArray &a1, const KDbEscapedString &a2)
{
    return a2.isValid() && a1 == a2.toByteArray();
}

inline bool operator==(const char *a1, const KDbEscapedString &a2)
{
    return a2.isValid() && a1 == a2.toByteArray();
}

Q_DECLARE_TYPEINFO(KDbEscapedString, Q_MOVABLE_TYPE);
//Q_DECLARE_SHARED(KDbEscapedString)

//! Sends escaped string @a string to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbEscapedString& string);

#endif
