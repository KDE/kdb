/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2007 David Faure <faure@kde.org>
   Copyright (C) 2015 Jarosław Staniek <staniek@kde.org>

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

#include "KDbJsonTrader_p.h"
#include "KDb.h"
#include "kdb_debug.h"

#include <QList>
#include <QPluginLoader>
#include <QJsonObject>
#include <QJsonArray>
#include <QDirIterator>
#include <QDir>
#include <QCoreApplication>

Q_GLOBAL_STATIC(KDbJsonTrader, KDbJsonTrader_instance)

class Q_DECL_HIDDEN KDbJsonTrader::Private
{
public:
    Private() : pluginPathFound(false)
    {
    }
    bool pluginPathFound;
    QStringList pluginPaths;
private:
    Q_DISABLE_COPY(Private)
};

// ---

KDbJsonTrader::KDbJsonTrader()
    : d(new Private)
{
    Q_ASSERT(!KDbJsonTrader_instance.exists());
}

KDbJsonTrader::~KDbJsonTrader()
{
    delete d;
}

KDbJsonTrader* KDbJsonTrader::self()
{
    return KDbJsonTrader_instance;
}

//! Checks loader @a loader
static bool checkLoader(QPluginLoader *loader, const QString &servicetype,
                        const QString &mimetype)
{
    QJsonObject json = loader->metaData().value(QLatin1String("MetaData")).toObject();
    if (json.isEmpty()) {
        //kdbDebug() << dirIter.filePath() << "has no json!";
        return false;
    }
    QJsonObject pluginData = json.value(QLatin1String("KPlugin")).toObject();
    if (!pluginData.value(QLatin1String("ServiceTypes")).toArray()
            .contains(QJsonValue(servicetype)))
    {
        return false;
    }

    if (!mimetype.isEmpty()) {
        QStringList mimeTypes
                = json.value(QLatin1String("MimeType")).toString().split(QLatin1Char(';'));
        mimeTypes += json.value(QLatin1String("X-KDE-NativeMimeType")).toString();
        if (! mimeTypes.contains(mimetype)) {
            return false;
        }
    }
    return true;
}

static QList<QPluginLoader *> findPlugins(const QString &path, const QString &servicetype,
                                          const QString &mimetype)
{
    QList<QPluginLoader*> list;
    QDirIterator dirIter(path, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (dirIter.hasNext()) {
        dirIter.next();
        if (dirIter.fileInfo().isFile()) {
            QPluginLoader *loader = new QPluginLoader(dirIter.filePath());
            if (checkLoader(loader, servicetype, mimetype)) {
                list.append(loader);
            } else {
                delete loader;
            }
        }
    }
    return list;
}

QList<QPluginLoader *> KDbJsonTrader::query(const QString &servicetype,
                                            const QString &mimetype)
{
    if (!d->pluginPathFound) {
        d->pluginPaths = KDb::libraryPaths();
    }

    QList<QPluginLoader *> list;
    for(const QString &path : std::as_const(d->pluginPaths)) {
        list += findPlugins(path, servicetype, mimetype);
    }
    return list;
}
