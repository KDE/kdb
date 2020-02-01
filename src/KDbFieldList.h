/* This file is part of the KDE project
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KDB_FIELDLIST_H
#define KDB_FIELDLIST_H

#include <QList>
#include <QHash>

#include "KDbGlobal.h"
#include "KDbField.h"
#include "KDbEscapedString.h"

class KDbConnection;

/*! Helper class that stores list of fields. */
class KDB_EXPORT KDbFieldList
{
public:
    /*! Creates empty list of fields. If @a owner is true, the list will be
     owner of any added field, what means that these field
     will be removed on the list destruction. Otherwise, the list
     just points any field that was added.
     @see isOwner()
    */
    explicit KDbFieldList(bool owner = false);

    /*! Copy constructor.
     If @a deepCopyFields is true, all fields are deeply copied, else only pointer are copied.
     Reimplemented in KDbQuerySchema constructor. */
    explicit KDbFieldList(const KDbFieldList& fl, bool deepCopyFields = true);

    /*! Destroys the list. If the list owns fields (see constructor),
     these are also deleted. */
    virtual ~KDbFieldList();

    /*! @return number of fields in the list. */
    int fieldCount() const;

    /*! @return true if the list is empty. */
    bool isEmpty() const;

    /*! Adds @a field at the and of field list. */
    bool addField(KDbField *field);

    /*! Inserts @a field into a specified @a index position.

     @c false is returned if @a field is @c nullptr or @a index is invalid.

     Note: You can reimplement this method but you should still call
     this implementation in your subclass. */
    virtual bool insertField(int index, KDbField *field);

    /*! Removes field from the field list and deletes it. Use with care.

     Note: You can reimplement this method but you should still call
     this implementation in your subclass.
     @return false if this field does not belong to this list. */
    virtual bool removeField(KDbField *field);

    /*! Moves fiels @a field from its current position to new position @a newIndex.
     If @a newIndex value is greater than fieldCount()-1, it is appended.
     @return @c false if this field does not belong to this list or is @c nullptr. */
    virtual bool moveField(KDbField *field, int newIndex);

    /*! @return field id or @c nullptr if there is no such a field. */
    virtual KDbField* field(int id);

    /*! @overload KDbField* field(int id) */
    virtual const KDbField* field(int id) const;

    /*! @return field with name @a name or @c nullptr if there is no such a field. */
    virtual KDbField* field(const QString& name);

    /*! @overload . DbField* field(const QString& name) const */
    virtual const KDbField* field(const QString& name) const;

    /*! @return true if this list contains given @a field. */
    bool hasField(const KDbField& field) const;

    /*! @return first occurrence of @a field in the list
     or -1 if this list does not contain this field. */
    int indexOf(const KDbField& field) const;

    /*! @return list of field names for this list. */
    QStringList names() const;

    //! @return iterator for fields
    KDbField::ListIterator fieldsIterator() const;

    //! @return iterator for fields
    KDbField::ListIterator fieldsIteratorConstEnd() const;

    //! @return list of fields
    KDbField::List *fields();

    //! @overload
    const KDbField::List* fields() const;

    /*! @return list of autoincremented fields. The list is owned by this KDbFieldList object. */
    KDbField::List* autoIncrementFields() const;

    /*! @return true if fields in the list are owned by this list. */
    bool isOwner() const;

    /*! Removes all fields from the list. */
    virtual void clear();

    /*! Creates and returns a list that contain fields selected by name.
     At least one field (exising on this list) should be selected, otherwise 0 is
     returned. Returned KDbFieldList object is not owned by any parent (so you need
     to destroy yourself) and KDbField objects included in it are not owned by it
     (but still as before, by 'this' object).
     Returned list can be usable e.g. as argument for KDbConnection::insertRecord().
     0 is returned if at least one name cannot be found.
    */
    Q_REQUIRED_RESULT KDbFieldList *subList(const QString& n1, const QString& n2 = QString(),
                       const QString& n3 = QString(), const QString& n4 = QString(),
                       const QString& n5 = QString(), const QString& n6 = QString(),
                       const QString& n7 = QString(), const QString& n8 = QString(),
                       const QString& n9 = QString(), const QString& n10 = QString(),
                       const QString& n11 = QString(), const QString& n12 = QString(),
                       const QString& n13 = QString(), const QString& n14 = QString(),
                       const QString& n15 = QString(), const QString& n16 = QString(),
                       const QString& n17 = QString(), const QString& n18 = QString()
                      );

    /*! Like above, but for QStringList. */
    Q_REQUIRED_RESULT KDbFieldList *subList(const QStringList &list);

    /*! @overload subList(const QStringList&) */
    Q_REQUIRED_RESULT KDbFieldList *subList(const QList<QByteArray> &list);

    /*! Like above, but with a list of field indices */
    Q_REQUIRED_RESULT KDbFieldList *subList(const QList<int> &list);

    /*! @return a string that is a result of all field names concatenated
     and with @a separator. This is usable e.g. as argument like "field1,field2"
     for "INSERT INTO (xxx) ..". The result of this method is effectively cached,
     and it is invalidated when set of fields changes (e.g. using clear()
     or addField()).

     @a tableOrAlias, if provided is prepended to each field, so the resulting
     names will be in form tableOrAlias.fieldName. This option is used for building
     queries with joins, where fields have to be spicified without ambiguity.
     See @ref KDbConnection::selectStatement() for example use.

     @a escapingType can be used to alter default escaping type.
     If @a conn is not provided for DriverEscaping, no escaping is performed.
    */
    KDbEscapedString sqlFieldsList(KDbConnection *conn, const QString& separator = QLatin1String(","),
                                const QString& tableOrAlias = QString(),
                                KDb::IdentifierEscapingType escapingType = KDb::DriverEscaping) const;

    /*! Like above, but this is convenient static function, so you can pass any @a list here. */
    static KDbEscapedString sqlFieldsList(const KDbField::List& list, KDbConnection *conn,
                                       const QString& separator = QLatin1String(","),
                                       const QString& tableOrAlias = QString(),
                                       KDb::IdentifierEscapingType escapingType = KDb::DriverEscaping);

    /*! Renames field @a oldName to @a newName.

     @c false is returned if field with @a oldName name does not exist or field with @a newName name
     already exists.

     @note Do not use this for physical renaming columns. Use KDbAlterTableHandler instead.
    */
    bool renameField(const QString& oldName, const QString& newName);

    //! @overload
    bool renameField(KDbField *field, const QString& newName);

private:
    class Private;
    Private * const d;
};

//! Sends information about field list  @a list to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbFieldList& list);

#endif
