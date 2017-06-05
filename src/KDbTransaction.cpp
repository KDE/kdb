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
#include "kdb_debug.h"

#include <assert.h>

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

KDbTransactionData::KDbTransactionData(KDbConnection *conn)
        : m_conn(conn)
        , m_active(true)
        , refcount(1)
{
    Q_ASSERT(conn);
#ifdef KDB_TRANSACTIONS_DEBUG
    KDbTransaction_globalcount++; // because of refcount(1) init.
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
        m_data->refcount++;
#ifdef KDB_TRANSACTIONS_DEBUG
        KDbTransaction_globalcount++;
#endif
    }
}

KDbTransaction::~KDbTransaction()
{
    if (m_data) {
        m_data->refcount--;
#ifdef KDB_TRANSACTIONS_DEBUG
        KDbTransaction_globalcount--;
#endif
        transactionsDebug() << "m_data->refcount==" << m_data->refcount;
        if (m_data->refcount == 0)
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
            m_data->refcount--;
#ifdef KDB_TRANSACTIONS_DEBUG
            KDbTransaction_globalcount--;
#endif
            transactionsDebug() << "m_data->refcount==" << m_data->refcount;
            if (m_data->refcount == 0)
                delete m_data;
        }
        m_data = trans.m_data;
        if (m_data) {
            m_data->refcount++;
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

KDbConnection* KDbTransaction::connection() const
{
    return m_data ? m_data->m_conn : nullptr;
}

bool KDbTransaction::isActive() const
{
    return m_data && m_data->m_active;
}

bool KDbTransaction::isNull() const
{
    return m_data == nullptr;
}

//---------------------------------------------------

KDbTransactionGuard::KDbTransactionGuard(KDbConnection *conn)
        : m_doNothing(false)
{
    Q_ASSERT(conn);
    m_trans = conn->beginTransaction();
}

KDbTransactionGuard::KDbTransactionGuard(const KDbTransaction& trans)
        : m_trans(trans)
        , m_doNothing(false)
{
}

KDbTransactionGuard::KDbTransactionGuard()
        : m_doNothing(false)
{
}

KDbTransactionGuard::~KDbTransactionGuard()
{
    if (!m_doNothing && m_trans.isActive() && m_trans.connection())
        m_trans.connection()->rollbackTransaction(m_trans);
}

bool KDbTransactionGuard::commit()
{
    if (m_trans.isActive() && m_trans.connection()) {
        return m_trans.connection()->commitTransaction(m_trans);
    }
    return false;
}

void KDbTransactionGuard::doNothing()
{
    m_doNothing = true;
}
