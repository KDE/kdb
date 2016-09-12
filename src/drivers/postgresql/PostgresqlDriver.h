/* This file is part of the KDE project
   Copyright (C) 2003 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2010-2016 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_POSTGRESQLDRIVER_H
#define KDB_POSTGRESQLDRIVER_H

#include "KDbDriver.h"

class KDbConnection;

//! PostgreSQL database driver.
class PostgresqlDriver : public KDbDriver
{
    Q_OBJECT

public:
    PostgresqlDriver(QObject *parent, const QVariantList &args);

    virtual ~PostgresqlDriver();

    //! @todo implement
    virtual bool isSystemObjectName(const QString& name) const;

    virtual bool isSystemDatabaseName(const QString& name) const;

    //! Escape a string for use as a value
    virtual KDbEscapedString escapeString(const QString& str) const;
    virtual KDbEscapedString escapeString(const QByteArray& str) const;
    //! Overrides the default implementation to allow for NUMERIC type natively
    virtual QString sqlTypeName(KDbField::Type type, const KDbField &field) const;

    //! Escape BLOB value @a array
    virtual KDbEscapedString escapeBLOB(const QByteArray& array) const;

    //! Converts information converted from PQfmod() to length. -1 if missing.
    inline static int pqfmodToLength(int pqfmod) {
        int len;
        if (pqfmod > 0) {
            const int PGSQL_VAR_HDRSZ = 4;
            len = pqfmod - PGSQL_VAR_HDRSZ; //!< See e.g. postgis_get_char_length()
        } else {
            len = -1;
        }
        return len;
    }

    //! Uses information obtained from PQfmod() and adjust type @a t if possible.
    //! Also sets @a maxTextLength.
    //! @todo using pqfmod not tested
    //! @todo more types such as decimal
    inline static KDbField::Type typeForSize(KDbField::Type t, int pqfmod, int *maxTextLength) {
        KDbField::Type newType = t;
        if (maxTextLength) {
            *maxTextLength = -1;
        }
        if (t == KDbField::Integer) {
            if (pqfmod == 1) {
                newType = KDbField::Byte;
            } else if (pqfmod == 2) {
                newType = KDbField::ShortInteger;
            } else if (pqfmod == 8) {
                newType = KDbField::BigInteger;
            }
        } else if (t == KDbField::LongText) {
            const int len = pqfmodToLength(pqfmod);
            if (len > 0 && len <= 255) {
                newType = KDbField::Text;
                if (maxTextLength) {
                    *maxTextLength = len;
                }
            }
        }
        return newType;
    }

    //! @return KDb field type for PostgreSQL type @a pqtype and modifier @a pqfmod.
    //! If type cannot be found KDbField::InvalidType is returned.
    //! Used in cursors to speed up data conversion.
    inline KDbField::Type pgsqlToKDbType(int pqtype, int pqfmod, int *maxTextLength) const {
        KDbField::Type t = m_pgsqlToKDbTypes.value(pqtype, KDbField::InvalidType);
        return typeForSize(t, pqfmod, maxTextLength);
    }

    //! Generates native (driver-specific) HEX() function call.
    //! Uses UPPER(ENCODE(val, 'hex')).
    //! See http://www.postgresql.org/docs/9.3/static/functions-string.html#FUNCTIONS-STRING-OTHER */
    virtual KDbEscapedString hexFunctionToString(const KDbNArgExpression &args,
                                                 KDbQuerySchemaParameterValueListIterator* params,
                                                 KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) IFNULL() function call.
    //! Uses COALESCE().
    virtual KDbEscapedString ifnullFunctionToString(const KDbNArgExpression &args,
                                                    KDbQuerySchemaParameterValueListIterator* params,
                                                    KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) LENGTH() function call.
    //! For text types default LENGTH(val) is used, for BLOBs OCTET_LENGTH(val) is used because
    //! LENGTH(val) for BLOB returns number of bits.
    virtual KDbEscapedString lengthFunctionToString(const KDbNArgExpression &args,
                                                    KDbQuerySchemaParameterValueListIterator* params,
                                                    KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) GREATEST() and LEAST() function calls.
    //! Since PostgreSQL's LEAST()/GREATEST() function ignores NULL values, it only returns NULL
    //! if all the expressions evaluate to NULL. So this is used for F(v0,..,vN):
    //! (CASE WHEN (v0) IS NULL OR .. OR (vN) IS NULL THEN NULL ELSE F(v0,..,vN) END)
    //! where F == GREATEST or LEAST.
    virtual KDbEscapedString greatestOrLeastFunctionToString(const QString &name,
                                                             const KDbNArgExpression &args,
                                                             KDbQuerySchemaParameterValueListIterator* params,
                                                             KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) UNICODE() function call.
    //! Uses ASCII(X).
    virtual KDbEscapedString unicodeFunctionToString(const KDbNArgExpression &args,
                                                     KDbQuerySchemaParameterValueListIterator* params,
                                                     KDb::ExpressionCallStack* callStack) const;

protected:
    virtual QString drv_escapeIdentifier(const QString& str) const;
    virtual QByteArray drv_escapeIdentifier(const QByteArray& str) const;
    virtual KDbConnection *drv_createConnection(const KDbConnectionData& connData,
                                                const KDbConnectionOptions &options);
    virtual bool drv_isSystemFieldName(const QString& name)const;

private:
    void initPgsqlToKDbMap();

    static const char *m_keywords[];
    QMap<int, KDbField::Type> m_pgsqlToKDbTypes;
    Q_DISABLE_COPY(PostgresqlDriver)
};

#endif // KDB_DRIVER_POSTGRESQLDRIVER_H
