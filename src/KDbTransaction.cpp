/* This file is part of the KDE project
   Copyright (C) 2003 Jarosław Staniek <staniek@kde.org>

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

//helper for debugging
KDB_EXPORT int KDbTransaction::globalcount = 0;
KDB_EXPORT int KDbTransaction::globalCount()
{
    return KDbTransaction::globalcount;
}
KDB_EXPORT int KDbTransactionData::globalcount = 0;
KDB_EXPORT int KDbTransactionData::globalCount()
{
    return KDbTransactionData::globalcount;
}

KDbTransactionData::KDbTransactionData(KDbConnection *conn)
        : m_conn(conn)
        , m_active(true)
        , refcount(1)
{
    Q_ASSERT(conn);
    KDbTransaction::globalcount++; //because refcount(1) init.
    KDbTransactionData::globalcount++;
    transactionsDebug() << "-- globalcount ==" << KDbTransactionData::globalcount;
}

KDbTransactionData::~KDbTransactionData()
{
    KDbTransactionData::globalcount--;
    transactionsDebug() << "-- globalcount ==" << KDbTransactionData::globalcount;
}

//---------------------------------------------------

KDbTransaction::KDbTransaction()
        : m_data(0)
{
}

KDbTransaction::KDbTransaction(const KDbTransaction& trans)
        : m_data(trans.m_data)
{
    if (m_data) {
        m_data->refcount++;
        KDbTransaction::globalcount++;
    }
}

KDbTransaction::~KDbTransaction()
{
    if (m_data) {
        m_data->refcount--;
        KDbTransaction::globalcount--;
        transactionsDebug() << "m_data->refcount==" << m_data->refcount;
        if (m_data->refcount == 0)
            delete m_data;
    } else {
        transactionsDebug() << "null";
    }
    transactionsDebug() << "-- globalcount == " << KDbTransaction::globalcount;
}

KDbTransaction& KDbTransaction::operator=(const KDbTransaction & trans)
{
    if (this != &trans) {
        if (m_data) {
            m_data->refcount--;
            KDbTransaction::globalcount--;
            transactionsDebug() << "m_data->refcount==" << m_data->refcount;
            if (m_data->refcount == 0)
                delete m_data;
        }
        m_data = trans.m_data;
        if (m_data) {
            m_data->refcount++;
            KDbTransaction::globalcount++;
        }
    }
    return *this;
}

bool KDbTransaction::operator==(const KDbTransaction& trans) const
{
    return m_data == trans.m_data;
}

KDbConnection* KDbTransaction::connection() const
{
    return m_data ? m_data->m_conn : 0;
}

bool KDbTransaction::active() const
{
    return m_data && m_data->m_active;
}

bool KDbTransaction::isNull() const
{
    return m_data == 0;
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
    if (!m_doNothing && m_trans.active() && m_trans.connection())
        m_trans.connection()->rollbackTransaction(m_trans);
}

bool KDbTransactionGuard::commit()
{
    if (m_trans.active() && m_trans.connection()) {
        return m_trans.connection()->commitTransaction(m_trans);
    }
    return false;
}

void KDbTransactionGuard::doNothing()
{
    m_doNothing = true;
}
