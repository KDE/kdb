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

#ifndef KDB_KDBTABLESCHEMACHANGELISTENER_H

#include <kdb_export.h>
#include <KDbTristate>

class KDbConnection;
class KDbQuerySchema;
class KDbTableSchema;
class KDbTableSchemaChangeListenerPrivate;

//! @short An interface allowing to listen for table schema changes
/**
 * The KDbTableSchemaChangeListener class can be used to listen for changes in table schema.
 * For example query designer window that uses given table schema can be informed about
 * planned changes and it can be decided about closing the window prior to changes in the schema.
 */
class KDB_EXPORT KDbTableSchemaChangeListener
{
public:
    KDbTableSchemaChangeListener();
    virtual ~KDbTableSchemaChangeListener();

    /**
     * Closes listening object so it will be deleted and thus no longer use a conflicting
     * table schema. For example if the listening object is a query designer in Kexi
     * application, the designer window will be closed.
     * This method can be used to avoid conflicts altering table schema or deleting it.
     */
    virtual tristate closeListener() = 0;

    /**
     * @return translated string that clearly identifies object that listens for changes
     * in a given table schema.
     *
     * For example it can be a query that uses the table, see KexiQueryPart in Kexi application
     * and the translated name can be "Query \"abc\"". This friendly identifier can be then
     * displayed by the application to inform users about objects depending on the table
     * so users can decide whether to approve schema changes or close the depending windows
     * to avoid conflicts.
     *
     * By default the name string is empty.
     */
    QString name() const;

    /**
     * @return translated string that clearly identifies object that listens for changes
     * in a given table schema.
     *
     * @see name()
     */
    void setName(const QString &name);

    /** Registers @a listener for receiving (listening) information about changes in table schema
     * @a table and all tables related to lookup fields. Changes can be related to altering and
     * removing.
     */
    static void registerForChanges(KDbConnection *conn,
                                   KDbTableSchemaChangeListener* listener,
                                   const KDbTableSchema* table);

    /**
     * Registers @a listener for receiving (listening) information about changes in query schema
     * @a query and all tables that the query uses.
     *
     * All tables related to lookup fields of these tables are also checked.
     * Changes can be related to table altering and removing.
     */
    static void registerForChanges(KDbConnection *conn,
                                   KDbTableSchemaChangeListener* listener,
                                   const KDbQuerySchema* query);

    /**
     * Unregisters @a listener for receiving (listening) information about changes
     * in table schema @a table.
     */
    static void unregisterForChanges(KDbConnection *conn,
                                     KDbTableSchemaChangeListener* listener,
                                     const KDbTableSchema* table);

    /**
     * Unregisters all listeners for receiving (listening) information about changes
     * in table schema @a table.
     */
    static void unregisterForChanges(KDbConnection *conn,
                                     const KDbTableSchema* table);

    /**
     * Unregisters @a listener for receiving (listening) information about changes
     * in any table or query schema.
     */
    static void unregisterForChanges(KDbConnection *conn,
                                     KDbTableSchemaChangeListener* listener);

    /**
     * Unregisters @a listener for receiving (listening) information about changes
     * in query schema @a query.
     */
    static void unregisterForChanges(KDbConnection *conn,
                                     KDbTableSchemaChangeListener* listener,
                                     const KDbQuerySchema* query);

    /**
     * Unregisters all listeners for receiving (listening) information about changes
     * in query schema @a query.
     */
    static void unregisterForChanges(KDbConnection *conn,
                                     const KDbQuerySchema* query);

    /**
     * @return list of all table schema listeners registered for receiving (listening)
     * information about changes in table schema @a table and other tables or queries depending
     * on @a table.
     */
    static QList<KDbTableSchemaChangeListener *> listeners(KDbConnection *conn,
                                                           const KDbTableSchema *table);

    /**
     * @return list of all table schema listeners registered for receiving (listening)
     * information about changes in query @a query and other tables or queries depending on @a query.
     */
    static QList<KDbTableSchemaChangeListener *> listeners(KDbConnection *conn,
                                                           const KDbQuerySchema *query);

    /**
     * Closes all table schema listeners for table schema @a table except for the ones from
     * the @a except list.
     *
     * See KDbTableSchemaChangeListener::closeListener() for explanation of the operation
     * of closing listener.
     *
     * @return true if all listenters for the table schema @a table have been successfully closed
     * (returned true) or @c false or @c cancelled if at least one listener returned
     * @c false or @c cancelled, respectively.
     * Regardless of returned value, closeListener() is called on all listeners for @a table.
     */
    static tristate closeListeners(KDbConnection *conn, const KDbTableSchema* table,
                                   const QList<KDbTableSchemaChangeListener*> &except
                                       = QList<KDbTableSchemaChangeListener*>());

    /**
     * Closes all table schema listeners for query schema @a query except for the ones from
     * the @a except list.
     *
     * See KDbTableSchemaChangeListener::closeListener() for explanation of the operation
     * of closing listener.
     *
     * @return true if all listenters for the table schema @a table have been successfully closed
     * (returned true) or @c false or @c cancelled if at least one listener returned
     * @c false or @c cancelled, respectively.
     * Regardless of returned value, closeListener() is called on all listeners for @a table.
     */
    static tristate closeListeners(KDbConnection *conn, const KDbQuerySchema* query,
                                   const QList<KDbTableSchemaChangeListener*> &except
                                       = QList<KDbTableSchemaChangeListener*>());

private:
    Q_DISABLE_COPY(KDbTableSchemaChangeListener)
    KDbTableSchemaChangeListenerPrivate * const d;
};

#endif
