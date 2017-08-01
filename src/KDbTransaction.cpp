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

#include "KDbTransaction.h"
#include "KDbConnection.h"
#include "KDbTransactionData.h"
#include "KDbTransactionGuard.h"
#include "kdb_debug.h"

#ifdef KDB_TRANSACTIONS_DEBUG
//helper for debugging
int KDbTransaction_globalcount = 0;

KDB_EXPORT int KDbTransaction::globalCount()
{
    return KDbTransaction_globalcount;
}

int KDbTransactionData_globalcount = 0;

KDB_EXPORT int KDbTransactionData::globalCount()
{
    return KDbTransactionData_globalcount;
}
#endif

class Q_DECL_HIDDEN KDbTransactionData::Private
{
public:
    Private(KDbConnection *c) : connection(c)
    {
        Q_ASSERT(connection);
    }
    KDbConnection *connection;
    bool active = true;
    int refcount = 1;
};

KDbTransactionData::KDbTransactionData(KDbConnection *connection)
    : d(new Private(connection))
{
#ifdef KDB_TRANSACTIONS_DEBUG
    KDbTransaction_globalcount++; // because of refcount is initialized to 1
    KDbTransactionData_globalcount++;
    transactionsDebug() << "-- globalcount ==" << KDbTransactionData_globalcount;
#endif
}

KDbTransactionData::~KDbTransactionData()
{
#ifdef KDB_TRANSACTIONS_DEBUG
    KDbTransactionData_globalcount--;
    transactionsDebug() << "-- globalcount ==" << KDbTransactionData_globalcount;
#endif
    delete d;
}

void KDbTransactionData::ref()
{
    d->refcount++;
}

void KDbTransactionData::deref()
{
    d->refcount--;
}

int KDbTransactionData::refcount() const
{
    return d->refcount;
}

bool KDbTransactionData::isActive() const
{
    return d->active;
}

void KDbTransactionData::setActive(bool set)
{
    d->active = set;
}

KDbConnection *KDbTransactionData::connection()
{
    return d->connection;
}

const KDbConnection *KDbTransactionData::connection() const
{
    return d->connection;
}

//---------------------------------------------------

KDbTransaction::KDbTransaction()
        : m_data(nullptr)
{
}

KDbTransaction::KDbTransaction(const KDbTransaction& trans)
        : m_data(trans.m_data)
{
    if (m_data) {
        m_data->ref();
#ifdef KDB_TRANSACTIONS_DEBUG
        KDbTransaction_globalcount++;
#endif
    }
}

KDbTransaction::~KDbTransaction()
{
    if (m_data) {
        m_data->deref();
#ifdef KDB_TRANSACTIONS_DEBUG
        KDbTransaction_globalcount--;
#endif
        transactionsDebug() << "m_data->refcount==" << m_data->refcount();
        if (m_data->refcount() == 0)
            delete m_data;
    } else {
        transactionsDebug() << "null";
    }
#ifdef KDB_TRANSACTIONS_DEBUG
    transactionsDebug() << "-- globalcount == " << KDbTransaction_globalcount;
#endif
}

KDbTransaction& KDbTransaction::operator=(const KDbTransaction & trans)
{
    if (this != &trans) {
        if (m_data) {
            m_data->deref();
#ifdef KDB_TRANSACTIONS_DEBUG
            KDbTransaction_globalcount--;
#endif
            transactionsDebug() << "m_data->refcount==" << m_data->refcount();
            if (m_data->refcount() == 0)
                delete m_data;
        }
        m_data = trans.m_data;
        if (m_data) {
            m_data->ref();
#ifdef KDB_TRANSACTIONS_DEBUG
            KDbTransaction_globalcount++;
#endif
        }
    }
    return *this;
}

bool KDbTransaction::operator==(const KDbTransaction& other) const
{
    return m_data == other.m_data;
}

KDbConnection* KDbTransaction::connection()
{
    return m_data ? m_data->connection() : nullptr;
}

const KDbConnection* KDbTransaction::connection() const
{
    return const_cast<KDbTransaction*>(this)->connection();
}

bool KDbTransaction::isActive() const
{
    return m_data && m_data->isActive();
}

bool KDbTransaction::isNull() const
{
    return m_data == nullptr;
}

//---------------------------------------------------

class Q_DECL_HIDDEN KDbTransactionGuard::Private
{
public:
    Private() {}
    KDbTransaction transaction;
    bool doNothing = false;
};

KDbTransactionGuard::KDbTransactionGuard(KDbConnection *connection)
    : KDbTransactionGuard()
{
    if (connection) {
        d->transaction = connection->beginTransaction();
    }
}

KDbTransactionGuard::KDbTransactionGuard(const KDbTransaction &transaction)
    : KDbTransactionGuard()
{
    d->transaction = transaction;
}

KDbTransactionGuard::KDbTransactionGuard()
    : d(new Private)
{
}

KDbTransactionGuard::~KDbTransactionGuard()
{
    if (!d->doNothing && d->transaction.isActive()) {
        const bool result = rollback();
#ifdef KDB_TRANSACTIONS_DEBUG
        transactionsDebug() << "~KDbTransactionGuard is rolling back transaction:" << result;
#else
        Q_UNUSED(result)
#endif
    }
    delete d;
}

void KDbTransactionGuard::setTransaction(const KDbTransaction& transaction)
{
    d->transaction = transaction;
}

bool KDbTransactionGuard::commit(KDbTransaction::CommitOptions options)
{
    if (d->transaction.connection()) {
        return d->transaction.connection()->commitTransaction(d->transaction, options);
    }
    return false;
}

bool KDbTransactionGuard::rollback(KDbTransaction::CommitOptions options)
{
    if (d->transaction.connection()) {
        return d->transaction.connection()->rollbackTransaction(d->transaction, options);
    }
    return false;
}

void KDbTransactionGuard::doNothing()
{
    d->doNothing = true;
}

const KDbTransaction KDbTransactionGuard::transaction() const
{
    return d->transaction;
}
