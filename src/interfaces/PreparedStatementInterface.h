/* This file is part of the KDE project
   Copyright (C) 2008-2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef PREDICATE_PREPAREDSTATEMENT_IFACE_H
#define PREDICATE_PREPAREDSTATEMENT_IFACE_H

#include <QVariant>
#include <QStringList>
#include <QSharedData>

#include <Predicate/Result.h>
#include <Predicate/PreparedStatement.h>

namespace Predicate
{

//! Prepared statement interface for backend-dependent implementations.
class PREDICATE_EXPORT PreparedStatementInterface : public Resultable
{
protected:
    PreparedStatementInterface() {}
    virtual ~PreparedStatementInterface() {}

    /*! For implementation. Initializes the prepared statement in a backend-dependent way
        using recently generated @a statement. 
        It is guaranteed that @a statement valid and not empty.
        For example sqlite3_prepare() is used for SQLite.
        This is called only when d->dirty == true is encountered on execute(),
        i.e. when attributes of the object (like WHERE field names) change. */
    virtual bool prepare(const EscapedString& statement) = 0;

    //! For implementation. Executes the prepared statement using parameters @a parameters. 
    virtual bool execute(
        PreparedStatement::Type type,
        const Field::List& fieldList,
        const PreparedStatementParameters& parameters) = 0;

    friend class Connection;
    friend class PreparedStatement;
};

} //namespace Predicate

#endif
