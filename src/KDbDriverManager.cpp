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
#include "KDbDriver_p.h"
#include "KDbJsonTrader_p.h"
#include "KDbDriverMetaData.h"
#include "kdb_debug.h"

#include <KPluginFactory>

#include <QObject>
#include <QApplication>
#include <QDir>
#include <QPluginLoader>
#include <QSettings>
#include <QWidget>

Q_GLOBAL_STATIC(DriverManagerInternal, s_self);

DriverManagerInternal::DriverManagerInternal() /* protected */
 : m_lookupDriversNeeded(true)
{
    qsrand(QTime::currentTime().msec()); // needed e.g. to create random table names
}

DriverManagerInternal::~DriverManagerInternal()
{
    drivermanagerDebug();
    qDeleteAll(m_drivers);
    m_drivers.clear();
    drivermanagerDebug() << "ok";
}

void DriverManagerInternal::slotAppQuits()
{
    if (qApp && !qApp->topLevelWidgets().isEmpty()
            && qApp->topLevelWidgets().first()->isVisible()) {
        return; //what a hack! - we give up when app is still there
    }
    drivermanagerDebug() << "let's clear drivers...";
    qDeleteAll(m_drivers);
    m_drivers.clear();
}

//static
DriverManagerInternal *DriverManagerInternal::self()
{
    return s_self;
}

#if 0
void DriverManagerInternal::lookupDriversForDirectory(const QString& pluginsDir)
{
    QDir dir(pluginsDir, QLatin1String("kdb_*.desktop"));
    const QFileInfoList infoFiles( dir.entryInfoList(QDir::Files|QDir::Readable) );
    foreach (const QFileInfo& infoFile, infoFiles) {
        QSettings config(infoFile.absoluteFilePath(), QSettings::IniFormat);
        config.beginGroup(QLatin1String("Desktop Entry"));
        KDbDriverInfo info;
        info.setName( config.value(QLatin1String("Name")).toString().toLower() );
        if (m_driversInfo.contains(info.name())) {
            kdbWarning() << "More than one driver named" << info.name() << "-- skipping this one";
            continue;
        }
        info.setVersion(config.value(QLatin1String("Version")).toString());
        const QString expectedVersion(QString::fromLatin1("%1.%2").arg(KDb::version().major()).arg(KDb::version().minor()));
        if (expectedVersion != info.version()) {
            kdbWarning() << "Incompatible database driver's" << info.name()
                << "version: found version" << info.version() << "expected version" << expectedVersion
                << "-- skipping this one";
        }
        info.setAbsoluteFilePath(pluginsDir + QLatin1String("/kdb_")
            + config.value(QLatin1String("Name")).toString() + QLatin1String(KDB_SHARED_LIB_EXTENSION));
        info.setCaption(config.value(QLatin1String("Caption")).toString());
//! @todo read translated [..]
        info.setComment(config.value(QLatin1String("Comment")).toString());
        if (info.caption().isEmpty())
            info.setCaption(info.name());
        info.setFileBased(config.value(QLatin1String("Type"),
                                       QLatin1String("Network")).toString().toLower() == QLatin1String("file"));
        QStringList mimeTypes;
        if (info.isFileBased()) {
            mimeTypes = config.value(QLatin1String("MimeTypes")).toStringList();
            QStringList mimeTypesTrimmed;
            foreach (const QString& mimeType, mimeTypes)
                mimeTypesTrimmed.append(mimeType.trimmed().toLower());
            mimeTypes = mimeTypesTrimmed;
            info.setMimeTypes(mimeTypes);
        }
        info.setImportingAllowed(config.value(QLatin1String("AllowImporting"), true).toBool());

        m_driversInfo.insert(info.name().toLower(), info);
        foreach (const QString& mimeType, mimeTypes) {
            m_infos_by_mimetype.insertMulti(mimeType, info);
        }
    }
}
#endif

bool DriverManagerInternal::lookupDrivers()
{
    if (!m_lookupDriversNeeded)
        return true;

    if (qApp) {
        connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotAppQuits()));
    }

    m_lookupDriversNeeded = false;
    clearResult();

    //drivermanagerDebug() << "Load all plugins";
    const QList<QPluginLoader*> offers
            = KDbJsonTrader::self()->query(QLatin1String("KDb/Driver"));
    foreach(QPluginLoader *loader, offers) {
        //QJsonObject json = loader->metaData();
        //drivermanagerDebug() << json;
        //! @todo check version
        KDbDriverMetaData *metaData = new KDbDriverMetaData(*loader);
        if (m_driversMetaData.contains(metaData->id())) {
            kdbWarning() << "More than one driver with ID" << metaData->id() << "-- skipping this one";
            continue;
        }
        foreach (const QString& mimeType, metaData->mimeTypes()) {
            m_metadata_by_mimetype.insertMulti(mimeType, metaData);
        }
        m_driversMetaData.insert(metaData->id(), metaData);
    }

#if 0
    /*! Try in all possible driver directories.
     Looks for "kdb" directory in $INSTALL/plugins, $QT_PLUGIN_PATH, and directory of the application executable.
     Plugin path "Plugins" entry can be added to qt.conf to override; see http://qt-project.org/doc/qt-4.8/qt-conf.html.
    */
    const QStringList libraryPaths(KDb::libraryPaths());
    drivermanagerDebug() << "libraryPaths:" << libraryPaths;
    foreach (const QString& path, libraryPaths) {
        lookupDriversForDirectory(path);
    }
    if (libraryPaths.isEmpty()) {
        m_result = KDbResult(ERR_DRIVERMANAGER, QObject::tr("Could not find directory for database drivers."));
        return false;
    }
#endif

#if 0 // todo?
    KService::List tlist = KServiceTypeTrader::self()->query("Kexi/DBDriver");
    KService::List::ConstIterator it(tlist.constBegin());
    for (; it != tlist.constEnd(); ++it) {
        KService::Ptr ptr = (*it);
        if (!ptr->property("Library").toString().startsWith("kdb_")) {
            kdbWarning() << "X-KDE-Library == " << ptr->property("Library").toString()
                << ": no \"kdb_\" prefix -- skipped to avoid potential conflicts!";
            continue;
        }
        QString srv_name = ptr->property("X-Kexi-DriverName").toString().toLower();
        if (srv_name.isEmpty()) {
            kdbWarning() << "X-Kexi-DriverName must be set for KDb driver \""
                << ptr->property("Name").toString() << "\" service!\n -- skipped!";
            continue;
        }
        if (m_services_lcase.contains(srv_name)) {
            kdbWarning() << "more than one driver named" << srv_name << "\n -- skipping this one!";
            continue;
        }

        QString srv_ver_str = ptr->property("X-Kexi-KexiDBVersion").toString();
        QStringList lst(srv_ver_str.split("."));
        uint minor_ver, major_ver;
        bool ok = (lst.count() == 2);
        if (ok)
            major_ver = lst[0].toUInt(&ok);
        if (ok)
            minor_ver = lst[1].toUInt(&ok);
        if (!ok) {
            kdbWarning() << "problem with detecting" << srv_name << "driver's version -- skipping it!";
            continue;
        }

        if (!KDb::version().matches(major_ver, minor_ver)) {
            kdbWarning() << QString("'%1' driver"
                               " has version '%2' but required KDb driver version is '%3.%4'\n"
                               " -- skipping this driver!").arg(srv_name).arg(srv_ver_str)
                              .arg(KDb::version().major).arg(KDb::version().minor);
            m_possibleProblems += QString("\"%1\" database driver has version \"%2\" "
                                        "but required driver version is \"%3.%4\"")
                                .arg(srv_name).arg(srv_ver_str)
                                .arg(KDb::version().major).arg(KDb::version().minor);
            continue;
        }

        QString drvType = ptr->property("X-Kexi-DriverType").toString().toLower();
        if (drvType == "file") {
            //new property: a list of supported mime types
            QStringList mimes(ptr->property("X-Kexi-FileDBDriverMimeList").toStringList());
            //single mime is obsolete, but we're handling it:
            {
                QString mime(ptr->property("X-Kexi-FileDBDriverMime").toString().toLower());
                if (!mime.isEmpty())
                    mimes.append(mime);
            }

            //store association of this driver with all listed mime types
            for (QStringList::ConstIterator mime_it = mimes.constBegin(); mime_it != mimes.constEnd(); ++mime_it) {
                QString mime((*mime_it).toLower());
                if (!m_services_by_mimetype.contains(mime)) {
                    m_services_by_mimetype.insert(mime, ptr);
                } else {
                    kdbWarning() << "more than one driver for" << mime << "m ime type!";
                }
            }
        }
        m_services.insert(srv_name, ptr);
        m_services_lcase.insert(srv_name,  ptr);
        drivermanagerDebug() << "registered driver: " << ptr->name() << "(" << ptr->library() << ")";
    }
#endif
    if (m_driversMetaData.isEmpty()) {
        m_result = KDbResult(ERR_DRIVERMANAGER,
                             QObject::tr("Could not find any database drivers."));
        return false;
    }
    return true;
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
        return 0;
    }
    const KDbDriverMetaData *metaData = m_driversMetaData.value(id.toLower());
    if (!metaData || m_result.isError()) {
        m_result = KDbResult(ERR_DRIVERMANAGER,
                             QObject::tr("Could not find database driver \"%1\".").arg(id));
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

#if 0
struct LibUnloader {
    LibUnloader(QLibrary *lib) : m_lib(lib) {}
    ~LibUnloader() {
        if (m_lib->isLoaded())
            m_lib->unload();
    }
private:
    QLibrary *m_lib;
};
#endif

KDbDriver* DriverManagerInternal::driver(const QString& id)
{
    if (!lookupDrivers())
        return 0;

    clearResult();
    drivermanagerDebug() << "loading" << id;

    KDbDriver *drv = 0;
    if (!id.isEmpty()) {
        drv = m_drivers.value(id.toLower());
    }
    if (drv)
        return drv; //cached

    if (!m_driversMetaData.contains(id.toLower())) {
        m_result = KDbResult(ERR_DRIVERMANAGER,
                             QObject::tr("Could not find database driver \"%1\".").arg(id));
        return 0;
    }

    const KDbDriverMetaData *metaData = m_driversMetaData.value(id.toLower());
    KPluginFactory *factory = qobject_cast<KPluginFactory*>(metaData->instantiate());
    if (!factory) {
        m_result = KDbResult(ERR_DRIVERMANAGER,
                             QObject::tr("Could not load database driver's plugin file \"%1\".")
                             .arg(metaData->fileName()));
        kdbWarning() << m_result.message();
        return 0;
    }
    KDbDriver *driver = factory->create<KDbDriver>();
    if (!driver) {
        m_result = KDbResult(ERR_DRIVERMANAGER,
                             QObject::tr("Could not open database driver from plugin \"%1\".")
                             .arg(metaData->fileName()));
        kdbWarning() << m_result.message();
        return 0;
    }
    driver->setMetaData(metaData);
    return driver;
}

#if 0
    QString libFileName(info.absoluteFilePath());
#if defined Q_OS_WIN && (defined(_DEBUG) || defined(DEBUG))
    libFileName += "_d";
#endif
    QLibrary lib(libFileName);
    LibUnloader unloader(&lib);
    drivermanagerDebug() << libFileName;
    if (!lib.load()) {
        m_result = KDbResult(ERR_DRIVERMANAGER, QObject::tr("Could not load library \"%1\".").arg(id));
        m_result.setServerMessage(lib.errorString());
        drivermanagerDebug() << lib.errorString();
        return 0;
    }
    
    const uint* foundMajor = (const uint*)lib.resolve("version_major");
    if (!foundMajor) {
       m_result = KDbResult(ERR_DRIVERMANAGER,
                         QObject::tr("Could not find \"%1\" entry point of library \"%2\".")
                         .arg(QLatin1String("version_major")).arg(id));
       return 0;
    }
    const uint* foundMinor = (const uint*)lib.resolve("version_minor");
    if (!foundMinor) {
       m_result = KDbResult(ERR_DRIVERMANAGER, QObject::tr("Could not find \"%1\" entry point of library \"%2\".")
                         .arg(QLatin1String("version_minor")).arg(id));
       return 0;
    }
    if (!KDb::version().matches(*foundMajor, *foundMinor)) {
        m_result = KDbResult(ERR_INCOMPAT_DRIVER_VERSION,
            QObject::tr("Incompatible database driver's \"%1\" version: found version %2, expected version %3.")
                 .arg(id)
                 .arg(QString::fromLatin1("%1.%2").arg(*foundMajor).arg(*foundMinor))
                 .arg(QString::fromLatin1("%1.%2").arg(KDb::version().major()).arg(KDb::version().minor()))
            );
        return 0;
    }
    lib.unload();

    QPluginLoader loader(libFileName);
    drv = dynamic_cast<KDbDriver*>(loader.instance());
    if (!drv) {
        m_result = KDbResult(ERR_DRIVERMANAGER, QObject::tr("Could not load database driver \"%1\".").arg(id));
        m_result.setServerMessage(loader.errorString());
        drivermanagerDebug() << loader.instance() << loader.errorString();
//! @todo
/*        if (m_componentLoadingErrors.isEmpty()) {//fill errtable on demand
            m_componentLoadingErrors[KLibLoader::ErrNoServiceFound] = "ErrNoServiceFound";
            m_componentLoadingErrors[KLibLoader::ErrServiceProvidesNoLibrary] = "ErrServiceProvidesNoLibrary";
            m_componentLoadingErrors[KLibLoader::ErrNoLibrary] = "ErrNoLibrary";
            m_componentLoadingErrors[KLibLoader::ErrNoFactory] = "ErrNoFactory";
            m_componentLoadingErrors[KLibLoader::ErrNoComponent] = "ErrNoComponent";
        }
        m_serverResultName = m_componentLoadingErrors[m_serverResultNum];*/
        return 0;
    }

    drv->setInfo( info );
    m_drivers.insert(info.name().toLower(), drv); //cache it
    return drv;
}
#endif

// ---------------------------
// --- KDbDriverManager impl. ---
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

KDbResultable KDbDriverManager::resultable() const
{
    return KDbResultable(static_cast<const KDbResultable&>(*s_self));
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
