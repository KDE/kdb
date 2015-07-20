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
    KDbFieldList& addField(KDbField *field);

    /*! Inserts @a field into a specified position (@a index).

     Note: You can reimplement this method but you should still call
     this implementation in your subclass. */
    virtual KDbFieldList& insertField(int index, KDbField *field);

    /*! Removes field from the field list. Use with care.

     Note: You can reimplement this method but you should still call
     this implementation in your subclass.
     @return false if this field does not belong to this list. */
    virtual bool removeField(KDbField *field);

    /*! Moves fiels @a field from its current position to new position @a newIndex.
     If @a newIndex value is greater than fieldCount()-1, it is appended.
     @return false if this field does not belong to this list. */
    virtual bool moveField(KDbField *field, int newIndex);

    /*! @return field id or NULL if there is no such a field. */
    KDbField* field(int id);

    /*! @overload KDbField* field(int id) */
    const KDbField* field(int id) const;

    /*! @return field with name @a name or NULL if there is no such a field. */
    virtual KDbField* field(const QString& name);

    /*! @overload . DbField* field(const QString& name) const */
    const KDbField* field(const QString& name) const;

    /*! @return true if this list contains given @a field. */
    bool hasField(const KDbField& field) const;

    /*! @return first occurrence of @a field in the list
     or -1 if this list does not contain this field. */
    int indexOf(const KDbField& field) const;

    /*! @return list of field names for this list. */
    QStringList names() const;

    KDbField::ListIterator fieldsIterator() const;

    KDbField::ListIterator fieldsIteratorConstEnd() const;

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
    KDbFieldList* subList(const QString& n1, const QString& n2 = QString(),
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
    KDbFieldList* subList(const QStringList& list);

    /*! @overload subList(const QStringList&) */
    KDbFieldList* subList(const QList<QByteArray>& list);

    /*! Like above, but with a list of field indices */
    KDbFieldList* subList(const QList<int>& list);

    /*! @return a string that is a result of all field names concatenated
     and with @a separator. This is usable e.g. as argument like "field1,field2"
     for "INSERT INTO (xxx) ..". The result of this method is effectively cached,
     and it is invalidated when set of fields changes (e.g. using clear()
     or addField()).

     @a tableAlias, if provided is prepended to each field, so the resulting
     names will be in form tableAlias.fieldName. This option is used for building
     queries with joins, where fields have to be spicified without ambiguity.
     See @ref KDbConnection::selectStatement() for example use.

     @a escapingType can be used to alter default escaping type.
     If @a conn is not provided for DriverEscaping, no escaping is performed.
    */
    KDbEscapedString sqlFieldsList(KDbConnection *conn, const QString& separator = QLatin1String(","),
                                const QString& tableAlias = QString(),
                                KDb::IdentifierEscapingType escapingType = KDb::DriverEscaping) const;

    /*! Like above, but this is convenient static function, so you can pass any @a list here. */
    static KDbEscapedString sqlFieldsList(const KDbField::List& list, KDbConnection *conn,
                                       const QString& separator = QLatin1String(","),
                                       const QString& tableAlias = QString(),
                                       KDb::IdentifierEscapingType escapingType = KDb::DriverEscaping);

    /*! @internal Renames field @a oldName to @a newName.
     Do not use this for physical renaming columns. Use KDbAlterTableHandler instead. */
    void renameField(const QString& oldName, const QString& newName);

    /*! @internal
     @overload void renameField(const QString& oldName, const QString& newName) */
    void renameField(KDbField *field, const QString& newName);

protected:
    KDbField::List m_fields;
    mutable QHash<QString, KDbField*> m_fields_by_name; //!< Fields collected by name. Not used by KDbQuerySchema.
    mutable KDbField::List *m_autoinc_fields;

private:
    void renameFieldInternal(KDbField *field, const QString& newNameLower);

    //! cached
    mutable KDbEscapedString m_sqlFields;
};

//! Sends information about field list  @a list to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbFieldList& list);

#endif
