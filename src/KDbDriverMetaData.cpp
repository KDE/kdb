/* This file is part of the KDE project
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>

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

#include "KDbDriverMetaData.h"

#include <QStringList>

class KDbDriverMetaData::Private
{
public:
    Private(KDbDriverMetaData *metaData)
    {
        mimeTypes = metaData->value(QLatin1String("MimeType")).split(QLatin1Char(';'));
        fileBased = 0 == metaData->value(QLatin1String("X-KDb-FileBased"))
                        .compare(QLatin1String("true"), Qt::CaseInsensitive);
        importingEnabled = 0 == metaData->value(QLatin1String("X-KDb-ImportingEnabled"))
                        .compare(QLatin1String("true"), Qt::CaseInsensitive);
    }

    QStringList mimeTypes;
    bool fileBased;
    bool importingEnabled;
};

// ---

KDbDriverMetaData::KDbDriverMetaData(const QPluginLoader &loader)
    : KPluginMetaData(loader), d(new Private(this))
{
}

KDbDriverMetaData::~KDbDriverMetaData()
{
    delete d;
}

QString KDbDriverMetaData::id() const
{
    return pluginId();
}

QStringList KDbDriverMetaData::mimeTypes() const
{
    return d->mimeTypes;
}

bool KDbDriverMetaData::isFileBased() const
{
    return d->fileBased;
}

bool KDbDriverMetaData::isImportingEnabled() const
{
    return d->importingEnabled;
}
