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

#ifndef PREDICATE_OBJECT_H
#define PREDICATE_OBJECT_H

#include <Predicate/Global.h>
#include <Predicate/Field.h>
#include <QtDebug>

namespace Predicate
{

/*! Provides common attributes for Predicate object like id, name, caption,
 help text. Predicate object is typically object definition storable
 in database, like table schema or query schema.
*/
shared class export=PREDICATE_EXPORT Object
{
public:
    /*!
    @getter
    @return the type of this object.
    */
    data_member int type default=Predicate::UnknownObjectType no_setter;

    /*!
    @getter
    @return the identifier of this object, default is -1.
    @setter
    Sets the identifier for this object.
    */
    data_member int id default=-1;

    /*!
    @getter
    @return the name of this object.
    @setter
    Sets the name for this object. It should be valid identifier,
    i.e. start with underscore or latin letter, contain underscores, latin letters and digits.
    */
    data_member QString name;

    /*!
    @getter
    @return the caption of this object, which is user-visible extended name which can be used
    in user interfaces and translated.
    @setter
    Sets the caption for this object.
    */
    data_member QString caption;

    /*!
    @getter
    @return the description of this object, which is explanation
            of the object's purpose, etc.
    It can be any text and can be used in user interfaces and translated.
    @setter
    Sets the description for this object.
    */
    data_member QString description;

    /*!
    @getter
    @return true if this object represents native database object,
    for example native table. This flag is set when object (currently -- database table)
    is not retrieved using kexi__* schema storage system,
    but just based on the information about native database object.

    Native database objects have no additional metadata like caption
    or description properties.

    Native objects schemas are used mostly for representing kexi system
    (kexi__*) tables in memory for later reference.
    @see Connection::tableNames().

    By default (on allocation) objects are not native.
    @setter
    Sets "native" flag for this object.
    */
    data_member bool native default=false;

    /*! @return caption of this object if it is not empty, else returns object's name.
    */
    QString captionOrName() const {
        return d->caption.isEmpty() ? d->name : d->caption;
    }

    /*!
    Creates new object of type @a type.
    */
    Object(int type = Predicate::UnknownObjectType);

protected:
    //! Clears all properties except 'type'.
    void clear();
/*    friend class Connection; */
};

} // namespace Predicate

//! Sends information about object @a object to debug output @a dbg.
PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Predicate::Object& object);

#endif
