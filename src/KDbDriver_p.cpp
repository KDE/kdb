/* This file is part of the KDE project
    Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>
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
#include "KDb.h"

#include <QtDebug>

KDbDriverBehaviour::KDbDriverBehaviour()
        : UNSIGNED_TYPE_KEYWORD(QLatin1String("UNSIGNED"))
        , AUTO_INCREMENT_FIELD_OPTION(QLatin1String("AUTO_INCREMENT"))
        , AUTO_INCREMENT_PK_FIELD_OPTION(QLatin1String("AUTO_INCREMENT PRIMARY KEY"))
        , SPECIAL_AUTO_INCREMENT_DEF(false)
        , AUTO_INCREMENT_REQUIRES_PK(false)
        , ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE(false)
        , QUOTATION_MARKS_FOR_IDENTIFIER('"')
        , USING_DATABASE_REQUIRED_TO_CONNECT(true)
        , CONNECTION_REQUIRED_TO_CHECK_DB_EXISTENCE(true)
        , CONNECTION_REQUIRED_TO_CREATE_DB(true)
        , CONNECTION_REQUIRED_TO_DROP_DB(true)
        , USE_TEMPORARY_DATABASE_FOR_CONNECTION_IF_NEEDED(false)
        , _1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY(false)
        , SELECT_1_SUBQUERY_SUPPORTED(false)
        , BOOLEAN_TRUE_LITERAL(QLatin1Char('1'))
        , BOOLEAN_FALSE_LITERAL(QLatin1Char('0'))
        , TEXT_TYPE_MAX_LENGTH(0)
{
}

KDbDriverBehaviour::~KDbDriverBehaviour()
{
}

//---------------------------------------------

DriverPrivate::DriverPrivate()
        : isDBOpenedAfterCreate(false)
        , features(KDbDriver::NoFeatures)
{
    adminTools = 0;

    properties["client_library_version"] = QString();
    propertyCaptions["client_library_version"] =
        QObject::tr("Client library version");

    properties["default_server_encoding"] = QString();
    propertyCaptions["default_server_encoding"] =
        QObject::tr("Default character encoding on server");
}

void DriverPrivate::initInternalProperties()
{
    properties["is_file_database"] = QVariant(info.isFileBased());
    propertyCaptions["is_file_database"] = QObject::tr("File-based database driver");
    if (info.isFileBased()) {
        properties["file_database_mimetypes"] = info.mimeTypes();
        propertyCaptions["file_database_mimetypes"] = QObject::tr("File-based database's MIME types");
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
// propertyCaptions["transaction_support"] = QObject::tr("Transaction support");
    properties["transaction_single"] = QVariant(features & KDbDriver::SingleTransactions);
    propertyCaptions["transaction_single"] = QObject::tr("Single transactions support");
    properties["transaction_multiple"] = QVariant(features & KDbDriver::MultipleTransactions);
    propertyCaptions["transaction_multiple"] = QObject::tr("Multiple transactions support");
    properties["transaction_nested"] = QVariant(features & KDbDriver::NestedTransactions);
    propertyCaptions["transaction_nested"] = QObject::tr("Nested transactions support");

    const KDbVersionInfo v(KDb::version());
    properties["kdb_driver_version"] =
        QString::fromLatin1("%1.%2").arg(v.major()).arg(v.minor());
    propertyCaptions["kdb_driver_version"] =
        QObject::tr("KDb driver version");
}

DriverPrivate::~DriverPrivate()
{
    delete adminTools;
}

//--------------------------

KDbAdminTools::Private::Private()
{
}

KDbAdminTools::Private::~Private()
{
}
