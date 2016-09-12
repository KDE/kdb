/* This file is part of the KDE project
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

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

static bool isTrue(KPluginMetaData *metaData, const char* fieldName)
{
    return 0 == metaData->value(QLatin1String(fieldName))
                .compare(QLatin1String("true"), Qt::CaseInsensitive);
}

class KDbDriverMetaData::Private
{
public:
    Private(KDbDriverMetaData *metaData)
    {
        fileBased = isTrue(metaData, "X-KDb-FileBased");
        importingEnabled = isTrue(metaData, "X-KDb-ImportingEnabled");
    }

    QStringList mimeTypes;
    bool fileBased;
    bool importingEnabled;
private:
    Q_DISABLE_COPY(Private)
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

bool KDbDriverMetaData::isFileBased() const
{
    return d->fileBased;
}

bool KDbDriverMetaData::isImportingEnabled() const
{
    return d->importingEnabled;
}
