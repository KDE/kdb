/* This file is part of the KDE project
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

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

#include "Driver.h"
#include "Driver_p.h"
#include "DriverManager.h"
#include "DriverManager_p.h"
#include "Error.h"
#include "DriverManager.h"
#include "Connection.h"
#include "ConnectionData.h"
#include "Admin.h"
#include "Tools/Static.h"
#include "Utils.h"

#include <QtDebug>

#include <assert.h>

using namespace Predicate;

/*! @internal Used in Driver::defaultSQLTypeName(int)
 when we do not have Driver instance yet, or when we cannot get one */
static const char* const Predicate_defaultSQLTypeNames[] = {
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

Driver::Driver()
 : beh(new DriverBehaviour())
 , d(new DriverPrivate())
{
    d->typeNames.resize(Field::LastType + 1);
}

Driver::~Driver()
{
    // make a copy because d->connections will be touched by ~Connection
    QSet<Connection*> connections(d->connections);
    qDeleteAll(connections);
    d->connections.clear();
    delete beh;
    delete d;
// PreDbg << "ok";
}

bool Driver::isValid()
{
    clearResult();
    QString inv_impl(tr("Invalid database driver's \"%1\" implementation.").arg(name()));
    QString not_init(tr("Value of \"%1\" is not initialized for the driver."));
    if (beh->ROW_ID_FIELD_NAME.isEmpty()) {
        m_result = Result(ERR_INVALID_DRIVER_IMPL,
                          inv_impl + QLatin1Char(' ')
                          + not_init.arg(QLatin1String("DriverBehaviour::ROW_ID_FIELD_NAME")));
        return false;
    }
    return true;
}

const QSet<Connection*> Driver::connections() const
{
    return d->connections;
}

DriverInfo Driver::info() const
{
    return d->info;
}

QString Driver::name() const
{
    return d->info.name();
}

bool Driver::isFileBased() const
{
    return d->info.isFileBased();
}

int Driver::features() const
{
    return d->features;
}

bool Driver::transactionsSupported() const
{
    return d->features & (SingleTransactions | MultipleTransactions);
}

AdminTools& Driver::adminTools() const
{
    if (!d->adminTools)
        d->adminTools = drv_createAdminTools();
    return *d->adminTools;
}

AdminTools* Driver::drv_createAdminTools() const
{
    return new AdminTools(); //empty impl.
}

QString Driver::sqlTypeName(int id_t, int /*p*/) const
{
    if (id_t > Field::InvalidType && id_t <= Field::LastType)
        return d->typeNames[(id_t>0 && id_t<=Field::LastType) ? id_t : Field::InvalidType /*sanity*/];

    return d->typeNames[Field::InvalidType];
}

Connection *Driver::createConnection(const ConnectionData& connData, int options)
{
    clearResult();
    if (!isValid())
        return 0;
    Connection *conn = drv_createConnection(connData);

    conn->setReadOnly(options & ReadOnlyConnection);

//! @todo needed? connData->setDriverName(name());
    d->connections.insert(conn);
    return conn;
}

Connection* Driver::removeConnection(Connection *conn)
{
    clearResult();
    if (d->connections.remove(conn))
        return conn;
    return 0;
}

QString Driver::defaultSQLTypeName(int id_t)
{
    if (id_t < 0 || id_t > (Field::LastType + 1))
        return QLatin1String("Null");
    return QLatin1String(Predicate_defaultSQLTypeNames[id_t]);
}

bool Driver::isSystemObjectName(const QString& n) const
{
    return Driver::isPredicateSystemObjectName(n);
}

bool Driver::isPredicateSystemObjectName(const QString& n)
{
    if (!n.startsWith(QLatin1String("kexi__"), Qt::CaseInsensitive))
        return false;
    return Connection::predicateSystemTableNames().contains(n, Qt::CaseInsensitive);
}

bool Driver::isSystemFieldName(const QString& n) const
{
    if (!beh->ROW_ID_FIELD_NAME.isEmpty()
        && 0 == n.compare(beh->ROW_ID_FIELD_NAME, Qt::CaseInsensitive))
    {
        return true;
    }
    return drv_isSystemFieldName(n);
}

static EscapedString valueToSQLInternal(const Predicate::Driver *driver, uint ftype, const QVariant& v)
{
    if (v.isNull())
        return EscapedString("NULL");
    switch (ftype) {
    case Field::Text:
    case Field::LongText: {
        return driver ? driver->escapeString(v.toString())
                      : EscapedString(Predicate::escapeString(v.toString()));
    }
    case Field::Byte:
    case Field::ShortInteger:
    case Field::Integer:
    case Field::BigInteger:
        return EscapedString(v.toByteArray());
    case Field::Float:
    case Field::Double: {
        if (v.type() == QVariant::String) {
            //workaround for values stored as string that should be casted to floating-point
            EscapedString s(v.toByteArray());
            return s.replace(',', '.');
        }
        return EscapedString(v.toByteArray());
    }
//! @todo here special encoding method needed
    case Field::Boolean:
        return EscapedString(v.toInt() == 0
                 ? driver->behaviour()->BOOLEAN_FALSE_LITERAL
                 : driver->behaviour()->BOOLEAN_TRUE_LITERAL);
    case Field::Time:
        return EscapedString('\'') + v.toTime().toString(Qt::ISODate) + '\'';
    case Field::Date:
        return EscapedString('\'') + v.toDate().toString(Qt::ISODate) + '\'';
    case Field::DateTime:
        return driver ? driver->dateTimeToSQL(v.toDateTime())
                      : Predicate::dateTimeToSQL(v.toDateTime());
    case Field::BLOB: {
        if (v.toByteArray().isEmpty()) {
            return EscapedString("NULL");
        }
        if (v.type() == QVariant::String) {
            return driver ? driver->escapeBLOB(v.toString().toUtf8())
                          : EscapedString(Predicate::escapeBLOB(v.toString().toUtf8(), Predicate::BLOBEscape0xHex));
        }
        return driver ? driver->escapeBLOB(v.toByteArray())
                      : EscapedString(Predicate::escapeBLOB(v.toByteArray(), Predicate::BLOBEscape0xHex));
    }
    case Field::InvalidType:
        return EscapedString("!INVALIDTYPE!");
    default:
        PreDbg << EscapedString("UNKNOWN!");
        return EscapedString();
    }
    return EscapedString();
}

EscapedString Driver::valueToSQL(uint ftype, const QVariant& v) const
{
    //! note, it was compatible with SQLite: http://www.sqlite.org/cvstrac/wiki?p=DateAndTimeFunctions.
    return valueToSQLInternal(this, ftype, v);
}

EscapedString Predicate::dateTimeToSQL(const QDateTime& v)
{
    /*! (was compatible with SQLite: http://www.sqlite.org/cvstrac/wiki?p=DateAndTimeFunctions)
        Now it's ISO 8601 DateTime format - with "T" delimiter:
        http://www.w3.org/TR/NOTE-datetime
        (e.g. "1994-11-05T13:15:30" not "1994-11-05 13:15:30")
        @todo add support for time zones?
    */
    return EscapedString('\'') + v.toString(Qt::ISODate) + EscapedString('\'');
}

QVariant Driver::propertyValue(const QByteArray& propName) const
{
    return d->properties.value(propName.toLower());
}

QString Driver::propertyCaption(const QByteArray& propName) const
{
    return d->propertyCaptions.value(propName.toLower());
}

QList<QByteArray> Driver::propertyNames() const
{
    QList<QByteArray> names(d->properties.keys());
    qSort(names);
    return names;
}

void Driver::initDriverSpecificKeywords(const char** keywords)
{
    d->driverSpecificSQLKeywords.setStrings(keywords);
}

bool Driver::isDriverSpecificKeyword(const QByteArray& word) const
{
    return d->driverSpecificSQLKeywords.contains(word);
}

void Driver::setInfo( const DriverInfo& info )
{
    d->info = info;
    d->initInternalProperties();
}

//---------------

PREDICATE_GLOBAL_STATIC_WITH_ARGS(
    Utils::StaticSetOfStrings,
    Predicate_predicateSQLKeywords,
    (DriverPrivate::predicateSQLKeywords) )

PREDICATE_EXPORT bool Predicate::isPredicateSQLKeyword(const QByteArray& word)
{
    return Predicate_predicateSQLKeywords->contains(word.toUpper());
}
