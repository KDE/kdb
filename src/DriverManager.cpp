/* This file is part of the KDE project
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

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

#include "DriverManager.h"
#include "DriverManager_p.h"
#include "Driver_p.h"
#include "Driver_p.h"
#include "tools/Static.h"

#include <assert.h>

#include <QObject>
#include <QApplication>
#include <QDir>
#include <QPluginLoader>
#include <QSettings>
#include <QtDebug>

//remove debug
#undef PreDbg
#define PreDbg if (0) qDebug()

using namespace Predicate;

PREDICATE_GLOBAL_STATIC(DriverManagerInternal, s_self);

DriverManagerInternal::DriverManagerInternal() /* protected */
 : lookupDriversNeeded(true)
{
//pred    m_serverResultNum = 0;
}

DriverManagerInternal::~DriverManagerInternal()
{
    PreDbg;
    qDeleteAll(m_drivers);
    m_drivers.clear();
    m_driverWeakPointers.clear();
//    if (s_self == this)
//        s_self = 0;
    PreDbg << "ok";
}

void DriverManagerInternal::slotAppQuits()
{
    if (qApp && !qApp->topLevelWidgets().isEmpty()
            && qApp->topLevelWidgets().first()->isVisible()) {
        return; //what a hack! - we give up when app is still there
    }
    PreDbg << "let's clear drivers...";
    qDeleteAll(m_drivers);
    m_drivers.clear();
    m_driverWeakPointers.clear();
}

//static
DriverManagerInternal *DriverManagerInternal::self()
{
    return s_self;
}

static QString findPluginsDir()
{
/*    QString fname(
#ifdef Q_WS_WIN
        "predicate.ini"
#else
        ".predicate"
#endif
    );
    if (qApp) {
        pluginsDir = qApp->applicationDirPath() + "/plugins";
        if (QDir(pluginsDir).exists())
            return pluginsDir;
    }
    if (QFileInfo(QDir::homePath() + "/" + fname).isReadable()) {
        // we have a config file - try to find plugins dir here
        QSettings config(QDir::homePath() + "/" + fname, QSettings::IniFormat);
        pluginsDir = config.value("Plugins").toString();
        if (QDir(pluginsDir).exists())
            return pluginsDir;
    }
*/
//! @todo try in XDG_* http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html ?
    foreach (const QString& path, qApp->libraryPaths()) {
        const QString pluginsDir(path + "/predicate");
        if (QDir(pluginsDir).exists() && QDir(pluginsDir).isReadable())
            return pluginsDir;
    }
    return QString();
}

void DriverManagerInternal::lookupDriversForDirectory(const QString& pluginsDir)
{
    QDir dir(pluginsDir, "predicate_*.desktop");
    const QFileInfoList infoFiles( dir.entryInfoList(QDir::Files|QDir::Readable) );
    foreach (const QFileInfo& infoFile, infoFiles) {
        QSettings config(infoFile.absoluteFilePath(), QSettings::IniFormat);
        config.beginGroup("Desktop Entry");
        DriverInfo info;
        info.setName( config.value("Name").toString().toLower() );
        if (m_driversInfo.contains(info.name())) {
            qWarning() << "More than one driver named" << info.name() << "-- skipping this one";
            continue;
        }
        info.setVersion( config.value("Version").toString() );
        const QString expectedVersion(QString("%1.%2").arg(Predicate::version().major()).arg(Predicate::version().minor()));
        if (expectedVersion != info.version()) {
            qWarning() << "Incompatible database driver's" << info.name()
                << "version: found version" << info.version() << "expected version" << expectedVersion
                << "-- skipping this one";
        }
#ifdef Q_OS_WIN
        QString suffix = ".dll";
#else
        QString suffix = ".so";
#endif
        info.setAbsoluteFilePath( pluginsDir + "/predicate_" + config.value("Name").toString() + suffix );
        info.setCaption( config.value("Caption").toString() );
//! @todo read translated [..]
        info.setComment( config.value("Comment").toString() );
        if (info.caption().isEmpty())
            info.setCaption( info.name() );
        info.setFileBased( config.value("Type", "Network").toString().toLower()=="file" );
        if (info.isFileBased())
            info.setFileDBMimeType( config.value("MimeType").toString().toLower() );
        info.setImportingAllowed( config.value("AllowImporting", true).toBool() );
        m_driversInfo.insert(info.name().toLower(), info);
    }
}

bool DriverManagerInternal::lookupDrivers()
{
    if (!lookupDriversNeeded)
        return true;

    if (qApp) {
        connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotAppQuits()));
    }
//TODO: for Qt-only version check for KComponentData wrapper
//  PreWarn << "cannot work without KComponentData (KGlobal::mainComponent()==0)!";
//  setError("Driver Manager cannot work without KComponentData (KGlobal::mainComponent()==0)!");

    lookupDriversNeeded = false;
    clearResult();

    /*! Try in all possible driver directories.
     Looks for "predicate" directory in $INSTALL/plugins, $QT_PLUGIN_PATH, and directory of the application executable.
     Plugin path "Plugins" entry can be added to qt.conf to override; see http://doc.trolltech.com/4.6/qt-conf.html.
    */
    qDebug() << "qApp->libraryPaths():" << qApp->libraryPaths();
    bool foundAtLeastOne = false;
    foreach (const QString& path, qApp->libraryPaths()) {
        const QString pluginsDir(path + "/predicate");
        if (QDir(pluginsDir).exists() && QDir(pluginsDir).isReadable()) {
            foundAtLeastOne = true;
            lookupDriversForDirectory(pluginsDir);
        }
    }
    if (!foundAtLeastOne) {
        m_result = Result(ERR_DRIVERMANAGER, QObject::tr("Could not find directory for database drivers."));
        return false;
    }

#if 0 // todo?
    KService::List tlist = KServiceTypeTrader::self()->query("Kexi/DBDriver");
    KService::List::ConstIterator it(tlist.constBegin());
    for (; it != tlist.constEnd(); ++it) {
        KService::Ptr ptr = (*it);
        if (!ptr->property("Library").toString().startsWith("predicate_")) {
            PreWarn << "X-KDE-Library == " << ptr->property("Library").toString()
                << ": no \"predicate_\" prefix -- skipped to avoid potential conflicts!";
            continue;
        }
        QString srv_name = ptr->property("X-Kexi-DriverName").toString().toLower();
        if (srv_name.isEmpty()) {
            PreWarn << "X-Kexi-DriverName must be set for Predicate driver \""
                << ptr->property("Name").toString() << "\" service!\n -- skipped!";
            continue;
        }
        if (m_services_lcase.contains(srv_name)) {
            PreWarn << "more than one driver named" << srv_name << "\n -- skipping this one!";
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
            PreWarn << "problem with detecting" << srv_name << "driver's version -- skipping it!";
            continue;
        }

        if (!Predicate::version().matches(major_ver, minor_ver)) {
            PreWarn << QString("'%1' driver"
                               " has version '%2' but required Predicate driver version is '%3.%4'\n"
                               " -- skipping this driver!").arg(srv_name).arg(srv_ver_str)
                              .arg(Predicate::version().major).arg(Predicate::version().minor);
            possibleProblems += QString("\"%1\" database Driver.has version \"%2\" "
                                        "but required driver version is \"%3.%4\"")
                                .arg(srv_name).arg(srv_ver_str)
                                .arg(Predicate::version().major).arg(Predicate::version().minor);
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
                    PreWarn << "more than one driver for" << mime << "m ime type!";
                }
            }
        }
        m_services.insert(srv_name, ptr);
        m_services_lcase.insert(srv_name,  ptr);
        PreDbg << "registered driver: " << ptr->name() << "(" << ptr->library() << ")";
    }
#endif
    if (m_driversInfo.isEmpty()) {
        m_result = Result(ERR_DRIVERMANAGER, QObject::tr("Could not find any database drivers."));
        return false;
    }
    return true;
}

DriverInfo DriverManagerInternal::driverInfo(const QString &name)
{
    DriverInfo i = m_driversInfo.value(name.toLower());
    if (!m_result.isError() && i.isValid())
        m_result = Result(ERR_DRIVERMANAGER, QObject::tr("Could not find database driver \"%1\".").arg(name));
    return i;
}

struct LibUnloader {
    LibUnloader(QLibrary *lib) : m_lib(lib) {}
    ~LibUnloader() {
        if (m_lib->isLoaded())
            m_lib->unload();
    }
private:
    QLibrary *m_lib;
};

Driver* DriverManagerInternal::driver(const QString& name)
{
    if (!lookupDrivers())
        return 0;

    clearResult();
    PreDbg << "loading" << name;

    Driver *drv = 0;
    if (!name.isEmpty()) {
        QSharedPointer<Driver> *dp = m_drivers.value(name.toLower());
        if (dp && !dp->isNull())
            drv = dp->data();
    }
    if (drv)
        return drv; //cached

    if (!m_driversInfo.contains(name.toLower())) {
        m_result = Result(ERR_DRIVERMANAGER, QObject::tr("Could not find database driver \"%1\".").arg(name));
        return 0;
    }

    const DriverInfo info(m_driversInfo.value(name.toLower()));

    QString libFileName(info.absoluteFilePath());
#if defined Q_OS_WIN && (defined(_DEBUG) || defined(DEBUG))
    libFileName += "_d";
#endif
    QLibrary lib(libFileName);
    LibUnloader unloader(&lib);
    qDebug() << libFileName;
    if (!lib.load()) {
        m_result = Result(ERR_DRIVERMANAGER, QObject::tr("Could not load library \"%1\".").arg(name));
        m_result.setServerMessage(lib.errorString());
        qDebug() << lib.errorString();
        return 0;
    }
    
    const uint* foundMajor = (const uint*)lib.resolve("version_major");
    if (!foundMajor) {
       m_result = Result(ERR_DRIVERMANAGER, QObject::tr("Could not find \"%1\" entry point of library \"%2\".").arg("version_major").arg(name));
       return 0;
    }
    const uint* foundMinor = (const uint*)lib.resolve("version_minor");
    if (!foundMinor) {
       m_result = Result(ERR_DRIVERMANAGER, QObject::tr("Could not find \"%1\" entry point of library \"%2\".")
                         .arg("version_minor").arg(name));
       return 0;
    }
    if (!Predicate::version().matches(*foundMajor, *foundMinor)) {
        m_result = Result(ERR_INCOMPAT_DRIVER_VERSION,
            QObject::tr("Incompatible database driver's \"%1\" version: found version %2, expected version %3.")
                 .arg(name)
                 .arg(QString("%1.%2").arg(*foundMajor).arg(*foundMinor))
                 .arg(QString("%1.%2").arg(Predicate::version().major()).arg(Predicate::version().minor()))
            );
        return 0;
    }
    lib.unload();

    QPluginLoader loader(libFileName);
    drv = dynamic_cast<Driver*>(loader.instance());
    if (!drv) {
        m_result = Result(ERR_DRIVERMANAGER, QObject::tr("Could not load database driver \"%1\".").arg(name));
        m_result.setServerMessage(loader.errorString());
        qDebug() << loader.instance() << loader.errorString();
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

    if (!drv->isValid()) {
        m_result = drv->result();
        delete drv;
        return 0;
    }
    QSharedPointer<Driver>* ptr = new QSharedPointer<Driver>(drv);
    m_drivers.insert(info.name().toLower(), ptr); //cache it
    m_driverWeakPointers.insert(drv, ptr->toWeakRef());
    return drv;
}

/*void DriverManagerInternal::incRefCount()
{
    m_refCount++;
    PreDbg << m_refCount;
}

void DriverManagerInternal::decRefCount()
{
    m_refCount--;
    PreDbg << m_refCount;
// if (m_refCount<1) {
//  PreDbg<<"reached m_refCount<1 -->deletelater()";
//  s_self=0;
//  deleteLater();
// }
}*/

#warning is DriverManagerInternal::aboutDelete() needed?
/*2.0
void DriverManagerInternal::aboutDelete(Driver* drv)
{
    m_drivers.remove(drv->name().toLower());
}*/



// ---------------------------
// --- DriverManager impl. ---
// ---------------------------

DriverManager::DriverManager()
{
//    d_int->incRefCount();
// if ( !s_self )
//  s_self = this;
// lookupDrivers();
}

DriverManager::~DriverManager()
{
    PreDbg;
    /* Connection *conn;
      for ( conn = m_connections.first(); conn ; conn = m_connections.next() ) {
        conn->disconnect();
        conn->m_driver = 0; //don't let the connection touch our driver now
        m_connections.remove();
        delete conn;
      }*/

//    d_int->decRefCount();
/*    if (d_int->refCount() == 0) {
        //delete internal drv manager!
        delete d_int;
    }*/
// if ( s_self == this )
    //s_self = 0;
    PreDbg << "ok";
}

Result DriverManager::result() const {
    return s_self->result();
}

const Resultable& DriverManager::resultable() const
{
    return static_cast<const Resultable&>(*s_self);
}

DriverInfoMap DriverManager::driversInfo()
{
    if (!s_self->lookupDrivers())
        return DriverInfoMap();

    return s_self->m_driversInfo;
}

const QStringList DriverManager::driverNames()
{
    if (!s_self->lookupDrivers())
        return QStringList();

    if (s_self->m_driversInfo.isEmpty() && result().isError())
        return QStringList();
    return s_self->m_driversInfo.keys();
}

DriverInfo DriverManager::driverInfo(const QString &name)
{
    s_self->lookupDrivers();
    DriverInfo i = s_self->driverInfo(name);
    return i;
}

/*KService::Ptr DriverManager::serviceInfo(const QString &name)
{
    if (!d_int->lookupDrivers()) {
        setError(d_int);
        return KService::Ptr();
    }

    clearError();
    KService::Ptr ptr = d_int->m_services_lcase.value(name.toLower());
    if (ptr)
        return ptr;
    setError(ERR_DRIVERMANAGER, QObject::tr("No such driver service: \"%1\".").arg(name));
    return KService::Ptr();
}

const DriverManager::ServicesHash& DriverManager::services()
{
    d_int->lookupDrivers();
    return d_int->m_services;
}*/

QString DriverManager::lookupByMime(const QString &mimeType)
{
    if (!s_self->lookupDrivers()) {
        return 0;
    }

    const DriverInfo info(s_self->m_infos_by_mimetype.value(mimeType.toLower()));
    if (!info.isValid())
        return QString();
    return info.name();
}

Driver* DriverManager::driver(const QString& name)
{
    Driver *drv = s_self->driver(name);
    return drv;
}

/*QString DriverManager::serverErrorMsg()
{
    return d_int->m_serverErrMsg;
}

int DriverManager::serverResult()
{
    return d_int->m_serverResultNum;
}

QString DriverManager::serverResultName()
{
    return d_int->m_serverResultName;
}

void DriverManager::drv_clearServerResult()
{
    d_int->m_serverErrMsg.clear();
    d_int->m_serverResultNum = 0;
//    d_int->m_serverResultName.clear();
}*/

QString DriverManager::possibleProblemsInfoMsg() const
{
    if (s_self->possibleProblems.isEmpty())
        return QString();
    QString str;
    str.reserve(1024);
    str = QLatin1String("<ul>");
    foreach (const QString& problem, s_self->possibleProblems)
        str += (QLatin1String("<li>") + problem + QLatin1String("</li>"));
    str += QLatin1String("</ul>");
    return str;
}
