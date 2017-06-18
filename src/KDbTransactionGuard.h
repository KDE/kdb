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

#ifndef KDB_TRANSACTIONGUARD_H
#define KDB_TRANSACTIONGUARD_H

#include "KDbTransaction.h"

/**
 * @brief KDbTransactionGuard class is a convenience class that simplifies handling transactions
 *
 * KDbTransactionGuard can be used in two ways:
 * - start a new transaction in constructor and call commit or allow to rollback on destruction,
 * - use already started transaction and call commit or allow to rollback on destruction.
 *
 * In either case if the guarded transaction is committed or rolled back outside the
 * KDbTransactionGuard object in the meantime, nothing happens on KDbTransactionGuard's destruction.
 *
 * Example usage:
 * <code>
 *  void MyClass::myMethod()
 *  {
 *    KDbTransaction transaction = connection->beginTransaction();
 *    KDbTransactionGuard tg(transaction);
 *    // <-- Some code that operates within the transaction here
 *    if (failure_condition)
 *      return; // After this return tg will be destroyed; connection->rollbackTransaction()
 *              // will be called automatically
 *    // success_condition:
 *    tg.commit();
 *    // Now tg won't do anything because transaction no longer exists
 *  }
 * </code>
 */
class KDB_EXPORT KDbTransactionGuard
{
public:
    /**
     * @brief Starts a new transaction for given connection and guards it
     *
     * When the new guard object is created a new transaction is started for connection @a
     * connection using KDbConnection::beginTransaction(). Started KDbTransaction handle is
     * available via transaction(). Unassigned guard is created if @a connection is @c nullptr or
     * if beginTransaction() fails.
     */
    explicit KDbTransactionGuard(KDbConnection *connection);

    /**
     * @brief Creates a new guard for already started transaction
     *
     * If transaction is not started i.e. @a transaction is null or not active, it will not be
     * guarded.
     *
     * setTransaction() can be used later to assign active transaction.
     */
    explicit KDbTransactionGuard(const KDbTransaction& transaction);

    /**
     * @brief Creates a new guard without assigning transaction
     *
     * setTransaction() can be used later to assign active transaction.
     */
    KDbTransactionGuard();

    /**
     * @brief Roll backs not committed transaction unless doNothing() was called before
     *
     * If doNothing() was called, transaction is left unaffected.
     */
    ~KDbTransactionGuard();

    /**
     * @brief Assigns transaction to this guard
     *
     * Previously assigned transaction will be unassigned from this guard without commiting or
     * rolling back.
     */
    void setTransaction(const KDbTransaction& transaction);

    /**
     * @brief Commits the guarded transaction
     *
     * This is an equivalent of transaction().connection()->commitTransaction(transaction(), options)
     * provided for convenience.
     *
     * @c false is also returned if transaction().connection() is @c nullptr.
     */
    bool commit(KDbTransaction::CommitOptions options = KDbTransaction::CommitOptions());

    /**
     * @brief Rolls back the guarded transaction
     *
     * This is an equivalent of transaction().connection()->rollbackTransaction(transaction(), options)
     * provided for convenience.
     *
     * @c false is also returned if transaction().connection() is @c nullptr.
     *
     * @since 3.1
     */
    bool rollback(KDbTransaction::CommitOptions options = KDbTransaction::CommitOptions());

    /**
     * @brief Deativates the transaction guard
     *
     * This means nothing will happen on guard's destruction.
     */
    void doNothing();

    /**
     * @brief Returns transaction that is controlled by this guard
     *
     * Null object is returned if no transaction is guarded.
     */
    const KDbTransaction transaction() const;

private:
    Q_DISABLE_COPY(KDbTransactionGuard)
    class Private;
    Private * const d;
};

#endif
