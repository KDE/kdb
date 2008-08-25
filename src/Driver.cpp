/* This file is part of the KDE project
   Copyright (C) 2003-2008 Jarosław Staniek <staniek@kde.org>

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
#include "tools/Static.h"

#include <QtDebug>

#include <assert.h>

using namespace Predicate;

/*! @internal Used in Driver::defaultSQLTypeName(int)
 when we do not have Driver instance yet, or when we cannot get one */
static const char* Predicate_defaultSQLTypeNames[] = {
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

DriverBehaviour::DriverBehaviour()
        : UNSIGNED_TYPE_KEYWORD("UNSIGNED")
        , AUTO_INCREMENT_FIELD_OPTION("AUTO_INCREMENT")
        , AUTO_INCREMENT_PK_FIELD_OPTION("AUTO_INCREMENT PRIMARY KEY")
        , SPECIAL_AUTO_INCREMENT_DEF(false)
        , AUTO_INCREMENT_REQUIRES_PK(false)
        , ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE(false)
        , QUOTATION_MARKS_FOR_IDENTIFIER('"')
        , USING_DATABASE_REQUIRED_TO_CONNECT(true)
        , _1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY(false)
        , SELECT_1_SUBQUERY_SUPPORTED(false)
{
}

//---------------------------------------------

Driver::Driver(QObject *parent, const QStringList &)
        : QObject(parent)
        , Object()
        , beh(new DriverBehaviour())
        , d(new DriverPrivate())
{
    d->typeNames.resize(Field::LastType + 1);
}

Driver::~Driver()
{
    DriverManagerInternal::self()->aboutDelete(this);
// PreDbg << "Driver::~Driver()";
    // make a copy because d->connections will be touched by ~Connection
    QSet<Connection*> connections(d->connections);
    qDeleteAll(connections);
    d->connections.clear();
    delete beh;
    delete d;
// PreDbg << "Driver::~Driver() ok";
}

bool Driver::isValid()
{
    clearError();
    if (Predicate::version().major != version().major
            || Predicate::version().minor != version().minor) {
        setError(ERR_INCOMPAT_DRIVER_VERSION,
                 tr(
                     "Incompatible database driver's \"%1\" version: found version %2, expected version %3.")
                 .arg(objectName(),
                      QString("%1.%2").arg(version().major).arg(version().minor),
                      QString("%1.%2").arg(Predicate::version().major).arg(Predicate::version().minor)));
        return false;
    }

    QString inv_impl( tr("Invalid database driver's \"%1\" implementation:\n").arg(name()) );
    QString not_init( tr("Value of \"%1\" is not initialized for the driver.") );
    if (beh->ROW_ID_FIELD_NAME.isEmpty()) {
        setError(ERR_INVALID_DRIVER_IMPL,
                 inv_impl + not_init.arg("DriverBehaviour::ROW_ID_FIELD_NAME"));
        return false;
    }

    return true;
}

const QSet<Connection*> Driver::connections() const
{
    return d->connections;
}

/* moved to info()
QString Driver::fileDBDriverMimeType() const
{
    return d->fileDBDriverMimeType;
}*/

//ported const KService* Driver::service() const
Driver::Info Driver::info() const
{
    return d->info;
}

/* moved to info()
bool Driver::isFileDriver() const
{
    return d->isFileDriver;
}*/

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

Connection *Driver::createConnection(ConnectionData &conn_data, int options)
{
    clearError();
    if (!isValid())
        return 0;
    if (d->info.isFileBased()) {
        if (conn_data.fileName().isEmpty()) {
            setError(ERR_MISSING_DB_LOCATION,
                     tr("File name expected for file-based database driver."));
            return 0;
        }
    }
// Connection *conn = new Connection( this, conn_data );
    Connection *conn = drv_createConnection(conn_data);

    conn->setReadOnly(options & ReadOnlyConnection);

    conn_data.driverName = name();
    d->connections.insert(conn);
    return conn;
}

Connection* Driver::removeConnection(Connection *conn)
{
    clearError();
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
    QString lcName = n.toLower();
    if (!lcName.startsWith("kexi__"))
        return false;
    const QStringList list(Connection::predicateSystemTableNames());
    return list.indexOf(lcName) != -1;
}

bool Driver::isSystemFieldName(const QString& n) const
{
    if (!beh->ROW_ID_FIELD_NAME.isEmpty() && n.toLower() == beh->ROW_ID_FIELD_NAME.toLower())
        return true;
    return drv_isSystemFieldName(n);
}

QString Driver::valueToSQL(uint ftype, const QVariant& v) const
{
    if (v.isNull())
        return "NULL";
    switch (ftype) {
    case Field::Text:
    case Field::LongText: {
        QString s = v.toString();
        return escapeString(s); //QString("'")+s.replace( '"', "\\\"" ) + "'";
    }
    case Field::Byte:
    case Field::ShortInteger:
    case Field::Integer:
    case Field::BigInteger:
        return v.toString();
    case Field::Float:
    case Field::Double: {
        if (v.type() == QVariant::String) {
            //workaround for values stored as string that should be casted to floating-point
            QString s(v.toString());
            return s.replace(',', ".");
        }
        return v.toString();
    }
//TODO: here special encoding method needed
    case Field::Boolean:
        return QString::number(v.toInt() ? 1 : 0); //0 or 1
    case Field::Time:
        return QString("\'") + v.toTime().toString(Qt::ISODate) + "\'";
    case Field::Date:
        return QString("\'") + v.toDate().toString(Qt::ISODate) + "\'";
    case Field::DateTime:
        return dateTimeToSQL(v.toDateTime());
    case Field::BLOB: {
        if (v.toByteArray().isEmpty())
            return QLatin1String("NULL");
        if (v.type() == QVariant::String)
            return escapeBLOB(v.toString().toUtf8());
        return escapeBLOB(v.toByteArray());
    }
    case Field::InvalidType:
        return "!INVALIDTYPE!";
    default:
        PreDbg << "Driver::valueToSQL(): UNKNOWN!";
        return QString();
    }
    return QString();
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

QString Driver::escapeIdentifier(const QString& str, int options) const
{
    QByteArray cstr(str.toLatin1());
    return QString(escapeIdentifier(cstr, options));
}

QByteArray Driver::escapeIdentifier(const QByteArray& str, int options) const
{
    bool needOuterQuotes = false;

// Need to use quotes if ...
// ... we have been told to, or ...
    if (options & EscapeAlways)
        needOuterQuotes = true;

// ... or if the driver does not have a list of keywords,
    else if (d->driverSpecificSQLKeywords.isEmpty())
        needOuterQuotes = true;

// ... or if it's a keyword in Kexi's SQL dialect,
    else if (Predicate::isKexiSQLKeyword(str))
        needOuterQuotes = true;

// ... or if it's a keyword in the backends SQL dialect,
    else if ((options & EscapeDriver) && d->driverSpecificSQLKeywords.contains(str))
        needOuterQuotes = true;

// ... or if the identifier has a space in it...
    else if (str.indexOf(' ') != -1)
        needOuterQuotes = true;

    if (needOuterQuotes && (options & EscapeKexi)) {
        const char quote = '"';
        return quote + QByteArray(str).replace(quote, "\"\"") + quote;
    } else if (needOuterQuotes) {
        const char quote = beh->QUOTATION_MARKS_FOR_IDENTIFIER.toLatin1();
        return quote + drv_escapeIdentifier(str) + quote;
    } else {
        return drv_escapeIdentifier(str);
    }
}

void Driver::initDriverSpecificKeywords(const char** keywords)
{
    d->driverSpecificSQLKeywords.setStrings(keywords);
}

bool Driver::isDriverSpecificKeyword(const QByteArray& word) const
{
    return d->driverSpecificSQLKeywords.contains(word);
}

void Driver::setInfo( const Driver::Info& info )
{
    d->info = info;
    setObjectName( info.name().toLower() );
    d->initInternalProperties();
}

//---------------

PREDICATE_GLOBAL_STATIC_WITH_ARGS(
    Utils::StaticSetOfStrings,
    Predicate_kexiSQLKeywords,
    (DriverPrivate::kexiSQLKeywords) )

PREDICATE_EXPORT bool Predicate::isKexiSQLKeyword(const QByteArray& word)
{
    return Predicate_kexiSQLKeywords->contains(word);
}
