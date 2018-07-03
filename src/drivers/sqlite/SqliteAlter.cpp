/* This file is part of the KDE project
   Copyright (C) 2006 Jarosław Staniek <staniek@kde.org>

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

// ** bits of SqliteConnection related to table altering **

#include "SqliteConnection.h"
#include "KDb.h"

#include <QHash>
#include <QGlobalStatic>

enum SqliteTypeAffinity { //as defined here: 2.1 Determination Of Column Affinity (https://sqlite.org/datatype3.html)
    NoAffinity = 0, IntAffinity = 1, TextAffinity = 2, BLOBAffinity = 3
};

//! @internal
struct SqliteTypeAffinityInternal {
    SqliteTypeAffinityInternal() {
        affinity.insert(KDbField::Byte, IntAffinity);
        affinity.insert(KDbField::ShortInteger, IntAffinity);
        affinity.insert(KDbField::Integer, IntAffinity);
        affinity.insert(KDbField::BigInteger, IntAffinity);
        affinity.insert(KDbField::Boolean, IntAffinity);
        affinity.insert(KDbField::Date, TextAffinity);
        affinity.insert(KDbField::DateTime, TextAffinity);
        affinity.insert(KDbField::Time, TextAffinity);
        affinity.insert(KDbField::Float, IntAffinity);
        affinity.insert(KDbField::Double, IntAffinity);
        affinity.insert(KDbField::Text, TextAffinity);
        affinity.insert(KDbField::LongText, TextAffinity);
        affinity.insert(KDbField::BLOB, BLOBAffinity);
    }
    QHash<KDbField::Type, SqliteTypeAffinity> affinity;
};

Q_GLOBAL_STATIC(SqliteTypeAffinityInternal, KDb_SQLite_affinityForType)

//! @return SQLite type affinity for @a type
//! See doc/dev/alter_table_type_conversions.ods, page 2 for more info
static SqliteTypeAffinity affinityForType(KDbField::Type type)
{
    return KDb_SQLite_affinityForType->affinity[type];
}

tristate SqliteConnection::drv_changeFieldProperty(KDbTableSchema *table, KDbField *field,
        const QString& propertyName, const QVariant& value)
{
    if (propertyName == QLatin1String("type")) {
        bool ok;
        KDbField::Type type = KDb::intToFieldType(value.toInt(&ok));
        if (!ok || KDbField::InvalidType == type) {
            //! @todo msg
            return false;
        }
        return changeFieldType(table, field, type);
    }
    // not found
    return cancelled;
}

/*!
 From https://sqlite.org/datatype3.html :
 Version 3 enhances provides the ability to store integer and real numbers in a more compact
 format and the capability to store BLOB data.

 Each value stored in an SQLite database (or manipulated by the database engine) has one
 of the following storage classes:
 * NULL. The value is a NULL value.
 * INTEGER. The value is a signed integer, stored in 1, 2, 3, 4, 6, or 8 bytes depending
    on the magnitude of the value.
 * REAL. The value is a floating point value, stored as an 8-byte IEEE floating point number.
 * TEXT. The value is a text string, stored using the database encoding (UTF-8, UTF-16BE or UTF-16-LE).
 * BLOB. The value is a blob of data, stored exactly as it was input.

 Column Affinity
 In SQLite version 3, the type of a value is associated with the value itself,
 not with the column or variable in which the value is stored.
.The type affinity of a column is the recommended type for data stored in that column.

 See alter_table_type_conversions.ods for details.
*/
tristate SqliteConnection::changeFieldType(KDbTableSchema *table, KDbField *field,
        KDbField::Type type)
{
    Q_UNUSED(table);
    const KDbField::Type oldType = field->type();
    const SqliteTypeAffinity oldAffinity = affinityForType(oldType);
    const SqliteTypeAffinity newAffinity = affinityForType(type);
    if (oldAffinity != newAffinity) {
        //type affinity will be changed
    }

    return cancelled;
}
