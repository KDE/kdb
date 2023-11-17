/* This file is part of the KDE project
   Copyright (C) 2004, 2006 Jarosław Staniek <staniek@kde.org>

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

#include "KDbValidator.h"

class Q_DECL_HIDDEN KDbValidator::Private
{
public:
    Private()
            : acceptsEmptyValue(false) {
    }
    bool acceptsEmptyValue;
};

//-----------------------------------------------------------

class Q_DECL_HIDDEN KDbMultiValidator::Private
{
public:
    Private() {
    }
    ~Private() {
        qDeleteAll(ownedSubValidators);
        ownedSubValidators.clear();
    }

    QList<QValidator*> ownedSubValidators;
    QList<QValidator*> subValidators;
};

//-----------------------------------------------------------

KDbValidator::KDbValidator(QObject * parent)
        : QValidator(parent)
        , d(new Private)
{
}

KDbValidator::~KDbValidator()
{
    delete d;
}

KDbValidator::Result KDbValidator::check(const QString &valueName, const QVariant& v,
                                   QString *message, QString *details)
{
    if (v.isNull() || (v.type() == QVariant::String && v.toString().isEmpty())) {
        if (!d->acceptsEmptyValue) {
            if (message) {
                *message = KDbValidator::messageColumnNotEmpty().arg(valueName);
            }
            return Error;
        }
        return Ok;
    }
    return internalCheck(valueName, v, message, details);
}

KDbValidator::Result KDbValidator::internalCheck(const QString &valueName,
        const QVariant& value, QString *message, QString *details)
{
    Q_UNUSED(valueName);
    Q_UNUSED(value);
    Q_UNUSED(message);
    Q_UNUSED(details);
    return Error;
}

QValidator::State KDbValidator::validate(QString & , int &) const
{
    return QValidator::Acceptable;
}

void KDbValidator::setAcceptsEmptyValue(bool set)
{
    d->acceptsEmptyValue = set;
}

bool KDbValidator::acceptsEmptyValue() const
{
    return d->acceptsEmptyValue;
}

const QString KDbValidator::messageColumnNotEmpty()
{
    return QLatin1String(QT_TR_NOOP("\"%1\" value has to be entered."));
}

//-----------------------------------------------------------

KDbMultiValidator::KDbMultiValidator(QObject* parent)
        : KDbValidator(parent)
        , d(new Private)
{
}

KDbMultiValidator::KDbMultiValidator(QValidator *validator, QObject * parent)
        : KDbValidator(parent)
        , d(new Private)
{
    addSubvalidator(validator);
}

KDbMultiValidator::~KDbMultiValidator()
{
    delete d;
}

void KDbMultiValidator::addSubvalidator(QValidator* validator, bool owned)
{
    if (!validator)
        return;
    d->subValidators.append(validator);
    if (owned && !validator->parent())
        d->ownedSubValidators.append(validator);
}

QValidator::State KDbMultiValidator::validate(QString & input, int & pos) const
{
    State s;
    for(QValidator* validator : std::as_const(d->subValidators)) {
        s = validator->validate(input, pos);
        if (s == Intermediate || s == Invalid)
            return s;
    }
    return Acceptable;
}

void KDbMultiValidator::fixup(QString & input) const
{
    for (QValidator* validator : std::as_const(d->subValidators)) {
        validator->fixup(input);
    }
}

KDbValidator::Result KDbMultiValidator::internalCheck(
    const QString &valueName, const QVariant& value,
    QString *message, QString *details)
{
    Result r;
    bool warning = false;
    for (QValidator* validator : std::as_const(d->subValidators)) {
        if (dynamic_cast<KDbValidator*>(validator))
            r = dynamic_cast<KDbValidator*>(validator)->internalCheck(valueName, value, message, details);
        else
            r = Ok; //ignore
        if (r == Error)
            return Error;
        else if (r == Warning)
            warning = true;
    }
    return warning ? Warning : Ok;
}

