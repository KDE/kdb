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

#ifndef PREDICATE_DRIVER_POSTGRESQL_H
#define PREDICATE_DRIVER_POSTGRESQL_H

#include <Predicate/Driver.h>

namespace Predicate
{

class Connection;

//! PostgreSQL database driver.
class PostgresqlDriver : public Driver
{
    Q_OBJECT
    PREDICATE_DRIVER

public:
    PostgresqlDriver();

    virtual ~PostgresqlDriver();

    virtual bool isSystemObjectName(const QString& n) const;

    virtual bool isSystemDatabaseName(const QString& n) const;

    //! Escape a string for use as a value
    virtual EscapedString escapeString(const QString& str) const;
    virtual EscapedString escapeString(const QByteArray& str) const;
    virtual QString sqlTypeName(int id_t, int p = 0) const;

    //! Escape BLOB value @a array
    virtual EscapedString escapeBLOB(const QByteArray& array) const;

    //! @return QVariant type for PostgreSQL type @a pgsqlType.
    //! If type cannot be found QVariant::Invalid is returned. Used in cursors to speed up conversion.
    inline QVariant::Type pgsqlToVariantType(int pgsqlType) const {
        return m_pgsqlToVariantTypes.value(pgsqlType, QVariant::Invalid);
    }

protected:
    virtual QByteArray drv_escapeIdentifier(const QString& str) const;
    virtual QByteArray drv_escapeIdentifier(const QByteArray& str) const;
    virtual Connection *drv_createConnection(const ConnectionData& connData);
    virtual bool drv_isSystemFieldName(const QString& n)const;

private:
    void initPgsqlToVariantMap();

    static const char *m_keywords[];
    QMap<int, QVariant::Type> m_pgsqlToVariantTypes;
};

}

#endif
