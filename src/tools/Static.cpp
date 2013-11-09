/* This file is part of the KDE project
   Copyright (C) 1999 Sirtaj Singh Kanq <taj@kde.org>
   Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
   Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>

   Based on kdelibs/kdecore/kernel/kstatic.*

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

/*
 * kglobal.cpp -- Implementation of namespace KGlobal.
 * Author:	Sirtaj Singh Kang
 * Generated:	Sat May  1 02:08:43 EST 1999
 */

#include "Static.h"

#include <QList>
#include <QSet>
#include <QCoreApplication>
#include <QTextCodec>

#ifndef NDEBUG
#define MYASSERT(x) if (!x) \
   qFatal("Fatal error: you need to have a KComponentData object before\n" \
         "you do anything that requires it! Examples of this are config\n" \
         "objects, standard directories or translations.");
#else
#define MYASSERT(x) /* nope */
#endif

typedef QSet<QString> KStringDict;

class KGlobalPrivate
{
    public:
        inline KGlobalPrivate()
            : stringDict(0)
        {
        }

        inline ~KGlobalPrivate()
        {
            delete stringDict;
            stringDict = 0;
        }

        KStringDict *stringDict;
};

PREDICATE_GLOBAL_STATIC(KGlobalPrivate, globalData)

#define PRIVATE_DATA KGlobalPrivate *d = globalData

/**
 * Create a static QString
 *
 * To be used inside functions(!) like:
 * static const QString &myString = KGlobal::staticQString(tr("My Text"));
 */
const QString &staticQString(const QString &str)
{
    PRIVATE_DATA;
    if (!d->stringDict) {
        d->stringDict = new KStringDict;
    }

   return *d->stringDict->insert(str);
}

/**
 * Create a static QString
 *
 * To be used inside functions(!) like:
 * static const QString &myString = KGlobal::staticQString("myText");
 */
const QString &staticQString(const char *str)
{
    return staticQString(QLatin1String(str));
}

#undef PRIVATE_DATA
