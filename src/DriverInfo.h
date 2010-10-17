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
        QString name, caption, comment, absoluteFilePath;
        QStringList mimeTypes;
        QString version; //!< x.y major+minor version
        bool fileBased;
        bool importingAllowed;
    };

    //! Constructs an invalid info.
    DriverInfo() : d( new Data() ) {}

    //! @return true if the info is valid. Valid info provides at least name and file path.
    //! @since 2.0
    bool isValid() const { return !d->name.isEmpty() && !d->absoluteFilePath.isEmpty(); }

    //! @return name of the driver. It is never empty.
    QString name() const { return d->name; }

    //! Sets name of the driver. Must not be empty.
    void setName(const QString& name) { d->name = name; }

    //! @return caption of the driver (translated name).
    QString caption() const { return d->caption; }

    //! Sets caption of the driver (translated name).
    void setCaption(const QString& caption) { d->caption = caption; }

    //! @return comment for driver (translated).
    QString comment() const { return d->comment; }

    //! Sets comment for driver (translated).
    void setComment(const QString& comment) { d->comment = comment; }

    //! @return version of the driver, e.g. "1.0".
    QString version() const { return d->version; }

    //! Sets comment for driver (translated).
    void setVersion(const QString& version) { d->version = version; }

    //! @return list of mime types supported by the driver.
    /*! This is only used for file-based databases like SQLite.
        The first element is the main mime type, the others are extra mimetypes acceptable,
        e.g. through importing or usable with with limited feature set. */
    QStringList mimeTypes() const { return d->mimeTypes; }

    //! Sets list of mime types supported by the driver.
    //! @see mimeTypes()
    void setMimeTypes(const QStringList& mimeTypes) { d->mimeTypes = mimeTypes; }

    //! @return absolute path for the driver plugin binary.
    /*! Its filename extension is operating system dependent ("so" for Unices, dll for Windows).
        Its prefix is "predicate_". */
    QString absoluteFilePath() const { return d->absoluteFilePath; }

    //! Sets absolute path for the driver plugin binary.
    //! @see absoluteFilePath()
    void setAbsoluteFilePath(const QString& absoluteFilePath) { d->absoluteFilePath = absoluteFilePath; }

    //! @return true if the driver is for file-based databases like SQLite.
    bool isFileBased() const { return d->fileBased; }

    //! Sets is-file-based flag for the driver.
    void setFileBased(bool set) { d->fileBased = set; }

    /*! @return true if the driver is for a backend that allows importing.
        Defined by AllowImporting field in "predicate_*.desktop" information files.
        Used for migration. */
    bool isImportingAllowed() const { return d->importingAllowed; }

    //! Sets importing-allowed flag for the driver.
    //! @see isImportingAllowed()
    void setImportingAllowed(bool set) { d->importingAllowed = set; }

private:
    QSharedDataPointer<Data> d;
};

} //namespace Predicate

#endif
