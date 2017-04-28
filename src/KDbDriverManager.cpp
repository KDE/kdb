/* This file is part of the KDE project
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2012 Dimitrios T. Tanis <dimitrios.tanis@kdemail.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KDbDriverManager.h"
#include "KDbDriverManager_p.h"
#include "KDbJsonTrader_p.h"
#include "KDbDriver.h"
#include "KDbDriverMetaData.h"
#include "kdb_debug.h"

#include <KPluginFactory>

#include <QApplication>
#include <QTime>
#include <QWidget>

Q_GLOBAL_STATIC(DriverManagerInternal, s_self)

DriverManagerInternal::DriverManagerInternal()
 : m_lookupDriversNeeded(true)
{
    qsrand(QTime::currentTime().msec()); // needed e.g. to create random table names
}

DriverManagerInternal::~DriverManagerInternal()
{
    drivermanagerDebug();
    clear();
    drivermanagerDebug() << "ok";
}

void DriverManagerInternal::clear()
{
    drivermanagerDebug() << "Clearing drivers...";
    qDeleteAll(m_drivers);
    m_drivers.clear();
    qDeleteAll(m_driversMetaData);
    m_driversMetaData.clear();
}

void DriverManagerInternal::slotAppQuits()
{
    if (qApp && !qApp->topLevelWidgets().isEmpty()
            && qApp->topLevelWidgets().first()->isVisible()) {
        return; //what a hack! - we give up when app is still there
    }
    clear();
}

//static
DriverManagerInternal *DriverManagerInternal::self()
{
    return s_self;
}

bool DriverManagerInternal::lookupDrivers()
{
    if (!m_lookupDriversNeeded)
        return true;

    if (!forceEmpty) {
        lookupDriversInternal();
        m_lookupDriversNeeded = false;
    }
    if (m_driversMetaData.isEmpty()) {
        m_result = KDbResult(ERR_DRIVERMANAGER,
                             tr("Could not find any database drivers."));
        return false;
    }
    return true;
}

void DriverManagerInternal::lookupDriversInternal()
{
    if (qApp) {
        connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotAppQuits()));
    }

    clearResult();

    //drivermanagerDebug() << "Load all plugins";
    QList<QPluginLoader*> offers
            = KDbJsonTrader::self()->query(QLatin1String("KDb/Driver"));
    const QString expectedVersion = QString::fromLatin1("%1.%2")
            .arg(KDB_STABLE_VERSION_MAJOR).arg(KDB_STABLE_VERSION_MINOR);
    foreach(const QPluginLoader *loader, offers) {
        //QJsonObject json = loader->metaData();
        //drivermanagerDebug() << json;
        QScopedPointer<KDbDriverMetaData> metaData(new KDbDriverMetaData(*loader));
        //qDebug() << "VER:" << metaData->version();
        if (metaData->version() != expectedVersion) {
            kdbWarning() << "Driver with ID" << metaData->id()
                         << "(" << metaData->fileName() << ")"
                         << "has version" << metaData->version() << "but expected version is"
                         << expectedVersion
                         << "-- skipping it";
            continue;
        }
        if (m_driversMetaData.contains(metaData->id())) {
            if (qgetenv("KDB_NO_DUPLICATED_DRIVER_WARNINGS").isEmpty()) {
                kdbWarning() << "Driver with ID" << metaData->id() << "already found at"
                             << m_driversMetaData.value(metaData->id())->fileName()
                             << "-- skipping another at" << metaData->fileName();
            }
            continue;
        }
        foreach (const QString& mimeType, metaData->mimeTypes()) {
            m_metadata_by_mimetype.insertMulti(mimeType, metaData.data());
        }
        m_driversMetaData.insert(metaData->id(), metaData.data());
        metaData.take();
    }
    qDeleteAll(offers);
    offers.clear();
}

QStringList DriverManagerInternal::driverIds()
{
    if (!lookupDrivers()) {
        return QStringList();
    }
    if (m_driversMetaData.isEmpty() && result().isError()) {
        return QStringList();
    }
    return m_driversMetaData.keys();
}

const KDbDriverMetaData* DriverManagerInternal::driverMetaData(const QString &id)
{
    if (!lookupDrivers()) {
        return nullptr;
    }
    const KDbDriverMetaData *metaData = m_driversMetaData.value(id.toLower());
    if (!metaData || m_result.isError()) {
        m_result = KDbResult(ERR_DRIVERMANAGER,
                             tr("Could not find database driver \"%1\".").arg(id));
    }
    return metaData;
}

QStringList DriverManagerInternal::driverIdsForMimeType(const QString &mimeType)
{
    if (!lookupDrivers()) {
        return QStringList();
    }
    const QList<KDbDriverMetaData*> metaDatas(m_metadata_by_mimetype.values(mimeType.toLower()));
    QStringList result;
    foreach (const KDbDriverMetaData* metaData, metaDatas) {
        result.append(metaData->id());
    }
    return result;
}

QStringList DriverManagerInternal::possibleProblems() const
{
    return m_possibleProblems;
}

KDbDriver* DriverManagerInternal::driver(const QString& id)
{
    if (!lookupDrivers())
        return nullptr;

    clearResult();
    drivermanagerDebug() << "loading" << id;

    KDbDriver *driver = nullptr;
    if (!id.isEmpty()) {
        driver = m_drivers.value(id.toLower());
    }
    if (driver)
        return driver; //cached

    if (!m_driversMetaData.contains(id.toLower())) {
        m_result = KDbResult(ERR_DRIVERMANAGER,
                             tr("Could not find database driver \"%1\".").arg(id));
        return nullptr;
    }

    const KDbDriverMetaData *metaData = m_driversMetaData.value(id.toLower());
    KPluginFactory *factory = qobject_cast<KPluginFactory*>(metaData->instantiate());
    if (!factory) {
        m_result = KDbResult(ERR_DRIVERMANAGER,
                             tr("Could not load database driver's plugin file \"%1\".")
                                .arg(metaData->fileName()));
        QPluginLoader loader(metaData->fileName()); // use this to get the message
        (void)loader.load();
        m_result.setServerMessage(loader.errorString());
        kdbWarning() << m_result.message() << m_result.serverMessage();
        return nullptr;
    }
    driver = factory->create<KDbDriver>();
    if (!driver) {
        m_result = KDbResult(ERR_DRIVERMANAGER,
                             tr("Could not open database driver \"%1\" from plugin file \"%2\".")
                                .arg(metaData->id(),
                                     metaData->fileName()));
        kdbWarning() << m_result.message();
        return nullptr;
    }
    driver->setMetaData(metaData);
    m_drivers.insert(id.toLower(), driver);
    return driver;
}

// ---------------------------

KDbDriverManager::KDbDriverManager()
{
}

KDbDriverManager::~KDbDriverManager()
{
}

KDbResult KDbDriverManager::result() const
{
    return s_self->result();
}

KDbResultable* KDbDriverManager::resultable() const
{
    return s_self;
}

QStringList KDbDriverManager::driverIds()
{
    return s_self->driverIds();
}

const KDbDriverMetaData* KDbDriverManager::driverMetaData(const QString &id)
{
    return s_self->driverMetaData(id);
}

QStringList KDbDriverManager::driverIdsForMimeType(const QString &mimeType)
{
    return s_self->driverIdsForMimeType(mimeType);
}

KDbDriver* KDbDriverManager::driver(const QString& id)
{
    return s_self->driver(id);
}

QString KDbDriverManager::possibleProblemsMessage() const
{
    if (s_self->possibleProblems().isEmpty()) {
        return QString();
    }
    QString str;
    str.reserve(1024);
    str = QLatin1String("<ul>");
    foreach (const QString& problem, s_self->possibleProblems())
        str += (QLatin1String("<li>") + problem + QLatin1String("</li>"));
    str += QLatin1String("</ul>");
    return str;
}

bool KDbDriverManager::hasDatabaseServerDrivers()
{
    foreach(const QString& id, driverIds()) {
        const KDbDriverMetaData *metaData = s_self->driverMetaData(id);
        if (!metaData->isFileBased()) {
            return true;
        }
    }
    return false;
}
