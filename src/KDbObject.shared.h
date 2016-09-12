/* This file is part of the KDE project
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_OBJECT_H
#define KDB_OBJECT_H

#include "kdb_export.h"
#include "KDbGlobal.h"

/*! Provides common attributes for KDb objects: id, name, caption,
 help text. A KDb object is typically storable in database, for example:
 table schema or query schema.
 Default type of object is KDb::UnknownObjectType.
*/
class KDB_EXPORT KDbObject //SDC: operator== virtual_dtor
{
public:
    /*!
    @getter
    @return the type of this object.
    */
    int type; //SDC: default=KDb::UnknownObjectType no_setter

    /*!
    @getter
    @return the identifier of this object, default is -1.
    @setter
    Sets the identifier for this object.
    */
    int id; //SDC: default=-1

    /*!
    @getter
    @return the name of this object.
    @setter
    Sets the name for this object. It should be valid identifier,
    i.e. start with underscore or latin letter, contain underscores, latin letters and digits.
    */
    QString name; //SDC:

    /*!
    @getter
    @return the caption of this object, which is user-visible extended name which can be used
    in user interfaces and translated.
    @setter
    Sets the caption for this object.
    */
    QString caption; //SDC:

    /*!
    @getter
    @return the description of this object, which is explanation
            of the object's purpose, etc.
    It can be any text and can be used in user interfaces and translated.
    @setter
    Sets the description for this object.
    */
    QString description; //SDC:

    /*! @return caption of this object if it is not empty, else returns object's name.
    */
    inline QString captionOrName() const {
        return d->caption.isEmpty() ? d->name : d->caption;
    }

    //! Creates new object of type @a type. */
    explicit KDbObject(int type);

protected:
    //! Clears all properties except 'type'.
    virtual void clear();
};

//! Sends information about object @a object to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbObject& object);

#endif
