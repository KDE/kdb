/* This file is part of the KDE project
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>

   Portions of kstandarddirs.cpp:
   Copyright (C) 1999 Sirtaj Singh Kang <taj@kde.org>
   Copyright (C) 1999,2007 Stephan Kulow <coolo@kde.org>
   Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2009 David Faure <faure@kde.org>

   Portions of kshell.cpp:
   Copyright (c) 2003,2007 Oswald Buddenhagen <ossi@kde.org>

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

#include "KDb.h"
#include "KDbConnection.h"
#include "KDbDriverManager.h"
#include "KDbUtils.h"
#include "kdb_debug.h"
#include "config-kdb.h"

#include <QRegularExpression>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#ifdef _WIN32_WCE
#include <basetyps.h>
#endif
#ifdef Q_OS_WIN64
//! @todo did not find a reliable way to fix with kdewin mingw header
#define interface struct
#endif
#endif

using namespace KDbUtils;

void KDbUtils::serializeMap(const QMap<QString, QString>& map, QByteArray *array)
{
    Q_ASSERT(array);
    QDataStream ds(array, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds << map;
}

void KDbUtils::serializeMap(const QMap<QString, QString>& map, QString *string)
{
    Q_ASSERT(string);
    QByteArray array;
    QDataStream ds(&array, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds << map;
    kdbDebug() << array[3] << array[4] << array[5];
    const int size = array.size();
    string->clear();
    string->reserve(size);
    for (int i = 0; i < size; i++) {
        (*string)[i] = QChar(ushort(array[i]) + 1);
    }
}

QMap<QString, QString> KDbUtils::deserializeMap(const QByteArray& array)
{
    QMap<QString, QString> map;
    QByteArray ba(array);
    QDataStream ds(&ba, QIODevice::ReadOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds >> map;
    return map;
}

QMap<QString, QString> KDbUtils::deserializeMap(const QString& string)
{
    QByteArray array;
    const int size = string.length();
    array.resize(size);
    for (int i = 0; i < size; i++) {
        array[i] = char(string[i].unicode() - 1);
    }
    QMap<QString, QString> map;
    QDataStream ds(&array, QIODevice::ReadOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds >> map;
    return map;
}

QString KDbUtils::stringToFileName(const QString& string)
{
    QString _string(string);
    _string.replace(QRegularExpression(QLatin1String("[\\\\/:\\*?\"<>|]")), QLatin1String(" "));
    return _string.simplified();
}

void KDbUtils::simpleCrypt(QString *string)
{
    Q_ASSERT(string);
    for (int i = 0; i < string->length(); i++) {
        ushort& unicode = (*string)[i].unicode();
        unicode += (47 + i);
    }
}

bool KDbUtils::simpleDecrypt(QString *string)
{
    Q_ASSERT(string);
    QString result(*string);
    for (int i = 0; i < result.length(); i++) {
        ushort& unicode = result[i].unicode();
        if (unicode <= (47 + i)) {
            return false;
        }
        unicode -= (47 + i);
    }
    *string = result;
    return true;
}

QString KDbUtils::pointerToStringInternal(void* pointer, int size)
{
    QString string;
    unsigned char* cstr_pointer = (unsigned char*) & pointer;
    for (int i = 0; i < size; i++) {
        QString s;
        s.sprintf("%2.2x", cstr_pointer[i]);
        string.append(s);
    }
    return string;
}

void* KDbUtils::stringToPointerInternal(const QString& string, int size)
{
    if ((string.length() / 2) < size)
        return nullptr;
    QByteArray array;
    array.resize(size);
    bool ok;
    for (int i = 0; i < size; i++) {
        array[i] = (unsigned char)(string.mid(i * 2, 2).toUInt(&ok, 16));
        if (!ok)
            return nullptr;
    }
    return static_cast<void*>(array.data());
}

//---------

//! @internal
class StaticSetOfStrings::Private
{
public:
    Private() : array(0), set(0) {}
    ~Private() {
        delete set;
    }
    const char* const * array;
    QSet<QByteArray> *set;
};

StaticSetOfStrings::StaticSetOfStrings()
        : d(new Private)
{
}

StaticSetOfStrings::StaticSetOfStrings(const char* const array[])
        : d(new Private)
{
    setStrings(array);
}

StaticSetOfStrings::~StaticSetOfStrings()
{
    delete d;
}

void StaticSetOfStrings::setStrings(const char* const array[])
{
    delete d->set;
    d->set = 0;
    d->array = array;
}

bool StaticSetOfStrings::isEmpty() const
{
    return d->array == 0;
}

bool StaticSetOfStrings::contains(const QByteArray& string) const
{
    if (!d->set) {
        d->set = new QSet<QByteArray>();
        for (const char * const * p = d->array;*p;p++)
            d->set->insert(QByteArray::fromRawData(*p, qstrlen(*p)));
    }
    return d->set->contains(string);
}

//---------

#ifdef Q_OS_MAC
//! Internal, from kdelibs' kstandarddirs.cpp
static QString getBundle(const QString& path, bool ignore)
{
    QFileInfo info;
    QString bundle = path;
    bundle += QLatin1String(".app/Contents/MacOS/") + bundle.section(QLatin1Char('/'), -1);
    info.setFile( bundle );
    FILE *file;
    if (file = fopen(info.absoluteFilePath().toUtf8().constData(), "r")) {
        fclose(file);
        struct stat _stat;
        if ((stat(info.absoluteFilePath().toUtf8().constData(), &_stat)) < 0) {
            return QString();
        }
        if ( ignore || (_stat.st_mode & S_IXUSR) ) {
            if ( ((_stat.st_mode & S_IFMT) == S_IFREG) || ((_stat.st_mode & S_IFMT) == S_IFLNK) ) {
                return bundle;
            }
        }
    }
    return QString();
}
#endif

//! Internal, from kdelibs' kstandarddirs.cpp
static QString checkExecutable(const QString& path, bool ignoreExecBit)
{
#ifdef Q_OS_MAC
    QString bundle = getBundle(path, ignoreExecBit);
    if (!bundle.isEmpty()) {
        return bundle;
    }
#endif
    QFileInfo info(path);
    QFileInfo orig = info;
#if defined(Q_OS_DARWIN) || defined(Q_OS_MAC)
    FILE *file;
    if (file = fopen(orig.absoluteFilePath().toUtf8().constData(), "r")) {
        fclose(file);
        struct stat _stat;
        if ((stat(orig.absoluteFilePath().toUtf8().constData(), &_stat)) < 0) {
            return QString();
        }
        if ( ignoreExecBit || (_stat.st_mode & S_IXUSR) ) {
            if ( ((_stat.st_mode & S_IFMT) == S_IFREG) || ((_stat.st_mode & S_IFMT) == S_IFLNK) ) {
                orig.makeAbsolute();
                return orig.filePath();
            }
        }
    }
    return QString();
#else
    if (info.exists() && info.isSymLink())
        info = QFileInfo(info.canonicalFilePath());
    if (info.exists() && ( ignoreExecBit || info.isExecutable() ) && info.isFile()) {
        // return absolute path, but without symlinks resolved in order to prevent
        // problems with executables that work differently depending on name they are
        // run as (for example gunzip)
        orig.makeAbsolute();
        return orig.filePath();
    }
    return QString();
#endif
}

//! Internal, from kdelibs' kstandarddirs.cpp
#if defined _WIN32 || defined _WIN64
# define KPATH_SEPARATOR ';'
# define ESCAPE '^'
#else
# define KPATH_SEPARATOR ':'
# define ESCAPE '\\'
#endif

//! Internal, from kdelibs' kstandarddirs.cpp
static inline QString equalizePath(QString &str)
{
#ifdef Q_OS_WIN
    // filter pathes through QFileInfo to have always
    // the same case for drive letters
    QFileInfo f(str);
    if (f.isAbsolute())
        return f.absoluteFilePath();
    else
#endif
        return str;
}

//! Internal, from kdelibs' kstandarddirs.cpp
static void tokenize(QStringList& tokens, const QString& str,
                     const QString& delim)
{
    const int len = str.length();
    QString token;

    for(int index = 0; index < len; index++) {
        if (delim.contains(str[index])) {
            tokens.append(equalizePath(token));
            token.clear();
        } else {
            token += str[index];
        }
    }
    if (!token.isEmpty()) {
        tokens.append(equalizePath(token));
    }
}

//! Internal, based on kdelibs' kshell.cpp
static QString tildeExpand(const QString &fname)
{
    if (!fname.isEmpty() && fname[0] == QLatin1Char('~')) {
        int pos = fname.indexOf( QLatin1Char('/') );
        QString ret = QDir::homePath(); // simplified
        if (pos > 0) {
            ret += fname.mid(pos);
        }
        return ret;
    } else if (fname.length() > 1 && fname[0] == QLatin1Char(ESCAPE) && fname[1] == QLatin1Char('~')) {
        return fname.mid(1);
    }
    return fname;
}

//! Internal, from kdelibs' kstandarddirs.cpp
static QStringList systemPaths(const QString& pstr)
{
    QStringList tokens;
    QString p = pstr;

    if (p.isEmpty()) {
        p = QString::fromLocal8Bit( qgetenv( "PATH" ) );
    }

    QString delimiters(QLatin1Char(KPATH_SEPARATOR));
    delimiters += QLatin1Char('\b');
    tokenize(tokens, p, delimiters);

    QStringList exePaths;

    // split path using : or \b as delimiters
    for(int i = 0; i < tokens.count(); i++) {
        exePaths << tildeExpand(tokens[ i ]);
    }
    return exePaths;
}

//! Internal, from kdelibs' kstandarddirs.cpp
#ifdef Q_OS_WIN
static QStringList executableExtensions()
{
    QStringList ret = QString::fromLocal8Bit(qgetenv("PATHEXT")).split(QLatin1Char(';'));
    if (!ret.contains(QLatin1String(".exe"), Qt::CaseInsensitive)) {
        // If %PATHEXT% does not contain .exe, it is either empty, malformed, or distorted in ways that we cannot support, anyway.
        ret.clear();
        ret << QLatin1String(".exe")
            << QLatin1String(".com")
            << QLatin1String(".bat")
            << QLatin1String(".cmd");
    }
    return ret;
}
#endif

//! Based on kdelibs' kstandarddirs.cpp
QString KDbUtils::findExe(const QString& appname,
                                  const QString& path,
                                  FindExeOptions options)
{
#ifdef Q_OS_WIN
    QStringList executable_extensions = executableExtensions();
    if (!executable_extensions.contains(
            appname.section(QLatin1Char('.'), -1, -1, QString::SectionIncludeLeadingSep),
            Qt::CaseInsensitive))
    {
        QString found_exe;
        foreach (const QString& extension, executable_extensions) {
            found_exe = findExe(appname + extension, path, options);
            if (!found_exe.isEmpty()) {
                return found_exe;
            }
        }
        return QString();
    }
#endif
    QFileInfo info;

    // absolute or relative path?
    if (appname.contains(QDir::separator())) {
        return checkExecutable(appname, options & IgnoreExecBit);
    }

    QString p;
    QString result;

    const QStringList exePaths = systemPaths(path);
    for (QStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); ++it)
    {
        p = (*it) + QLatin1Char('/');
        p += appname;

        // Check for executable in this tokenized path
        result = checkExecutable(p, options & IgnoreExecBit);
        if (!result.isEmpty()) {
            return result;
        }
    }

    // Not found in PATH, look into a bin dir
    p = QFile::decodeName(BIN_INSTALL_DIR "/");
    p += appname;
    result = checkExecutable(p, options & IgnoreExecBit);
    if (!result.isEmpty()) {
        return result;
    }

    // If we reach here, the executable wasn't found.
    // So return empty string.
    return QString();
}

// ---

class PropertySet::Private
{
public:
    AutodeletedHash<QByteArray, Property*> data;
};

PropertySet::PropertySet()
 : d(new Private)
{
}

PropertySet::PropertySet(const PropertySet &other)
    : d(new Private)
{
    for (AutodeletedHash<QByteArray, Property*>::ConstIterator it(other.d->data.constBegin());
         it != other.d->data.constEnd(); ++it)
    {
        d->data.insert(it.key(), new Property(*it.value()));
    }
}

PropertySet::~PropertySet()
{
    delete d;
}

void PropertySet::insert(const QByteArray &name, const QVariant &value, const QString &caption)
{
    QString realCaption = caption;
    Property *existing = d->data.value(name);
    if (existing) {
        existing->isNull = false;
        existing->value = value;
        if (!caption.isEmpty()) { // if not, reuse
            existing->caption = caption;
        }
    } else {
        d->data.insert(name, new Property(value, realCaption));
    }
}

void PropertySet::remove(const QByteArray &name)
{
    d->data.remove(name);
}

Property PropertySet::property(const QByteArray &name) const
{
    Property *result = d->data.value(name);
    return result ? *result : Property();
}

QList<QByteArray> PropertySet::names() const
{
    return d->data.keys();
}
