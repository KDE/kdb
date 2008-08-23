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

#ifndef PREDICATE_TOOLS_UTILS_H
#define PREDICATE_TOOLS_UTILS_H

#include <Predicate/predicate_export.h>

#include <QPointer>
#include <QObject>
#include <QDateTime>
#include <QMetaMethod>
#include <QFont>
#include <QFrame>

class QColor;
class QMetaProperty;
class KAction;

namespace Predicate
{
namespace Utils
{

//! \return true if \a o has parent \a par.
inline bool hasParent(QObject* par, QObject* o)
{
    if (!o || !par)
        return false;
    while (o && o != par)
        o = o->parent();
    return o == par;
}

//! \return parent object of \a o that is of type \a type or NULL if no such parent
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

/* //! Const version of findParent()
  template<class type>
  inline const type findParentConst(const QObject* o, const char* className = 0)
  {
    if (!o) // || !className || className[0]=='\0')
      return 0;
    while ((o=o->parent())) {
      if (dynamic_cast< type >(o) && (!className || o->inherits(className)))
        return dynamic_cast< type >(o);
    }
    return 0;
  }*/

/*! \return first found child of \a o, inheriting \a className.
 If objName is 0 (the default), all object names match.
 Returned pointer type is casted. */
PREDICATE_EXPORT QObject* findFirstQObjectChild(QObject *o, const char* className /* compat with Qt3 */, const char* objName);

/*! \return first found child of \a o, that inherit \a className.
 If \a objName is 0 (the default), all object names match.
 Returned pointer type is casted. */
template<class type>
inline type findFirstChild(QObject *o, const char* className /* compat with Qt3 */, const char* objName = 0)
{
    return ::qobject_cast< type >(findFirstQObjectChild(o, className, objName));
}

//! Finds property name and returns its index; otherwise returns -1.
//! Like QMetaObject::indexOfProperty() but also looks at superclasses.
PREDICATE_EXPORT int indexOfPropertyWithSuperclasses(
    const QObject *object, const char* name);

//! Finds property for name \a name and object \a object returns it index;
//! otherwise returns a null QMetaProperty.
PREDICATE_EXPORT QMetaProperty findPropertyWithSuperclasses(const QObject* object,
        const char* name);

//! Finds property for index \a index and object \a object returns it index;
//! otherwise returns a null QMetaProperty.
PREDICATE_EXPORT QMetaProperty findPropertyWithSuperclasses(const QObject* object,
        int index);

//! \return true is \a object object is of class name \a className
inline bool objectIsA(QObject* object, const char* className)
{
    return 0 == qstrcmp(object->metaObject()->className(), className);
}

//! \return true is \a object object is of the class names inside \a classNames
PREDICATE_EXPORT bool objectIsA(QObject* object, const QList<QByteArray>& classNames);

//! \return a list of methods for \a metaObject meta object.
//! The methods are of type declared in \a types and have access declared
//! in \a access.
PREDICATE_EXPORT QList<QMetaMethod> methodsForMetaObject(
    const QMetaObject *metaObject, QFlags<QMetaMethod::MethodType> types
    = QFlags<QMetaMethod::MethodType>(QMetaMethod::Method | QMetaMethod::Signal | QMetaMethod::Slot),
    QFlags<QMetaMethod::Access> access
    = QFlags<QMetaMethod::Access>(QMetaMethod::Private | QMetaMethod::Protected | QMetaMethod::Public));

//! Like \ref KexiUtils::methodsForMetaObject() but includes methods from all
//! parent meta objects of the \a metaObject.
PREDICATE_EXPORT QList<QMetaMethod> methodsForMetaObjectWithParents(
    const QMetaObject *metaObject, QFlags<QMetaMethod::MethodType> types
    = QFlags<QMetaMethod::MethodType>(QMetaMethod::Method | QMetaMethod::Signal | QMetaMethod::Slot),
    QFlags<QMetaMethod::Access> access
    = QFlags<QMetaMethod::Access>(QMetaMethod::Private | QMetaMethod::Protected | QMetaMethod::Public));

//! \return a list with all this class's properties.
PREDICATE_EXPORT QList<QMetaProperty> propertiesForMetaObject(
    const QMetaObject *metaObject);

//! \return a list with all this class's properties including thise inherited.
PREDICATE_EXPORT QList<QMetaProperty> propertiesForMetaObjectWithInherited(
    const QMetaObject *metaObject);

//! \return a list of enum keys for meta property \a metaProperty.
PREDICATE_EXPORT QStringList enumKeysForProperty(const QMetaProperty& metaProperty);

//! QDateTime - a hack needed because QVariant(QTime) has broken isNull()
inline QDateTime stringToHackedQTime(const QString& s)
{
    if (s.isEmpty())
        return QDateTime();
    //  PreDbg << QDateTime( QDate(0,1,2), QTime::fromString( s, Qt::ISODate ) ).toString(Qt::ISODate);
    return QDateTime(QDate(0, 1, 2), QTime::fromString(s, Qt::ISODate));
}

#if 0
//! @todo
/*! \return filter string in QFileDialog format for a mime type pointed by \a mime
 If \a kdeFormat is true, QFileDialog-compatible filter string is generated,
 eg. "Image files (*.png *.xpm *.jpg)", otherwise KFileDialog -compatible
 filter string is generated, eg. "*.png *.xpm *.jpg|Image files (*.png *.xpm *.jpg)".
 "\\n" is appended if \a kdeFormat is true, otherwise ";;" is appended. */
PREDICATE_EXPORT QString fileDialogFilterString(const KMimeType::Ptr& mime, bool kdeFormat = true);

/*! @overload QString fileDialogFilterString(const KMimeType::Ptr& mime, bool kdeFormat = true) */
PREDICATE_EXPORT QString fileDialogFilterString(const QString& mimeString, bool kdeFormat = true);

/*! Like QString fileDialogFilterString(const KMimeType::Ptr& mime, bool kdeFormat = true)
 but returns a list of filter strings. */
PREDICATE_EXPORT QString fileDialogFilterStrings(const QStringList& mimeStrings, bool kdeFormat);

/*! A global setting for minimal readable font.
 \a init is a widget that should be passed if no qApp->mainWidget() is available yet.
 The size of font is not smaller than he one returned by
 KGlobalSettings::smallestReadableFont(). */
PREDICATE_EXPORT QFont smallFont(QWidget *init = 0);
#endif

/*! \return a color being a result of blending \a c1 with \a c2 with \a factor1
 and \a factor1 factors: (c1*factor1+c2*factor2)/(factor1+factor2). */
PREDICATE_EXPORT QColor blendedColors(const QColor& c1, const QColor& c2, int factor1 = 1, int factor2 = 1);

/*! \return a contrast color for a color \a c:
 If \a c is light color, darker color created using c.dark(200) is returned;
 otherwise lighter color created using c.light(200) is returned. */
PREDICATE_EXPORT QColor contrastColor(const QColor& c);

/*! \return a lighter color for a color \a c and a factor \a factor.
 For colors like Qt::red or Qt::green where hue and saturation are near to 255,
 hue is decreased so the result will be more bleached.
 For black color the result is dark gray rather than black. */
PREDICATE_EXPORT QColor bleachedColor(const QColor& c, int factor);

#if 0
//! @todo
/*! \return icon set computed as a result of colorizing \a icon pixmap with "buttonText"
 color of \a palette palette. This function is useful for displaying monochromed icons
 on the list view or table view header, to avoid bloat, but still have the color compatible
 with accessibility settings. */
PREDICATE_EXPORT QIcon colorizeIconToTextColor(const QPixmap& icon, const QPalette& palette);

/*! \return empty (fully transparent) pixmap that can be used as a place for icon of size \a iconGroup */
PREDICATE_EXPORT QPixmap emptyIcon(KIconLoader::Group iconGroup);
#endif

/*! Serializes \a map to \a array.
 KexiUtils::deserializeMap() can be used to deserialize this array back to map. */
PREDICATE_EXPORT void serializeMap(const QMap<QString, QString>& map, QByteArray& array);
PREDICATE_EXPORT void serializeMap(const QMap<QString, QString>& map, QString& string);

/*! \return a map deserialized from a byte array \a array.
 \a array need to contain data previously serialized using KexiUtils::serializeMap(). */
PREDICATE_EXPORT QMap<QString, QString> deserializeMap(const QByteArray& array);

/*! \return a map deserialized from \a string.
 \a string need to contain data previously serialized using KexiUtils::serializeMap(). */
PREDICATE_EXPORT QMap<QString, QString> deserializeMap(const QString& string);

/*! \return a valid filename converted from \a string by:
 - replacing \\, /, :, *, ?, ", <, >, |, \n \\t characters with a space
 - simplifing whitespace by removing redundant space characters using QString::simplified()
 Do not pass full paths here, but only filename strings. */
PREDICATE_EXPORT QString stringToFileName(const QString& string);

/*! Performs a simple \a string  encryption using rot47-like algorithm.
 Each character's unicode value is increased by 47 + i (where i is index of the character).
 The resulting string still contains redable characters.
 Do not use this for data that can be accessed by attackers! */
PREDICATE_EXPORT void simpleCrypt(QString& string);

/*! Performs a simple \a string decryption using rot47-like algorithm,
 using opposite operations to KexiUtils::simpleCrypt(). */
PREDICATE_EXPORT void simpleDecrypt(QString& string);

//! @internal
PREDICATE_EXPORT QString ptrToStringInternal(void* ptr, uint size);
//! @internal
PREDICATE_EXPORT void* stringToPtrInternal(const QString& str, uint size);

//! \return a pointer \a ptr safely serialized to string
template<class type>
QString ptrToString(type *ptr)
{
    return ptrToStringInternal(ptr, sizeof(type*));
}

//! \return a pointer of type \a type safely deserialized from \a str
template<class type>
type* stringToPtr(const QString& str)
{
    return static_cast<type*>(stringToPtrInternal(str, sizeof(type*)));
}

//! Sets focus for widget \a widget with reason \a reason.
PREDICATE_EXPORT void setFocusWithReason(QWidget* widget, Qt::FocusReason reason);

//! Unsets focus for widget \a widget with reason \a reason.
PREDICATE_EXPORT void unsetFocusWithReason(QWidget* widget, Qt::FocusReason reason);

//! @short A convenience class that simplifies usage of QWidget::getContentsMargins() and QWidget::setContentsMargins
class PREDICATE_EXPORT WidgetMargins
{
public:
    //! Creates object with all margins set to 0
    WidgetMargins();
    //! Creates object with margins copied from \a widget
    WidgetMargins(QWidget *widget);
    //! Creates object with margins set to given values
    WidgetMargins(int _left, int _top, int _right, int _bottom);
    //! Creates object with all margins set to commonMargin
    WidgetMargins(int commonMargin);
    //! Copies margins from \a widget to this object
    void copyFromWidget(QWidget *widget);
    //! Creates margins from this object copied to \a widget
    void copyToWidget(QWidget *widget);
    //! Adds the given margins \a margins to this object, and returns a reference to this object
    WidgetMargins& operator+= (const WidgetMargins& margins);

    int left, top, right, bottom;
};

//! \return the sum of \a margins1 and \a margins1; each component is added separately.
const WidgetMargins operator+ (const WidgetMargins& margins1, const WidgetMargins& margins2);

//! Draws pixmap on painter \a p using predefined parameters.
//! Used in KexiDBImageBox and KexiBlobTableEdit.
PREDICATE_EXPORT void drawPixmap(QPainter& p, const WidgetMargins& margins, const QRect& rect,
                                 const QPixmap& pixmap, Qt::Alignment alignment, bool scaledContents, bool keepAspectRatio);

//! A helper for automatic deleting of contents of containers.
template <typename Container>
class PREDICATE_EXPORT ContainerDeleter
{
public:
    ContainerDeleter(Container& container) : m_container(container) {}
    ~ContainerDeleter() {
        clear();
    }
    void clear() {
        qDeleteAll(m_container); m_container.clear();
    }
private:
    Container& m_container;
};

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
bool m_autoDelete : 1;
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
bool m_autoDelete : 1;
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
bool m_autoDelete : 1;
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

/*! A modified QFrame which sets up sunken styled panel frame style depending
 on the current widget style. The widget also reacts on style changes. */
class PREDICATE_EXPORT KTextEditorFrame : public QFrame
{
public:
    KTextEditorFrame(QWidget * parent = 0, Qt::WindowFlags f = 0);
protected:
    virtual void changeEvent(QEvent *event);
};
}
}

//! sometimes we leave a space in the form of empty QFrame and want to insert here
//! a widget that must be instantiated by hand.
//! This macro inserts a widget \a what into a frame \a where.
#define GLUE_WIDGET(what, where) \
    { Q3VBoxLayout *lyr = new Q3VBoxLayout(where); \
        lyr->addWidget(what); }


#endif //KEXIUTILS_UTILS_H
