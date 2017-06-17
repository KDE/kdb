/* This file is part of the KDE project
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KDB_DRIVER_P_H
#define KDB_DRIVER_P_H

#include <QSet>

#include "KDbUtils.h"
#include "KDbDriverBehavior.h"

class KDbAdminTools;
class KDbConnection;
class KDbDriver;
class KDbDriverMetaData;

/*! Private driver's data members. */
class KDbDriverPrivate
{
public:
    explicit KDbDriverPrivate(KDbDriver *aDriver);
    virtual ~KDbDriverPrivate();

    //! Accessor to the KDbDriverBehavior object for driver @a driver.
    inline static const KDbDriverBehavior *behavior(const KDbDriver *driver) { return driver->behavior(); }

    //! @overload
    inline static KDbDriverBehavior *behavior(KDbDriver *driver) { return driver->behavior(); }

    KDbDriver *driver;

    KDbDriverBehavior driverBehavior;

    QSet<KDbConnection*> connections;

    /*! Driver's metadata. */
    const KDbDriverMetaData *metaData;

    /*! Provides a number of database administration tools for the driver. */
    KDbAdminTools *adminTools;

    /*! Driver-specific SQL keywords that need to be escaped if used as an
      identifier (e.g. for a table or column name) that aren't also KDbSQL
      keywords.  These don't necessarily need to be escaped when displayed by
      the front-end, because they won't confuse the parser.  However, they do
      need to be escaped before sending to the DB-backend which will have
      it's own parser.
    */
    KDbUtils::StaticSetOfStrings driverSpecificSqlKeywords;

    /*! KDbSQL keywords that need to be escaped if used as an identifier (e.g.
    for a table or column name).  These keywords will be escaped by the
    front-end, even if they are not recognised by the backend to provide
    UI consistency and to allow DB migration without changing the queries.
    */
    static const char* const kdbSQLKeywords[];

    friend class KDbDriver;
private:
    Q_DISABLE_COPY(KDbDriverPrivate)
};

#endif
