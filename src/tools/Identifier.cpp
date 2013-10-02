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

#include "Identifier.h"
#include "transliteration/transliteration_table.h"

using namespace Predicate::Utils;

bool Predicate::Utils::isIdentifier(const QString& s)
{
    uint i;
    const uint sLength = s.length();
    for (i = 0; i < sLength; i++) {
        const char c = s.at(i).toLower().toLatin1();
        if (c == 0 || !(c == '_' || (c >= 'a' && c <= 'z') || (i > 0 && c >= '0' && c <= '9')))
            break;
    }
    return i > 0 && i == sLength;
}

inline QString charToIdentifier(const QChar& c)
{
    if (c.unicode() >= TRANSLITERATION_TABLE_SIZE)
        return QLatin1String("_");
    const char *const s = transliteration_table[c.unicode()];
    return s ? QString::fromLatin1(s) : QLatin1String("_");
}

QString Predicate::Utils::stringToIdentifier(const QString &s)
{
    if (s.isEmpty())
        return QString();
    QString r, id = s.simplified();
    if (id.isEmpty())
        return QString();
    r.reserve(id.length());
    id.replace(QLatin1Char(' '), QLatin1String("_"));
    const QChar c = id[0];
    const char ch = c.toLatin1();
    QString add;
    bool wasUnderscore = false;

    if (ch >= '0' && ch <= '9') {
        r += QLatin1Char('_');
        r += c;
    } else {
        add = charToIdentifier(c);
        r += add;
        wasUnderscore = add == QLatin1String("_");
    }

    const uint idLength = id.length();
    for (uint i = 1; i < idLength; i++) {
        add = charToIdentifier(id.at(i));
        if (wasUnderscore && add == QLatin1String("_"))
            continue;
        wasUnderscore = add == QLatin1String("_");
        r += add;
    }
    return r;
}

//--------------------------------------------------------------------------------

QString Predicate::Utils::identifierExpectedMessage(const QString &valueName, const QVariant& v)
{
    return QLatin1String("<p>") + QObject::tr("Value of \"%1\" column must be an identifier.").arg(valueName)
           + QLatin1String("</p><p>")
           + QObject::tr("\"%1\" is not a valid identifier.").arg(v.toString()) + QLatin1String("</p>");
}

//--------------------------------------------------------------------------------

IdentifierValidator::IdentifierValidator(QObject * parent)
        : Validator(parent)
{
}

IdentifierValidator::~IdentifierValidator()
{
}

QValidator::State IdentifierValidator::validate(QString& input, int& pos) const
{
    uint i;
    for (i = 0; (int)i < input.length() && input.at(i) == QLatin1Char(' '); i++)
        ;
    pos -= i; //i chars will be removed from beginning
    if ((int)i < input.length() && input.at(i) >= QLatin1Char('0') && input.at(i) <= QLatin1Char('9'))
        pos++; //_ will be added at the beginning
    bool addspace = (input.right(1) == QLatin1String(" "));
    input = stringToIdentifier(input);
    if (addspace)
        input += QLatin1Char('_');
    if (pos > input.length())
        pos = input.length();
    return input.isEmpty() ? QValidator::Intermediate : Acceptable;
}

Validator::Result IdentifierValidator::internalCheck(
    const QString &valueName, const QVariant& v,
    QString &message, QString & /*details*/)
{
    if (isIdentifier(v.toString()))
        return Validator::Ok;
    message = identifierExpectedMessage(valueName, v);
    return Validator::Error;
}

