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

#include "KDbIdentifierValidator.h"
#include "KDb.h"

class Q_DECL_HIDDEN KDbIdentifierValidator::Private
{
public:
    Private() : isLowerCaseForced(false) {}
    bool isLowerCaseForced;
private:
    Q_DISABLE_COPY(Private)
};

KDbIdentifierValidator::KDbIdentifierValidator(QObject * parent)
        : KDbValidator(parent), d(new Private)
{
}

KDbIdentifierValidator::~KDbIdentifierValidator()
{
    delete d;
}

QValidator::State KDbIdentifierValidator::validate(QString& input, int& pos) const
{
    int i;
    for (i = 0; i < input.length() && input.at(i) == QLatin1Char(' '); i++)
        ;
    pos -= i; //i chars will be removed from beginning
    if (i < input.length() && input.at(i) >= QLatin1Char('0') && input.at(i) <= QLatin1Char('9'))
        pos++; //_ will be added at the beginning
    bool addspace = (input.right(1) == QLatin1String(" "));
    input = d->isLowerCaseForced ? KDb::stringToIdentifier(input).toLower() : KDb::stringToIdentifier(input);
    if (addspace)
        input += QLatin1Char('_');
    if (pos > input.length())
        pos = input.length();
    return (input.isEmpty() && !acceptsEmptyValue())
            ? Intermediate
            : Acceptable;
}

KDbValidator::Result KDbIdentifierValidator::internalCheck(
    const QString &valueName, const QVariant& value,
    QString *message, QString *details)
{
    Q_UNUSED(details);
    if (KDb::isIdentifier(value.toString()))
        return KDbValidator::Ok;
    if (message) {
        *message = KDb::identifierExpectedMessage(valueName, value);
    }
    return KDbValidator::Error;
}

bool KDbIdentifierValidator::isLowerCaseForced() const
{
    return d->isLowerCaseForced;
}

void KDbIdentifierValidator::setLowerCaseForced(bool set)
{
    d->isLowerCaseForced = set;
}
