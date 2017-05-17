/* This file is part of the KDE project
   Copyright (C) 2003, 2006 Jarosław Staniek <staniek@kde.org>

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

#ifndef SQLTYPES_H
#define SQLTYPES_H

#include <QVariant>
#include <QList>

#include "KDbExpression.h"
#include "KDbOrderByColumn.h"

struct dateType {
    int year;
    int month;
    int day;
};

struct realType {
    int integer;
    int fractional;
};

//! @internal
struct OrderByColumnInternal {
    class List : public QList<OrderByColumnInternal> {
    public:
        List() {}
        ~List() {}
    };
    OrderByColumnInternal()
            : columnNumber(-1)
            , order(KDbOrderByColumn::SortOrder::Ascending) {
    }

    void setColumnByNameOrNumber(const QVariant& nameOrNumber) {
        if (nameOrNumber.type() == QVariant::String) {
            aliasOrName = nameOrNumber.toString();
            columnNumber = -1;
        } else {
            columnNumber = nameOrNumber.toInt();
            aliasOrName.clear();
        }
    }

    //! Can include a "tablename." prefix
    QString aliasOrName;
    //! Optional, used instead of aliasOrName to refer to column
    //! by its number rather than name.
    int columnNumber;
    KDbOrderByColumn::SortOrder order;
};

//! @internal
struct SelectOptionsInternal {
    SelectOptionsInternal() : orderByColumns(nullptr) {}
    ~SelectOptionsInternal() {
        delete orderByColumns; // delete because this is internal temp. structure
    }
    KDbExpression whereExpr;
    QList<OrderByColumnInternal>* orderByColumns;
};

class KDbExpressionPtr
{
public:
    inline explicit KDbExpressionPtr(KDbExpression *exp) : e(exp) {}
    inline KDbExpression toExpr() {
        KDbExpression exp(*e);
        delete e;
        e = nullptr;
        return exp;
    }
    inline KDbNArgExpression toNArg() {
        KDbNArgExpression exp(e->toNArg());
        delete e;
        e = nullptr;
        return exp;
    }
//private:
    KDbExpression *e;
private:
    Q_DISABLE_COPY(KDbExpressionPtr)
};

QDebug operator<<(QDebug dbg, const KDbExpressionPtr& expr);

#endif
