/* This file is part of the KDE project
   Copyright (C) 2006-2015 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_LOOKUPFIELDSCHEMA_H
#define KDB_LOOKUPFIELDSCHEMA_H

#include <QMap>
#include "KDbGlobal.h"

class QStringList;
class QDomElement;
class QDomDocument;
class QVariant;

//! default value for KDbLookupFieldSchema::columnHeadersVisible()
#define KDB_LOOKUP_FIELD_DEFAULT_HEADERS_VISIBLE false

//! default value for KDbLookupFieldSchema::maxVisibleRecords()
#define KDB_LOOKUP_FIELD_DEFAULT_MAX_VISIBLE_RECORDS 8

//! upper limit for KDbLookupFieldSchema::maxVisibleRecords()
#define KDB_LOOKUP_FIELD_LIMIT_MAX_VISIBLE_RECORDS 100

//! default value for KDbLookupFieldSchema::limitToList()
#define KDB_LOOKUP_FIELD_DEFAULT_LIMIT_TO_LIST true

//! default value for KDbLookupFieldSchema::displayWidget()
#define KDB_LOOKUP_FIELD_DEFAULT_DISPLAY_WIDGET KDbLookupFieldSchema::ComboBox

//! @short Provides information about lookup field's setup.
/*!
 KDbLookupFieldSchema object is owned by KDbTableSchema and created upon creating or retrieving the table schema
 from the database metadata.

 @see KDbLookupFieldSchema *KDbTableSchema::lookupFieldSchema( KDbField& field ) const
*/
class KDB_EXPORT KDbLookupFieldSchema
{
public:

    //! Record source information that can be specified for the lookup field schema
    class KDB_EXPORT RecordSource
    {
    public:
        //! Record source type
        enum Type {
            NoType,         //!< used for invalid schema
            Table,        //!< table as lookup record source
            Query,        //!< named query as lookup record source
            SQLStatement, //!< anonymous query as lookup record source
            ValueList,    //!< a fixed list of values as lookup record source
            KDbFieldList     //!< a list of column names from a table/query will be displayed
        };

        RecordSource();
        RecordSource(const RecordSource& other);
        ~RecordSource();

        /*! @return record source type: table, query, anonymous; in the future it will
         be also fixed value list and field list. The latter is basically a list
         of column names of a table/query, "Field List" in MSA. */
        Type type() const;

        /*! Sets record source type to @a type. */
        void setType(Type type);

        /*! @return record source type name. @see setTypeByName() */
        QString typeName() const;

        /*! Sets record source type by name using @a typeName. Accepted (case sensitive)
         names are "table", "query", "sql", "valuelist", "fieldlist".
         For other value NoType type is set. */
        void setTypeByName(const QString& typeName);

        /*! @return a string for record source: table name, query name or anonymous query
         provided as KEXISQL string. If recordSourceType() is a ValueList,
         recordSourceValues() should be used instead. If recordSourceType() is a KDbFieldList,
         recordSource() should return table or query name. */
        QString name() const;

        /*! Sets record source value. @see value() */
        void setName(const QString& name);

        /*! @return record source values specified if type() is ValueList. */
        QStringList values() const;

        /*! Sets record source values used if type() is ValueList.
         Using it clears name (see name()). */
        void setValues(const QStringList& values);

        //! Assigns other to this record source and returns a reference to this record source.
        RecordSource& operator=(const RecordSource& other);

    private:
        class Private;
        Private * const d;
    };

    KDbLookupFieldSchema();

    KDbLookupFieldSchema(const KDbLookupFieldSchema &schema);

    ~KDbLookupFieldSchema();

    /*! @return record source information for the lookup field schema */
    RecordSource recordSource() const;

    /*! Sets record source for the lookup field schema */
    void setRecordSource(const RecordSource& recordSource);

    /*! @return bound column: an integer specifying a column that is bound
     (counted from 0). -1 means unspecified value. */
//! @todo in later implementation there can be more columns
    int boundColumn() const;

    /*! Sets bound column number to @a column. @see boundColumn() */
    void setBoundColumn(int column);

    /*! @return a list of visible columns: a list of integers specifying indices (counted from 0)
     of columns within the row source that are visible in the combo box.
     Empty list means unspecified value. */
    QList<uint> visibleColumns() const;

    /*! Sets a list of visible columns to \a list. @see visibleColumns() */
    void setVisibleColumns(const QList<uint>& list);

    /*! A helper method.
     If index >= visibleColumns().count(), -1 is returned,
     else \a index is returned. */
    int visibleColumn(uint index) const;

    /*! @return a number of ordered integers specifying column widths;
     -1 means 'default width' for a given column. */
    QList<int> columnWidths() const;

    /*! Sets column widths. @see columnWidths() */
    void setColumnWidths(const QList<int>& widths);

    /*! @return true if column headers are visible in the associated
     combo box popup or the list view. The default is false. */
    bool columnHeadersVisible() const;

    /*! Sets "column headers visibility" flag. @see columnHeadersVisible() */
    void setColumnHeadersVisible(bool set);

    /*! @return integer property specifying a maximum number of records
     that can be displayed in a combo box popup or a list box. The default is
     equal to KDB_LOOKUP_FIELD_DEFAULT_MAX_VISIBLE_RECORD_COUNT constant. */
    uint maxVisibleRecords() const;

    /*! Sets maximum number of records that can be displayed in a combo box popup
     or a list box. If @a count is 0, KDB_LOOKUP_FIELD_DEFAULT_MAX_VISIBLE_RECORD_COUNT is set.
     If @a count is greater than KDB_LOOKUP_FIELD_MAX_LIST_ROWS,
     KDB_LOOKUP_FIELD_MAX_LIST_ROWS is set. */
    void setMaxVisibleRecords(uint count);

    /*! @return true if , only values present on the list can be selected using
     the combo box. The default is true. */
    bool limitToList() const;

    /*! Sets "limit to list" flag. @see limitToList() */
    void setLimitToList(bool set);

    //! used in displayWidget()
    enum DisplayWidget {
        ComboBox = 0, //!< (the default) combobox widget should be displayed in forms for this lookup field
        ListBox = 1   //!< listbox widget should be displayed in forms for this lookup field
    };

    /*! @return the widget type that should be displayed within
     the forms for this lookup field. The default is ComboBox.
     For the Table View, combo box is always displayed. */
    DisplayWidget displayWidget() const;

    /*! Sets type of widget to display within the forms for this lookup field. @see displayWidget() */
    void setDisplayWidget(DisplayWidget widget);

    /*! Loads data of lookup column schema from DOM tree.
     The data can be outdated or invalid, so the app should handle such cases.
     @return a new KDbLookupFieldSchema object even if lookupEl contains no valid contents. */
    static KDbLookupFieldSchema* loadFromDom(const QDomElement& lookupEl);

    /*! Saves data of lookup column schema to @a parentEl DOM element of @a doc document. */
    void saveToDom(QDomDocument *doc, QDomElement *parentEl);

    /*! Gets property values for the lookup schema.
     @a values is cleared before filling.
     This function is used e.g. for altering table design. */
    void getProperties(QMap<QByteArray, QVariant> *values) const;

    /*! Sets property of name @a propertyName and value @a value for the lookup schema @a lookup
     @return true on successful set and false on failure because of invalid value or invalid property name. */
    bool setProperty(const QByteArray& propertyName, const QVariant& value);

    /*! Sets property values for the lookup schema.
     Properties coming from extended schema are also supported.
     Properties not listed are kept untouched.
     This function is used e.g. for altering table design.
     @return true on successful set and false on failure because of invalid value or invalid property name. */
    bool setProperties(const QMap<QByteArray, QVariant>& values);

private:
    class Private;
    Private * const d;
};

//! Sends lookup field schema's record source information @a source to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbLookupFieldSchema::RecordSource& source);

//! Sends lookup field schema information @a lookup to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbLookupFieldSchema& lookup);

#endif
