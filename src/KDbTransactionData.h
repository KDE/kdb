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

#ifndef KDB_TRANSACTIONDATA_H
#define KDB_TRANSACTIONDATA_H

#include "config-kdb.h"
#include "kdb_export.h"

#include <QtGlobal>

class KDbConnection;

/**
 * @brief Internal prototype for storing transaction handle for KDbTransaction object
 *
 * Only for driver developers. Teimplement this class for database driver that supports transactions.
 */
class KDB_EXPORT KDbTransactionData
{
public:
    explicit KDbTransactionData(KDbConnection *connection);

    ~KDbTransactionData();

    //! Increments the value of reference counter for this data.
    void ref();

    //! Decrements the value of reference counter for this data.
    void deref();

    //! @return value of reference counter for this data.
    int refcount() const;

    //! @return "active" flag of this data
    bool isActive() const;

    //! Sets "active" flag of this data
    void setActive(bool set);

    //! @return connection for this data
    KDbConnection *connection();

    //! @overload
    //! @since 3.1
    const KDbConnection *connection() const;

#ifdef KDB_TRANSACTIONS_DEBUG
    //! Helper for debugging, returns value of global transaction data reference counter
    static int globalCount();
#endif

private:
    Q_DISABLE_COPY(KDbTransactionData)
    class Private;
    Private * const d;
};

#endif
