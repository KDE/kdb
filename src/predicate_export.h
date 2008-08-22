/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2005 Martin Ellis <martin.ellis@kdemail.net>

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

#ifndef _PREDICATE_EXPORT_H_
#define _PREDICATE_EXPORT_H_

#include <QtGlobal>

#ifndef PREDICATE_EXPORT
# ifdef MAKE_PREDICATE_LIB
#  define PREDICATE_EXPORT Q_DECL_EXPORT
# else
#  define PREDICATE_EXPORT Q_DECL_IMPORT
# endif
#endif

#endif
