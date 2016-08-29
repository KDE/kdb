/* This file is part of the KDE project
   Copyright (C) 2016 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_SQLRESULT_H
#define KDB_SQLRESULT_H

#include "kdb_export.h"
#include <QString>

class KDbField;
class KDbRecordData;
class KDbResult;
class KDbSqlField;
class KDbSqlRecord;

//! The KDbSqlResult class abstracts result of execution of raw SQL query executed using KDbConnection::executeSQL()
/**
 * KDbSqlResult allows to return low-level information about fields of the result
 * and fetch records.
 */
class KDB_EXPORT KDbSqlResult
{
public:
    KDbSqlResult();

    virtual ~KDbSqlResult();

    //! @return number of fields in this result
    virtual int fieldsCount() = 0;

    //! @return field @a index from this result
    virtual KDbSqlField *field(int index) Q_REQUIRED_RESULT = 0;

    //! Creates a KDb field for field @a index and returns it
    //! On failure returns @c nullptr.
    //! @a tableName is the table name and may be used to retrieve information but may
    //! be ignored as well if the KDbSqlResult already has field metadata available.
    virtual KDbField* createField(const QString &tableName, int index) Q_REQUIRED_RESULT = 0;

    //! Fetches one record and returns it. @return nullptr if there is no record to fetch or on error.
    //! Check lastResult() for errors.
    virtual KDbSqlRecord* fetchRecord() Q_REQUIRED_RESULT = 0;

    //! Convenience method. Fetches one record and all values into @a data.
    //! @return record data object and passes its ownership
    //! @c nullptr is returned on error or when there is no record to fetch.
    //! Check lastResult() for errors.
    KDbRecordData* fetchRecordData() Q_REQUIRED_RESULT;

    //! @return result of last operation on this SQL result
    virtual KDbResult lastResult() = 0;
};

#endif
