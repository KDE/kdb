/* This file is part of the KDE project
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_DRIVER_SQLITE_H
#define KDB_DRIVER_SQLITE_H

#include "KDbDriver.h"

class KDbConnection;
class SqliteDriverPrivate;

//! SQLite database driver.
class SqliteDriver : public KDbDriver
{
    Q_OBJECT

public:
    SqliteDriver(QObject *parent, const QVariantList &args);

    ~SqliteDriver() override;

    /*! @return true if @a n is a system object name;
      for this driver any object with name prefixed with "sqlite_"
      is considered as system object.
    */
    bool isSystemObjectName(const QString& n) const override;

    /*! @return false for this driver. */
    bool isSystemDatabaseName(const QString&) const override;

    //! Escape a string for use as a value
    KDbEscapedString escapeString(const QString& str) const override;
    KDbEscapedString escapeString(const QByteArray& str) const override;

    //! Escape BLOB value @a array
    KDbEscapedString escapeBLOB(const QByteArray& array) const override;

    /*! Implemented for KDbDriver class.
     @return SQL clause to add for unicode text collation sequence
     used in ORDER BY clauses of SQL statements generated by KDb.
     Later other clauses may use this statement.
     One space character should be be prepended.
     Can be reimplemented for other drivers, e.g. the SQLite3 driver returns " COLLATE ''".
     Default implementation returns empty string. */
    KDbEscapedString collationSQL() const override;

    //! Generates native (driver-specific) GREATEST() and LEAST() function calls.
    //! Uses MAX() and MIN(), respectively.
    //! If arguments are of text type, to each argument default (unicode) collation
    //! is assigned that is configured for SQLite by KexiDB.
    //! Example: SELECT MAX('ą' COLLATE '', 'z' COLLATE '').
    KDbEscapedString greatestOrLeastFunctionToString(const QString &name, const KDbNArgExpression &args,
                                    KDbQuerySchemaParameterValueListIterator *params,
                                    KDb::ExpressionCallStack *callStack) const override;

    //! Generates native (driver-specific) RANDOM() and RANDOM(X,Y) function calls.
    //! Accepted @a args can contain zero or two positive integer arguments X, Y; X < Y.
    //! In case of numeric arguments, RANDOM(X, Y) returns a random integer that is equal
    //! or greater than X and less than Y.
    //! Because SQLite returns integer between -9223372036854775808 and +9223372036854775807,
    //! RANDOM() for SQLite is equal to (RANDOM()+9223372036854775807)/18446744073709551615.
    //! Similarly, RANDOM(X,Y) for SQLite is equal to
    //! (X + CAST((Y-X) * (RANDOM()+9223372036854775807)/18446744073709551615 AS INT)).
    KDbEscapedString randomFunctionToString(const KDbNArgExpression &args,
                                            KDbQuerySchemaParameterValueListIterator* params,
                                            KDb::ExpressionCallStack* callStack) const override;

    //! Generates native (driver-specific) CEILING() and FLOOR() function calls.
    //! Default implementation USES CEILING() and FLOOR(), respectively.
    //! For CEILING() uses:
    //! (CASE WHEN X = CAST(X AS INT) THEN CAST(X AS INT) WHEN X >= 0 THEN CAST(X AS INT) + 1 ELSE CAST(X AS INT) END).
    //! For FLOOR() uses:
    //! (CASE WHEN X >= 0 OR X = CAST(X AS INT) THEN CAST(X AS INT) ELSE CAST(X AS INT) - 1 END).
    KDbEscapedString ceilingOrFloorFunctionToString(const QString &name, const KDbNArgExpression &args,
                                   KDbQuerySchemaParameterValueListIterator *params,
                                   KDb::ExpressionCallStack *callStack) const override;

protected:
    QString drv_escapeIdentifier(const QString& str) const override;
    QByteArray drv_escapeIdentifier(const QByteArray& str) const override;
    KDbConnection *drv_createConnection(const KDbConnectionData& connData,
                                                const KDbConnectionOptions &options) override;
    KDbAdminTools* drv_createAdminTools() const override;

    /*! @return true if @a n is a system field name;
      for this driver fields with name equal "_ROWID_"
      is considered as system field.
    */
    bool drv_isSystemFieldName(const QString& n) const override;

    SqliteDriverPrivate * const dp;

private:
    static const char * const keywords[];
    Q_DISABLE_COPY(SqliteDriver)
};

#endif
