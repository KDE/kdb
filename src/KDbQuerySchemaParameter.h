/* This file is part of the KDE project
   Copyright (C) 2006-2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_QUERYSCHEMAPARAMETER_H
#define KDB_QUERYSCHEMAPARAMETER_H

#include "KDbField.h"

//! @short A single parameter of a query schema
class KDB_EXPORT KDbQuerySchemaParameter
{
public:
    KDbQuerySchemaParameter();
    ~KDbQuerySchemaParameter();

    KDbField::Type type; //!< A datatype of the parameter
    QString message; //!< A user-visible message that will be displayed to ask for value of the parameter
};

//! @short An iterator for a list of values of query schema parameters
//! Allows to iterate over parameters and returns QVariant value or well-formatted string.
//! The iterator is initially set to the last item because of the parser requirements
class KDB_EXPORT KDbQuerySchemaParameterValueListIterator
{
public:
    KDbQuerySchemaParameterValueListIterator(const QList<QVariant>& params);
    ~KDbQuerySchemaParameterValueListIterator();

    //! @return previous value
    QVariant getPreviousValue();

protected:
    class Private;
    Private * const d;
};

//! Sends information about query schema parameter @a parameter to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbQuerySchemaParameter& parameter);

//! Sends information about query schema parameter list @a list to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const QList<KDbQuerySchemaParameter>& list);

#endif
