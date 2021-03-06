/* This file is part of the KDE project
    Copyright (C) 2003-2017 Jarosław Staniek <staniek@kde.org>
    Copyright (C) 2004 Martin Ellis <martin.ellis@kdemail.net>

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

#include "KDbDriver_p.h"
#include "KDbAdmin.h"
#include "KDbDriverBehavior.h"
#include "KDbDriverMetaData.h"
#include "KDbVersionInfo.h"

class Q_DECL_HIDDEN KDbDriverBehavior::Private
{
public:
    Private() {}
    KDbDriver *driver;
};

KDbDriverBehavior::KDbDriverBehavior(KDbDriver *driver)
        : features(KDbDriver::NoFeatures)
        , UNSIGNED_TYPE_KEYWORD(QLatin1String("UNSIGNED"))
        , AUTO_INCREMENT_FIELD_OPTION(QLatin1String("AUTO_INCREMENT"))
        , AUTO_INCREMENT_PK_FIELD_OPTION(QLatin1String("AUTO_INCREMENT PRIMARY KEY"))
        , SPECIAL_AUTO_INCREMENT_DEF(false)
        , AUTO_INCREMENT_REQUIRES_PK(false)
        , ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE(false)
        , OPENING_QUOTATION_MARK_BEGIN_FOR_IDENTIFIER('"')
        , CLOSING_QUOTATION_MARK_BEGIN_FOR_IDENTIFIER('"')
        , USING_DATABASE_REQUIRED_TO_CONNECT(true)
        , CONNECTION_REQUIRED_TO_CHECK_DB_EXISTENCE(true)
        , CONNECTION_REQUIRED_TO_CREATE_DB(true)
        , CONNECTION_REQUIRED_TO_DROP_DB(true)
        , USE_TEMPORARY_DATABASE_FOR_CONNECTION_IF_NEEDED(false)
        , IS_DB_OPEN_AFTER_CREATE(false)
        , _1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY(false)
        , SELECT_1_SUBQUERY_SUPPORTED(false)
        , BOOLEAN_TRUE_LITERAL(QLatin1Char('1'))
        , BOOLEAN_FALSE_LITERAL(QLatin1Char('0'))
        , TEXT_TYPE_MAX_LENGTH(0)
        , LIKE_OPERATOR(QLatin1String("LIKE"))
        , RANDOM_FUNCTION(QLatin1String("RANDOM"))
        , d(new Private)
{
    d->driver = driver;
    properties.insert("client_library_version", QVariant(),
                      KDbDriver::tr("Client library version"));
    properties.insert("default_server_encoding", QVariant(),
                      KDbDriver::tr("Default character encoding on server"));
}

KDbDriverBehavior::~KDbDriverBehavior()
{
    delete d;
}

void KDbDriverBehavior::initInternalProperties()
{
    properties.insert("is_file_database", d->driver->metaData()->isFileBased(),
                      KDbDriver::tr("File-based database driver"));
    if (d->driver->metaData()->isFileBased()) {
        properties.insert("file_database_mimetypes", d->driver->metaData()->mimeTypes(),
                          KDbDriver::tr("File-based database's MIME types"));
    }

#if 0
    QString str;
    if (features & KDbDriver::SingleTransactions)
        str = futureTr("Single transactions support");
    else if (features & KDbDriver::MultipleTransactions)
        str = futureTr("Multiple transactions support");
    else if (features & KDbDriver::NestedTransactions)
        str = futureTr("Nested transactions support");
    else if (features & KDbDriver::IgnoreTransactions)
        str = futureTr("Ignored", "Ignored transactions");
    else
        str = futureTr2("None", "No transactions");
#endif
// properties["transaction_support"] = features & KDbDriver::TransactionsMask;
// propertyCaptions["transaction_support"] = KDbDriver::tr("Transaction support");
    properties.insert("transactions_single", bool(d->driver->behavior()->features & KDbDriver::SingleTransactions),
                      KDbDriver::tr("Single transactions support"));
    properties.insert("transactions_multiple", bool(d->driver->behavior()->features & KDbDriver::MultipleTransactions),
                      KDbDriver::tr("Multiple transactions support"));
    properties.insert("transactions_nested", bool(d->driver->behavior()->features & KDbDriver::NestedTransactions),
                      KDbDriver::tr("Nested transactions support"));
    properties.insert("transactions_ignored", bool(d->driver->behavior()->features & KDbDriver::IgnoreTransactions),
                      KDbDriver::tr("Ignored transactions support"));
    //! @todo more KDbDriver::Features

    const KDbVersionInfo v(KDb::version());
    properties.insert("kdb_driver_version", QString::fromLatin1("%1.%2.%3").arg(v.major()).arg(v.minor()).arg(v.release()),
                      KDbDriver::tr("KDb driver version"));
}

//---------------------------------------------

KDbDriverPrivate::KDbDriverPrivate(KDbDriver *aDriver)
        : driver(aDriver)
        , driverBehavior(driver)
        , metaData(nullptr)
        , adminTools(nullptr)
{
}

KDbDriverPrivate::~KDbDriverPrivate()
{
    delete adminTools;
}
