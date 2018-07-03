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

#include "SqliteDriver.h"
#include "SqliteConnection.h"
#include "SqliteConnection_p.h"
#include "SqliteAdmin.h"

#include "KDbConnection.h"
#include "KDbDriverManager.h"
#include "KDbDriverBehavior.h"
#include "KDbExpression.h"
#include "KDb.h"

#include <KPluginFactory>

#include <sqlite3.h>

KDB_DRIVER_PLUGIN_FACTORY(SqliteDriver, "kdb_sqlitedriver.json")

//! driver specific private data
//! @internal
class SqliteDriverPrivate
{
public:
    SqliteDriverPrivate()
     : collate(QLatin1String(" COLLATE ''"))
    {
    }
    KDbEscapedString collate;
    Q_DISABLE_COPY(SqliteDriverPrivate)
};

SqliteDriver::SqliteDriver(QObject *parent, const QVariantList &args)
        : KDbDriver(parent, args)
        , dp(new SqliteDriverPrivate)
{
    KDbDriverBehavior *beh = behavior();
    beh->features = SingleTransactions | CursorForward | CompactingDatabaseSupported;

    //special method for autoincrement definition
    beh->SPECIAL_AUTO_INCREMENT_DEF = true;
    beh->AUTO_INCREMENT_FIELD_OPTION = QString(); //not available
    beh->AUTO_INCREMENT_TYPE = QLatin1String("INTEGER");
    beh->AUTO_INCREMENT_PK_FIELD_OPTION = QLatin1String("PRIMARY KEY");
    beh->AUTO_INCREMENT_REQUIRES_PK = true;
    beh->ROW_ID_FIELD_NAME = QLatin1String("OID");
    beh->IS_DB_OPEN_AFTER_CREATE = true;
    beh->_1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY = true;
    beh->OPENING_QUOTATION_MARK_BEGIN_FOR_IDENTIFIER = '[';
    beh->CLOSING_QUOTATION_MARK_BEGIN_FOR_IDENTIFIER = ']';
    beh->SELECT_1_SUBQUERY_SUPPORTED = true;
    beh->CONNECTION_REQUIRED_TO_CHECK_DB_EXISTENCE = false;
    beh->CONNECTION_REQUIRED_TO_CREATE_DB = false;
    beh->CONNECTION_REQUIRED_TO_DROP_DB = false;
    beh->GET_TABLE_NAMES_SQL
        = KDbEscapedString("SELECT name FROM sqlite_master WHERE type='table'");

    initDriverSpecificKeywords(keywords);

    // internal properties
    beh->properties.insert("client_library_version", QLatin1String(sqlite3_libversion()));
    beh->properties.insert("default_server_encoding", QLatin1String("UTF8")); //OK?

    beh->typeNames[KDbField::Byte] = QLatin1String("Byte");
    beh->typeNames[KDbField::ShortInteger] = QLatin1String("ShortInteger");
    beh->typeNames[KDbField::Integer] = QLatin1String("Integer");
    beh->typeNames[KDbField::BigInteger] = QLatin1String("BigInteger");
    beh->typeNames[KDbField::Boolean] = QLatin1String("Boolean");
    beh->typeNames[KDbField::Date] = QLatin1String("Date"); // In fact date/time types could be declared as datetext etc.
    beh->typeNames[KDbField::DateTime] = QLatin1String("DateTime"); // to force text affinity..., see https://sqlite.org/datatype3.html
    beh->typeNames[KDbField::Time] = QLatin1String("Time");
    beh->typeNames[KDbField::Float] = QLatin1String("Float");
    beh->typeNames[KDbField::Double] = QLatin1String("Double");
    beh->typeNames[KDbField::Text] = QLatin1String("Text");
    beh->typeNames[KDbField::LongText] = QLatin1String("CLOB");
    beh->typeNames[KDbField::BLOB] = QLatin1String("BLOB");
}

SqliteDriver::~SqliteDriver()
{
    delete dp;
}


KDbConnection*
SqliteDriver::drv_createConnection(const KDbConnectionData& connData,
                                   const KDbConnectionOptions &options)
{
    return new SqliteConnection(this, connData, options);
}

bool SqliteDriver::isSystemObjectName(const QString& name) const
{
    return name.startsWith(QLatin1String("sqlite_"), Qt::CaseInsensitive);
}

bool SqliteDriver::isSystemDatabaseName(const QString& name) const
{
    Q_UNUSED(name);
    return false;
}

bool SqliteDriver::drv_isSystemFieldName(const QString& name) const
{
    return    0 == name.compare(QLatin1String("_rowid_"), Qt::CaseInsensitive)
           || 0 == name.compare(QLatin1String("rowid"), Qt::CaseInsensitive)
           || 0 == name.compare(QLatin1String("oid"), Qt::CaseInsensitive);
}

KDbEscapedString SqliteDriver::escapeString(const QString& str) const
{
    return KDbEscapedString("'") + KDbEscapedString(str).replace('\'', "''") + '\'';
}

KDbEscapedString SqliteDriver::escapeString(const QByteArray& str) const
{
    return KDbEscapedString("'") + KDbEscapedString(str).replace('\'', "''") + '\'';
}

KDbEscapedString SqliteDriver::escapeBLOB(const QByteArray& array) const
{
    return KDbEscapedString(KDb::escapeBLOB(array, KDb::BLOBEscapingType::XHex));
}

QString SqliteDriver::drv_escapeIdentifier(const QString& str) const
{
    return QString(str).replace(QLatin1Char('"'), QLatin1String("\"\""));
}

QByteArray SqliteDriver::drv_escapeIdentifier(const QByteArray& str) const
{
    return QByteArray(str).replace('"', "\"\"");
}

KDbAdminTools* SqliteDriver::drv_createAdminTools() const
{
    return new SqliteAdminTools();
}

KDbEscapedString SqliteDriver::collationSql() const
{
    return dp->collate;
}

KDbEscapedString SqliteDriver::greatestOrLeastFunctionToString(const QString &name,
                                                      const KDbNArgExpression &args,
                                                      KDbQuerySchemaParameterValueListIterator* params,
                                                      KDb::ExpressionCallStack* callStack) const
{
    Q_ASSERT(args.argCount() >= 2);
    static QString greatestString(QLatin1String("GREATEST"));
    static QString maxString(QLatin1String("MAX"));
    static QString minString(QLatin1String("MIN"));
    const QString realName(
        name == greatestString ? maxString : minString);
    if (args.argCount() >= 2 && KDbField::isTextType(args.arg(0).type())) {
        KDbEscapedString s;
        s.reserve(256);
        for(int i=0; i < args.argCount(); ++i) {
            if (!s.isEmpty()) {
                s += ", ";
            }
            s += QLatin1Char('(') + args.arg(i).toString(this, params, callStack) + QLatin1String(") ") + collationSql();
        }
        return realName + QLatin1Char('(') + s + QLatin1Char(')');
    }
    return KDbFunctionExpression::toString(realName, this, args, params, callStack);
}

KDbEscapedString SqliteDriver::randomFunctionToString(const KDbNArgExpression &args,
                                             KDbQuerySchemaParameterValueListIterator* params,
                                             KDb::ExpressionCallStack* callStack) const
{
    if (args.isNull() || args.argCount() < 1 ) {
        static KDbEscapedString randomStatic("((RANDOM()+9223372036854775807)/18446744073709551615)");
        return randomStatic;
    }
    Q_ASSERT(args.argCount() == 2);
    const KDbEscapedString x(args.arg(0).toString(this, params, callStack));
    const KDbEscapedString y(args.arg(1).toString(this, params, callStack));
    static KDbEscapedString floorRandomStatic("+CAST(((");
    static KDbEscapedString floorRandomStatic2("))*(RANDOM()+9223372036854775807)/18446744073709551615 AS INT))");
    //! (X + CAST((Y - X) * (RANDOM()+9223372036854775807)/18446744073709551615 AS INT)).
    return KDbEscapedString("((") + x + QLatin1Char(')') + floorRandomStatic + y + QLatin1Char(')')
            + QLatin1String("-(") + x + floorRandomStatic2;
}

KDbEscapedString SqliteDriver::ceilingOrFloorFunctionToString(const QString &name,
                                                     const KDbNArgExpression &args,
                                                     KDbQuerySchemaParameterValueListIterator* params,
                                                     KDb::ExpressionCallStack* callStack) const
{
    Q_ASSERT(args.argCount() == 1);
    static QLatin1String ceilingString("CEILING");
    KDbEscapedString x(args.arg(0).toString(this, params, callStack));
    if (name == ceilingString) {
        return KDbEscapedString("(CASE WHEN ")
            + x + QLatin1String("=CAST(") + x + QLatin1String(" AS INT) THEN CAST(")
            + x + QLatin1String(" AS INT) WHEN ")
            + x + QLatin1String(">=0 THEN CAST(")
            + x + QLatin1String(" AS INT)+1 ELSE CAST(")
            + x + QLatin1String(" AS INT) END)");
    }
    // floor():
    return KDbEscapedString("(CASE WHEN ") + x + QLatin1String(">=0 OR ")
            + x + QLatin1String("=CAST(") + x + QLatin1String(" AS INT) THEN CAST(")
            + x + QLatin1String(" AS INT) ELSE CAST(")
            + x + QLatin1String(" AS INT)-1 END)");
}

#include "SqliteDriver.moc"
