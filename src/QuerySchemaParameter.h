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

#ifndef PREDICATE_QUERYSCHEMAPARAMETER_H
#define PREDICATE_QUERYSCHEMAPARAMETER_H

#include <Predicate/QuerySchema.h>
#include <QtDebug>

namespace Predicate
{

//! @short A single parameter of a query schema
class PREDICATE_EXPORT QuerySchemaParameter
{
public:
    QuerySchemaParameter();
    ~QuerySchemaParameter();

    Field::Type type; //!< A datatype of the parameter
    QString message; //!< A user-visible message that will be displayed to ask for value of the parameter
};

typedef QList<QuerySchemaParameter>::Iterator QuerySchemaParameterListIterator;
typedef QList<QuerySchemaParameter>::ConstIterator QuerySchemaParameterListConstIterator;

//! @short An iteratof for a list of values of query schema parameters providing
//! Allows to iterate over parameters and return QVariant value or well-formatted string.
//! The iterator is initially set to the last item because of the parser requirements
class PREDICATE_EXPORT QuerySchemaParameterValueListIterator
{
public:
    QuerySchemaParameterValueListIterator(Driver* driver, const QList<QVariant>& params);
    ~QuerySchemaParameterValueListIterator();

    //! @return previous value
    QVariant getPreviousValue();

    //! @return previous value as string formatted using driver's escaping
    EscapedString getPreviousValueAsString(Field::Type type);
protected:
    class Private;
    Private * const d;
};

} //namespace Predicate

//! Sends information about query schema parameter @a parameter to debug output @a dbg.
PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Predicate::QuerySchemaParameter& parameter);

//! Sends information about query schema parameter list @a list to debug output @a dbg.
PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Predicate::QuerySchemaParameterList& list);

#endif
