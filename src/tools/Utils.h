/* This file is part of the KDE project
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

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

#ifndef PREDICATE_TOOLS_UTILS_H
#define PREDICATE_TOOLS_UTILS_H

#include <Predicate/predicate_export.h>

#include <QPointer>
#include <QObject>
#include <QDateTime>
#include <QMetaMethod>
#include <QFont>
#include <QFrame>

class QMetaProperty;

namespace Predicate
{
namespace Utils
{

//! @return true if @a o has parent @a par.
inline bool hasParent(QObject* par, QObject* o)
{
    if (!o || !par)
        return false;
    while (o && o != par)
        o = o->parent();
    return o == par;
}

//! @return parent object of @a o that is of type @a type or NULL if no such parent
template<class type>
inline type findParent(QObject* o, const char* className = 0)
{
    if (!o)
        return 0;
    while ((o = o->parent())) {
        if (::qobject_cast< type >(o) && (!className || o->inherits(className)))
            return ::qobject_cast< type >(o);
    }
    return 0;
}

//! QDateTime - a hack needed because QVariant(QTime) has broken isNull()
inline QDateTime stringToHackedQTime(const QString& s)
{
    if (s.isEmpty())
        return QDateTime();
    //  PreDbg << QDateTime( QDate(0,1,2), QTime::fromString( s, Qt::ISODate ) ).toString(Qt::ISODate);
    return QDateTime(QDate(0, 1, 2), QTime::fromString(s, Qt::ISODate));
}

/*! Serializes @a map to @a array.
 KexiUtils::deserializeMap() can be used to deserialize this array back to map. */
PREDICATE_EXPORT void serializeMap(const QMap<QString, QString>& map, QByteArray& array);
PREDICATE_EXPORT void serializeMap(const QMap<QString, QString>& map, QString& string);

/*! @return a map deserialized from a byte array @a array.
 @a array need to contain data previously serialized using KexiUtils::serializeMap(). */
PREDICATE_EXPORT QMap<QString, QString> deserializeMap(const QByteArray& array);

/*! @return a map deserialized from @a string.
 @a string need to contain data previously serialized using KexiUtils::serializeMap(). */
PREDICATE_EXPORT QMap<QString, QString> deserializeMap(const QString& string);

/*! @return a valid filename converted from @a string by:
 - replacing \\, /, :, *, ?, ", <, >, |, \n \\t characters with a space
 - simplifing whitespace by removing redundant space characters using QString::simplified()
 Do not pass full paths here, but only filename strings. */
PREDICATE_EXPORT QString stringToFileName(const QString& string);

/*! Performs a simple @a string  encryption using rot47-like algorithm.
 Each character's unicode value is increased by 47 + i (where i is index of the character).
 The resulting string still contains redable characters.
 Do not use this for data that can be accessed by attackers! */
PREDICATE_EXPORT void simpleCrypt(QString& string);

/*! Performs a simple @a string decryption using rot47-like algorithm,
 using opposite operations to KexiUtils::simpleCrypt(). */
PREDICATE_EXPORT void simpleDecrypt(QString& string);

//! @internal
PREDICATE_EXPORT QString ptrToStringInternal(void* ptr, uint size);
//! @internal
PREDICATE_EXPORT void* stringToPtrInternal(const QString& str, uint size);

//! @return a pointer @a ptr safely serialized to string
template<class type>
QString ptrToString(type *ptr)
{
    return ptrToStringInternal(ptr, sizeof(type*));
}

//! @return a pointer of type @a type safely deserialized from @a str
template<class type>
type* stringToPtr(const QString& str)
{
    return static_cast<type*>(stringToPtrInternal(str, sizeof(type*)));
}

//! @short Autodeleted hash
template <class Key, class T>
class PREDICATE_EXPORT AutodeletedHash : public QHash<Key, T>
{
public:
    AutodeletedHash(const AutodeletedHash& other) : QHash<Key, T>(other), m_autoDelete(false) {}
    AutodeletedHash(bool autoDelete = true) : QHash<Key, T>(), m_autoDelete(autoDelete) {}
    void setAutoDelete(bool set) {
        m_autoDelete = set;
    }
    bool autoDelete() const {
        return m_autoDelete;
    }
    ~AutodeletedHash() {
        if (m_autoDelete) qDeleteAll(*this);
    }
private:
    bool m_autoDelete;
};

//! @short Autodeleted list
template <typename T>
class PREDICATE_EXPORT AutodeletedList : public QList<T>
{
public:
    AutodeletedList(const AutodeletedList& other)
            : QList<T>(other), m_autoDelete(false) {}
    AutodeletedList(bool autoDelete = true) : QList<T>(), m_autoDelete(autoDelete) {}
    ~AutodeletedList() {
        if (m_autoDelete) qDeleteAll(*this);
    }
    void setAutoDelete(bool set) {
        m_autoDelete = set;
    }
    bool autoDelete() const {
        return m_autoDelete;
    }
    void removeAt(int i) {
        T item = QList<T>::takeAt(i); if (m_autoDelete) delete item;
    }
    void removeFirst() {
        T item = QList<T>::takeFirst(); if (m_autoDelete) delete item;
    }
    void removeLast() {
        T item = QList<T>::takeLast(); if (m_autoDelete) delete item;
    }
    void replace(int i, const T& value) {
        T item = QList<T>::takeAt(i); insert(i, value); if (m_autoDelete) delete item;
    }
    void insert(int i, const T& value) {
        QList<T>::insert(i, value);
    }
    typename QList<T>::iterator erase(typename QList<T>::iterator pos) {
        T item = *pos;
        typename QList<T>::iterator res = QList<T>::erase(pos);
        if (m_autoDelete)
            delete item;
        return res;
    }
    typename QList<T>::iterator erase(
        typename QList<T>::iterator afirst,
        typename QList<T>::iterator alast) {
        if (!m_autoDelete)
            return QList<T>::erase(afirst, alast);
        while (afirst != alast) {
            T item = *afirst;
            afirst = QList<T>::erase(afirst);
            delete item;
        }
        return alast;
    }
    void pop_back() {
        removeLast();
    }
    void pop_front() {
        removeFirst();
    }
    int removeAll(const T& value) {
        if (!m_autoDelete)
            return QList<T>::removeAll(value);
        typename QList<T>::iterator it(QList<T>::begin());
        int removedCount = 0;
        while (it != QList<T>::end()) {
            if (*it == value) {
                T item = *it;
                it = QList<T>::erase(it);
                delete item;
                removedCount++;
            } else
                ++it;
        }
        return removedCount;
    }
    void clear() {
        if (!m_autoDelete)
            return QList<T>::clear();
        while (!QList<T>::isEmpty()) {
            T item = QList<T>::takeFirst();
            delete item;
        }
    }

private:
    bool m_autoDelete;
};

//! @short Case insensitive hash container supporting QString or QByteArray keys.
//! Keys are turned to lowercase before inserting. Also supports option for autodeletion.
template <typename Key, typename T>
class PREDICATE_EXPORT CaseInsensitiveHash : public QHash<Key, T>
{
public:
    CaseInsensitiveHash() : QHash<Key, T>(), m_autoDelete(false) {}
    ~CaseInsensitiveHash() {
        if (m_autoDelete) qDeleteAll(*this);
    }
    typename QHash<Key, T>::iterator find(const Key& key) const {
        return QHash<Key, T>::find(key.toLower());
    }
    typename QHash<Key, T>::const_iterator constFind(const Key& key) const {
        return QHash<Key, T>::constFind(key.toLower());
    }
    bool contains(const Key& key) const {
        return QHash<Key, T>::contains(key.toLower());
    }
    int count(const Key& key) const {
        return QHash<Key, T>::count(key.toLower());
    }
    typename QHash<Key, T>::iterator insert(const Key& key, const T& value) {
        return QHash<Key, T>::insert(key.toLower(), value);
    }
    typename QHash<Key, T>::iterator insertMulti(const Key& key, const T& value) {
        return QHash<Key, T>::insertMulti(key.toLower(), value);
    }
    const Key key(const T& value, const Key& defaultKey) const {
        return QHash<Key, T>::key(value, key.toLower());
    }
    int remove(const Key& key) {
        return QHash<Key, T>::remove(key.toLower());
    }
    const T take(const Key& key) {
        return QHash<Key, T>::take(key.toLower());
    }
    const T value(const Key& key) const {
        return QHash<Key, T>::value(key.toLower());
    }
    const T value(const Key& key, const T& defaultValue) const {
        return QHash<Key, T>::value(key.toLower(), defaultValue);
    }
    QList<T> values(const Key& key) const {
        return QHash<Key, T>::values(key.toLower());
    }
    T& operator[](const Key& key) {
        return QHash<Key, T>::operator[](key.toLower());
    }
    const T operator[](const Key& key) const {
        return QHash<Key, T>::operator[](key.toLower());
    }
    //! Controls autodeletion flag.
    void setAutoDelete(bool set) {
        m_autoDelete = set;
    }
private:
    bool m_autoDelete;
};

//! A set created from static (0-terminated) array of raw null-terminated strings.
class PREDICATE_EXPORT StaticSetOfStrings
{
public:
    StaticSetOfStrings();
    StaticSetOfStrings(const char* array[]);
    ~StaticSetOfStrings();
    void setStrings(const char* array[]);
    bool isEmpty() const;
    bool contains(const QByteArray& string) const;
private:
    class Private;
    Private * const d;
};

} // Utils
} // Predicate

//! @def PREDICATE_SHARED_LIB_EXTENSION operating system-dependent extension for shared library files
#if defined(Q_OS_WIN)
#define PREDICATE_SHARED_LIB_EXTENSION ".dll"
#elif defined(Q_OS_MAC)
#define PREDICATE_SHARED_LIB_EXTENSION ".dylib"
#else
#define PREDICATE_SHARED_LIB_EXTENSION ".so"
#endif

#endif //PREDICATE_TOOLS_UTILS_H
