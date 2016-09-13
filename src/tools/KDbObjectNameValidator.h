/* This file is part of the KDE project
   Copyright (C) 2004-2016 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDBOBJECTNAMEVALIDATOR_H
#define KDBOBJECTNAMEVALIDATOR_H

#include <QString>
#include "KDbValidator.h"

class KDbDriver;

/*! Validates input:
 accepts if the name is not reserved for internal kexi objects. */
class KDB_EXPORT KDbObjectNameValidator : public KDbValidator
{
    Q_OBJECT
public:
    /*! @a drv is a KDb driver on which isSystemObjectName() will be
     called inside check(). If @a drv is 0, KDbDriver::isKDbSystemObjectName()
     static function is called instead. */
    explicit KDbObjectNameValidator(const KDbDriver *drv, QObject * parent = nullptr);
    virtual ~KDbObjectNameValidator();

protected:
    virtual KDbValidator::Result internalCheck(const QString &valueName, const QVariant& value,
                                               QString *message, QString *details);
private:
    class Private;
    Private * const d;
    Q_DISABLE_COPY(KDbObjectNameValidator)
};

#endif
