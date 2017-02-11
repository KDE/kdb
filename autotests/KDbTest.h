/* This file is part of the KDE project
   Copyright (C) 2015-2016 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDBTEST_H
#define KDBTEST_H

#include "KDbTestUtils.h"

class KDbTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testVersionInfo();
    void testFieldTypes();
    void testFieldTypesForGroup_data();
    void testFieldTypesForGroup();
    void testFieldTypeNamesAndStringsForGroup_data();
    void testFieldTypeNamesAndStringsForGroup();
    void testDefaultFieldTypeForGroup();
    void testSimplifiedFieldTypeName();
    void testIsEmptyValue_data();
    void testIsEmptyValue();
    void testUnescapeString_data();
    void testUnescapeString();
    void testEscapeBLOB_data();
    void testEscapeBLOB();
    void testPgsqlByteaToByteArray();
    void testXHexToByteArray_data();
    void testXHexToByteArray();
    void testZeroXHexToByteArray_data();
    void testZeroXHexToByteArray();
    void testCstringToVariant_data();
    void testCstringToVariant();
    void testTemporaryTableName();
    void deleteRecordWithOneConstraintsTest();
    void deleteNonExistingRecordTest();
    void deleteRecordWithTwoConstraintsTest();
    void deleteRecordWithThreeConstraintsTest();
    void deleteAllRecordsTest();
    void cleanupTestCase();
private:
    void testUnescapeStringHelper(const QString &sequenceString, const QString &resultString,
                                  char quote, int errorPosition, int offset);
    KDbTestUtils utils;
};

#endif
