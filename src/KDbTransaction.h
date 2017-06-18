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

#ifndef KDB_TRANSACTION_H
#define KDB_TRANSACTION_H

#include "config-kdb.h"
#include "kdb_export.h"

#include <QtGlobal>

class KDbConnection;
class KDbTransactionData;

/**
 * @brief This class encapsulates a single database transaction
 *
 * The KDbTransaction handle abstracts a database transaction for given database connection.
 * Transaction objects are value-based, implicitly shared.
 *
 * Lifetime of the transaction object is closely related to a KDbConnection object.
 * Deleting the either instance does not commit or rollback the actual transaction.
 * Use KDbTransactionGuard for automatic commits or rolls back.
 *
 * @see KDbConnection::beginTransaction()
 */
class KDB_EXPORT KDbTransaction
{
public:
    /**
     * @brief Constructs a null transaction.
     *
     * @note It can be initialized only by KDbConnection.
     */
    KDbTransaction();

    /**
     * @brief Copy constructor
     */
    KDbTransaction(const KDbTransaction& trans);

    ~KDbTransaction();

    //! Options for commiting and rolling back transactions
    //! @see KDbConnection::beginTransaction() KDbConnection::rollbackTransaction()
    //! @see KDbTransactionGuard::commit() KDbTransactionGuard::rollback()
    enum class CommitOption {
        None = 0,
        IgnoreInactive = 1 //!< Do not return error for inactive or null transactions when
                           //!< requesting commit or rollback
    };
    Q_DECLARE_FLAGS(CommitOptions, CommitOption)

    KDbTransaction& operator=(const KDbTransaction& trans);

    /**
     * @brief Returns @c true if this transaction is equal to @a other; otherwise returns @c false
     *
     * Two transactions are equal if they encapsulate the same physical transaction,
     * i.e. copy constructor or assignment operator was used.
     * Two null transaction objects are equal.
     */
    bool operator==(const KDbTransaction& other) const;

    //! @return @c true if this transaction is not equal to @a other; otherwise returns @c false.
    //! @since 3.1
    inline bool operator!=(const KDbTransaction &other) const { return !operator==(other); }

    /**
     * @brief Returns database connection for which the transaction belongs.
     *
     * @c nullptr is returned for null transactions.
     */
    KDbConnection* connection() const;

    /**
     * @brief Returns @c true if transaction is active (i.e. started)
     *
     * @return @c false also if transaction is uninitialised (null) or not started.
     * @see KDbConnection::beginTransaction()
     * @since 3.1
     */
    bool isActive() const;

    /**
     * @brief Returns @c true if this transaction is null.
     *
     * Null implies !isActive().
     */
    bool isNull() const;

#ifdef KDB_TRANSACTIONS_DEBUG
    //! Helper for debugging, returns value of global transaction data reference counter
    static int globalCount();
#endif

protected:
    KDbTransactionData *m_data;

    friend class KDbConnection;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDbTransaction::CommitOptions)

#endif
