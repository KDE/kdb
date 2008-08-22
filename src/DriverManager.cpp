/* This file is part of the KDE project
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003-2006 Jarosław Staniek <staniek@kde.org>

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
#include "Driver.h"
#include "Driver_p.h"
#include "Error.h"

//#include <klibloader.h>
//#include <kservicetypetrader.h>
//#include <kdebug.h>
//#include <klocale.h>
//#include <kservice.h>

#include <assert.h>

#include <QApplication>
#include <QDir>
#include <QPluginLoader>
#include <QSettings>
#include <QtDebug>

//remove debug
#undef PreDbg
#define PreDbg if (0) qDebug()

using namespace Predicate;

DriverManagerInternal* DriverManagerInternal::s_self = 0L;


DriverManagerInternal::DriverManagerInternal() /* protected */
        : QObject(0)
        , Object()
        , m_refCount(0)
        , lookupDriversNeeded(true)
{
    setObjectName("Predicate::DriverManager");
    m_serverResultNum = 0;
}

DriverManagerInternal::~DriverManagerInternal()
{
    PreDbg << "DriverManagerInternal::~DriverManagerInternal()";
    qDeleteAll(m_drivers);
    m_drivers.clear();
    if (s_self == this)
        s_self = 0;
    PreDbg << "DriverManagerInternal::~DriverManagerInternal() ok";
}

void DriverManagerInternal::slotAppQuits()
{
    if (qApp && !qApp->topLevelWidgets().isEmpty()
            && qApp->topLevelWidgets().first()->isVisible()) {
        return; //what a hack! - we give up when app is still there
    }
    PreDbg << "DriverManagerInternal::slotAppQuits(): let's clear drivers...";
    qDeleteAll(m_drivers);
    m_drivers.clear();
}

DriverManagerInternal *DriverManagerInternal::self()
{
    if (!s_self)
        s_self = new DriverManagerInternal();

    return s_self;
}

/*! Find driver's directory.
 First, checks for existence of 'plugins' subdirectory.
 Then reads "Plugins" entry from ~/predicate.ini (Windows) or ~/.predicate (Unix) files.
*/
static QString findPluginsDir()
{
    QString pluginsDir;
    QString fname(
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
    return QString::null;
}

bool DriverManagerInternal::lookupDrivers()
{
    if (!lookupDriversNeeded)
        return true;

    if (qApp) {
        connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotAppQuits()));
    }
//TODO: for QT-only version check for KComponentData wrapper
//  PreWarn << "DriverManagerInternal::lookupDrivers(): cannot work without KComponentData (KGlobal::mainComponent()==0)!";
//  setError("Driver Manager cannot work without KComponentData (KGlobal::mainComponent()==0)!");

    lookupDriversNeeded = false;
    clearError();

    m_pluginsDir = findPluginsDir();
    if (m_pluginsDir.isEmpty()) {
        setError(ERR_DRIVERMANAGER, QObject::tr("Could not find directory for database drivers.") );
        return false;
    }
    
    QDir dir(m_pluginsDir, "predicate_*.desktop");
    const QFileInfoList infoFiles( dir.entryInfoList(QDir::Files|QDir::Readable) );
    foreach (const QFileInfo& infoFile, infoFiles) {
        QSettings config(infoFile.absoluteFilePath(), QSettings::IniFormat);
        config.beginGroup("Desktop Entry");
        Driver::Info info;
        info.setName( config.value("DriverName").toString() );
        info.setFileName( config.value("FileName").toString() );
        info.setCaption( config.value("Name").toString() );
//js TODO read translated [..]
        info.setComment( config.value("Comment").toString() );
        if (info.caption().isEmpty())
            info.setCaption( info.name() );
        info.setFileBased( config.value("DriverType", "Network").toString().toLower()=="file" );
        if (info.isFileBased())
            info.setFileDBMimeType( config.value("FileDBDriverMime").toString().toLower() );
        info.setImportingAllowed( config.value("AllowImporting", true).toBool() );
        m_driversInfo.insert(info.name().toLower(), info);
    }


#if 0 // todo?
    KService::List tlist = KServiceTypeTrader::self()->query("Kexi/DBDriver");
    KService::List::ConstIterator it(tlist.constBegin());
    for (; it != tlist.constEnd(); ++it) {
        KService::Ptr ptr = (*it);
        if (!ptr->property("Library").toString().startsWith("predicate_")) {
            PreWarn << "DriverManagerInternal::lookupDrivers():"
            " X-KDE-Library == " << ptr->property("Library").toString()
            << ": no \"predicate_\" prefix -- skipped to avoid potential conflicts!";
            continue;
        }
        QString srv_name = ptr->property("X-Kexi-DriverName").toString().toLower();
        if (srv_name.isEmpty()) {
            PreWarn << "DriverManagerInternal::lookupDrivers():"
            " X-Kexi-DriverName must be set for Predicate driver \""
            << ptr->property("Name").toString() << "\" service!\n -- skipped!";
            continue;
        }
        if (m_services_lcase.contains(srv_name)) {
            PreWarn << "DriverManagerInternal::lookupDrivers(): more than one driver named '"
            << srv_name << "'\n -- skipping this one!";
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
            PreWarn << "DriverManagerInternal::lookupDrivers(): problem with detecting '"
            << srv_name << "' driver's version -- skipping it!";
            continue;
        }
        if (major_ver != Predicate::version().major || minor_ver != Predicate::version().minor) {
            PreWarn << QString("DriverManagerInternal::lookupDrivers(): '%1' driver"
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
                    PreWarn << "DriverManagerInternal::lookupDrivers(): more than one driver for '"
                    << mime << "' mime type!";
                }
            }
        }
        m_services.insert(srv_name, ptr);
        m_services_lcase.insert(srv_name,  ptr);
        PreDbg << "Predicate::DriverManager::lookupDrivers(): registered driver: "
        << ptr->name() << "(" << ptr->library() << ")";
    }
#endif
    if (m_driversInfo.isEmpty()) {
        setError(ERR_DRIVERMANAGER, QObject::tr("Could not find any database drivers."));
        return false;
    }
    return true;
}

Predicate::Driver::Info DriverManagerInternal::driverInfo(const QString &name)
{
    Predicate::Driver::Info i = m_driversInfo[name.toLower()];
    if (!error() && i.isValid())
        setError(ERR_DRIVERMANAGER, QObject::tr("Could not find database driver \"%1\".").arg(name));
    return i;
}

Driver* DriverManagerInternal::driver(const QString& name)
{
    if (!lookupDrivers())
        return 0;

    clearError();
    PreDbg << "DriverManagerInternal::driver(): loading " << name;

    Driver *drv = 0;
    if (!name.isEmpty())
        drv = m_drivers.value(name.toLower());
    if (drv)
        return drv; //cached

    if (!m_driversInfo.contains(name.toLower())) {
        setError(ERR_DRIVERMANAGER, QObject::tr("Could not find database driver \"%1\".").arg(name));
        return 0;
    }

    const Driver::Info info = m_driversInfo[name.toLower()];

    QPluginLoader loader(m_pluginsDir + "/bin/" + info.fileName()
#if defined Q_WS_WIN && (defined(_DEBUG) || defined(DEBUG))
        + "_d"
#endif
    );
    drv = qobject_cast<Driver*>(loader.instance());
    if (!drv) {
        setError(ERR_DRIVERMANAGER, QObject::tr("Could not load database driver \"%1\".").arg(name));
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
        setError(drv);
        delete drv;
        return 0;
    }
    m_drivers.insert(info.name().toLower(), drv); //cache it
    return drv;
}

void DriverManagerInternal::incRefCount()
{
    m_refCount++;
    PreDbg << "DriverManagerInternal::incRefCount(): " << m_refCount;
}

void DriverManagerInternal::decRefCount()
{
    m_refCount--;
    PreDbg << "DriverManagerInternal::decRefCount(): " << m_refCount;
// if (m_refCount<1) {
//  PreDbg<<"Predicate::DriverManagerInternal::decRefCount(): reached m_refCount<1 -->deletelater()"<<endl;
//  s_self=0;
//  deleteLater();
// }
}

void DriverManagerInternal::aboutDelete(Driver* drv)
{
    m_drivers.remove(drv->name());
}



// ---------------------------
// --- DriverManager impl. ---
// ---------------------------

DriverManager::DriverManager()
        : QObject(0)
        , Object()
        , d_int(DriverManagerInternal::self())
{
    setObjectName("Predicate::DriverManager");
    d_int->incRefCount();
// if ( !s_self )
//  s_self = this;
// lookupDrivers();
}

DriverManager::~DriverManager()
{
    PreDbg << "DriverManager::~DriverManager()";
    /* Connection *conn;
      for ( conn = m_connections.first(); conn ; conn = m_connections.next() ) {
        conn->disconnect();
        conn->m_driver = 0; //don't let the connection touch our driver now
        m_connections.remove();
        delete conn;
      }*/

    d_int->decRefCount();
    if (d_int->refCount() == 0) {
        //delete internal drv manager!
        delete d_int;
    }
// if ( s_self == this )
    //s_self = 0;
    PreDbg << "DriverManager::~DriverManager() ok";
}

Predicate::Driver::Info::Map DriverManager::driversInfo()
{
    if (!d_int->lookupDrivers())
        return Predicate::Driver::Info::Map();

    return d_int->m_driversInfo;
}

const QStringList DriverManager::driverNames()
{
    if (!d_int->lookupDrivers())
        return QStringList();

    if (d_int->m_driversInfo.isEmpty() && d_int->error())
        return QStringList();
    return d_int->m_driversInfo.keys();
}

Predicate::Driver::Info DriverManager::driverInfo(const QString &name)
{
    d_int->lookupDrivers();
    Predicate::Driver::Info i = d_int->driverInfo(name);
    if (d_int->error())
        setError(d_int);
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
    if (!d_int->lookupDrivers()) {
        setError(d_int);
        return 0;
    }

    const Driver::Info info( d_int->m_infos_by_mimetype[mimeType.toLower()] );
    if (!info.isValid())
        return QString();
    return info.name();
}

Driver* DriverManager::driver(const QString& name)
{
    Driver *drv = d_int->driver(name);
    if (d_int->error())
        setError(d_int);
    return drv;
}

QString DriverManager::serverErrorMsg()
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
}

QString DriverManager::possibleProblemsInfoMsg() const
{
    if (d_int->possibleProblems.isEmpty())
        return QString();
    QString str;
    str.reserve(1024);
    str = QLatin1String("<ul>");
    foreach (const QString& problem, d_int->possibleProblems)
        str += (QLatin1String("<li>") + problem + QLatin1String("</li>"));
    str += QLatin1String("</ul>");
    return str;
}
