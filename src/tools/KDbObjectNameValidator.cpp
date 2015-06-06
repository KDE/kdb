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

#include "KDbObjectNameValidator.h"

#include "KDbDriver.h"

//! @todo IMPORTANT: replace QPointer<KDbDriver> m_drv;
KDbObjectNameValidator::KDbObjectNameValidator(KDbDriver *drv, QObject * parent)
        : KDbValidator(parent)
        , m_drv(drv)
{
}

KDbObjectNameValidator::~KDbObjectNameValidator()
{
}

KDbValidator::Result KDbObjectNameValidator::internalCheck(
    const QString &valueName, const QVariant& value,
    QString *message, QString *details)
{
    Q_UNUSED(valueName);
    if (!m_drv ? !KDbDriver::isKDbSystemObjectName(value.toString())
            : !m_drv->isSystemObjectName(value.toString()))
        return KDbValidator::Ok;
    if (message) {
        *message = QObject::tr("You cannot use name \"%1\" for your object. "
                               "It is reserved for internal objects. Please choose another name.")
                               .arg(value.toString());
    }
    if (details) {
        *details = QObject::tr("Names of internal database objects are starting with \"kexi__\".");
    }
    return KDbValidator::Error;
}
