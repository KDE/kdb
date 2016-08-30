/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 Joseph Wenninger<jowenn@kde.org>
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_MYSQLDRIVER_H
#define KDB_MYSQLDRIVER_H

#include "KDbDriver.h"

//! MySQL database driver.
class MysqlDriver : public KDbDriver
{
    Q_OBJECT

public:
    /*!
     * Constructor sets database features and
     * maps the types in KDbField::Type to the MySQL types.
     *
     * See: http://dev.mysql.com/doc/mysql/en/Column_types.html
     */
    MysqlDriver(QObject *parent, const QVariantList &args);

    virtual ~MysqlDriver();

    /*! @return false for this driver. */
    virtual bool isSystemObjectName(const QString& n) const;

    /*! @return true if @a is "mysql", "information_schema" or "performance_schema". */
    virtual bool isSystemDatabaseName(const QString &n) const;

    //! Escape a string for use as a value
    virtual KDbEscapedString escapeString(const QString& str) const;
    virtual KDbEscapedString escapeString(const QByteArray& str) const;

    //! Escape BLOB value @a array
    virtual KDbEscapedString escapeBLOB(const QByteArray& array) const;

    //! Overrides the default implementation
    virtual QString sqlTypeName(KDbField::Type type, const KDbField &field) const;

    //! Generates native (driver-specific) LENGTH() function call.
    //! char_length(val) is used because length(val) in mysql returns number of bytes,
    //! what is not right for multibyte (unicode) encodings. */
    virtual KDbEscapedString lengthFunctionToString(const KDbNArgExpression &args,
                                                    KDbQuerySchemaParameterValueListIterator* params,
                                                    KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) GREATEST() and LEAST() function call.
    //! Since MySQL's LEAST()/GREATEST() function ignores NULL values, it only returns NULL
    //! if all the expressions evaluate to NULL. So this is used for F(v0,..,vN):
    //! (CASE WHEN (v0) IS NULL OR .. OR (vN) IS NULL THEN NULL ELSE F(v0,..,vN) END)
    //! where F == GREATEST or LEAST.
    virtual KDbEscapedString greatestOrLeastFunctionToString(const QString &name,
                                                    const KDbNArgExpression &args,
                                                    KDbQuerySchemaParameterValueListIterator* params,
                                                    KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) UNICODE() function call.
    //! Uses ORD(CONVERT(X USING UTF16)).
    virtual KDbEscapedString unicodeFunctionToString(const KDbNArgExpression &args,
                                            KDbQuerySchemaParameterValueListIterator* params,
                                            KDb::ExpressionCallStack* callStack) const;

    //! Generates native (driver-specific) function call for concatenation of two strings.
    //! Uses CONCAT().
    KDbEscapedString concatenateFunctionToString(const KDbBinaryExpression &args,
                                                 KDbQuerySchemaParameterValueListIterator* params,
                                                 KDb::ExpressionCallStack* callStack) const;

protected:
    virtual QString drv_escapeIdentifier(const QString& str) const;
    virtual QByteArray drv_escapeIdentifier(const QByteArray& str) const;
    virtual KDbConnection *drv_createConnection(const KDbConnectionData& connData,
                                                const KDbConnectionOptions &options);
    virtual bool drv_isSystemFieldName(const QString& n) const;
    bool supportsDefaultValue(const KDbField &field) const Q_DECL_OVERRIDE;

private:
    static const char *keywords[];
    QString m_longTextPrimaryKeyType;
};

#endif
