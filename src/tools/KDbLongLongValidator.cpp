/* This file is part of the KDE project
   Copyright (C) 2006-2016 Jarosław Staniek <staniek@kde.org>

   Based on KIntValidator code by Glen Parker <glenebob@nwlink.com>

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

#include "KDbLongLongValidator.h"

#include <QWidget>

class Q_DECL_HIDDEN KDbLongLongValidator::Private
{
public:
    Private()
        : min(0)
        , max(0)
    {
    }
    qint64 base;
    qint64 min;
    qint64 max;
};

KDbLongLongValidator::KDbLongLongValidator(QWidget * parent, int base)
        : QValidator(parent)
        , d(new Private)
{
    setBase(base);
}

KDbLongLongValidator::KDbLongLongValidator(qint64 bottom, qint64 top,
                                           QWidget * parent, int base)
        : QValidator(parent)
        , d(new Private)
{
    setBase(base);
    setRange(bottom, top);
}

KDbLongLongValidator::~KDbLongLongValidator()
{
    delete d;
}

QValidator::State KDbLongLongValidator::validate(QString &str, int &) const
{
    bool ok;
    qint64 val = 0;
    QString newStr;

    newStr = str.trimmed();
    if (d->base > 10)
        newStr = newStr.toUpper();

    if (newStr == QString::fromLatin1("-")) {// a special case
        if ((d->min || d->max) && d->min >= 0)
            ok = false;
        else
            return QValidator::Acceptable;
    } else if (!newStr.isEmpty())
        val = newStr.toLongLong(&ok, d->base);
    else {
        val = 0;
        ok = true;
    }

    if (! ok)
        return QValidator::Invalid;

    if ((! d->min && ! d->max) || (val >= d->min && val <= d->max))
        return QValidator::Acceptable;

    if (d->max && d->min >= 0 && val < 0)
        return QValidator::Invalid;

    return QValidator::Acceptable;
}

void KDbLongLongValidator::fixup(QString &str) const
{
    int dummy;
    qint64 val;
    QValidator::State state;

    state = validate(str, dummy);

    if (state == QValidator::Invalid || state == QValidator::Acceptable)
        return;

    if (! d->min && ! d->max)
        return;

    val = str.toLongLong(0, d->base);

    if (val < d->min)
        val = d->min;
    if (val > d->max)
        val = d->max;

    str.setNum(val, d->base);
}

void KDbLongLongValidator::setRange(qint64 bottom, qint64 top)
{
    d->min = bottom;
    d->max = top;

    if (d->max < d->min)
        d->max = d->min;
}

void KDbLongLongValidator::setBase(int base)
{
    d->base = base;
    if (d->base < 2)
        d->base = 2;
    if (d->base > 36)
        d->base = 36;
}

qint64 KDbLongLongValidator::bottom() const
{
    return d->min;
}

qint64 KDbLongLongValidator::top() const
{
    return d->max;
}

int KDbLongLongValidator::base() const
{
    return d->base;
}
