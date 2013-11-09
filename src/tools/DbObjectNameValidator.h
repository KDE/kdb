/* This file is part of the KDE project
   Copyright (C) 2004-2005 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDBOBJECTNAMEVALIDATOR_H
#define KEXIDBOBJECTNAMEVALIDATOR_H

#include <QString>
#include <QPointer>
#include <Predicate/Global>
#include <Predicate/Tools/Validator>

namespace Predicate
{

class Driver;

/*! Validates input:
 accepts if the name is not reserved for internal kexi objects. */
class PREDICATE_EXPORT ObjectNameValidator : public Utils::Validator
{
public:
    /*! @a drv is a Predicate driver on which isSystemObjectName() will be
     called inside check(). If @a drv is 0, Predicate::Driver::isPredicateSystemObjectName()
     static function is called instead. */
    ObjectNameValidator(Driver *drv, QObject * parent = 0);
    virtual ~ObjectNameValidator();

protected:
    virtual Utils::Validator::Result internalCheck(const QString &valueName, const QVariant& value,
            QString *message, QString *details);
    Driver* m_drv;
};
}

#endif
