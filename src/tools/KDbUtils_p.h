/* This file is part of the KDE project
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>

   Portions of kstandarddirs.cpp:
   Copyright (C) 1999 Sirtaj Singh Kang <taj@kde.org>
   Copyright (C) 1999,2007 Stephan Kulow <coolo@kde.org>
   Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2009 David Faure <faure@kde.org>

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

#ifndef KDB_TOOLS_UTILS_P_H
#define KDB_TOOLS_UTILS_P_H

#include <QtGlobal>

template <typename T> struct QAddConst {
    typedef const T Type;
};

#if QT_VERSION < 0x050700
//! Adds const to non-const objects (like std::as_const)
template <typename T>
Q_DECL_CONSTEXPR typename QAddConst<T>::Type& qAsConst(T& t) Q_DECL_NOTHROW { return t; }

// prevent rvalue arguments:
template <typename T>
void qAsConst(const T &&) Q_DECL_EQ_DELETE;
#endif

//! @def KDB_SHARED_LIB_EXTENSION operating system-dependent extension for shared library files
#if defined(Q_OS_WIN)
#define KDB_SHARED_LIB_EXTENSION ".dll"
#elif defined(Q_OS_MAC)
// shared libraries indeed have a dylib extension on OS X, but most apps use .so for plugins
#define KDB_SHARED_LIB_EXTENSION ".so"
#else
#define KDB_SHARED_LIB_EXTENSION ".so"
#endif

#endif //KDB_TOOLS_UTILS_P_H
