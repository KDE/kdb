/* This file is part of the KDE project
   Copyright (C) 2015 Laurent Montel <montel@kde.org>

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

#ifndef KDB_DEBUG_H
#define KDB_DEBUG_H

#include "KDbGlobal.h"

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(KDB_LOG)

//! Debug command for the core KDb code
#define kdbDebug(...) qCDebug(KDB_LOG, __VA_ARGS__)

//! Warning command for the core KDb code
#define kdbWarning(...) qCWarning(KDB_LOG, __VA_ARGS__)

//! Critical command for the core KDb code
#define kdbCritical(...) qCCritical(KDB_LOG, __VA_ARGS__)

#ifdef KDB_EXPRESSION_DEBUG
# define ExpressionDebug kdbDebug()
#else
# define ExpressionDebug if (1) {} else kdbDebug()
#endif

#ifdef KDB_DRIVERMANAGER_DEBUG
# define drivermanagerDebug(...) kdbDebug(__VA_ARGS__)
#else
# define drivermanagerDebug(...) if (1) {} else kdbDebug(__VA_ARGS__)
#endif

#ifdef KDB_TRANSACTIONS_DEBUG
# define transactionsDebug(...) kdbDebug(__VA_ARGS__)
#else
# define transactionsDebug(...) if (1) {} else kdbDebug(__VA_ARGS__)
#endif

#endif
