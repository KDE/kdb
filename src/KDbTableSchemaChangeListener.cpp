/* This file is part of the KDE project
   Copyright (C) 2003-2017 Jarosław Staniek <staniek@kde.org>

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

#include "KDbTableSchemaChangeListener.h"
#include "KDbConnection.h"
#include "KDbConnection_p.h"
#include "KDbLookupFieldSchema.h"
#include "kdb_debug.h"

#ifdef KDB_TABLESCHEMACHANGELISTENER_DEBUG
# define localDebug(...) kdbDebug(__VA_ARGS__)
#else
# define localDebug(...) if (true) {} else kdbDebug(__VA_ARGS__)
#endif

class KDbTableSchemaChangeListenerPrivate
{
public:
    KDbTableSchemaChangeListenerPrivate()
    {
    }

    //! Registers listener @a listener for changes in table @a table
    static void registerForChanges(KDbConnection *conn, KDbTableSchemaChangeListener *listener,
                                   const KDbTableSchema *table)
    {
        Q_ASSERT(conn);
        Q_ASSERT(listener);
        Q_ASSERT(table);
        QSet<KDbTableSchemaChangeListener*>* listeners = conn->d->tableSchemaChangeListeners.value(table);
        if (!listeners) {
            listeners = new QSet<KDbTableSchemaChangeListener*>();
            conn->d->tableSchemaChangeListeners.insert(table, listeners);
        }
        localDebug() << "listener=" << listener->name() << "table=" << table->name();
        listeners->insert(listener);
    }

    //! Registers listener @a listener for changes in query @a query
    static void registerForChanges(KDbConnection *conn, KDbTableSchemaChangeListener *listener,
                                   const KDbQuerySchema *query)
    {
        Q_ASSERT(conn);
        Q_ASSERT(listener);
        Q_ASSERT(query);
        QSet<KDbTableSchemaChangeListener *> *listeners
            = conn->d->queryTableSchemaChangeListeners.value(query);
        if (!listeners) {
            listeners = new QSet<KDbTableSchemaChangeListener*>();
            conn->d->queryTableSchemaChangeListeners.insert(query, listeners);
        }
        localDebug() << "listener=" << listener->name() << "query=" << query->name();
        listeners->insert(listener);
    }

    //! Unregisters listener @a listener for changes in table @a table
    static void unregisterForChanges(KDbConnection *conn, KDbTableSchemaChangeListener *listener,
                                     const KDbTableSchema *table)
    {
        Q_ASSERT(conn);
        Q_ASSERT(table);
        QSet<KDbTableSchemaChangeListener *> *listeners
            = conn->d->tableSchemaChangeListeners.value(table);
        if (!listeners) {
            return;
        }
        localDebug() << "listener=" << (listener ? listener->name() : QString::fromLatin1("<all>"))
                     << "table=" << table->name();
        if (listener) {
            listeners->remove(listener);
        } else {
            listeners->clear();
        }
    }

    //! Unregisters listener @a listener for changes in query @a query
    static void unregisterForChanges(KDbConnection *conn, KDbTableSchemaChangeListener *listener,
                                     const KDbQuerySchema *query)
    {
        Q_ASSERT(conn);
        Q_ASSERT(query);
        QSet<KDbTableSchemaChangeListener *> *listeners
            = conn->d->queryTableSchemaChangeListeners.value(query);
        if (!listeners) {
            return;
        }
        localDebug() << "listener=" << (listener ? listener->name() : QString::fromLatin1("<all>"))
                     << "query=" << query->name();
        if (listener) {
            listeners->remove(listener);
        } else {
            listeners->clear();
        }
    }

    //! Unregisters listener @a listener for any changes
    static void unregisterForChanges(KDbConnection *conn, KDbTableSchemaChangeListener* listener)
    {
        Q_ASSERT(conn);
        Q_ASSERT(listener);
        localDebug() << "listener=" << listener->name();
        for (QSet<KDbTableSchemaChangeListener*> *listeners : conn->d->tableSchemaChangeListeners) {
            listeners->remove(listener);
        }
        for (QSet<KDbTableSchemaChangeListener*> *listeners : conn->d->queryTableSchemaChangeListeners) {
            listeners->remove(listener);
        }
    }

    //! Returns @c true if @a table1 depends on @a table2, that is, if:
    //! - @a table1 == @a table2, or
    //! - @a table1 has lookup columns that reference @a table2
    static bool tableDependsOnTable(QSet<const KDbTableSchema *> *checkedTables,
                                    QSet<const KDbQuerySchema *> *checkedQueries,
                                    KDbConnection *conn, const KDbTableSchema *table1,
                                    const KDbTableSchema *table2)
    {
        if (checkedTables->contains(table1)) {
            localDebug() << "Table" << table1->name() << "already checked";
            return false; // protection against infinite recursion
        }
        checkedTables->insert(table1);
        localDebug() << "Checking if table" << table1->name() << "depends on table" << table2->name();
        if (table1 == table2) {
            localDebug() << "Yes";
            return true;
        }
        for (KDbLookupFieldSchema *lookup : table1->lookupFields()) {
            switch (lookup->recordSource().type()) {
            case KDbLookupFieldSchemaRecordSource::Type::Table: {
                const KDbTableSchema *sourceTable
                    = conn->tableSchema(lookup->recordSource().name());
                if (sourceTable
                    && tableDependsOnTable(checkedTables, checkedQueries, conn, sourceTable, table2))
                {
                    return true;
                }
                break;
            }
            case KDbLookupFieldSchemaRecordSource::Type::Query: {
                const KDbQuerySchema *sourceQuery
                    = conn->querySchema(lookup->recordSource().name());
                if (sourceQuery
                    && queryDependsOnTable(checkedTables, checkedQueries, conn, sourceQuery, table2))
                {
                    return true;
                }
                break;
            }
            default:
                kdbWarning() << "Unsupported lookup field's source type" << lookup->recordSource().typeName();
                //! @todo support more record source types
                break;
            }
        }
        return false;
    }

    //! Returns @c true if @a table depends on @a query, that is, if:
    //! - @a table has lookup columns that reference @a query
    static bool tableDependsOnQuery(QSet<const KDbTableSchema *> *checkedTables,
                                    QSet<const KDbQuerySchema *> *checkedQueries,
                                    KDbConnection *conn, const KDbTableSchema *table,
                                    const KDbQuerySchema *query)
    {
        if (checkedTables->contains(table)) {
            localDebug() << "Table" << table->name() << "already checked";
            return false; // protection against infinite recursion
        }
        checkedTables->insert(table);
        localDebug() << "Checking if table" << table->name() << "depends on query" << query->name();
        for (KDbLookupFieldSchema *lookup : table->lookupFields()) {
            switch (lookup->recordSource().type()) {
            case KDbLookupFieldSchemaRecordSource::Type::Table: {
                const KDbTableSchema *sourceTable
                    = conn->tableSchema(lookup->recordSource().name());
                if (sourceTable
                    && tableDependsOnQuery(checkedTables, checkedQueries, conn, sourceTable, query))
                {
                    return true;
                }
                break;
            }
            case KDbLookupFieldSchemaRecordSource::Type::Query: {
                const KDbQuerySchema *sourceQuery
                    = conn->querySchema(lookup->recordSource().name());
                if (sourceQuery
                    && queryDependsOnQuery(checkedTables, checkedQueries, conn, sourceQuery, query))
                {
                    return true;
                }
                break;
            }
            default:
                kdbWarning() << "Unsupported lookup field's source type" << lookup->recordSource().typeName();
                //! @todo support more record source types
            }
        }
        return false;
    }

    //! Returns @c true if @a query depends on @a table, that is, if:
    //! - @a query references table that depends on @a table (dependency is checked using
    //!   tableDependsOnTable())
    static bool queryDependsOnTable(QSet<const KDbTableSchema *> *checkedTables,
                                    QSet<const KDbQuerySchema *> *checkedQueries,
                                    KDbConnection *conn, const KDbQuerySchema *query,
                                    const KDbTableSchema *table)
    {
        if (checkedQueries->contains(query)) {
            localDebug() << "Query" << query->name() << "already checked";
            return false; // protection against infinite recursion
        }
        checkedQueries->insert(query);
        localDebug() << "Checking if query" << query->name() << "depends on table" << table->name();
        for (const KDbTableSchema *queryTable : *query->tables()) {
            if (tableDependsOnTable(checkedTables, checkedQueries, conn, queryTable, table)) {
                return true;
            }
        }
        return false;
    }

    //! Returns @c true if @a query1 depends on @a query2, that is, if:
    //! - @a query1 == @a query2, or
    //! - @a query2 references table that depends on @a query (dependency is checked using
    //!   tableDependsOnQuery())
    static bool queryDependsOnQuery(QSet<const KDbTableSchema *> *checkedTables,
                                    QSet<const KDbQuerySchema *> *checkedQueries,
                                    KDbConnection *conn, const KDbQuerySchema *query1,
                                    const KDbQuerySchema *query2)
    {
        if (checkedQueries->contains(query1)) {
            localDebug() << "Query" << query1->name() << "already checked";
            return false; // protection against infinite recursion
        }
        checkedQueries->insert(query1);
        localDebug() << "Checking if query" << query1->name() << "depends on query" << query2->name();
        if (query1 == query2) {
            localDebug() << "Yes";
            return true;
        }
        for (const KDbTableSchema *queryTable : *query1->tables()) {
            if (tableDependsOnQuery(checkedTables, checkedQueries, conn, queryTable, query2)) {
                return true;
            }
        }
        return false;
    }

    //! Inserts to @a *result all listeners that listen to changes in table @a table and other tables
    //! or queries depending on @a table.
    static void collectListeners(QSet<KDbTableSchemaChangeListener *> *result,
                                 KDbConnection *conn,
                                 const KDbTableSchema *table)
    {
        Q_ASSERT(result);
        Q_ASSERT(conn);
        Q_ASSERT(table);
        // for all tables with listeners:
        for (QHash<const KDbTableSchema*, QSet<KDbTableSchemaChangeListener*>* >::ConstIterator it(
                 conn->d->tableSchemaChangeListeners.constBegin());
             it != conn->d->tableSchemaChangeListeners.constEnd(); ++it)
        {
            // check if it depends on our table
            QSet<const KDbTableSchema *> checkedTables;
            QSet<const KDbQuerySchema *> checkedQueries;
            if (tableDependsOnTable(&checkedTables, &checkedQueries, conn, it.key(), table)) {
                QSet<KDbTableSchemaChangeListener*>* set = it.value();
                result->unite(*set);
            }
        }
        // for all queries with listeners:
        for (QHash<const KDbQuerySchema*, QSet<KDbTableSchemaChangeListener*>* >::ConstIterator it(
                 conn->d->queryTableSchemaChangeListeners.constBegin());
             it != conn->d->queryTableSchemaChangeListeners.constEnd(); ++it)
        {
            // check if it depends on our table
            QSet<const KDbTableSchema *> checkedTables;
            QSet<const KDbQuerySchema *> checkedQueries;
            if (queryDependsOnTable(&checkedTables, &checkedQueries, conn, it.key(), table)) {
                QSet<KDbTableSchemaChangeListener*>* set = it.value();
                result->unite(*set);
            }
        }
    }

    //! Inserts to @a *result all listeners that listen to changes in query @a table and other tables
    //! or queries depending on @a query.
    static void collectListeners(QSet<KDbTableSchemaChangeListener *> *result,
                                 KDbConnection *conn,
                                 const KDbQuerySchema *query)
    {
        Q_ASSERT(result);
        Q_ASSERT(conn);
        Q_ASSERT(query);
        QSet<KDbTableSchemaChangeListener*>* set = conn->d->queryTableSchemaChangeListeners.value(query);
        if (set) {
            result->unite(*set);
        }
        // for all tables with listeners:
        for (QHash<const KDbTableSchema*, QSet<KDbTableSchemaChangeListener*>* >::ConstIterator it(
                 conn->d->tableSchemaChangeListeners.constBegin());
             it != conn->d->tableSchemaChangeListeners.constEnd(); ++it)
        {
            // check if it depends on our query
            QSet<const KDbTableSchema *> checkedTables;
            QSet<const KDbQuerySchema *> checkedQueries;
            if (tableDependsOnQuery(&checkedTables, &checkedQueries, conn, it.key(), query)) {
                QSet<KDbTableSchemaChangeListener*>* set = it.value();
                result->unite(*set);
            }
        }
        // for all queries with listeners:
        for (QHash<const KDbQuerySchema*, QSet<KDbTableSchemaChangeListener*>* >::ConstIterator it(
                 conn->d->queryTableSchemaChangeListeners.constBegin());
             it != conn->d->queryTableSchemaChangeListeners.constEnd(); ++it)
        {
            // check if it depends on our query
            QSet<const KDbTableSchema *> checkedTables;
            QSet<const KDbQuerySchema *> checkedQueries;
            if (queryDependsOnQuery(&checkedTables, &checkedQueries, conn, it.key(), query)) {
                QSet<KDbTableSchemaChangeListener*>* set = it.value();
                result->unite(*set);
            }
        }
    }

    QString name;
    Q_DISABLE_COPY(KDbTableSchemaChangeListenerPrivate)
};

KDbTableSchemaChangeListener::KDbTableSchemaChangeListener()
 : d(new KDbTableSchemaChangeListenerPrivate)
{
}

KDbTableSchemaChangeListener::~KDbTableSchemaChangeListener()
{
    delete d;
}

QString KDbTableSchemaChangeListener::name() const
{
    return d->name;
}

void KDbTableSchemaChangeListener::setName(const QString &name)
{
    d->name = name;
}

// static
void KDbTableSchemaChangeListener::registerForChanges(KDbConnection *conn,
                                                      KDbTableSchemaChangeListener* listener,
                                                      const KDbTableSchema* table)
{
    if (!conn) {
        kdbWarning() << "Missing connection";
        return;
    }
    if (!listener) {
        kdbWarning() << "Missing listener";
        return;
    }
    if (!table) {
        kdbWarning() << "Missing table";
        return;
    }
    KDbTableSchemaChangeListenerPrivate::registerForChanges(conn, listener, table);
}

// static
void KDbTableSchemaChangeListener::registerForChanges(KDbConnection *conn,
                                                      KDbTableSchemaChangeListener *listener,
                                                      const KDbQuerySchema *query)
{
    if (!conn) {
        kdbWarning() << "Missing connection";
        return;
    }
    if (!listener) {
        kdbWarning() << "Missing listener";
        return;
    }
    if (!query) {
        kdbWarning() << "Missing query";
        return;
    }
    KDbTableSchemaChangeListenerPrivate::registerForChanges(conn, listener, query);
}

// static
void KDbTableSchemaChangeListener::unregisterForChanges(KDbConnection *conn,
                                         KDbTableSchemaChangeListener* listener,
                                         const KDbTableSchema* table)
{
    if (!conn) {
        kdbWarning() << "Missing connection";
        return;
    }
    if (!listener) {
        kdbWarning() << "Missing listener";
        return;
    }
    if (!table) {
        kdbWarning() << "Missing table";
        return;
    }
    KDbTableSchemaChangeListenerPrivate::unregisterForChanges(conn, listener, table);
}

// static
void KDbTableSchemaChangeListener::unregisterForChanges(KDbConnection *conn,
                                                        const KDbTableSchema* table)
{
    if (!conn) {
        kdbWarning() << "Missing connection";
        return;
    }
    if (!table) {
        kdbWarning() << "Missing table";
        return;
    }
    KDbTableSchemaChangeListenerPrivate::unregisterForChanges(conn, nullptr, table);
}

void KDbTableSchemaChangeListener::unregisterForChanges(KDbConnection *conn,
                                                        KDbTableSchemaChangeListener *listener,
                                                        const KDbQuerySchema *query)
{
    if (!conn) {
        kdbWarning() << "Missing connection";
        return;
    }
    if (!listener) {
        kdbWarning() << "Missing listener";
        return;
    }
    if (!query) {
        kdbWarning() << "Missing query";
        return;
    }
    KDbTableSchemaChangeListenerPrivate::unregisterForChanges(conn, listener, query);
}

// static
void KDbTableSchemaChangeListener::unregisterForChanges(KDbConnection *conn,
                                                        const KDbQuerySchema *query)
{
    if (!conn) {
        kdbWarning() << "Missing connection";
        return;
    }
    if (!query) {
        kdbWarning() << "Missing query";
        return;
    }
    KDbTableSchemaChangeListenerPrivate::unregisterForChanges(conn, nullptr, query);
}

// static
void KDbTableSchemaChangeListener::unregisterForChanges(
        KDbConnection *conn, KDbTableSchemaChangeListener* listener)
{
    if (!conn) {
        kdbWarning() << "Missing connection";
        return;
    }
    if (!listener) {
        kdbWarning() << "Missing listener";
        return;
    }
    KDbTableSchemaChangeListenerPrivate::unregisterForChanges(conn, listener);
}

// static
QList<KDbTableSchemaChangeListener*> KDbTableSchemaChangeListener::listeners(
        KDbConnection *conn, const KDbTableSchema* table)
{
    if (!conn) {
        kdbWarning() << "Missing connection";
        return QList<KDbTableSchemaChangeListener*>();
    }
    if (!table) {
        kdbWarning() << "Missing table";
        return QList<KDbTableSchemaChangeListener*>();
    }
    QSet<KDbTableSchemaChangeListener *> result;
    KDbTableSchemaChangeListenerPrivate::collectListeners(&result, conn, table);
    return result.toList();
}

// static
QList<KDbTableSchemaChangeListener*> KDbTableSchemaChangeListener::listeners(
        KDbConnection *conn, const KDbQuerySchema *query)
{
    if (!conn) {
        kdbWarning() << "Missing connection";
        return QList<KDbTableSchemaChangeListener*>();
    }
    if (!query) {
        kdbWarning() << "Missing query";
        return QList<KDbTableSchemaChangeListener*>();
    }
    QSet<KDbTableSchemaChangeListener *> result;
    KDbTableSchemaChangeListenerPrivate::collectListeners(&result, conn, query);
    return result.toList();
}

// static
tristate KDbTableSchemaChangeListener::closeListeners(KDbConnection *conn,
    const KDbTableSchema *table, const QList<KDbTableSchemaChangeListener *> &except)
{
    if (!conn) {
        kdbWarning() << "Missing connection";
        return false;
    }
    if (!table) {
        kdbWarning() << "Missing table";
        return false;
    }
    QSet<KDbTableSchemaChangeListener*> toClose(listeners(conn, table).toSet().subtract(except.toSet()));
    tristate result = true;
    for (KDbTableSchemaChangeListener *listener : toClose) {
        const tristate localResult = listener->closeListener();
        if (localResult != true) {
            result = localResult;
        }
    }
    return result;
}

// static
tristate KDbTableSchemaChangeListener::closeListeners(KDbConnection *conn,
    const KDbQuerySchema *query, const QList<KDbTableSchemaChangeListener *> &except)
{
    if (!conn) {
        kdbWarning() << "Missing connection";
        return false;
    }
    if (!query) {
        kdbWarning() << "Missing query";
        return false;
    }
    QSet<KDbTableSchemaChangeListener*> toClose(listeners(conn, query).toSet().subtract(except.toSet()));
    tristate result = true;
    for (KDbTableSchemaChangeListener *listener : toClose) {
        const tristate localResult = listener->closeListener();
        if (localResult != true) {
            result = localResult;
        }
    }
    return result;
}
