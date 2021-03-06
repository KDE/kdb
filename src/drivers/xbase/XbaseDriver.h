/* This file is part of the KDE project
   Copyright (C) 2008 Sharan Rao <sharanrao@gmail.com>

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

#ifndef KDB_DRIVER_XBASEDRIVER_H
#define KDB_DRIVER_XBASEDRIVER_H

#include "KDbDriver.h"

class xBaseDriverPrivate;

class xBaseDriver : public KDbDriver
{
    Q_OBJECT

public:
    xBaseDriver(QObject *parent, const QVariantList &args);

    virtual ~xBaseDriver();

    /*! \return true if \a n is a system object name;
    */
    virtual bool isSystemObjectName( const QString& n ) const;

    /*! \return false for this driver. */
    virtual bool isSystemDatabaseName(const QString &n) const;

    //! Escape a string for use as a value
    virtual KDbEscapedString escapeString(const QString& str) const;
    virtual KDbEscapedString escapeString(const QByteArray& str) const;

    //! Escape BLOB value \a array
    virtual KDbEscapedString escapeBLOB(const QByteArray& array) const;

protected:
    virtual QByteArray drv_escapeIdentifier( const QString& str) const;
    virtual QByteArray drv_escapeIdentifier( const QByteArray& str) const;
    virtual KDbConnection *drv_createConnection(const KDbConnectionData& connData,
                                                const KDbConnectionOptions &options);
    virtual bool drv_isSystemFieldName( const QString& n ) const;

private:
    xBaseDriverPrivate* dp;
    static const char *keywords[];
};

#endif // KDB_DRIVER_XBASEDRIVER_H
