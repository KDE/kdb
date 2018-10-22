/* This file is part of the KDE project
   Copyright (C) 2004-2018 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_P_H
#define KDB_P_H

#include "KDbField.h"

class KDbDriver;

//! @internal Dummy class to get simply translation markup expressions
//! of the form kdb::tr("foo") instead of the complicated and harder to read
//! QCoreApplication::translate("KDb", "foo") which also runs the chance of
//! typos in the class context argument
class kdb
{
    Q_DECLARE_TR_FUNCTIONS(KDb)
};

KDbEscapedString valueToSqlInternal(const KDbDriver *driver, KDbField::Type ftype,
                                    const QVariant &v);

#endif
