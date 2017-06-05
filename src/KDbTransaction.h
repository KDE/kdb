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

/*! Internal prototype for storing KDbTransaction handles for KDbTransaction object.
 Only for driver developers: reimplement this class for driver that
 support KDbTransaction handles.
*/
class KDB_EXPORT KDbTransactionData
{
public:
    explicit KDbTransactionData(KDbConnection *conn);
    ~KDbTransactionData();

#ifdef KDB_TRANSACTIONS_DEBUG
    //! Helper for debugging, returns value of global transaction data reference counter
    static int globalCount();
#endif
    KDbConnection *m_conn;
    bool m_active;
    int refcount;
private:
    Q_DISABLE_COPY(KDbTransactionData)
};

//! This class encapsulates KDbTransaction handle.
/*! KDbTransaction handle is sql driver-dependent,
  but outside KDbTransaction is visible as universal container
  for any handler implementation.

  KDbTransaction object is value-based, internal data (handle) structure,
  reference-counted.
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

    virtual ~KDbTransaction();

    KDbTransaction& operator=(const KDbTransaction& trans);

    //! @return @c true if this transaction is equal to @a other; otherwise returns @c false.
    bool operator==(const KDbTransaction& other) const;

    //! @return @c true if this transaction is not equal to @a other; otherwise returns @c false.
    //! @since 3.1
    inline bool operator!=(const KDbTransaction &other) const { return !operator==(other); }

    KDbConnection* connection() const;

    /**
     * @brief Returns @c true if transaction is active (ie. started)
     *
     * @return @c false also if transaction is uninitialised (null).
     * @since 3.1
     */
    bool isActive() const;

    /*! @return true if this transaction is null. */
    bool isNull() const;

#ifdef KDB_TRANSACTIONS_DEBUG
    //! Helper for debugging, returns value of global transaction data reference counter
    static int globalCount();
#endif

protected:
    KDbTransactionData *m_data;

    friend class KDbConnection;
};

//! Helper class for using inside methods for given connection.
/*! It can be used in two ways:
  - start new transaction in constructor and rollback on destruction (1st constructor),
  - use already started transaction and rollback on destruction (2nd constructor).
  In any case, if transaction is committed or rolled back outside this KDbTransactionGuard
  object in the meantime, nothing happens on KDbTransactionGuard destruction.
  <code>
  Example usage:
  void myclas::my_method()
  {
    KDbTransaction *transaction = connection->beginTransaction();
    KDbTransactionGuard tg(transaction);
    ...some code that operates inside started transaction...
    if (something)
      return //after return from this code block: tg will call
               //connection->rollbackTransaction() automatically
    if (something_else)
      transaction->commit();
    //for now tg won't do anything because transaction does not exist
  }
  </code>
*/
class KDB_EXPORT KDbTransactionGuard
{
public:
    /*! Constructor #1: Starts new transaction constructor for @a connection.
     Started KDbTransaction handle is available via transaction().*/
    explicit KDbTransactionGuard(KDbConnection *conn);

    /*! Constructor #2: Uses already started transaction. */
    explicit KDbTransactionGuard(const KDbTransaction& trans);

    /*! Constructor #3: Creates KDbTransactionGuard without transaction assigned.
     setTransaction() can be used later to do so. */
    KDbTransactionGuard();

    /*! Rollbacks not committed transaction. */
    ~KDbTransactionGuard();

    /*! Assigns transaction @a trans to this guard.
     Previously assigned transaction will be unassigned from this guard. */
    inline void setTransaction(const KDbTransaction& trans) {
        m_trans = trans;
    }

    /*! Comits the guarded transaction.
     It is convenient shortcut to connection->commitTransaction(this->transaction()) */
    bool commit();

    /*! Makes guarded transaction not guarded, so nothing will be performed on guard's desctruction. */
    void doNothing();

    /*! KDbTransaction that are controlled by this guard. */
    inline const KDbTransaction transaction() const {
        return m_trans;
    }

protected:
    KDbTransaction m_trans;
    bool m_doNothing;

private:
    Q_DISABLE_COPY(KDbTransactionGuard)
};

#endif
