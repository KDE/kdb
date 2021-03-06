/* This file is part of the KDE project
   Copyright (C) 2003-2019 Jarosław Staniek <staniek@kde.org>

   Portions of kstandarddirs.cpp:
   Copyright (C) 1999 Sirtaj Singh Kang <taj@kde.org>
   Copyright (C) 1999,2007 Stephan Kulow <coolo@kde.org>
   Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2009 David Faure <faure@kde.org>

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

#ifndef KDB_TOOLS_UTILS_H
#define KDB_TOOLS_UTILS_H

#include "kdb_export.h"
#include "config-kdb.h"

#include <QObject>
#include <QDateTime>
#include <QMetaMethod>
#include <QDebug>

namespace KDbUtils
{

//! @return true if @a o has parent @a par (checks recursively)
KDB_EXPORT bool hasParent(QObject* par, QObject* o);

//! @return parent object of @a o that is of type @a type or @c nullptr if no such parent
template<class type>
inline type findParent(QObject* o, const char* className = nullptr)
{
    if (!o)
        return nullptr;
    while ((o = o->parent())) {
        if (::qobject_cast< type >(o) && (!className || o->inherits(className)))
            return ::qobject_cast< type >(o);
    }
    return nullptr;
}

/**
 * Returns the time as a string using Qt::ISODateWithMs format
 *
 * If milliseconds is zero then they are not appended.
 * Works also for Qt < 5.8 which does not offer QTime::toString(Qt::ISODateWithMs).
 *
 * @since 3.2
 */
KDB_EXPORT QString toISODateStringWithMs(const QTime &time);

/**
 * Returns the date/time as a string using Qt::ISODateWithMs format
 *
 * If milliseconds in time is zero then they are not appended.
 * Works also for Qt < 5.8 which does not offer QDateTime::toString(Qt::ISODateWithMs).
 *
 * @since 3.2
 */
KDB_EXPORT QString toISODateStringWithMs(const QDateTime &dateTime);

/**
 * Returns the time represented by the string using the Qt::ISODateWithMs format
 *
 * Milliseconds are optional.
 * Works also for Qt < 5.8 which does not offer QTime::fromString(QString, Qt::ISODateWithMs).
 *
 * @note Equal to QTime::fromString(string, Qt::ISODate) since the Qt::ISODate mode already
 * supports milliseconds in case of fromString().
 *
 * @since 3.2
 */
KDB_EXPORT QTime timeFromISODateStringWithMs(const QString &string);

/**
 * Returns the date/time represented by the string using the Qt::ISODateWithMs format
 *
 * Milliseconds of time are optional.
 * Works also for Qt < 5.8 which does not offer QDateTime::fromString(QString, Qt::ISODateWithMs).
 *
 * @note Equal to QDateTime::fromString(string, Qt::ISODate) since the Qt::ISODate mode already
 * supports milliseconds in case of fromString().
 *
 * @since 3.2
 */
KDB_EXPORT QDateTime dateTimeFromISODateStringWithMs(const QString &string);

//! QDateTime - a hack needed because QVariant(QTime) has broken isNull()
KDB_EXPORT QDateTime stringToHackedQTime(const QString& s);

/*! Serializes @a map to the array pointed by @a array.
 KDbUtils::deserializeMap() can be used to deserialize this array back to map.
 Does nothing if @a array is @c nullptr. */
KDB_EXPORT void serializeMap(const QMap<QString, QString>& map, QByteArray *array);

/*! Serializes @a map to the string pointed by @a string.
 KDbUtils::deserializeMap() can be used to deserialize this array back to map.
 Does nothing if @a string is @c nullptr. */
KDB_EXPORT void serializeMap(const QMap<QString, QString>& map, QString *string);

/*! @return a map deserialized from a byte array @a array.
 @a array need to contain data previously serialized using KexiUtils::serializeMap(). */
KDB_EXPORT QMap<QString, QString> deserializeMap(const QByteArray& array);

/*! @return a map deserialized from @a string.
 @a string need to contain data previously serialized using KexiUtils::serializeMap(). */
KDB_EXPORT QMap<QString, QString> deserializeMap(const QString& string);

/**
 * Returns a valid filename converted from given string
 *
 * Following steps are performed:
 * - replace \\, /, :, *, ?, ", <, >, |, \n \\t characters with a space,
 * - simplify whitespace by removing redundant space characters using QString::simplified().
 * - prepend '_' character if the first character is '.', so the file will not be considered hidden
 *   (since 3.2.0).
 *
 * @note Do not pass full paths here but only filename strings.
 */
KDB_EXPORT QString stringToFileName(const QString& string);

/*! Performs a simple @a string encryption using rot47-like algorithm.
 Each character's unicode value is increased by 47 + i (where i is index of the character).
 The resulting string still contains readable characters but some of them can be non-ASCII.
 Does nothing if @a string is @c nullptr.
 @note Do not use this for data that can be accessed by attackers! */
KDB_EXPORT void simpleCrypt(QString *string);

/*! Performs a simple @a string decryption using rot47-like algorithm,
 using opposite operations to KexiUtils::simpleCrypt().
 @return true on success and false on failure. Failue means that one or more characters have unicode
 numbers smaller than value of 47 + i. On failure @a string is not altered.
 Does nothing and returns @c false if @a string is @c nullptr.
*/
KDB_EXPORT bool simpleDecrypt(QString *string);

//! @internal
KDB_EXPORT QString pointerToStringInternal(void* pointer, int size);
//! @internal
KDB_EXPORT void* stringToPointerInternal(const QString& string, int size);

//! @return a pointer @a pointer safely serialized to string
template<class type>
QString pointerToString(type *pointer)
{
    return pointerToStringInternal(pointer, sizeof(type*));
}

//! @return a pointer of type @a type safely deserialized from @a string
template<class type>
type* stringToPointer(const QString& string)
{
    return static_cast<type*>(stringToPointerInternal(string, sizeof(type*)));
}

//! @return value converted to text, squeezed to reasonable length, useful for debugging
//! If the value is not a byte array or string, or if it's not longer than 1024 characters,
//! @a value is returned.
//! @since 3.1
KDB_EXPORT QVariant squeezedValue(const QVariant &value);

//! @short Autodeleting hash
template <class Key, class T>
class AutodeletedHash : public QHash<Key, T>
{
public:
    //! Creates autodeleting hash as a copy of @a other.
    //! Auto-deletion is not enabled as it would cause double deletion for items.
    //! If you enable auto-deletion on here, make sure you disable it in the @a other hash.
    AutodeletedHash(const AutodeletedHash& other) : QHash<Key, T>(other), m_autoDelete(false) {}

    //! Creates empty autodeleting hash.
    //! Auto-deletion is enabled by default.
    AutodeletedHash(bool autoDelete = true) : QHash<Key, T>(), m_autoDelete(autoDelete) {}

    ~AutodeletedHash() {
        if (m_autoDelete) {
            qDeleteAll(*this);
        }
    }
    void setAutoDelete(bool set) {
        m_autoDelete = set;
    }
    bool autoDelete() const {
        return m_autoDelete;
    }
    void clear() {
        if (m_autoDelete) {
            qDeleteAll(*this);
        }
        QHash<Key, T>::clear();
    }
    typename QHash<Key, T>::iterator erase(typename QHash<Key, T>::iterator pos) {
        typename QHash<Key, T>::iterator it = QHash<Key, T>::erase(pos);
        if (m_autoDelete) {
            delete it.value();
            it.value() = 0;
            return it;
        }
    }
    typename QHash<Key, T>::iterator insert(const Key &key, const T &value) {
        if (m_autoDelete) {
            T &oldValue = QHash<Key, T>::operator[](key);
            if (oldValue && oldValue != value) { // only delete if differs
                delete oldValue;
            }
        }
        return QHash<Key, T>::insert(key, value);
    }
    int remove(const Key &key) {
        if (m_autoDelete) {
            const QList<T> values(QHash<Key, T>::values(key));
            const int result = QHash<Key, T>::remove(key);
            for (T item : values) {
                delete item;
            }
            return result;
        } else {
            return QHash<Key, T>::remove(key);
        }
    }
    // note: no need to override insertMulti(), unite(), take(), they do not replace items

private:
    bool m_autoDelete;
};

//! @short Autodeleting list
template <typename T>
class AutodeletedList : public QList<T>
{
public:
    //! Creates autodeleting list as a copy of @a other.
    //! Auto-deletion is not enabled as it would cause double deletion for items.
    //! If you enable auto-deletion on here, make sure you disable it in the @a other list.
    AutodeletedList(const AutodeletedList& other)
            : QList<T>(other), m_autoDelete(false) {}

    //! Creates empty autodeleting list.
    //! Auto-deletion is enabled by default.
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
//! Keys are turned to lowercase before inserting.
template <typename Key, typename T>
class CaseInsensitiveHash : public QHash<Key, T>
{
public:
    CaseInsensitiveHash() : QHash<Key, T>() {}
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
        return QHash<Key, T>::key(value, defaultKey.toLower());
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
};

//! A set created from static (0-terminated) array of raw null-terminated strings.
class KDB_EXPORT StaticSetOfStrings
{
public:
    StaticSetOfStrings();
    explicit StaticSetOfStrings(const char* const array[]);
    ~StaticSetOfStrings();
    void setStrings(const char* const array[]);
    bool isEmpty() const;

    //! @return true if @a string can be found within set, comparison is case sensitive
    bool contains(const QByteArray& string) const;
private:
    class Private;
    Private * const d;
    Q_DISABLE_COPY(StaticSetOfStrings)
};

/*! @return debugging string for object @a object of type @a T */
template <typename T>
QString debugString(const T& object)
{
    QString result;
    QDebug dbg(&result);
    dbg << object;
    return result;
}

//! Used by findExe().
enum class FindExeOption {
    //! No options
    None = 0,
    //! If set, the path returned may not have the executable bit set.
    IgnoreExecBit = 1
};
Q_DECLARE_FLAGS(FindExeOptions, FindExeOption)

/**
 * Finds the executable in the system path.
 *
 * A valid executable must be a file and have its executable bit set.
 *
 * @param appname The name of the executable file for which to search.
 *                if this contains a path separator, it will be resolved
 *                according to the current working directory
 *                (shell-like behavior).
 * @param path    The path which will be searched. If this is
 *                null (default), the @c $PATH environment variable will
 *                be searched.
 * @param options Options, see FindExeOption.
 *
 * @return The path of the executable. If it was not found, returns QString().
 */
QString findExe(const QString& appname,
                const QString& path = QString(),
                FindExeOptions options = FindExeOption::None);

//! A single property
//! @note This property is general-purpose and not related to Qt Properties.
//! @see KDbUtils::PropertySet
class KDB_EXPORT Property {
public:
    //! Constructs a null property
    Property();

    Property(const QVariant &aValue, const QString &aCaption);

    Property(const Property &other);

    ~Property();

    bool operator==(const Property &other) const;

    bool operator!=(const Property &other) const { return !operator==(other); }

    bool isNull() const;

    QVariant value() const;

    void setValue(const QVariant &value);

    QString caption() const;

    void setCaption(const QString &caption);

private:
    class Private;
    Private * const d;
};

//! A set of properties.
//! @note These properties are general-purpose and not related to Qt Properties.
//! @see KDbUtils::Property
class KDB_EXPORT PropertySet
{
public:
    PropertySet();

    PropertySet(const PropertySet &other);

    ~PropertySet();

    //! Assigns @a other to this property set and returns a reference to this property set.
    PropertySet& operator=(const PropertySet &other);

    //! @return true if this property set has exactly the same properties as @a other
    //! @since 3.1
    bool operator==(const PropertySet &other) const;

    //! @return true if this property differs in at least one property from @a other
    //! @since 3.1
    bool operator!=(const PropertySet &other) const { return !operator==(other); }

    //! Inserts property with a given @a name, @a value and @a caption.
    //! If @a caption is empty, caption from existing property is reused.
    //! @a name must be a valid identifier (see KDb::isIdentifier()).
    void insert(const QByteArray &name, const QVariant &value, const QString &caption = QString());

    //! Sets caption for property @a name to @a caption.
    //! If such property does not exist, does nothing.
    //! @since 3.1
    void setCaption(const QByteArray &name, const QString &caption);

    //! Sets value for property @a name to @a value.
    //! If such property does not exist, does nothing.
    //! @since 3.1
    void setValue(const QByteArray &name, const QVariant &value);

    //! Removes property with a given @a name.
    void remove(const QByteArray &name);

    //! @return property with a given @a name.
    //! If not found, a null Property is returned (Property::isNull).
    Property property(const QByteArray &name) const;

    //! @return a list of property names.
    QList<QByteArray> names() const;

private:
    class Private;
    Private * const d;
};

} // KDbUtils

#endif //KDB_TOOLS_UTILS_H
