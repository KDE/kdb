/* This file is part of the KDE project
   Copyright (C) 2016 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_FIELD_P_H
#define KDB_FIELD_P_H

#include <QtDebug>
class KDbField;

//! Options for debug(QDebug dbg, const KDbField& field, KDbFieldDebugOptions options)
//! @internal
//! @since 3.1
enum KDbFieldDebugOption {
    KDbFieldDebugNoOptions = 0,
    KDbFieldDebugAddName = 1 //!< Adds field name
};
Q_DECLARE_FLAGS(KDbFieldDebugOptions, KDbFieldDebugOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(KDbFieldDebugOptions)

//! Sends information about field @a field to debug output @a dbg.
//! Uses options @a options to control the output.
//! @internal
//! @since 3.1
void debug(QDebug dbg, const KDbField& field, KDbFieldDebugOptions options);

#endif
