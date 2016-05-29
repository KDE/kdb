/* This file is part of the KDE project
   Copyright (C) 2003-2005 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2005 Martin Ellis <martin.ellis@kdemail.net>

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

#ifndef KDB_TOOLS_IDENTIFIER_H
#define KDB_TOOLS_IDENTIFIER_H

#include "KDbValidator.h"

//! Validates input for identifier.
class KDB_EXPORT KDbIdentifierValidator : public KDbValidator
{
public:
    explicit KDbIdentifierValidator(QObject * parent = 0);

    virtual ~KDbIdentifierValidator();

    virtual State validate(QString & input, int & pos) const;

    //! @return true if upper-case letters in the input are replaced to lower-case.
    //! @c false by default.
    bool isLowerCaseForced() const;

    //! If @a set is true, upper-case letters in the input are replaced to lower-case.
    void setLowerCaseForced(bool set);

protected:
    virtual KDbValidator::Result internalCheck(const QString &valueName, const QVariant& value,
                                               QString *message, QString *details);

    class Private;
    Private* const d;
};

#endif
