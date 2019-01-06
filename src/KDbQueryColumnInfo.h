/* This file is part of the KDE project
   Copyright (C) 2003-2018 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_QUERYCOLUMNINFO_H
#define KDB_QUERYCOLUMNINFO_H

#include "kdb_export.h"

#include <QList>
#include <QVector>
#include <QString>

class KDbConnection;
class KDbField;
class KDbQuerySchema;

//! @short Helper class that assigns additional information for the column in a query
/*! The following information is assigned:
   - alias
   - visibility
  KDbQueryColumnInfo::Vector is created and returned by KDbQuerySchema::fieldsExpanded().
  It is efficiently cached within the KDbQuerySchema object.
*/
class KDB_EXPORT KDbQueryColumnInfo
{
public:
    typedef QVector<KDbQueryColumnInfo*> Vector;
    typedef QList<KDbQueryColumnInfo*> List;
    typedef QList<KDbQueryColumnInfo*>::ConstIterator ListIterator;

    KDbQueryColumnInfo(KDbField *f, const QString &alias, bool visible,
                       KDbQueryColumnInfo *foreignColumn = nullptr);
    ~KDbQueryColumnInfo();

    //! @return field for this column
    KDbField *field();

    //! @overload KDbField *field()
    const KDbField *field() const;

    //! Sets the field
    void setField(KDbField *field);

    //! @return alias for this column
    QString alias() const;

    //! Sets the alias
    void setAlias(const QString &alias);

    //! @return alias if it is not empty, field's name otherwise.
    QString aliasOrName() const;

    //! @return field's caption if it is not empty, field's alias otherwise.
    //! If alias is also empty - returns field's name.
    QString captionOrAliasOrName() const;

    //! @return true is this column is visible
    bool isVisible() const;

    //! Sets the visible flag
    void setVisible(bool set);

    /*! @return index of column with visible lookup value within the 'fields expanded' vector.
     -1 means no visible lookup value is available because there is no lookup for the column defined.
     Cached for efficiency as we use this information frequently.
     @see KDbLookupFieldSchema::visibleVolumn() */
    int indexForVisibleLookupValue() const;

    /*! Sets index of column with visible lookup value within the 'fields expanded' vector. */
    void setIndexForVisibleLookupValue(int index);

    //! @return non-nullptr if this column is a visible column for other column
    KDbQueryColumnInfo *foreignColumn();

    //! @overload KDbQueryColumnInfo *foreignColumn();
    const KDbQueryColumnInfo *foreignColumn() const;

    /**
     * Returns query schema for this column
     *
     * @since 3.2
     */
    const KDbQuerySchema* querySchema() const;

    /**
     * Returns connection for this column
     *
     * @since 3.2
     */
    KDbConnection* connection();

    /**
     * @overload
     *
     * @since 3.2
     */
    const KDbConnection* connection() const;

private:
    friend class KDbQuerySchema;
    class Private;
    Private * const d;
    Q_DISABLE_COPY(KDbQueryColumnInfo)
};

//! Sends information about column info @a info to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbQueryColumnInfo& info);

#endif
