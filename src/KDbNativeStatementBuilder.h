/* This file is part of the KDE project
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_KDBNATIVESTATEMENTBUILDER_H
#define KDB_KDBNATIVESTATEMENTBUILDER_H

#include "KDbSelectStatementOptions.h"
#include <QList>
#include <QVariant>

#include "kdb_export.h"

class KDbConnection;
class KDbEscapedString;
class KDbQuerySchema;
class KDbTableSchema;

//! A builder for generating various types of native SQL statements
/*! The statement strings can be specific for the used connection and database driver,
    and thus generally not portable across connections. */
class KDB_EXPORT KDbNativeStatementBuilder
{
public:
    //! Creates a new native builder object. If @a connection is nullptr,
    //! generated statement strings are of KDbSQL dialect, else they are specific
    //! to database connection or connection's database driver.
    explicit KDbNativeStatementBuilder(KDbConnection *connection = nullptr);

    ~KDbNativeStatementBuilder();

    /*! Generates a native "SELECT ..." statement string that can be used for executing
     query defined by @a querySchema, @a params and @a options.

     @a target and @a querySchema must not be 0. The statement is written to @ref *target on success.
     @return true on success. */
    bool generateSelectStatement(KDbEscapedString *target, KDbQuerySchema* querySchema,
                                 const KDbSelectStatementOptions& options,
                                 const QList<QVariant>& parameters = QList<QVariant>()) const;

    /*! @overload generateSelectStatement(KDbEscapedString *target, KDbQuerySchema* querySchema,
                                         const KDbSelectStatementOptions& options,
                                         const QList<QVariant>& parameters) const. */
    bool generateSelectStatement(KDbEscapedString *target,
                                 KDbQuerySchema* querySchema,
                                 const QList<QVariant>& parameters = QList<QVariant>()) const;

    /*! Generates a native "SELECT ..." statement string that can be used for executing
     query defined by an functional equivalent of a "SELECT * FROM table_name" statement
     where <i>table_name</i> is @a tableSchema's name. @a params and @a options are used
     like in the
     @ref toSelectStatement(KDbEscapedString*, KDbQuerySchema*, const KDbSelectStatementOptions&, const QList<QVariant>&)
     variant.
     @a target and @a querySchema must not be 0.
     @return true on success. */
    bool generateSelectStatement(KDbEscapedString *target, KDbTableSchema* tableSchema,
                                 const KDbSelectStatementOptions& options = KDbSelectStatementOptions()) const;

    /*! Generates a native "CREATE TABLE ..." statement string that can be used for creation
     of @a tableSchema in the database. The statement is written to @ref *target on success.
     @a target must not be 0.
     @return true on success. */
    bool generateCreateTableStatement(KDbEscapedString *target,
                                      const KDbTableSchema& tableSchema) const;

private:
    Q_DISABLE_COPY(KDbNativeStatementBuilder)
    class Private;
    Private * const d;
};

#endif
