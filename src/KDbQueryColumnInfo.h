/* This file is part of the KDE project
   Copyright (C) 2003-2014 Jarosław Staniek <staniek@kde.org>

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

#include <QVector>
#include <QHash>

class KDbField;

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

    KDbQueryColumnInfo(KDbField *f, const QString& _alias, bool _visible,
                       KDbQueryColumnInfo *foreignColumn = 0);
    ~KDbQueryColumnInfo();

    //! @return alias if it is not empty, field's name otherwise.
    QString aliasOrName() const;

    //! @return field's caption if it is not empty, field's alias otherwise.
    //! If alias is also empty - returns field's name.
    QString captionOrAliasOrName() const;

    KDbField *field;
    QString alias;

    /*! @return index of column with visible lookup value within the 'fields expanded' vector.
     -1 means no visible lookup value is available because there is no lookup for the column defined.
     Cached for efficiency as we use this information frequently.
     @see KDbLookupFieldSchema::visibleVolumn() */
    int indexForVisibleLookupValue() const;

    /*! Sets index of column with visible lookup value within the 'fields expanded' vector. */
    void setIndexForVisibleLookupValue(int index);

    //! @return non-0 if this column is a visible column for other column
    KDbQueryColumnInfo *foreignColumn() const;

    //! true if this column is visible to the user (and its data is fetched by the engine)
    bool visible;

private:
    /*! Index of column with visible lookup value within the 'fields expanded' vector.
     @see indexForVisibleLookupValue() */
    int m_indexForVisibleLookupValue;

    //! Non-0 if this column is a visible column for @a m_foreignColumn
    KDbQueryColumnInfo *m_foreignColumn;
};

//! Sends information about column info @a info to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbQueryColumnInfo& info);

#endif
