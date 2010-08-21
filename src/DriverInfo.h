/* This file is part of the KDE project
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef PREDICATE_DRIVERINFO_H
#define PREDICATE_DRIVERINFO_H

#include <QString>
#include <QSharedData>

//#include <Predicate/Global.h>
//#include <Predicate/Object.h>
//#include <Predicate/Field.h>

class QDateTime;

namespace Predicate
{

/*! Provides information about driver. */
class PREDICATE_EXPORT DriverInfo
{
public:
    struct Data : public QSharedData {
        Data()
        : fileBased(false)
        , importingAllowed(true) {}
        QString name, caption, comment, fileDBMimeType, absoluteFilePath;
        QString version; //!< x.y major+minor version
        bool fileBased;
        bool importingAllowed;
    };

    //! Constructs an invalid info.
    DriverInfo() : d( new Data() ) {}

    //! @return true if the info is valid. Valid info provides at least name and file path.
    //! @since 2.0
    bool isValid() const { return !d->name.isEmpty() && !d->absoluteFilePath.isEmpty(); }

    QString name() const { return d->name; }
    void setName(const QString& name) { d->name = name; }

    QString caption() const { return d->caption; }
    void setCaption(const QString& caption) { d->caption = caption; }

    QString comment() const { return d->comment; }
    void setComment(const QString& comment) { d->comment = comment; }

    QString version() const { return d->version; }
    void setVersion(const QString& version) { d->version = version; }

    QString fileDBMimeType() const { return d->fileDBMimeType; }
    void setFileDBMimeType(const QString& fileDBMimeType) { d->fileDBMimeType = fileDBMimeType; }

    QString absoluteFilePath() const { return d->absoluteFilePath; }
    void setAbsoluteFilePath(const QString& absoluteFilePath) { d->absoluteFilePath = absoluteFilePath; }

    //! @return true if the driver is for file-based database backend
    bool isFileBased() const { return d->fileBased; }
    void setFileBased(bool set) { d->fileBased = set; }

    /*! @return true if the driver is for a backend that allows importing.
        Defined by AllowImporting field in "predicate_*.desktop" information files.
        Used for migration. */
    bool isImportingAllowed() const { return d->importingAllowed; }
    void setImportingAllowed(bool set) { d->importingAllowed = set; }
private:
    QSharedDataPointer<Data> d;
};

typedef QMap<QString,DriverInfo> DriverInfoMap;

} //namespace Predicate

#endif
