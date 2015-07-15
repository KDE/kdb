/* This file is part of the KDE project
   Copyright (C) 2004-2015 Jarosław Staniek <staniek@kde.org>
   Copyright (c) 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

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

#include "KDb.h"
#include "KDbCursor.h"
#include "KDbDriverManager.h"
#include "KDbLookupFieldSchema.h"
#include "KDbTableOrQuerySchema.h"
#include "KDbMessageHandler.h"
#include "KDbConnectionData.h"
#include "KDbNativeStatementBuilder.h"
#include "kdb_debug.h"
#include "transliteration/transliteration_table.h"
#include "config-kdb.h"

#include <QMap>
#include <QHash>
#include <QBuffer>
#include <QPixmap>
#include <QSet>
#include <QTimer>
#include <QThread>
#include <QProgressDialog>
#include <QDomNode>
#include <QApplication>
#include <QDir>
#include <QProcess>
#include <QtDebug>

#include <memory>

class ConnectionTestDialog;

class ConnectionTestThread : public QThread
{
    Q_OBJECT
public:
    ConnectionTestThread(ConnectionTestDialog *dlg, const KDbConnectionData& connData);
    virtual void run();
Q_SIGNALS:
    void error(const QString& msg, const QString& details);
protected:
    void emitError(const KDbResultable& KDbResultable);

    ConnectionTestDialog* m_dlg;
    KDbConnectionData m_connData;
    KDbDriver *m_driver;
};

class ConnectionTestDialog : public QProgressDialog // krazy:exclude=qclasses
{
    Q_OBJECT
public:
    ConnectionTestDialog(QWidget* parent, const KDbConnectionData& data, KDbMessageHandler* msgHandler);
    virtual ~ConnectionTestDialog();

    int exec();

public Q_SLOTS:
    void error(const QString& msg, const QString& details);

protected Q_SLOTS:
    void slotTimeout();
    virtual void reject();

protected:
    ConnectionTestThread* m_thread;
    KDbConnectionData m_connData;
    QTimer m_timer;
    KDbMessageHandler* m_msgHandler;
    uint m_elapsedTime;
    bool m_error;
    QString m_msg;
    QString m_details;
    bool m_stopWaiting;
};

ConnectionTestThread::ConnectionTestThread(ConnectionTestDialog* dlg, const KDbConnectionData& connData)
        : m_dlg(dlg), m_connData(connData)
{
    connect(this, SIGNAL(error(QString,QString)),
            dlg, SLOT(error(QString,QString)), Qt::QueuedConnection);

    // try to load driver now because it's not supported in different thread
    KDbDriverManager manager;
    m_driver = manager.driver(m_connData.driverId());
    if (manager.result().isError()) {
        emitError(manager.resultable());
        m_driver = 0;
    }
}

void ConnectionTestThread::emitError(const KDbResultable& KDbResultable)
{
    QString msg;
    QString details;
    KDb::getHTMLErrorMesage(KDbResultable, &msg, &details);
    emit error(msg, details);
}

void ConnectionTestThread::run()
{
    if (!m_driver) {
        return;
    }
    QScopedPointer<KDbConnection> conn(m_driver->createConnection(m_connData));
    if (conn.isNull() || m_driver->result().isError()) {
        emitError(*m_driver);
        return;
    }
    if (conn->connect() || conn->result().isError()) {
        emitError(*conn);
        return;
    }
    // SQL database backends like PostgreSQL require executing "USE database"
    // if we really want to know connection to the server succeeded.
    QString tmpDbName;
    if (!conn->useTemporaryDatabaseIfNeeded(&tmpDbName)) {
        emitError(*conn);
        return;
    }
    emitError(KDbResultable());
}

ConnectionTestDialog::ConnectionTestDialog(QWidget* parent,
        const KDbConnectionData& data,
        KDbMessageHandler* msgHandler)
        : QProgressDialog(parent)
        , m_thread(new ConnectionTestThread(this, data))
        , m_connData(data)
        , m_msgHandler(msgHandler)
        , m_elapsedTime(0)
        , m_error(false)
        , m_stopWaiting(false)
{
    setWindowTitle(tr("Test Connection"));
    setLabelText(tr("Testing connection to \"%1\" database server...")
                 .arg(data.toUserVisibleString()));
    setModal(true);
    setRange(0, 0); //to show busy indicator
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
    adjustSize();
    resize(250, height());
}

ConnectionTestDialog::~ConnectionTestDialog()
{
    m_thread->terminate();
    delete m_thread;
}

int ConnectionTestDialog::exec()
{
    //kdbDebug() << "tid:" << QThread::currentThread() << "this_thread:" << thread();
    m_timer.start(20);
    m_thread->start();
    const int res = QProgressDialog::exec(); // krazy:exclude=qclasses
    m_thread->wait();
    m_timer.stop();
    return res;
}

void ConnectionTestDialog::slotTimeout()
{
    //kdbDebug() << "tid:" << QThread::currentThread() << "this_thread:" << thread();
    kdbDebug() << m_error;
    bool notResponding = false;
    if (m_elapsedTime >= 1000*5) {//5 seconds
        m_stopWaiting = true;
        notResponding = true;
    }
    kdbDebug() << m_elapsedTime << m_stopWaiting << notResponding;
    if (m_stopWaiting) {
        m_timer.disconnect(this);
        m_timer.stop();
        reject();
        kdbDebug() << "after reject";
        if (m_error) {
            kdbDebug() << "show?";
            if (m_msgHandler) {
                m_msgHandler->showErrorMessage(KDbMessageHandler::Sorry, m_msg, m_details);
            }
            kdbDebug() << "shown";
            m_error = false;
        } else if (notResponding) {
            if (m_msgHandler) {
                m_msgHandler->showErrorMessage(
                    KDbMessageHandler::Sorry,
                    tr("Test connection to \"%1\" database server failed. The server is not responding.")
                        .arg(m_connData.toUserVisibleString()),
                    QString(),
                    tr("Test Connection"));
            }
        } else {
            if (m_msgHandler) {
                m_msgHandler->showErrorMessage(
                    KDbMessageHandler::Information,
                    tr("Test connection to \"%1\" database server established successfully.")
                        .arg(m_connData.toUserVisibleString()),
                    QString(),
                    tr("Test Connection"));
            }
        }
        return;
    }
    m_elapsedTime += 20;
    setValue(m_elapsedTime);
}

void ConnectionTestDialog::error(const QString& msg, const QString& details)
{
    //kdbDebug() << "tid:" << QThread::currentThread() << "this_thread:" << thread();
    kdbDebug() << msg << details;
    m_stopWaiting = true;
    if (!msg.isEmpty() || !details.isEmpty()) {
        m_error = true;
        m_msg = msg;
        m_details = details;
        kdbDebug() << "ERR!";
    }
}

void ConnectionTestDialog::reject()
{
    m_thread->terminate();
    m_timer.disconnect(this);
    m_timer.stop();
    QProgressDialog::reject(); // krazy:exclude=qclasses
}

// ----

//! @internal Dummy class to get simply translation markup expressions
//! of the form kdb::tr("foo") instead of the complicated and harder to read
//! QCoreApplication::translate("KDb", "foo") which also runs the chance of
//! typos in the class context argument
class kdb
{
Q_DECLARE_TR_FUNCTIONS(KDb)
};


KDbVersionInfo KDb::version()
{
    return KDbVersionInfo(
        KDB_VERSION_MAJOR, KDB_VERSION_MINOR, KDB_VERSION_PATCH);
}

bool KDb::deleteRecord(KDbConnection* conn, KDbTableSchema *table,
                             const QString &keyname, const QString &keyval)
{
    return table != 0 && conn->executeSQL(KDbEscapedString("DELETE FROM ")
        + table->name() + " WHERE "
        + keyname + '=' + conn->driver()->valueToSQL(KDbField::Text, QVariant(keyval)));
}

bool KDb::deleteRecord(KDbConnection* conn, const QString &tableName,
                             const QString &keyname, const QString &keyval)
{
    return conn->executeSQL(KDbEscapedString("DELETE FROM ") + tableName + " WHERE "
                           + keyname + '=' + conn->driver()->valueToSQL(KDbField::Text, QVariant(keyval)));
}

bool KDb::deleteRecord(KDbConnection* conn, KDbTableSchema *table,
                             const QString& keyname, int keyval)
{
    return table != 0 && conn->executeSQL(KDbEscapedString("DELETE FROM ")
        + table->name() + " WHERE "
        + keyname + '=' + conn->driver()->valueToSQL(KDbField::Integer, QVariant(keyval)));
}

bool KDb::deleteRecord(KDbConnection* conn, const QString &tableName,
                             const QString &keyname, int keyval)
{
    return conn->executeSQL(KDbEscapedString("DELETE FROM ") + tableName + " WHERE "
                           + keyname + '=' + conn->driver()->valueToSQL(KDbField::Integer, QVariant(keyval)));
}

bool KDb::deleteRecord(KDbConnection* conn, const QString &tableName,
                             const QString &keyname1, KDbField::Type keytype1, const QVariant& keyval1,
                             const QString &keyname2, KDbField::Type keytype2, const QVariant& keyval2)
{
    return conn->executeSQL(KDbEscapedString("DELETE FROM ") + tableName + " WHERE "
        + keyname1 + '=' + conn->driver()->valueToSQL(keytype1, keyval1)
        + " AND " + keyname2 + '=' + conn->driver()->valueToSQL(keytype2, keyval2));
}

bool KDb::deleteRecord(KDbConnection* conn, const QString &tableName,
                             const QString &keyname1, KDbField::Type keytype1, const QVariant& keyval1,
                             const QString &keyname2, KDbField::Type keytype2, const QVariant& keyval2,
                             const QString &keyname3, KDbField::Type keytype3, const QVariant& keyval3)
{
    return conn->executeSQL(KDbEscapedString("DELETE FROM ") + tableName + " WHERE "
                           + keyname1 + "=" + conn->driver()->valueToSQL(keytype1, keyval1)
                           + " AND " + keyname2 + "=" + conn->driver()->valueToSQL(keytype2, keyval2)
                           + " AND " + keyname3 + "=" + conn->driver()->valueToSQL(keytype3, keyval3));
}

bool KDb::isEmptyValue(KDbField::Type type, const QVariant &v)
{
    if (KDbField::hasEmptyProperty(type) && v.toString().isEmpty() && !v.toString().isNull())
        return true;
    return v.isNull();
}

KDbEscapedString KDb::sqlWhere(KDbDriver *drv, KDbField::Type t,
                            const QString& fieldName, const QVariant& value)
{
    if (value.isNull())
        return KDbEscapedString(fieldName) + " is NULL";
    return KDbEscapedString(fieldName) + '=' + drv->valueToSQL(t, value);
}

//! Cache
struct TypeCache {
    TypeCache() {
        for (KDbField::Type t = KDbField::InvalidType; t <= KDbField::LastType; t = KDbField::Type(int(t) + 1)) {
            const KDbField::TypeGroup tg = KDbField::typeGroup(t);
            QList<KDbField::Type> list;
            QStringList name_list, str_list;
            if (tlist.contains(tg)) {
                list = tlist.value(tg);
                name_list = nlist.value(tg);
                str_list = slist.value(tg);
            }
            list += t;
            name_list += KDbField::typeName(t);
            str_list += KDbField::typeString(t);
            tlist[ tg ] = list;
            nlist[ tg ] = name_list;
            slist[ tg ] = str_list;
        }

        def_tlist[ KDbField::InvalidGroup ] = KDbField::InvalidType;
        def_tlist[ KDbField::TextGroup ] = KDbField::Text;
        def_tlist[ KDbField::IntegerGroup ] = KDbField::Integer;
        def_tlist[ KDbField::FloatGroup ] = KDbField::Double;
        def_tlist[ KDbField::BooleanGroup ] = KDbField::Boolean;
        def_tlist[ KDbField::DateTimeGroup ] = KDbField::Date;
        def_tlist[ KDbField::BLOBGroup ] = KDbField::BLOB;
    }

    QHash< KDbField::TypeGroup, QList<KDbField::Type> > tlist;
    QHash< KDbField::TypeGroup, QStringList > nlist;
    QHash< KDbField::TypeGroup, QStringList > slist;
    QHash< KDbField::TypeGroup, KDbField::Type > def_tlist;
};

Q_GLOBAL_STATIC(TypeCache, KDb_typeCache)

const QList<KDbField::Type> KDb::fieldTypesForGroup(KDbField::TypeGroup typeGroup)
{
    return KDb_typeCache->tlist.value(typeGroup);
}

QStringList KDb::fieldTypeNamesForGroup(KDbField::TypeGroup typeGroup)
{
    return KDb_typeCache->nlist.value(typeGroup);
}

QStringList KDb::fieldTypeStringsForGroup(KDbField::TypeGroup typeGroup)
{
    return KDb_typeCache->slist.value(typeGroup);
}

KDbField::Type KDb::defaultFieldTypeForGroup(KDbField::TypeGroup typeGroup)
{
    return (typeGroup <= KDbField::LastTypeGroup) ? KDb_typeCache->def_tlist.value(typeGroup) : KDbField::InvalidType;
}

void KDb::getHTMLErrorMesage(const KDbResultable& resultable, QString *msg, QString *details)
{
    Q_ASSERT(msg);
    Q_ASSERT(details);
    const KDbResult result(resultable.result());
    if (!result.isError())
        return;
    //lower level message is added to the details, if there is alread message specified
    if (!result.messageTitle().isEmpty())
        *msg += QLatin1String("<p>") + result.messageTitle();

    if (msg->isEmpty())
        *msg = QLatin1String("<p>") + result.message();
    else
        *details += QLatin1String("<p>") + result.message();

    if (!result.serverMessage().isEmpty())
        *details += QLatin1String("<p><b>") + kdb::tr("Message from server:")
                   + QLatin1String("</b> ") + result.serverMessage();
    if (!result.recentSQLString().isEmpty())
        *details += QLatin1String("<p><b>") + kdb::tr("SQL statement:")
                   + QString::fromLatin1("</b> <tt>%1</tt>").arg(result.recentSQLString().toString());
    int serverErrorCode;
    QString serverResultName;
    if (result.isError()) {
        serverErrorCode = result.serverErrorCode();
        serverResultName = resultable.serverResultName();
    }
    if (   !details->isEmpty()
        && (   !result.serverMessage().isEmpty()
            || !result.recentSQLString().isEmpty()
            || !serverResultName.isEmpty()
            || serverErrorCode != 0)
           )
    {
        *details += (QLatin1String("<p><b>") + kdb::tr("Server result code:")
                    + QLatin1String("</b> ") + QString::number(serverErrorCode));
        if (!serverResultName.isEmpty()) {
            *details += QString::fromLatin1(" (%1)").arg(serverResultName);
        }
    }
    else {
        if (!serverResultName.isEmpty()) {
            *details += (QLatin1String("<p><b>") + kdb::tr("Server result:")
                        + QLatin1String("</b> ") + serverResultName);
        }
    }

    if (!details->isEmpty() && !details->startsWith(QLatin1String("<qt>"))) {
        if (!details->startsWith(QLatin1String("<p>")))
            details->prepend(QLatin1String("<p>"));
    }
}

void KDb::getHTMLErrorMesage(const KDbResultable& resultable, QString *msg)
{
    Q_ASSERT(msg);
    getHTMLErrorMesage(resultable, msg, msg);
}

void KDb::getHTMLErrorMesage(const KDbResultable& resultable, KDbResultInfo *info)
{
    Q_ASSERT(info);
    getHTMLErrorMesage(resultable, &info->msg, &info->desc);
}

int KDb::idForObjectName(KDbConnection* conn, const QString& objName, int objType)
{
    KDbRecordData data;
    if (true != conn->querySingleRecord(
                KDbEscapedString("SELECT o_id FROM kexi__objects WHERE o_name='%1' AND o_type=%2")
                .arg(KDbEscapedString(objName), KDbEscapedString::number(objType)), &data))
    {
        return 0;
    }
    bool ok;
    int id = data[0].toInt(&ok);
    return ok ? id : 0;
}

//-----------------------------------------

void KDb::connectionTestDialog(QWidget* parent, const KDbConnectionData& data,
                               KDbMessageHandler* msgHandler)
{
    ConnectionTestDialog dlg(parent, data, msgHandler);
    dlg.exec();
}

int KDb::recordCount(KDbConnection* conn, const KDbEscapedString& sql)
{
    int count = -1; //will be changed only on success of querySingleNumber()
    conn->querySingleNumber(KDbEscapedString("SELECT COUNT() FROM (") + sql
        + ") AS kdb__subquery", &count);
    return count;
}

int KDb::recordCount(const KDbTableSchema& tableSchema)
{
//! @todo does not work with non-SQL data sources
    if (!tableSchema.connection()) {
        kdbWarning() << "no tableSchema.connection()";
        return -1;
    }
    int count = -1; //will be changed only on success of querySingleNumber()
    tableSchema.connection()->querySingleNumber(
        KDbEscapedString("SELECT COUNT(*) FROM ")
        + tableSchema.connection()->escapeIdentifier(tableSchema.name()),
        &count
    );
    return count;
}

int KDb::recordCount(KDbQuerySchema* querySchema, const QList<QVariant>& params)
{
//! @todo does not work with non-SQL data sources
    if (!querySchema->connection()) {
        kdbWarning() << "no querySchema->connection()";
        return -1;
    }
    int count = -1; //will be changed only on success of querySingleNumber()
    KDbNativeStatementBuilder builder(querySchema->connection());
    KDbEscapedString subSql;
    if (!builder.generateSelectStatement(&subSql, querySchema, params)) {
        return -1;
    }
    tristate result = querySchema->connection()->querySingleNumber(
        KDbEscapedString("SELECT COUNT(*) FROM (") + subSql + ") AS kdb__subquery", &count
    );
    return true == result ? count : -1;
}

int KDb::recordCount(KDbTableOrQuerySchema* tableOrQuery, const QList<QVariant>& params)
{
    if (tableOrQuery->table())
        return recordCount(*tableOrQuery->table());
    if (tableOrQuery->query())
        return recordCount(tableOrQuery->query(), params);
    return -1;
}

int KDb::fieldCount(KDbTableOrQuerySchema* tableOrQuery)
{
    if (tableOrQuery->table())
        return tableOrQuery->table()->fieldCount();
    if (tableOrQuery->query())
        return tableOrQuery->query()->fieldsExpanded().count();
    return -1;
}

bool KDb::splitToTableAndFieldParts(const QString& string,
                                          QString *tableName, QString *fieldName,
                                          SplitToTableAndFieldPartsOptions option)
{
    Q_ASSERT(tableName);
    Q_ASSERT(fieldName);
    const int id = string.indexOf(QLatin1Char('.'));
    if (option & SetFieldNameIfNoTableName && id == -1) {
        tableName->clear();
        *fieldName = string;
        return !fieldName->isEmpty();
    }
    if (id <= 0 || id == int(string.length() - 1))
        return false;
    *tableName = string.left(id);
    *fieldName = string.mid(id + 1);
    return !tableName->isEmpty() && !fieldName->isEmpty();
}

bool KDb::supportsVisibleDecimalPlacesProperty(KDbField::Type type)
{
//! @todo add check for decimal type as well
    return KDbField::isFPNumericType(type);
}

QString KDb::formatNumberForVisibleDecimalPlaces(double value, int decimalPlaces)
{
//! @todo round?
    if (decimalPlaces < 0) {
        QString s(QString::number(value, 'f', 10 /*reasonable precision*/));
        uint i = s.length() - 1;
        while (i > 0 && s[i] == QLatin1Char('0'))
            i--;
        if (s[i] == QLatin1Char('.')) //remove '.'
            i--;
        s = s.left(i + 1).replace(QLatin1Char('.'), QLocale().decimalPoint());
        return s;
    }
    if (decimalPlaces == 0)
        return QString::number((int)value);
    return QLocale().toString(value, 'g', decimalPlaces);
}

KDbField::Type KDb::intToFieldType(int type)
{
    if (type < int(KDbField::InvalidType) || type > int(KDbField::LastType)) {
        kdbWarning() << "invalid type" << type;
        return KDbField::InvalidType;
    }
    return static_cast<KDbField::Type>(type);
}

KDbField::TypeGroup KDb::intToFieldTypeGroup(int typeGroup)
{
    if (typeGroup < int(KDbField::InvalidGroup) || typeGroup > int(KDbField::LastTypeGroup)) {
        kdbWarning() << "invalid type group" << typeGroup;
        return KDbField::InvalidGroup;
    }
    return static_cast<KDbField::TypeGroup>(typeGroup);
}

static bool setIntToFieldType(KDbField *field, const QVariant& value)
{
    Q_ASSERT(field);
    bool ok;
    const int intType = value.toInt(&ok);
    if (!ok || KDbField::InvalidType == KDb::intToFieldType(intType)) {//for sanity
        kdbWarning() << "invalid type";
        return false;
    }
    field->setType((KDbField::Type)intType);
    return true;
}

//! @internal for KDb::isBuiltinTableFieldProperty()
struct KDb_BuiltinFieldProperties {
    KDb_BuiltinFieldProperties() {
#define ADD(name) set.insert(name)
        ADD("type");
        ADD("primaryKey");
        ADD("indexed");
        ADD("autoIncrement");
        ADD("unique");
        ADD("notNull");
        ADD("allowEmpty");
        ADD("unsigned");
        ADD("name");
        ADD("caption");
        ADD("description");
        ADD("maxLength");
        ADD("maxLengthIsDefault");
        ADD("precision");
        ADD("defaultValue");
        ADD("defaultWidth");
        ADD("visibleDecimalPlaces");
//! @todo always update this when new builtins appear!
#undef ADD
    }
    QSet<QByteArray> set;
};

//! for KDb::isBuiltinTableFieldProperty()
Q_GLOBAL_STATIC(KDb_BuiltinFieldProperties, KDb_builtinFieldProperties)


bool KDb::isBuiltinTableFieldProperty(const QByteArray& propertyName)
{
    return KDb_builtinFieldProperties->set.contains(propertyName);
}

void KDb::getProperties(const KDbLookupFieldSchema *lookup, QMap<QByteArray, QVariant> *values)
{
    Q_ASSERT(values);
    KDbLookupFieldSchema::RecordSource recordSource;
    if (lookup) {
        recordSource = lookup->recordSource();
    }
    values->insert("rowSource", lookup ? recordSource.name() : QVariant());
    values->insert("rowSourceType", lookup ? recordSource.typeName() : QVariant());
    values->insert("rowSourceValues",
        (lookup && !recordSource.values().isEmpty()) ? recordSource.values() : QVariant());
    values->insert("boundColumn", lookup ? lookup->boundColumn() : QVariant());
    QList<QVariant> variantList;
    if (!lookup || lookup->visibleColumns().count() == 1) {
        values->insert("visibleColumn", lookup ? lookup->visibleColumns().first() : QVariant());
    }
    else {
        QList<uint> visibleColumns = lookup->visibleColumns();
        foreach(const QVariant& variant, visibleColumns) {
            variantList.append(variant);
        }
        values->insert("visibleColumn", variantList);
    }
    QList<int> columnWidths;
    variantList.clear();
    if (lookup) {
        columnWidths = lookup->columnWidths();
        foreach(const QVariant& variant, columnWidths) {
            variantList.append(variant);
        }
    }
    values->insert("columnWidths", lookup ? variantList : QVariant());
    values->insert("showColumnHeaders", lookup ? lookup->columnHeadersVisible() : QVariant());
    values->insert("listRows", lookup ? lookup->maxVisibleRecords() : QVariant());
    values->insert("limitToList", lookup ? lookup->limitToList() : QVariant());
    values->insert("displayWidget", lookup ? uint(lookup->displayWidget()) : QVariant());
}

void KDb::getFieldProperties(const KDbField &field, QMap<QByteArray, QVariant> *values)
{
    Q_ASSERT(values);
    values->clear();
    // normal values
    values->insert("type", field.type());
    const uint constraints = field.constraints();
    values->insert("primaryKey", constraints & KDbField::PrimaryKey);
    values->insert("indexed", constraints & KDbField::Indexed);
    values->insert("autoIncrement", KDbField::isAutoIncrementAllowed(field.type())
                                    && (constraints & KDbField::AutoInc));
    values->insert("unique", constraints & KDbField::Unique);
    values->insert("notNull", constraints & KDbField::NotNull);
    values->insert("allowEmpty", !(constraints & KDbField::NotEmpty));
    const uint options = field.options();
    values->insert("unsigned", options & KDbField::Unsigned);
    values->insert("name", field.name());
    values->insert("caption", field.caption());
    values->insert("description", field.description());
    values->insert("maxLength", field.maxLength());
    values->insert("maxLengthIsDefault", field.maxLengthStrategy() & KDbField::DefaultMaxLength);
    values->insert("precision", field.precision());
    values->insert("defaultValue", field.defaultValue());
//! @todo IMPORTANT: values->insert("defaultWidth", field.defaultWidth());
    if (KDb::supportsVisibleDecimalPlacesProperty(field.type())) {
        values->insert("visibleDecimalPlaces", field.defaultValue());
    }
    // insert lookup-related values
    KDbLookupFieldSchema *lookup = field.table()->lookupFieldSchema(field);
    KDb::getProperties(lookup, values);
}

static bool containsLookupFieldSchemaProperties(const QMap<QByteArray, QVariant>& values)
{
    for (QMap<QByteArray, QVariant>::ConstIterator it(values.constBegin());
         it != values.constEnd(); ++it)
    {
        if (KDb::isLookupFieldSchemaProperty(it.key())) {
            return true;
        }
    }
    return false;
}

bool KDb::setFieldProperties(KDbField *field, const QMap<QByteArray, QVariant>& values)
{
    Q_ASSERT(field);
    QMap<QByteArray, QVariant>::ConstIterator it;
    if ((it = values.find("type")) != values.constEnd()) {
        if (!setIntToFieldType(field, *it))
            return false;
    }

#define SET_BOOLEAN_FLAG(flag, value) { \
        constraints |= KDbField::flag; \
        if (!value) \
            constraints ^= KDbField::flag; \
    }

    KDbField::Constraints constraints = field->constraints();
    bool ok = true;
    if ((it = values.find("primaryKey")) != values.constEnd())
        SET_BOOLEAN_FLAG(PrimaryKey, (*it).toBool());
    if ((it = values.find("indexed")) != values.constEnd())
        SET_BOOLEAN_FLAG(Indexed, (*it).toBool());
    if ((it = values.find("autoIncrement")) != values.constEnd()
            && KDbField::isAutoIncrementAllowed(field->type()))
        SET_BOOLEAN_FLAG(AutoInc, (*it).toBool());
    if ((it = values.find("unique")) != values.constEnd())
        SET_BOOLEAN_FLAG(Unique, (*it).toBool());
    if ((it = values.find("notNull")) != values.constEnd())
        SET_BOOLEAN_FLAG(NotNull, (*it).toBool());
    if ((it = values.find("allowEmpty")) != values.constEnd())
        SET_BOOLEAN_FLAG(NotEmpty, !(*it).toBool());
    field->setConstraints(constraints);

    KDbField::Options options;
    if ((it = values.find("unsigned")) != values.constEnd()) {
        options |= KDbField::Unsigned;
        if (!(*it).toBool())
            options ^= KDbField::Unsigned;
    }
    field->setOptions(options);

    if ((it = values.find("name")) != values.constEnd())
        field->setName((*it).toString());
    if ((it = values.find("caption")) != values.constEnd())
        field->setCaption((*it).toString());
    if ((it = values.find("description")) != values.constEnd())
        field->setDescription((*it).toString());
    if ((it = values.find("maxLength")) != values.constEnd())
        field->setMaxLength((*it).isNull() ? 0/*default*/ : (*it).toUInt(&ok));
    if (!ok)
        return false;
    if ((it = values.find("maxLengthIsDefault")) != values.constEnd()
            && (*it).toBool())
    {
        field->setMaxLengthStrategy(KDbField::DefaultMaxLength);
    }
    if ((it = values.find("precision")) != values.constEnd())
        field->setPrecision((*it).isNull() ? 0/*default*/ : (*it).toUInt(&ok));
    if (!ok)
        return false;
    if ((it = values.find("defaultValue")) != values.constEnd())
        field->setDefaultValue(*it);
//! @todo IMPORTANT: defaultWidth
#if 0
    if ((it = values.find("defaultWidth")) != values.constEnd())
        field.setDefaultWidth((*it).isNull() ? 0/*default*/ : (*it).toUInt(&ok));
    if (!ok)
        return false;
#endif

    // -- extended properties
    if ((it = values.find("visibleDecimalPlaces")) != values.constEnd()
            && KDb::supportsVisibleDecimalPlacesProperty(field->type()))
        field->setVisibleDecimalPlaces((*it).isNull() ? -1/*default*/ : (*it).toInt(&ok));
    if (!ok)
        return false;

    if (field->table() && containsLookupFieldSchemaProperties(values)) {
        KDbLookupFieldSchema *lookup = field->table()->lookupFieldSchema(*field);
        QScopedPointer<KDbLookupFieldSchema> createdLookup;
        if (!lookup) { // create lookup if needed
            createdLookup.reset(lookup = new KDbLookupFieldSchema());
        }
        if (lookup->setProperties(values)) {
            if (createdLookup) {
                if (field->table()->setLookupFieldSchema(field->name(), lookup)) {
                    createdLookup.take(); // ownership passed
                    lookup = 0;
                }
            }
        }
    }

    return true;
#undef SET_BOOLEAN_FLAG
}

//! @internal for isExtendedTableProperty()
struct KDb_ExtendedProperties {
    KDb_ExtendedProperties() {
#define ADD(name) set.insert( name )
        ADD("visibledecimalplaces");
        ADD("rowsource");
        ADD("rowsourcetype");
        ADD("rowsourcevalues");
        ADD("boundcolumn");
        ADD("visiblecolumn");
        ADD("columnwidths");
        ADD("showcolumnheaders");
        ADD("listrows");
        ADD("limittolist");
        ADD("displaywidget");
#undef ADD
    }
    QSet<QByteArray> set;
};

//! for isExtendedTableProperty()
Q_GLOBAL_STATIC(KDb_ExtendedProperties, KDb_extendedProperties)

bool KDb::isExtendedTableFieldProperty(const QByteArray& propertyName)
{
    return KDb_extendedProperties->set.contains(QByteArray(propertyName).toLower());
}

//! @internal for isLookupFieldSchemaProperty()
struct KDb_LookupFieldSchemaProperties {
    KDb_LookupFieldSchemaProperties() {
        QMap<QByteArray, QVariant> tmp;
        KDb::getProperties(0, &tmp);
        foreach (const QByteArray &p, tmp.keys()) {
            set.insert(p.toLower());
        }
    }
    QSet<QByteArray> set;
};

//! for isLookupFieldSchemaProperty()
Q_GLOBAL_STATIC(KDb_LookupFieldSchemaProperties, KDb_lookupFieldSchemaProperties)

bool KDb::isLookupFieldSchemaProperty(const QByteArray& propertyName)
{
    return KDb_lookupFieldSchemaProperties->set.contains(QByteArray(propertyName).toLower());
}

bool KDb::setFieldProperty(KDbField *field, const QByteArray& propertyName, const QVariant& value)
{
    Q_ASSERT(field);
#define SET_BOOLEAN_FLAG(flag, value) { \
        constraints |= KDbField::flag; \
        if (!value) \
            constraints ^= KDbField::flag; \
        field->setConstraints( constraints ); \
        return true; \
    }
#define GET_INT(method) { \
        const uint ival = value.toUInt(&ok); \
        if (!ok) \
            return false; \
        field->method( ival ); \
        return true; \
    }

    if (propertyName.isEmpty())
        return false;

    bool ok;
    if (KDb::isExtendedTableFieldProperty(propertyName)) {
        //a little speedup: identify extended property in O(1)
        if ("visibleDecimalPlaces" == propertyName
                && KDb::supportsVisibleDecimalPlacesProperty(field->type())) {
            GET_INT(setVisibleDecimalPlaces);
        }
        else if (KDb::isLookupFieldSchemaProperty(propertyName)) {
            if (!field->table()) {
                kdbWarning() << "Cannot set" << propertyName << "property - no table assigned for field";
            } else {
                KDbLookupFieldSchema *lookup = field->table()->lookupFieldSchema(*field);
                const bool createLookup = !lookup;
                if (createLookup) // create lookup if needed
                    lookup = new KDbLookupFieldSchema();
                if (lookup->setProperty(propertyName, value)) {
                    if (createLookup)
                        field->table()->setLookupFieldSchema(field->name(), lookup);
                    return true;
                }
                if (createLookup)
                    delete lookup; // not set, delete
            }
        }
    } else {//non-extended
        if ("type" == propertyName)
            return setIntToFieldType(field, value);

        KDbField::Constraints constraints = field->constraints();
        if ("primaryKey" == propertyName)
            SET_BOOLEAN_FLAG(PrimaryKey, value.toBool());
        if ("indexed" == propertyName)
            SET_BOOLEAN_FLAG(Indexed, value.toBool());
        if ("autoIncrement" == propertyName
                && KDbField::isAutoIncrementAllowed(field->type()))
            SET_BOOLEAN_FLAG(AutoInc, value.toBool());
        if ("unique" == propertyName)
            SET_BOOLEAN_FLAG(Unique, value.toBool());
        if ("notNull" == propertyName)
            SET_BOOLEAN_FLAG(NotNull, value.toBool());
        if ("allowEmpty" == propertyName)
            SET_BOOLEAN_FLAG(NotEmpty, !value.toBool());

        KDbField::Options options;
        if ("unsigned" == propertyName) {
            options |= KDbField::Unsigned;
            if (!value.toBool())
                options ^= KDbField::Unsigned;
            field->setOptions(options);
            return true;
        }

        if ("name" == propertyName) {
            if (value.toString().isEmpty())
                return false;
            field->setName(value.toString());
            return true;
        }
        if ("caption" == propertyName) {
            field->setCaption(value.toString());
            return true;
        }
        if ("description" == propertyName) {
            field->setDescription(value.toString());
            return true;
        }
        if ("maxLength" == propertyName)
            GET_INT(setMaxLength);
        if ("maxLengthIsDefault" == propertyName) {
            field->setMaxLengthStrategy(KDbField::DefaultMaxLength);
        }
        if ("precision" == propertyName)
            GET_INT(setPrecision);
        if ("defaultValue" == propertyName) {
            field->setDefaultValue(value);
            return true;
        }

//! @todo IMPORTANT: defaultWidth
#if 0
        if ("defaultWidth" == propertyName)
            GET_INT(setDefaultWidth);
#endif
        // last chance that never fails: custom field property
        field->setCustomProperty(propertyName, value);
    }

    kdbWarning() << "property" << propertyName << "not found!";
    return false;
#undef SET_BOOLEAN_FLAG
#undef GET_INT
}

int KDb::loadIntPropertyValueFromDom(const QDomNode& node, bool* ok)
{
    QByteArray valueType = node.nodeName().toLatin1();
    if (valueType.isEmpty() || valueType != "number") {
        if (ok)
            *ok = false;
        return 0;
    }
    const QString text(QDomNode(node).toElement().text());
    int val = text.toInt(ok);
    return val;
}

QString KDb::loadStringPropertyValueFromDom(const QDomNode& node, bool* ok)
{
    QByteArray valueType = node.nodeName().toLatin1();
    if (valueType != "string") {
        if (ok)
            *ok = false;
        return QString();
    }
    if (ok)
        *ok = true;
    return QDomNode(node).toElement().text();
}

QVariant KDb::loadPropertyValueFromDom(const QDomNode& node, bool* ok)
{
    QByteArray valueType = node.nodeName().toLatin1();
    if (valueType.isEmpty()) {
        if (ok)
            *ok = false;
        return QVariant();
    }
    if (ok)
        *ok = true;
    const QString text(QDomNode(node).toElement().text());
    bool _ok;
    if (valueType == "string") {
        return text;
    }
    else if (valueType == "cstring") {
        return text.toLatin1();
    }
    else if (valueType == "number") { // integer or double
        if (text.indexOf(QLatin1Char('.')) != -1) {
            double val = text.toDouble(&_ok);
            if (_ok)
                return val;
        }
        else {
            const int val = text.toInt(&_ok);
            if (_ok)
                return val;
            const qint64 valLong = text.toLongLong(&_ok);
            if (_ok)
                return valLong;
        }
    }
    else if (valueType == "bool") {
        return text.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0
               || text == QLatin1String("1");
    }
    else {
//! @todo add more QVariant types
        kdbWarning() << "KDb::loadPropertyValueFromDom(): unknown type '" << valueType << "'";
    }
    if (ok)
        *ok = false;
    return QVariant();
}

QDomElement KDb::saveNumberElementToDom(QDomDocument *doc, QDomElement *parentEl,
        const QString& elementName, int value)
{
    Q_ASSERT(doc);
    Q_ASSERT(parentEl);
    QDomElement el(doc->createElement(elementName));
    parentEl->appendChild(el);
    QDomElement numberEl(doc->createElement(QLatin1String("number")));
    el.appendChild(numberEl);
    numberEl.appendChild(doc->createTextNode(QString::number(value)));
    return el;
}

QDomElement KDb::saveBooleanElementToDom(QDomDocument *doc, QDomElement *parentEl,
        const QString& elementName, bool value)
{
    Q_ASSERT(doc);
    Q_ASSERT(parentEl);
    QDomElement el(doc->createElement(elementName));
    parentEl->appendChild(el);
    QDomElement numberEl(doc->createElement(QLatin1String("bool")));
    el.appendChild(numberEl);
    numberEl.appendChild(doc->createTextNode(
                             value ? QLatin1String("true") : QLatin1String("false")));
    return el;
}

//! @internal Used in KDb::emptyValueForFieldType()
struct KDb_EmptyValueForFieldTypeCache {
    KDb_EmptyValueForFieldTypeCache()
            : values(int(KDbField::LastType) + 1) {
#define ADD(t, value) values.insert(t, value);
        ADD(KDbField::Byte, 0);
        ADD(KDbField::ShortInteger, 0);
        ADD(KDbField::Integer, 0);
        ADD(KDbField::BigInteger, 0);
        ADD(KDbField::Boolean, false);
        ADD(KDbField::Float, 0.0);
        ADD(KDbField::Double, 0.0);
//! @todo ok? we have no better defaults
        ADD(KDbField::Text, QLatin1String(" "));
        ADD(KDbField::LongText, QLatin1String(" "));
        ADD(KDbField::BLOB, QByteArray());
#undef ADD
    }
    QVector<QVariant> values;
};

//! Used in KDb::emptyValueForFieldType()
Q_GLOBAL_STATIC(KDb_EmptyValueForFieldTypeCache, KDb_emptyValueForFieldTypeCache)

QVariant KDb::emptyValueForFieldType(KDbField::Type type)
{
    const QVariant val(KDb_emptyValueForFieldTypeCache->values.at(
                           (type <= KDbField::LastType) ? type : KDbField::InvalidType));
    if (!val.isNull())
        return val;
    else { //special cases
        if (type == KDbField::Date)
            return QDate::currentDate();
        if (type == KDbField::DateTime)
            return QDateTime::currentDateTime();
        if (type == KDbField::Time)
            return QTime::currentTime();
    }
    kdbWarning() << "no value for type" << KDbField::typeName(type);
    return QVariant();
}

//! @internal Used in KDb::notEmptyValueForFieldType()
struct KDb_NotEmptyValueForFieldTypeCache {
    KDb_NotEmptyValueForFieldTypeCache()
            : values(int(KDbField::LastType) + 1) {
#define ADD(t, value) values.insert(t, value);
        // copy most of the values
        for (int i = int(KDbField::InvalidType) + 1; i <= KDbField::LastType; i++) {
            if (i == KDbField::Date || i == KDbField::DateTime || i == KDbField::Time)
                continue; //'current' value will be returned
            if (i == KDbField::Text || i == KDbField::LongText) {
                ADD(i, QVariant(QLatin1String("")));
                continue;
            }
            if (i == KDbField::BLOB) {
//! @todo blobs will contain other MIME types too
                QByteArray ba;
//! @todo port to Qt4
#if 0
                QBuffer buffer(&ba);
                buffer.open(QIODevice::WriteOnly);
                QPixmap pm(SmallIcon("document-new"));
                pm.save(&buffer, "PNG"/*! @todo default? */);
#endif
                ADD(i, ba);
                continue;
            }
            ADD(i, KDb::emptyValueForFieldType((KDbField::Type)i));
        }
#undef ADD
    }
    QVector<QVariant> values;
};
//! Used in KDb::notEmptyValueForFieldType()
Q_GLOBAL_STATIC(KDb_NotEmptyValueForFieldTypeCache, KDb_notEmptyValueForFieldTypeCache)

QVariant KDb::notEmptyValueForFieldType(KDbField::Type type)
{
    const QVariant val(KDb_notEmptyValueForFieldTypeCache->values.at(
                           (type <= KDbField::LastType) ? type : KDbField::InvalidType));
    if (!val.isNull())
        return val;
    else { //special cases
        if (type == KDbField::Date)
            return QDate::currentDate();
        if (type == KDbField::DateTime)
            return QDateTime::currentDateTime();
        if (type == KDbField::Time)
            return QTime::currentTime();
    }
    kdbWarning() << "no value for type" << KDbField::typeName(type);
    return QVariant();
}

//! @internal @return nestimated new length after escaping of string @a string
template<typename T>
inline static int estimatedNewLength(const T &string, bool addQuotes)
{
    if (string.length() < 10)
        return string.length() * 2 + (addQuotes ? 2 : 0);
    return string.length() * 3 / 2;
}

//! @internal @return @a string string with applied KDbSQL identifier escaping.
//! If @a addQuotes is true, '"' characer is prepended and appended.
template<typename T, typename Latin1StringType, typename Latin1CharType, typename CharType>
inline static T escapeIdentifier(const T& string, bool addQuotes)
{
    const Latin1CharType quote('"');
    // create
    Latin1StringType escapedQuote("\"\"");
    T newString;
    newString.reserve(estimatedNewLength(string, addQuotes));
    if (addQuotes) {
        newString.append(quote);
    }
    for (int i = 0; i < string.length(); i++) {
        const CharType c = string.at(i);
        if (c == quote)
            newString.append(escapedQuote);
        else
            newString.append(c);
    }
    if (addQuotes) {
        newString.append(quote);
    }
    newString.squeeze();
    return newString;
}

QString KDb::escapeIdentifier(const QString& string)
{
    return ::escapeIdentifier<QString, QLatin1String, QLatin1Char, QChar>(string, false);
}

QByteArray KDb::escapeIdentifier(const QByteArray& string)
{
    return ::escapeIdentifier<QByteArray, QByteArray, char, char>(string, false);
}

QString KDb::escapeIdentifierAndAddQuotes(const QString& string)
{
    return ::escapeIdentifier<QString, QLatin1String, QLatin1Char, QChar>(string, true);
}

QByteArray KDb::escapeIdentifierAndAddQuotes(const QByteArray& string)
{
    return ::escapeIdentifier<QByteArray, QByteArray, char, char>(string, true);
}

QString KDb::escapeString(const QString& string)
{
    const QLatin1Char quote('\'');
    // find out the length ot the destination string
    // create
    QString newString(quote);
    newString.reserve(estimatedNewLength(string, true));
    for (int i = 0; i < string.length(); i++) {
        const QChar c = string.at(i);
        const ushort unicode = c.unicode();
        if (unicode == quote)
            newString.append(QLatin1String("''"));
        else if (unicode == '\t')
            newString.append(QLatin1String("\\t"));
        else if (unicode == '\\')
            newString.append(QLatin1String("\\\\"));
        else if (unicode == '\n')
            newString.append(QLatin1String("\\n"));
        else if (unicode == '\r')
            newString.append(QLatin1String("\\r"));
        else if (unicode == '\0')
            newString.append(QLatin1String("\\0"));
        else
            newString.append(c);
    }
    newString.append(QLatin1Char(quote));
    return newString;
}

QString KDb::escapeBLOB(const QByteArray& array, BLOBEscapingType type)
{
    const int size = array.size();
    if (size == 0)
        return QString();
    int escaped_length = size * 2;
    if (type == BLOBEscape0xHex || type == BLOBEscapeOctal)
        escaped_length += 2/*0x or X'*/;
    else if (type == BLOBEscapeXHex)
        escaped_length += 3; //X' + '
    QString str;
    str.reserve(escaped_length);
    if (str.capacity() < escaped_length) {
        kdbWarning() << "no enough memory (cannot allocate" << escaped_length << "chars)";
        return QString();
    }
    if (type == BLOBEscapeXHex)
        str = QString::fromLatin1("X'");
    else if (type == BLOBEscape0xHex)
        str = QString::fromLatin1("0x");
    else if (type == BLOBEscapeOctal)
        str = QString::fromLatin1("'");

    int new_length = str.length(); //after X' or 0x, etc.
    if (type == BLOBEscapeOctal) {
        // only escape nonprintable characters as in Table 8-7:
        // http://www.postgresql.org/docs/8.1/interactive/datatype-binary.html
        // i.e. escape for bytes: < 32, >= 127, 39 ('), 92(\).
        for (int i = 0; i < size; i++) {
            const unsigned char val = array[i];
            if (val < 32 || val >= 127 || val == 39 || val == 92) {
                str[new_length++] = '\\';
                str[new_length++] = '\\';
                str[new_length++] = '0' + val / 64;
                str[new_length++] = '0' + (val % 64) / 8;
                str[new_length++] = '0' + val % 8;
            } else {
                str[new_length++] = val;
            }
        }
    } else {
        for (int i = 0; i < size; i++) {
            const unsigned char val = array[i];
            str[new_length++] = (val / 16) < 10 ? ('0' + (val / 16)) : ('A' + (val / 16) - 10);
            str[new_length++] = (val % 16) < 10 ? ('0' + (val % 16)) : ('A' + (val % 16) - 10);
        }
    }
    if (type == BLOBEscapeXHex || type == BLOBEscapeOctal)
        str[new_length++] = '\'';
    return str;
}

QByteArray KDb::pgsqlByteaToByteArray(const char* data, int length)
{
    QByteArray array;
    int output = 0;
    for (int pass = 0; pass < 2; pass++) {//2 passes to avoid allocating buffer twice:
        //  0: count #of chars; 1: copy data
        const char* s = data;
        const char* end = s + length;
        if (pass == 1) {
            kdbDebug() << "processBinaryData(): real size == " << output;
            array.resize(output);
            output = 0;
        }
        for (int input = 0; s < end; output++) {
            //  kdbDebug()<<(int)s[0]<<" "<<(int)s[1]<<" "<<(int)s[2]<<" "<<(int)s[3]<<" "<<(int)s[4];
            if (s[0] == '\\' && (s + 1) < end) {
                //special cases as in http://www.postgresql.org/docs/8.1/interactive/datatype-binary.html
                if (s[1] == '\'') {// \'
                    if (pass == 1)
                        array[output] = '\'';
                    s += 2;
                } else if (s[1] == '\\') { // 2 backslashes
                    if (pass == 1)
                        array[output] = '\\';
                    s += 2;
                } else if ((input + 3) < length) {// \\xyz where xyz are 3 octal digits
                    if (pass == 1)
                        array[output] = char((int(s[1] - '0') * 8 + int(s[2] - '0')) * 8 + int(s[3] - '0'));
                    s += 4;
                } else {
                    kdbWarning() << "no octal value after backslash";
                    s++;
                }
            } else {
                if (pass == 1)
                    array[output] = s[0];
                s++;
            }
            //  kdbDebug()<<output<<": "<<(int)array[output];
        }
    }
    return array;
}

QList<int> KDb::stringListToIntList(const QStringList &list, bool *ok)
{
    QList<int> result;
    foreach (const QString &item, list) {
        int val = item.toInt(ok);
        if (ok && !*ok) {
            return QList<int>();
        }
        result.append(val);
    }
    if (ok) {
        *ok = true;
    }
    return result;
}

// Based on KConfigGroupPrivate::serializeList() from kconfiggroup.cpp (kdelibs 4)
QString KDb::serializeList(const QStringList &list)
{
    QString value;

    if (!list.isEmpty()) {
        QStringList::ConstIterator it = list.constBegin();
        const QStringList::ConstIterator end = list.constEnd();

        value = QString(*it).replace(QLatin1Char('\\'), QLatin1String("\\\\"))
                            .replace(QLatin1Char(','), QLatin1String("\\,"));

        while (++it != end) {
            // In the loop, so it is not done when there is only one element.
            // Doing it repeatedly is a pretty cheap operation.
            value.reserve(4096);

            value += QLatin1Char(',')
                     + QString(*it).replace(QLatin1Char('\\'), QLatin1String("\\\\"))
                                   .replace(QLatin1Char(','), QLatin1String("\\,"));
        }

        // To be able to distinguish an empty list from a list with one empty element.
        if (value.isEmpty())
            value = QLatin1String("\\0");
    }

    return value;
}

// Based on KConfigGroupPrivate::deserializeList() from kconfiggroup.cpp (kdelibs 4)
QStringList KDb::deserializeList(const QString &data)
{
    if (data.isEmpty())
        return QStringList();
    if (data == QLatin1String("\\0"))
        return QStringList(QString());
    QStringList value;
    QString val;
    val.reserve(data.size());
    bool quoted = false;
    for (int p = 0; p < data.length(); p++) {
        if (quoted) {
            val += data[p];
            quoted = false;
        } else if (data[p].unicode() == QLatin1Char('\\')) {
            quoted = true;
        } else if (data[p].unicode() == QLatin1Char(',')) {
            val.squeeze(); // release any unused memory
            value.append(val);
            val.clear();
            val.reserve(data.size() - p);
        } else {
            val += data[p];
        }
    }
    value.append(val);
    return value;
}

QList<int> KDb::deserializeIntList(const QString &data, bool *ok)
{
    return KDb::stringListToIntList(
        KDb::deserializeList(data), ok);
}

QString KDb::variantToString(const QVariant& v)
 {
    if (v.type() == QVariant::ByteArray) {
        return KDb::escapeBLOB(v.toByteArray(), KDb::BLOBEscapeHex);
    }
    else if (v.type() == QVariant::StringList) {
        return serializeList(v.toStringList());
    }
    return v.toString();
}

QVariant KDb::stringToVariant(const QString& s, QVariant::Type type, bool* ok)
{
    if (s.isNull()) {
        if (ok)
            *ok = true;
        return QVariant();
    }
    switch (type) {
    case QVariant::Invalid:
        if (ok)
            *ok = false;
        return QVariant();
    case QVariant::ByteArray: {//special case: hex string
        const uint len = s.length();
        QByteArray ba;
        ba.resize(len / 2 + len % 2);
        for (uint i = 0; i < (len - 1); i += 2) {
            bool _ok;
            int c = s.mid(i, 2).toInt(&_ok, 16);
            if (!_ok) {
                if (ok)
                    *ok = _ok;
                kdbWarning() << "Error in digit" << i;
                return QVariant();
            }
            ba[i/2] = (char)c;
        }
        if (ok)
            *ok = true;
        return ba;
    }
    case QVariant::StringList:
        *ok = true;
        return KDb::deserializeList(s);
    default:;
    }

    QVariant result(s);
    if (!result.convert(type)) {
        if (ok)
            *ok = false;
        return QVariant();
    }
    if (ok)
        *ok = true;
    return result;
}

bool KDb::isDefaultValueAllowed(KDbField* field)
{
    return field && !field->isUniqueKey();
}

void KDb::getLimitsForFieldType(KDbField::Type type, qlonglong *minValue, qlonglong *maxValue,
                                Signedness signedness)
{
    Q_ASSERT(minValue);
    Q_ASSERT(maxValue);
    switch (type) {
    case KDbField::Byte:
//! @todo always ok?
        *minValue = signedness == KDb::Signed ? -0x80 : 0;
        *maxValue = signedness == KDb::Signed ? 0x7F : 0xFF;
        break;
    case KDbField::ShortInteger:
        *minValue = signedness == KDb::Signed ? -0x8000 : 0;
        *maxValue = signedness == KDb::Signed ? 0x7FFF : 0xFFFF;
        break;
    case KDbField::Integer:
    case KDbField::BigInteger: //!< @todo cannot return anything larger?
    default:
        *minValue = signedness == KDb::Signed ? qlonglong(-0x07FFFFFFF) : qlonglong(0);
        *maxValue = signedness == KDb::Signed ? qlonglong(0x07FFFFFFF) : qlonglong(0x0FFFFFFFF);
    }
}

KDbField::Type KDb::maximumForIntegerFieldTypes(KDbField::Type t1, KDbField::Type t2)
{
    if (!KDbField::isIntegerType(t1) || !KDbField::isIntegerType(t2))
        return KDbField::InvalidType;
    if (t1 == t2)
        return t2;
    if (t1 == KDbField::ShortInteger && t2 != KDbField::Integer && t2 != KDbField::BigInteger)
        return t1;
    if (t1 == KDbField::Integer && t2 != KDbField::BigInteger)
        return t1;
    if (t1 == KDbField::BigInteger)
        return t1;
    return KDb::maximumForIntegerFieldTypes(t2, t1); //swap
}

QString KDb::simplifiedFieldTypeName(KDbField::Type type)
{
    if (KDbField::isNumericType(type))
        return KDbField::tr("Number"); //simplify
    else if (type == KDbField::BLOB)
//! @todo support names of other BLOB subtypes
        return KDbField::tr("Image"); //simplify

    return KDbField::typeGroupName(KDbField::typeGroup(type));
}

QString KDb::defaultFileBasedDriverMimeType()
{
    return QLatin1String("application/x-kexiproject-sqlite3");
}

QString KDb::defaultFileBasedDriverId()
{
    return QLatin1String("org.kde.kdb.sqlite");
}

// Try to convert from string to type T
template <typename T>
QVariant convert(T (QString::*ConvertToT)(bool*,int) const, const char *data, int size,
                 qlonglong minValue, qlonglong maxValue, bool *ok)
{
    T v = (QString::fromLatin1(data, size).*ConvertToT)(ok, 10);
    if (*ok) {
        *ok = minValue <= v && v <= maxValue;
    }
    return KDb::iif(*ok, QVariant(v));
}

QVariant KDb::cstringToVariant(const char* data, KDbField::Type type, bool *ok, int length,
                               KDb::Signedness signedness)
{
    bool tempOk;
    bool *thisOk = ok ? ok : &tempOk;
    if (type < KDbField::Byte || type > KDbField::LastType) {
        *thisOk = false;
        return QVariant();
    }
    if (!data) { // NULL value
        *thisOk = true;
        return QVariant();
    }
    // from most to least frequently used types:

    if (KDbField::isTextType(type)) {
        *thisOk = true;
        //! @todo use KDbDriverBehaviour::TEXT_TYPE_MAX_LENGTH for Text type?
        return QString::fromUtf8(data, length);
    }
    if (KDbField::isIntegerType(type)) {
        qlonglong minValue, maxValue;
        const bool isUnsigned = signedness == KDb::Unsigned;
        KDb::getLimitsForFieldType(type, &minValue, &maxValue, signedness);
        switch (type) {
        case KDbField::Byte: // Byte here too, minValue/maxValue will take care of limits
        case KDbField::ShortInteger:
            return isUnsigned ?
                convert(&QString::toUShort, data, length, minValue, maxValue, thisOk)
                : convert(&QString::toShort, data, length, minValue, maxValue, thisOk);
        case KDbField::Integer:
            return isUnsigned ?
                convert(&QString::toUInt, data, length, minValue, maxValue, thisOk)
                : convert(&QString::toInt, data, length, minValue, maxValue, thisOk);
        case KDbField::BigInteger:
            return isUnsigned ?
                convert(&QString::toULongLong, data, length, minValue, maxValue, thisOk)
                : convert(&QString::toLongLong, data, length, minValue, maxValue, thisOk);
        default:
            qFatal("Unsupported integer type %d", type);
        }
    }
    if (KDbField::isFPNumericType(type)) {
        return KDb::iif(*thisOk, QVariant(QString::fromLatin1(data, length).toDouble(thisOk)));
    }
    if (type == KDbField::BLOB) {
        *thisOk = length >= 0;
        return *thisOk ? QVariant(QByteArray(data, length)) : QVariant();
    }
    // the default
//! @todo date/time?
    QVariant result(QString::fromUtf8(data, length));
    if (!result.convert(KDbField::variantType(type))) {
        *thisOk = false;
        return QVariant();
    }
    *thisOk = true;
    return result;
}

QStringList KDb::libraryPaths()
{
    QStringList result;
    foreach (const QString& path, qApp->libraryPaths()) {
        const QString dir(path + QLatin1String("/kdb"));
        if (QDir(dir).exists() && QDir(dir).isReadable()) {
            result += dir;
        }
    }
    return result;
}

QString KDb::temporaryTableName(KDbConnection *conn, const QString &baseName)
{
    while (true) {
        QString name = QLatin1String("tmp__") + baseName;
        for (int i = 0; i < 10; ++i) {
            name += QString::number(qrand() % 0x10, 16);
        }
        if (!conn->drv_containsTable(name)) {
            return name;
        }
    }
}

QString KDb::sqlite3ProgramPath()
{
    QString path = KDbUtils::findExe(QLatin1String("sqlite3"));
    if (path.isEmpty()) {
        kdbWarning() << "Could not find program \"sqlite3\"";
    }
    return path;
}

bool KDb::importSqliteFile(const QString &inputFileName, const QString &outputFileName)
{
    const QString sqlite_app = KDb::sqlite3ProgramPath();
    if (sqlite_app.isEmpty()) {
        return false;
    }

    QFileInfo fi(inputFileName);
    if (!fi.isReadable()) {
        kdbWarning() << "No readable input file" << fi.absoluteFilePath();
        return false;
    }
    QFileInfo fo(outputFileName);
    if (QFile(fo.absoluteFilePath()).exists()) {
        if (!QFile::remove(fo.absoluteFilePath())) {
            kdbWarning() << "Cannot remove output file" << fo.absoluteFilePath();
            return false;
        }
    }
    kdbDebug() << inputFileName << fi.absoluteDir().path() << fo.absoluteFilePath();

    QProcess p;
    p.start(sqlite_app, QStringList() << fo.absoluteFilePath());
    if (!p.waitForStarted()) {
        kdbWarning() << "Failed to start program" << sqlite_app;
        return false;
    }
    QByteArray line(".read " + QFile::encodeName(fi.absoluteFilePath()));
    if (p.write(line) != line.length() || !p.waitForBytesWritten()) {
        kdbWarning() << "Failed to send \".read\" command to program" << sqlite_app;
        return false;
    }
    p.closeWriteChannel();
    if (!p.waitForFinished()) {
        kdbWarning() << "Failed to finish program" << sqlite_app;
        return false;
    }
    return true;
}

//---------

bool KDb::isIdentifier(const QString& s)
{
    uint i;
    const uint sLength = s.length();
    for (i = 0; i < sLength; i++) {
        const char c = s.at(i).toLower().toLatin1();
        if (c == 0 || !(c == '_' || (c >= 'a' && c <= 'z') || (i > 0 && c >= '0' && c <= '9')))
            break;
    }
    return i > 0 && i == sLength;
}

static inline QString charToIdentifier(const QChar& c)
{
    if (c.unicode() >= TRANSLITERATION_TABLE_SIZE)
        return QLatin1String("_");
    const char *const s = transliteration_table[c.unicode()];
    return s ? QString::fromLatin1(s) : QLatin1String("_");
}

QString KDb::stringToIdentifier(const QString &s)
{
    if (s.isEmpty())
        return QString();
    QString r, id = s.simplified();
    if (id.isEmpty())
        return QString();
    r.reserve(id.length());
    id.replace(QLatin1Char(' '), QLatin1String("_"));
    const QChar c = id[0];
    const char ch = c.toLatin1();
    QString add;
    bool wasUnderscore = false;

    if (ch >= '0' && ch <= '9') {
        r += QLatin1Char('_') + c;
    } else {
        add = charToIdentifier(c);
        r += add;
        wasUnderscore = add == QLatin1String("_");
    }

    const uint idLength = id.length();
    for (uint i = 1; i < idLength; i++) {
        add = charToIdentifier(id.at(i));
        if (wasUnderscore && add == QLatin1String("_"))
            continue;
        wasUnderscore = add == QLatin1String("_");
        r += add;
    }
    return r;
}

QString KDb::identifierExpectedMessage(const QString &valueName, const QVariant& v)
{
    return QLatin1String("<p>") + kdb::tr("Value of \"%1\" column must be an identifier.")
            .arg(valueName)
           + QLatin1String("</p><p>")
           + kdb::tr("\"%1\" is not a valid identifier.").arg(v.toString()) + QLatin1String("</p>");
}

//--------------------------------------------------------------------------------

#ifdef KDB_DEBUG_GUI

static KDb::DebugGUIHandler s_debugGUIHandler = 0;

void KDb::setDebugGUIHandler(KDb::DebugGUIHandler handler)
{
    s_debugGUIHandler = handler;
}

void KDb::debugGUI(const QString& text)
{
    if (s_debugGUIHandler)
        s_debugGUIHandler(text);
}

static KDb::AlterTableActionDebugGUIHandler s_alterTableActionDebugHandler = 0;

void KDb::setAlterTableActionDebugHandler(KDb::AlterTableActionDebugGUIHandler handler)
{
    s_alterTableActionDebugHandler = handler;
}

void KDb::alterTableActionDebugGUI(const QString& text, int nestingLevel)
{
    if (s_alterTableActionDebugHandler)
        s_alterTableActionDebugHandler(text, nestingLevel);
}

#endif // KDB_DEBUG_GUI

#include "KDb.moc"
