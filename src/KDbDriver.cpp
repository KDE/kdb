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

#include "KDbDriver.h"
#include "KDbDriver_p.h"
#include "KDbDriverManager_p.h"
#include "KDbError.h"
#include "KDbConnection.h"
#include "KDbConnectionData.h"
#include "KDbAdmin.h"
#include "KDbExpression.h"
#include "kdb_debug.h"

#include <assert.h>

/*! @internal Used in KDbDriver::defaultSQLTypeName(int)
 when we do not have KDbDriver instance yet, or when we cannot get one */
static const char* const KDb_defaultSQLTypeNames[] = {
    "InvalidType",
    "Byte",
    "ShortInteger",
    "Integer",
    "BigInteger",
    "Boolean",
    "Date",
    "DateTime",
    "Time",
    "Float",
    "Double",
    "Text",
    "LongText",
    "BLOB"
};

//---------------------------------------------

KDbDriver::KDbDriver(QObject *parent, const QVariantList &args)
 : QObject(parent)
 , beh(new KDbDriverBehaviour())
 , d(new DriverPrivate(this))
{
    Q_UNUSED(args);
    d->typeNames.resize(KDbField::LastType + 1);
}

KDbDriver::~KDbDriver()
{
    // make a copy because d->connections will be touched by ~KDbConnection
    QSet<KDbConnection*> connections(d->connections);
    qDeleteAll(connections);
    d->connections.clear();
    delete beh;
    delete d;
// kdbDebug() << "ok";
}

bool KDbDriver::isValid()
{
    clearResult();
    QString inv_impl(tr("Invalid database driver's \"%1\" implementation.").arg(metaData()->name()));
    QString not_init(tr("Value of \"%1\" is not initialized for the driver."));
    if (beh->ROW_ID_FIELD_NAME.isEmpty()) {
        m_result = KDbResult(ERR_INVALID_DRIVER_IMPL,
                          inv_impl + QLatin1Char(' ')
                          + not_init.arg(QLatin1String("KDbDriverBehaviour::ROW_ID_FIELD_NAME")));
        return false;
    }
    return true;
}

const QSet<KDbConnection*> KDbDriver::connections() const
{
    return d->connections;
}

const KDbDriverMetaData* KDbDriver::metaData() const
{
    return d->metaData;
}

int KDbDriver::features() const
{
    return d->features;
}

bool KDbDriver::transactionsSupported() const
{
    return d->features & (SingleTransactions | MultipleTransactions);
}

KDbAdminTools& KDbDriver::adminTools() const
{
    if (!d->adminTools)
        d->adminTools = drv_createAdminTools();
    return *d->adminTools;
}

KDbAdminTools* KDbDriver::drv_createAdminTools() const
{
    return new KDbAdminTools(); //empty impl.
}

QString KDbDriver::sqlTypeName(int id_t, int p) const
{
    Q_UNUSED(p);
    if (id_t > KDbField::InvalidType && id_t <= KDbField::LastType) { /*sanity*/
        return d->typeNames[id_t];
    }
    return d->typeNames[KDbField::InvalidType];
}

KDbConnection *KDbDriver::createConnection(const KDbConnectionData& connData,
                                           const KDbConnectionOptions &options)
{
    clearResult();
    if (!isValid())
        return 0;

    KDbConnection *conn = drv_createConnection(connData, options);

//! @todo needed? connData->setDriverId(id());
    d->connections.insert(conn);
    return conn;
}

KDbConnection *KDbDriver::createConnection(const KDbConnectionData& connData)
{
    return createConnection(connData, KDbConnectionOptions());
}

KDbConnection* KDbDriver::removeConnection(KDbConnection *conn)
{
    clearResult();
    if (d->connections.remove(conn))
        return conn;
    return 0;
}

QString KDbDriver::defaultSQLTypeName(int id_t)
{
    if (id_t < 0 || id_t > KDbField::LastType)
        return QLatin1String("Null");
    return QLatin1String(KDb_defaultSQLTypeNames[id_t]);
}

bool KDbDriver::isKDbSystemObjectName(const QString& n)
{
    if (!n.startsWith(QLatin1String("kexi__"), Qt::CaseInsensitive))
        return false;
    return KDbConnection::kdbSystemTableNames().contains(n, Qt::CaseInsensitive);
}

bool KDbDriver::isSystemFieldName(const QString& n) const
{
    if (!beh->ROW_ID_FIELD_NAME.isEmpty()
        && 0 == n.compare(beh->ROW_ID_FIELD_NAME, Qt::CaseInsensitive))
    {
        return true;
    }
    return drv_isSystemFieldName(n);
}

static KDbEscapedString valueToSQLInternal(const KDbDriver *driver, KDbField::Type ftype, const QVariant& v)
{
    if (v.isNull() || ftype == KDbField::Null) {
        return KDbEscapedString("NULL");
    }
    switch (ftype) {
    case KDbField::Text:
    case KDbField::LongText: {
        return driver ? driver->escapeString(v.toString())
                      : KDbEscapedString(KDb::escapeString(v.toString()));
    }
    case KDbField::Byte:
    case KDbField::ShortInteger:
    case KDbField::Integer:
    case KDbField::BigInteger:
        return KDbEscapedString(v.toByteArray());
    case KDbField::Float:
    case KDbField::Double: {
        if (v.type() == QVariant::String) {
            //workaround for values stored as string that should be casted to floating-point
            KDbEscapedString s(v.toByteArray());
            return s.replace(',', '.');
        }
        return KDbEscapedString(v.toByteArray());
    }
//! @todo here special encoding method needed
    case KDbField::Boolean:
        return driver
            ? KDbEscapedString(v.toInt() == 0 ? driver->behaviour()->BOOLEAN_FALSE_LITERAL
                                              : driver->behaviour()->BOOLEAN_TRUE_LITERAL)
            : KDbEscapedString(v.toInt() == 0 ? "FALSE" : "TRUE");
    case KDbField::Time:
        return KDbEscapedString('\'') + v.toTime().toString(Qt::ISODate) + '\'';
    case KDbField::Date:
        return KDbEscapedString('\'') + v.toDate().toString(Qt::ISODate) + '\'';
    case KDbField::DateTime:
        return driver ? driver->dateTimeToSQL(v.toDateTime())
                      : KDb::dateTimeToSQL(v.toDateTime());
    case KDbField::BLOB: {
        if (v.toByteArray().isEmpty()) {
            return KDbEscapedString("NULL");
        }
        if (v.type() == QVariant::String) {
            return driver ? driver->escapeBLOB(v.toString().toUtf8())
                          : KDbEscapedString(KDb::escapeBLOB(v.toString().toUtf8(), KDb::BLOBEscape0xHex));
        }
        return driver ? driver->escapeBLOB(v.toByteArray())
                      : KDbEscapedString(KDb::escapeBLOB(v.toByteArray(), KDb::BLOBEscape0xHex));
    }
    case KDbField::InvalidType:
        return KDbEscapedString("!INVALIDTYPE!");
    default:
        kdbDebug() << KDbEscapedString("UNKNOWN!");
    }
    return KDbEscapedString();
}

KDbEscapedString KDbDriver::valueToSQL(KDbField::Type ftype, const QVariant& v) const
{
    //! note, it was compatible with SQLite: http://www.sqlite.org/cvstrac/wiki?p=DateAndTimeFunctions.
    return valueToSQLInternal(this, ftype, v);
}

KDbEscapedString KDb::valueToSQL(KDbField::Type ftype, const QVariant& v)
{
    return valueToSQLInternal(0, ftype, v);
}

KDbEscapedString KDb::dateTimeToSQL(const QDateTime& v)
{
    /*! (was compatible with SQLite: http://www.sqlite.org/cvstrac/wiki?p=DateAndTimeFunctions)
        Now it's ISO 8601 DateTime format - with "T" delimiter:
        http://www.w3.org/TR/NOTE-datetime
        (e.g. "1994-11-05T13:15:30" not "1994-11-05 13:15:30")
        @todo add support for time zones?
    */
    return KDbEscapedString('\'') + v.toString(Qt::ISODate) + KDbEscapedString('\'');
}

KDbEscapedString KDbDriver::dateTimeToSQL(const QDateTime& v) const
{
    return KDb::dateTimeToSQL(v);
}

QString KDbDriver::escapeIdentifier(const QString& str) const
{
    return QLatin1Char(beh->QUOTATION_MARKS_FOR_IDENTIFIER) + drv_escapeIdentifier(str)
            + QLatin1Char(beh->QUOTATION_MARKS_FOR_IDENTIFIER);
}

QByteArray KDbDriver::escapeIdentifier(const QByteArray& str) const
{
    return beh->QUOTATION_MARKS_FOR_IDENTIFIER + drv_escapeIdentifier(str)
        + beh->QUOTATION_MARKS_FOR_IDENTIFIER;
}

KDbUtils::Property KDbDriver::internalProperty(const QByteArray& name) const
{
    return d->properties.property(name);
}

QList<QByteArray> KDbDriver::internalPropertyNames() const
{
    QList<QByteArray> names(d->properties.names());
    qSort(names);
    return names;
}

void KDbDriver::initDriverSpecificKeywords(const char* const* keywords)
{
    d->driverSpecificSQLKeywords.setStrings(keywords);
}

KDbEscapedString KDbDriver::addLimitTo1(const KDbEscapedString& sql, bool add)
{
    return add ? (sql + " LIMIT 1") : sql;
}

bool KDbDriver::isDriverSpecificKeyword(const QByteArray& word) const
{
    return d->driverSpecificSQLKeywords.contains(word);
}

void KDbDriver::setMetaData(const KDbDriverMetaData *metaData)
{
    d->metaData = metaData;
    d->initInternalProperties();
}

KDbEscapedString KDbDriver::hexFunctionToString(
                                       const KDbNArgExpression &args,
                                       KDbQuerySchemaParameterValueListIterator* params,
                                       KDb::ExpressionCallStack* callStack) const
{
    return KDbFunctionExpression::toString(QLatin1String("HEX"), this, args, params, callStack);
}

KDbEscapedString KDbDriver::ifnullFunctionToString(
                                          const KDbNArgExpression &args,
                                          KDbQuerySchemaParameterValueListIterator* params,
                                          KDb::ExpressionCallStack* callStack) const
{
    return KDbFunctionExpression::toString(QLatin1String("IFNULL"), this, args, params, callStack);
}

KDbEscapedString KDbDriver::lengthFunctionToString(
                                          const KDbNArgExpression &args,
                                          KDbQuerySchemaParameterValueListIterator* params,
                                          KDb::ExpressionCallStack* callStack) const
{
    return KDbFunctionExpression::toString(QLatin1String("LENGTH"), this, args, params, callStack);
}

KDbEscapedString KDbDriver::greatestOrLeastFunctionToString(
                                            const QString &name,
                                            const KDbNArgExpression &args,
                                            KDbQuerySchemaParameterValueListIterator* params,
                                            KDb::ExpressionCallStack* callStack) const
{
    return KDbFunctionExpression::toString(name, this, args, params, callStack);
}

KDbEscapedString KDbDriver::randomFunctionToString(
                                            const KDbNArgExpression &args,
                                            KDbQuerySchemaParameterValueListIterator* params,
                                            KDb::ExpressionCallStack* callStack) const
{
    static QLatin1String randomStatic("()");
    if (!args.isNull() || args.argCount() < 1 ) {
        return KDbEscapedString(beh->RANDOM_FUNCTION + randomStatic);
    }
    Q_ASSERT(args.argCount() == 2);
    const KDbEscapedString x(args.arg(0).toString(this, params, callStack));
    const KDbEscapedString y(args.arg(1).toString(this, params, callStack));
    static KDbEscapedString floorRandomStatic("+FLOOR(");
    static KDbEscapedString floorRandomStatic2("()*(");
    static KDbEscapedString floorRandomStatic3(")))");
    return KDbEscapedString('(') + x + floorRandomStatic + beh->RANDOM_FUNCTION
            + floorRandomStatic2 + y + QLatin1Char('-') + x + floorRandomStatic3;
}

KDbEscapedString KDbDriver::ceilingOrFloorFunctionToString(
                                                const QString &name,
                                                const KDbNArgExpression &args,
                                                KDbQuerySchemaParameterValueListIterator* params,
                                                KDb::ExpressionCallStack* callStack) const
{
    return KDbFunctionExpression::toString(name, this, args, params, callStack);
}

KDbEscapedString KDbDriver::unicodeFunctionToString(
                                        const KDbNArgExpression &args,
                                        KDbQuerySchemaParameterValueListIterator* params,
                                        KDb::ExpressionCallStack* callStack) const
{
    return KDbFunctionExpression::toString(QLatin1String("UNICODE"), this, args, params, callStack);
}

KDbEscapedString KDbDriver::concatenateFunctionToString(const KDbBinaryExpression &args,
                                               KDbQuerySchemaParameterValueListIterator* params,
                                               KDb::ExpressionCallStack* callStack) const
{
    return args.left().toString(this, params, callStack) + KDbEscapedString("||")
            + args.right().toString(this, params, callStack);
}

//---------------

Q_GLOBAL_STATIC_WITH_ARGS(
    KDbUtils::StaticSetOfStrings,
    KDb_kdbSQLKeywords,
    (DriverPrivate::kdbSQLKeywords) )

KDB_EXPORT bool KDb::isKDbSQLKeyword(const QByteArray& word)
{
    return KDb_kdbSQLKeywords->contains(word.toUpper());
}

KDB_EXPORT QString KDb::escapeIdentifier(const KDbDriver* driver,
                                         const QString& str)
{
    return driver ? driver->escapeIdentifier(str)
                  : KDb::escapeIdentifier(str);
}

KDB_EXPORT QByteArray KDb::escapeIdentifier(const KDbDriver* driver,
                                            const QByteArray& str)
{
    return driver ? driver->escapeIdentifier(str)
                  : KDb::escapeIdentifier(str);
}
