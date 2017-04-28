/* This file is part of the KDE project
   Copyright (C) 2003 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_POSTGRESQLCURSOR_H
#define KDB_POSTGRESQLCURSOR_H

#include "KDbCursor.h"
#include "KDbField.h"

#include <libpq-fe.h>

class KDbConnection;
class PostgresqlCursorData;

class PostgresqlCursor: public KDbCursor
{
public:
    explicit PostgresqlCursor(KDbConnection* conn, const KDbEscapedString& sql,
                              int options = NoOptions);
    PostgresqlCursor(KDbConnection* conn, KDbQuerySchema* query, int options = NoOptions);
    ~PostgresqlCursor() override;

    QVariant value(int pos) override;
    const char** recordData() const override;
    bool drv_storeCurrentRecord(KDbRecordData* data) const override;
    bool drv_open(const KDbEscapedString& sql) override;
    bool drv_close() override;
    void drv_getNextRecord() override;
    void drv_appendCurrentRecordToBuffer() override;
    void drv_bufferMovePointerNext() override;
    void drv_bufferMovePointerPrev() override;
    void drv_bufferMovePointerTo(qint64 to) override;

    void storeResultAndClear(PGresult **pgResult, ExecStatusType execStatus);

private:
    QVariant pValue(int pos)const;

    unsigned long m_numRows;
    QVector<KDbField::Type> m_realTypes;
    QVector<int> m_realLengths;

    PostgresqlCursorData * const d;
    Q_DISABLE_COPY(PostgresqlCursor)
};

#endif
