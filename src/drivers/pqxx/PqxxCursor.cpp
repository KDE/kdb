/* This file is part of the KDE project
   Copyright (C) 2003 Adam Pigg <adam@piggz.co.uk>

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

#include "PqxxCursor.h"
#include "PqxxConnection.h"
#include "PqxxConnection_p.h"

#include <Predicate/Error.h>
#include <Predicate/Global.h>


#include <QtDebug>

using namespace Predicate;

unsigned int pqxxSqlCursor_trans_num = 0; //!< debug helper

static QByteArray pgsqlByteaToByteArray(const pqxx::result::field& r)
{
    return Predicate::pgsqlByteaToByteArray(r.c_str(), r.size());
}

//==================================================================================
//Constructor based on query statement
pqxxSqlCursor::pqxxSqlCursor(Predicate::Connection* conn, const EscapedString& statement, uint options):
        Cursor(conn, statement, options)
{
// PreDrvDbg << "PQXXSQLCURSOR: constructor for query statement";
    my_conn = static_cast<pqxxSqlConnection*>(conn)->d->pqxxsql;
    m_options = Buffered;
    m_res = 0;
// m_tran = 0;
    m_implicityStarted = false;
}

//==================================================================================
//Constructor base on query object
pqxxSqlCursor::pqxxSqlCursor(Connection* conn, QuerySchema* query, uint options)
        : Cursor(conn, query, options)
{
// PreDrvDbg << "PQXXSQLCURSOR: constructor for query schema";
    my_conn = static_cast<pqxxSqlConnection*>(conn)->d->pqxxsql;
    m_options = Buffered;
    m_res = 0;
// m_tran = 0;
    m_implicityStarted = false;
}

//==================================================================================
//Destructor
pqxxSqlCursor::~pqxxSqlCursor()
{
    close();
}

//==================================================================================
//Create a cursor result set
bool pqxxSqlCursor::drv_open(const EscapedString& sql)
{
// PreDrvDbg << sql;

    if (!my_conn->is_open()) {
//! @todo this check should be moved to Connection! when drv_prepareQuery() arrive
        //should never happen, but who knows
        setError(ERR_NO_CONNECTION, tr("No connection for cursor open operation specified"));
        return false;
    }

    //QByteArray cur_name;
    //Set up a transaction
    try {
        //m_tran = new pqxx::work(*my_conn, "cursor_open");
        //cur_name.sprintf("cursor_transaction%d", pqxxSqlCursor_trans_num++);

//  m_tran = new pqxx::nontransaction(*my_conn, (const char*)cur_name);
        if (!((pqxxSqlConnection*)connection())->m_trans) {
//   my_conn->drv_beginTransaction();
//  if (implicityStarted)
            (void)new pqxxTransactionData((pqxxSqlConnection*)connection(), true);
            m_implicityStarted = true;
        }

        m_res = new pqxx::result(((pqxxSqlConnection*)connection())->m_trans->data->exec(std::string(sql.toByteArray())));
        ((pqxxSqlConnection*)connection())
        ->drv_commitTransaction(((pqxxSqlConnection*)connection())->m_trans);
//  my_conn->m_trans->commit();
//  PreDrvDbg << "trans. committed:" << cur_name;

        //We should now be placed before the first row, if any
        m_fieldsToStoreInRecord = m_res->columns();
        m_fieldCount = m_fieldsToStoreInRecord - (m_containsROWIDInfo ? 1 : 0);

//js  m_opened=true;
        m_afterLast = false;
        m_records_in_buf = m_res->size();
        m_buffering_completed = true;
        return true;
    } catch (const std::exception &e) {
        setError(ERR_DB_SPECIFIC, QString::fromUtf8(e.what()));
        PreDrvWarn << "pqxxSqlCursor::drv_open:exception - " << QString::fromUtf8(e.what());
    } catch (...) {
        setError();
    }
// delete m_tran;
// m_tran = 0;
    if (m_implicityStarted) {
        delete((pqxxSqlConnection*)connection())->m_trans;
        m_implicityStarted = false;
    }
// PreDrvDbg << "trans. rolled back! - " << cur_name;
    return false;
}

//==================================================================================
//Delete objects
bool pqxxSqlCursor::drv_close()
{
//js m_opened=false;

    delete m_res;
    m_res = 0;

// if (m_implicityStarted) {
//  delete m_tran;
//  m_tran = 0;
//  m_implicityStarted = false;
// }

    return true;
}

//==================================================================================
//Gets the next record...does not need to do much, just return fetchend if at end of result set
void pqxxSqlCursor::drv_getNextRecord()
{
// PreDrvDbg << "size is" <<m_res->size() << "current Position is" << (long)at();
    if (at() < m_res->size() && at() >= 0) {
        m_fetchResult = FetchOK;
    } else if (at() >= m_res->size()) {
        m_fetchResult = FetchEnd;
    } else {
        // control will reach here only when at() < 0 ( which is usually -1 )
        // -1 is same as "1 beyond the End"
        m_fetchResult = FetchEnd;
    }
}

//==================================================================================
//Check the current position is within boundaries
void pqxxSqlCursor::drv_getPrevRecord()
{
// PreDrvDbg;

    if (at() < m_res->size() && at() >= 0) {
        m_fetchResult = FetchOK;
    } else if (at() >= m_res->size()) {
        m_fetchResult = FetchEnd;
    } else {
        m_fetchResult = FetchError;
    }
}

//==================================================================================
//Return the value for a given column for the current record
QVariant pqxxSqlCursor::value(uint pos)
{
    if (pos < m_fieldCount)
        return pValue(pos);
    else
        return QVariant();
}

//==================================================================================
//Return the value for a given column for the current record - Private const version
QVariant pqxxSqlCursor::pValue(uint pos)const
{
    if (m_res->size() <= 0) {
        PreDrvWarn << "pqxxSqlCursor::value - ERROR: result size not greater than 0";
        return QVariant();
    }

    if (pos >= m_fieldsToStoreInRecord) {
//  PreDrvWarn << "pqxxSqlCursor::value - ERROR: requested position is greater than the number of fields";
        return QVariant();
    }

    Predicate::Field *f = (m_fieldsExpanded && pos < qMin((uint)m_fieldsExpanded->count(), m_fieldCount))
                       ? m_fieldsExpanded->at(pos)->field : 0;

// PreDrvDbg << "pos:" << pos;

    //from most to least frequently used types:
    if (f) { //We probably have a schema type query so can use kexi to determin the row type
        if ((f->isIntegerType()) || (/*ROWID*/!f && m_containsROWIDInfo && pos == m_fieldCount)) {
            return (*m_res)[at()][pos].as(int());
        } else if (f->isTextType()) {
            return QString::fromUtf8((*m_res)[at()][pos].c_str()); //utf8?
        } else if (f->isFPNumericType()) {
            return (*m_res)[at()][pos].as(double());
        } else if (f->type() == Field::Boolean) {
            return QString((*m_res)[at()][pos].c_str()).toLower() == "t" ? QVariant(true) : QVariant(false);
        } else if (f->typeGroup() == Field::BLOBGroup) {
//   pqxx::result::field r = (*m_res)[at()][pos];
//   PreDrvDbg << r.name() << ", " << r.c_str() << ", " << r.type() << ", " << r.size();
            return ::pgsqlByteaToByteArray((*m_res)[at()][pos]);
        } else {
            return pgsqlCStrToVariant((*m_res)[at()][pos]);
        }
    } else { // We probably have a raw type query so use pqxx to determin the column type
        return pgsqlCStrToVariant((*m_res)[at()][pos]);
    }

    return QString::fromUtf8((*m_res)[at()][pos].c_str(), (*m_res)[at()][pos].size()); //utf8?
}

//==================================================================================
//Return the current record as a char**
//who'd have thought we'd be using char** in this day and age :o)
const char** pqxxSqlCursor::recordData() const
{
// PreDrvDbg;

    const char** row;

    row = (const char**)malloc(m_res->columns() + 1);
    row[m_res->columns()] = NULL;
    if (at() >= 0 && at() < m_res->size()) {
        for (int i = 0; i < (int)m_res->columns(); i++) {
            row[i] = (char*)malloc(strlen((*m_res)[at()][i].c_str()) + 1);
            strcpy((char*)(*m_res)[at()][i].c_str(), row[i]);
//   PreDrvDbg << row[i];
        }
    } else {
        PreDrvWarn << "pqxxSqlCursor::recordData: m_at is invalid";
    }
    return row;
}

//==================================================================================
//Store the current record in [data]
bool pqxxSqlCursor::drv_storeCurrentRecord(RecordData* data) const
{
// PreDrvDbg << "POSITION IS" << (long)m_at;

    if (m_res->size() <= 0)
        return false;

// const uint realCount = m_fieldCount + (m_containsROWIDInfo ? 1 : 0);
//not needed data.resize(realCount);

    for (uint i = 0; i < m_fieldsToStoreInRecord; i++)
        (*data)[i] = pValue(i);
    return true;
}

//==================================================================================
//
/*void pqxxSqlCursor::drv_clearServerResult()
{
//! @todo pqxxSqlCursor: stuff with server results
}*/

//==================================================================================
//Add the current record to the internal buffer
//Implementation required but no need in this driver
//Result set is a buffer so do not need another
void pqxxSqlCursor::drv_appendCurrentRecordToBuffer()
{

}

//==================================================================================
//Move internal pointer to internal buffer +1
//Implementation required but no need in this driver
void pqxxSqlCursor::drv_bufferMovePointerNext()
{

}

//==================================================================================
//Move internal pointer to internal buffer -1
//Implementation required but no need in this driver
void pqxxSqlCursor::drv_bufferMovePointerPrev()
{

}

//==================================================================================
//Move internal pointer to internal buffer to N
//Implementation required but no need in this driver
void pqxxSqlCursor::drv_bufferMovePointerTo(qint64 to)
{
    Q_UNUSED(to);
}
