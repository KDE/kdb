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

#include "KDbNativeStatementBuilder.h"
#include "KDbConnection.h"
#include "kdb_debug.h"
#include "KDbDriverBehavior.h"
#include "KDbExpression.h"
#include "KDbLookupFieldSchema.h"
#include "KDbOrderByColumn.h"
#include "KDbQueryAsterisk.h"
#include "KDbQuerySchema.h"
#include "KDbQuerySchemaParameter.h"
#include "KDbRelationship.h"

KDbSelectStatementOptions::KDbSelectStatementOptions()
        : alsoRetrieveRecordId(false)
        , addVisibleLookupColumns(true)
{
}

//================================================

class Q_DECL_HIDDEN KDbNativeStatementBuilder::Private
{
public:
    Private() {}
    //! @todo use equivalent of QPointer<KDbConnection>
    KDbConnection *connection;

    inline KDbDriver *driver() { return connection ? connection->driver() : nullptr; }
private:
    Q_DISABLE_COPY(Private)
};

//================================================

KDbNativeStatementBuilder::KDbNativeStatementBuilder(KDbConnection *connection)
    : d(new Private)
{
    d->connection = connection;
}

KDbNativeStatementBuilder::~KDbNativeStatementBuilder()
{
    delete d;
}

static bool selectStatementInternal(KDbEscapedString *target,
                                    KDbConnection *connection,
                                    KDbQuerySchema* querySchema,
                                    const KDbSelectStatementOptions& options,
                                    const QList<QVariant>& parameters)
{
    Q_ASSERT(target);
    Q_ASSERT(querySchema);
//"SELECT FROM ..." is theoretically allowed "
//if (querySchema.fieldCount()<1)
//  return QString();
// Each SQL identifier needs to be escaped in the generated query.

    const KDbDriver *driver = connection ? connection->driver() : 0;

    if (!querySchema->statement().isEmpty()) {
//! @todo replace with KDbNativeQuerySchema? It shouldn't be here.
        *target = querySchema->statement();
        return true;
    }

//! @todo looking at singleTable is visually nice but a field name can conflict
//!   with function or variable name...
    int number = 0;
    bool singleTable = querySchema->tables()->count() <= 1;
    if (singleTable) {
        //make sure we will have single table:
        foreach(KDbField *f, *querySchema->fields()) {
            if (querySchema->isColumnVisible(number) && f->table() && f->table()->lookupFieldSchema(*f)) {
                //uups, no, there's at least one left join
                singleTable = false;
                break;
            }
            number++;
        }
    }

    KDbEscapedString sql; //final sql string
    sql.reserve(4096);
    KDbEscapedString s_additional_joins; //additional joins needed for lookup fields
    KDbEscapedString s_additional_fields; //additional fields to append to the fields list
    int internalUniqueTableAliasNumber = 0; //used to build internalUniqueTableAliases
    int internalUniqueQueryAliasNumber = 0; //used to build internalUniqueQueryAliases
    number = 0;
    QList<KDbQuerySchema*> subqueries_for_lookup_data; // subqueries will be added to FROM section
    KDbEscapedString kdb_subquery_prefix("__kdb_subquery_");
    foreach(KDbField *f, *querySchema->fields()) {
        if (querySchema->isColumnVisible(number)) {
            if (!sql.isEmpty())
                sql += ", ";

            if (f->isQueryAsterisk()) {
                if (!singleTable && static_cast<KDbQueryAsterisk*>(f)->isSingleTableAsterisk()) { //single-table *
                    sql.append(KDb::escapeIdentifier(driver, f->table()->name())).append(".*");
                }
                else { //all-tables * (or simplified table.* when there's only one table)
                    sql += '*';
                }
            } else {
                if (f->isExpression()) {
                    sql += f->expression().toString(driver);
                } else {
                    if (!f->table()) {//sanity check
                        return false;
                    }

                    QString tableName;
                    int tablePosition = querySchema->tableBoundToColumn(number);
                    if (tablePosition >= 0) {
                        tableName = KDb::iifNotEmpty(querySchema->tableAlias(tablePosition),
                                                           f->table()->name());
                    }
                    if (options.addVisibleLookupColumns) { // try to find table/alias name harder
                        if (tableName.isEmpty()) {
                            tableName = querySchema->tableAlias(f->table()->name());
                        }
                        if (tableName.isEmpty()) {
                            tableName = f->table()->name();
                        }
                    }
                    if (!singleTable && !tableName.isEmpty()) {
                        sql.append(KDb::escapeIdentifier(driver, tableName)).append('.');
                    }
                    sql += KDb::escapeIdentifier(driver, f->name());
                }
                const QString aliasString(querySchema->columnAlias(number));
                if (!aliasString.isEmpty()) {
                    sql.append(" AS ").append(aliasString);
                }
//! @todo add option that allows to omit "AS" keyword
            }
            KDbLookupFieldSchema *lookupFieldSchema = (options.addVisibleLookupColumns && f->table())
                                                   ? f->table()->lookupFieldSchema(*f) : 0;
            if (lookupFieldSchema && lookupFieldSchema->boundColumn() >= 0) {
                // Lookup field schema found
                // Now we also need to fetch "visible" value from the lookup table, not only the value of binding.
                // -> build LEFT OUTER JOIN clause for this purpose (LEFT, not INNER because the binding can be broken)
                // "LEFT OUTER JOIN lookupTable ON thisTable.thisField=lookupTable.boundField"
                KDbLookupFieldSchemaRecordSource recordSource = lookupFieldSchema->recordSource();
                if (recordSource.type() == KDbLookupFieldSchemaRecordSource::Table) {
                    KDbTableSchema *lookupTable = querySchema->connection()->tableSchema(recordSource.name());
                    KDbFieldList* visibleColumns = 0;
                    KDbField *boundField = 0;
                    if (lookupTable
                            && lookupFieldSchema->boundColumn() < lookupTable->fieldCount()
                            && (visibleColumns = lookupTable->subList(lookupFieldSchema->visibleColumns()))
                            && (boundField = lookupTable->field(lookupFieldSchema->boundColumn()))) {
                        //add LEFT OUTER JOIN
                        if (!s_additional_joins.isEmpty())
                            s_additional_joins += ' ';
                        const QString internalUniqueTableAlias(
                            QLatin1String("__kdb_") + lookupTable->name() + QLatin1Char('_')
                            + QString::number(internalUniqueTableAliasNumber++));
                        s_additional_joins += KDbEscapedString("LEFT OUTER JOIN %1 AS %2 ON %3.%4=%5.%6")
                            .arg(KDb::escapeIdentifier(driver, lookupTable->name()))
                            .arg(internalUniqueTableAlias)
                            .arg(KDb::escapeIdentifier(driver, querySchema->tableAliasOrName(f->table()->name())))
                            .arg(KDb::escapeIdentifier(driver, f->name()))
                            .arg(internalUniqueTableAlias)
                            .arg(KDb::escapeIdentifier(driver, boundField->name()));

                        //add visibleField to the list of SELECTed fields //if it is not yet present there
                        if (!s_additional_fields.isEmpty())
                            s_additional_fields += ", ";
//! @todo Add lookup schema option for separator other than ' ' or even option for placeholders like "Name ? ?"
//! @todo Add possibility for joining the values at client side.
                        s_additional_fields += visibleColumns->sqlFieldsList(
                                                   connection, QLatin1String(" || ' ' || "), internalUniqueTableAlias,
                                                   driver ? KDb::DriverEscaping : KDb::KDbEscaping);
                    }
                    delete visibleColumns;
                } else if (recordSource.type() == KDbLookupFieldSchemaRecordSource::Query) {
                    KDbQuerySchema *lookupQuery = querySchema->connection()->querySchema(recordSource.name());
                    if (!lookupQuery) {
                        kdbWarning() << "!lookupQuery";
                        return false;
                    }
                    const KDbQueryColumnInfo::Vector fieldsExpanded(lookupQuery->fieldsExpanded());
                    if (lookupFieldSchema->boundColumn() >= fieldsExpanded.count()) {
                        kdbWarning() << "lookupFieldSchema->boundColumn() >= fieldsExpanded.count()";
                        return false;
                    }
                    KDbQueryColumnInfo *boundColumnInfo = fieldsExpanded.at(lookupFieldSchema->boundColumn());
                    if (!boundColumnInfo) {
                        kdbWarning() << "!boundColumnInfo";
                        return false;
                    }
                    KDbField *boundField = boundColumnInfo->field();
                    if (!boundField) {
                        kdbWarning() << "!boundField";
                        return false;
                    }
                    //add LEFT OUTER JOIN
                    if (!s_additional_joins.isEmpty())
                        s_additional_joins += ' ';
                    KDbEscapedString internalUniqueQueryAlias
                        = kdb_subquery_prefix + connection->escapeString(lookupQuery->name()) + '_'
                        + QString::number(internalUniqueQueryAliasNumber++);
                    KDbNativeStatementBuilder builder(connection);
                    KDbEscapedString subSql;
                    if (!builder.generateSelectStatement(&subSql, lookupQuery, options,
                                                         parameters))
                    {
                        return false;
                    }
                    s_additional_joins += KDbEscapedString("LEFT OUTER JOIN (%1) AS %2 ON %3.%4=%5.%6")
                        .arg(subSql)
                        .arg(internalUniqueQueryAlias)
                        .arg(KDb::escapeIdentifier(driver, f->table()->name()))
                        .arg(KDb::escapeIdentifier(driver, f->name()))
                        .arg(internalUniqueQueryAlias)
                        .arg(KDb::escapeIdentifier(driver, boundColumnInfo->aliasOrName()));

                    if (!s_additional_fields.isEmpty())
                        s_additional_fields += ", ";
                    const QList<int> visibleColumns(lookupFieldSchema->visibleColumns());
                    KDbEscapedString expression;
                    foreach(int visibleColumnIndex, visibleColumns) {
//! @todo Add lookup schema option for separator other than ' ' or even option for placeholders like "Name ? ?"
//! @todo Add possibility for joining the values at client side.
                        if (fieldsExpanded.count() <= visibleColumnIndex) {
                            kdbWarning() << "fieldsExpanded.count() <= (*visibleColumnsIt) : "
                            << fieldsExpanded.count() << " <= " << visibleColumnIndex;
                            return false;
                        }
                        if (!expression.isEmpty())
                            expression += " || ' ' || ";
                        expression += (
                            internalUniqueQueryAlias + '.'
                            + KDb::escapeIdentifier(driver, fieldsExpanded.value(visibleColumnIndex)->aliasOrName())
                        );
                    }
                    s_additional_fields += expression;
                }
                else {
                    kdbWarning() << "unsupported record source type" << recordSource.typeName();
                    return false;
                }
            }
        }
        number++;
    }

    //add lookup fields
    if (!s_additional_fields.isEmpty())
        sql += (", " + s_additional_fields);

    if (driver && options.alsoRetrieveRecordId) { //append rowid column
        KDbEscapedString s;
        if (!sql.isEmpty())
            s = ", ";
        if (querySchema->masterTable())
            s += KDbEscapedString(querySchema->tableAliasOrName(querySchema->masterTable()->name())) + '.';
        s += driver->behavior()->ROW_ID_FIELD_NAME;
        sql += s;
    }

    sql.prepend("SELECT ");
    QList<KDbTableSchema*>* tables = querySchema->tables();
    if ((tables && !tables->isEmpty()) || !subqueries_for_lookup_data.isEmpty()) {
        sql += " FROM ";
        KDbEscapedString s_from;
        if (tables) {
            number = 0;
            foreach(KDbTableSchema *table, *tables) {
                if (!s_from.isEmpty())
                    s_from += ", ";
                s_from += KDb::escapeIdentifier(driver, table->name());
                const QString aliasString(querySchema->tableAlias(number));
                if (!aliasString.isEmpty())
                    s_from.append(" AS ").append(aliasString);
                number++;
            }
        }
        // add subqueries for lookup data
        int subqueries_for_lookup_data_counter = 0;
        foreach(KDbQuerySchema* subQuery, subqueries_for_lookup_data) {
            if (!s_from.isEmpty())
                s_from += ", ";
            KDbEscapedString subSql;
            if (!selectStatementInternal(&subSql, connection, subQuery, options, parameters)) {
                return false;
            }
            s_from += '(' + subSql + ") AS " + kdb_subquery_prefix
                      + KDbEscapedString::number(subqueries_for_lookup_data_counter++);
        }
        sql += s_from;
    }
    KDbEscapedString s_where;
    s_where.reserve(4096);

    //JOINS
    if (!s_additional_joins.isEmpty()) {
        sql += ' ' + s_additional_joins + ' ';
    }

//! @todo: we're using WHERE for joins now; use INNER/LEFT/RIGHT JOIN later

    //WHERE
    bool wasWhere = false; //for later use
    foreach(KDbRelationship *rel, *querySchema->relationships()) {
        if (s_where.isEmpty()) {
            wasWhere = true;
        } else
            s_where += " AND ";
        KDbEscapedString s_where_sub;
        foreach(const KDbField::Pair &pair, *rel->fieldPairs()) {
            if (!s_where_sub.isEmpty())
                s_where_sub += " AND ";
            s_where_sub +=
               KDbEscapedString(KDb::escapeIdentifier(driver, pair.first->table()->name())) + '.' +
               KDb::escapeIdentifier(driver, pair.first->name()) + " = " +
               KDb::escapeIdentifier(driver, pair.second->table()->name()) + '.' +
               KDb::escapeIdentifier(driver, pair.second->name());
        }
        if (rel->fieldPairs()->count() > 1) {
            s_where_sub.prepend('(');
            s_where_sub += ')';
        }
        s_where += s_where_sub;
    }
    //EXPLICITLY SPECIFIED WHERE EXPRESSION
    if (driver && !querySchema->whereExpression().isNull()) {
        KDbQuerySchemaParameterValueListIterator paramValuesIt(parameters);
        KDbQuerySchemaParameterValueListIterator *paramValuesItPtr = parameters.isEmpty() ? 0 : &paramValuesIt;
        if (wasWhere) {
//! @todo () are not always needed
            s_where = '(' + s_where + ") AND ("
                + querySchema->whereExpression().toString(driver, paramValuesItPtr) + ')';
        } else {
            s_where = querySchema->whereExpression().toString(driver, paramValuesItPtr);
        }
    }
    if (!s_where.isEmpty())
        sql += " WHERE " + s_where;
//! @todo (js) add other sql parts
    //(use wasWhere here)

    // ORDER BY
    KDbEscapedString orderByString(
        querySchema->orderByColumnList()->toSQLString(
            !singleTable/*includeTableName*/, connection, driver ? KDb::DriverEscaping : KDb::KDbEscaping)
    );
    const QVector<int> pkeyFieldsOrder(querySchema->pkeyFieldsOrder());
    if (orderByString.isEmpty() && !pkeyFieldsOrder.isEmpty()) {
        //add automatic ORDER BY if there is no explicitly defined (especially helps when there are complex JOINs)
        KDbOrderByColumnList automaticPKOrderBy;
        const KDbQueryColumnInfo::Vector fieldsExpanded(querySchema->fieldsExpanded());
        foreach(int pkeyFieldsIndex, pkeyFieldsOrder) {
            if (pkeyFieldsIndex < 0) // no field mentioned in this query
                continue;
            if (pkeyFieldsIndex >= fieldsExpanded.count()) {
                kdbWarning() << "ORDER BY: (*it) >= fieldsExpanded.count() - "
                        << pkeyFieldsIndex << " >= " << fieldsExpanded.count();
                continue;
            }
            KDbQueryColumnInfo *ci = fieldsExpanded[ pkeyFieldsIndex ];
            automaticPKOrderBy.appendColumn(ci);
        }
        orderByString = automaticPKOrderBy.toSQLString(!singleTable/*includeTableName*/,
                        connection, driver ? KDb::DriverEscaping : KDb::KDbEscaping);
    }
    if (!orderByString.isEmpty())
        sql += (" ORDER BY " + orderByString);

    //kdbDebug() << sql;
    *target = sql;
    return true;
}

bool KDbNativeStatementBuilder::generateSelectStatement(KDbEscapedString *target,
                                                        KDbQuerySchema* querySchema,
                                                        const KDbSelectStatementOptions& options,
                                                        const QList<QVariant>& parameters) const
{
    return selectStatementInternal(target, d->connection, querySchema, options, parameters);
}

bool KDbNativeStatementBuilder::generateSelectStatement(KDbEscapedString *target,
                                                        KDbQuerySchema* querySchema,
                                                        const QList<QVariant>& parameters) const
{
    return selectStatementInternal(target, d->connection, querySchema, KDbSelectStatementOptions(),
                                   parameters);
}

bool KDbNativeStatementBuilder::generateSelectStatement(KDbEscapedString *target,
                                                        KDbTableSchema* tableSchema,
                                                        const KDbSelectStatementOptions& options) const
{
    return generateSelectStatement(target, tableSchema->query(), options);
}

bool KDbNativeStatementBuilder::generateCreateTableStatement(KDbEscapedString *target,
                                                             const KDbTableSchema& tableSchema) const
{
    Q_ASSERT(target);
    // Each SQL identifier needs to be escaped in the generated query.
    const KDbDriver *driver = d->connection ? d->connection->driver() : nullptr;
    KDbEscapedString sql;
    sql.reserve(4096);
    sql = KDbEscapedString("CREATE TABLE ")
            + KDb::escapeIdentifier(driver, tableSchema.name()) + " (";
    bool first = true;
    foreach(KDbField *field, tableSchema.m_fields) {
        if (first)
            first = false;
        else
            sql += ", ";
        KDbEscapedString v = KDbEscapedString(KDb::escapeIdentifier(driver, field->name())) + ' ';
        const bool autoinc = field->isAutoIncrement();
        const bool pk = field->isPrimaryKey() || (autoinc && driver && driver->beh->AUTO_INCREMENT_REQUIRES_PK);
//! @todo warning: ^^^^^ this allows only one autonumber per table when AUTO_INCREMENT_REQUIRES_PK==true!
        const KDbField::Type type = field->type(); // cache: evaluating type of expressions can be expensive
        if (autoinc && d->driver()->beh->SPECIAL_AUTO_INCREMENT_DEF) {
            if (pk)
                v.append(d->driver()->beh->AUTO_INCREMENT_TYPE).append(' ')
                 .append(d->driver()->beh->AUTO_INCREMENT_PK_FIELD_OPTION);
            else
                v.append(d->driver()->beh->AUTO_INCREMENT_TYPE).append(' ')
                 .append(d->driver()->beh->AUTO_INCREMENT_FIELD_OPTION);
        } else {
            if (autoinc && !d->driver()->beh->AUTO_INCREMENT_TYPE.isEmpty())
                v += d->driver()->beh->AUTO_INCREMENT_TYPE;
            else
                v += d->driver()->sqlTypeName(type, *field);

            if (KDbField::isIntegerType(type) && field->isUnsigned()) {
                v.append(' ').append(d->driver()->beh->UNSIGNED_TYPE_KEYWORD);
            }

            if (KDbField::isFPNumericType(type) && field->precision() > 0) {
                if (field->scale() > 0)
                    v += QString::fromLatin1("(%1,%2)").arg(field->precision()).arg(field->scale());
                else
                    v += QString::fromLatin1("(%1)").arg(field->precision());
            }
            else if (type == KDbField::Text) {
                int realMaxLen;
                if (d->driver()->beh->TEXT_TYPE_MAX_LENGTH == 0) {
                    realMaxLen = field->maxLength(); // allow to skip (N)
                }
                else { // max length specified by driver
                    if (field->maxLength() == 0) { // as long as possible
                        realMaxLen = d->driver()->beh->TEXT_TYPE_MAX_LENGTH;
                    }
                    else { // not longer than specified by driver
                        realMaxLen = qMin(d->driver()->beh->TEXT_TYPE_MAX_LENGTH, field->maxLength());
                    }
                }
                if (realMaxLen > 0) {
                    v += QString::fromLatin1("(%1)").arg(realMaxLen);
                }
            }

            if (autoinc) {
                v.append(' ').append(pk ? d->driver()->beh->AUTO_INCREMENT_PK_FIELD_OPTION
                                        : d->driver()->beh->AUTO_INCREMENT_FIELD_OPTION);
            }
            else {
                //! @todo here is automatically a single-field key created
                if (pk)
                    v += " PRIMARY KEY";
            }
            if (!pk && field->isUniqueKey())
                v += " UNIQUE";
///@todo IS this ok for all engines?: if (!autoinc && !field->isPrimaryKey() && field->isNotNull())
            if (!autoinc && !pk && field->isNotNull())
                v += " NOT NULL"; //only add not null option if no autocommit is set
            if (d->driver()->supportsDefaultValue(*field) && field->defaultValue().isValid()) {
                KDbEscapedString valToSQL(d->driver()->valueToSQL(field, field->defaultValue()));
                if (!valToSQL.isEmpty()) //for sanity
                    v += " DEFAULT " + valToSQL;
            }
        }
        sql += v;
    }
    sql += ')';
    *target = sql;
    return true;
}
