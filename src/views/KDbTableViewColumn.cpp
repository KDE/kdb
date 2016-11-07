/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003-2014 Jarosław Staniek <staniek@kde.org>

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

   Original Author:  Till Busch <till@bux.at>
   Original Project: buX (www.bux.at)
*/

#include "KDbTableViewColumn.h"

#include "KDbConnection.h"
#include "KDbConnectionOptions.h"
#include "KDbCursor.h"
#include "KDb.h"
#include "KDbQuerySchema.h"
#include "KDbRecordEditBuffer.h"
#include "KDbTableViewData.h"
#include "KDbValidator.h"

#include <QIcon>

class Q_DECL_HIDDEN KDbTableViewColumn::Private
{
public:
  Private()
      : data(0)
      , validator(0)
      , relatedData(0)
      , field(0)
      , columnInfo(0)
      , visibleLookupColumnInfo(0)
      , width(0)
      , readOnly(false)
      , visible(true)
      , relatedDataEditable(false)
      , headerTextVisible(true)
    {
    }

    //! Data that this column is assigned to. Set by KDbTableViewColumn::setData()
    KDbTableViewData* data;

    QString captionAliasOrName;

    QIcon icon;

    KDbValidator* validator;

    KDbTableViewData* relatedData;
    int relatedDataPKeyID;

    KDbField* field;

    //! @see columnInfo()
    KDbQueryColumnInfo* columnInfo;

    //! @see visibleLookupColumnInfo()
    KDbQueryColumnInfo* visibleLookupColumnInfo;

    int width;
    bool isDBAware; //!< true if data is stored in DB, not only in memeory
    bool readOnly;
    bool fieldOwned;
    bool visible;
    bool relatedDataEditable;
    bool headerTextVisible;
};

//------------------------

KDbTableViewColumn::KDbTableViewColumn(KDbField *f, bool owner)
        : d(new Private)
{
    d->field = f;
    d->isDBAware = false;
    d->fieldOwned = owner;
    d->captionAliasOrName = d->field->captionOrName();
}

KDbTableViewColumn::KDbTableViewColumn(const QString &name, KDbField::Type ctype,
        KDbField::Constraints cconst,
        KDbField::Options options,
        int maxLength, int precision,
        QVariant defaultValue,
        const QString &caption, const QString &description)
        : d(new Private)
{
    d->field = new KDbField(
        name, ctype, cconst, options, maxLength, precision, defaultValue, caption, description);

    d->isDBAware = false;
    d->fieldOwned = true;
    d->captionAliasOrName = d->field->captionOrName();
}

KDbTableViewColumn::KDbTableViewColumn(const QString &name, KDbField::Type ctype,
        const QString &caption, const QString &description)
        : d(new Private)
{
    d->field = new KDbField(
        name, ctype,
        KDbField::NoConstraints,
        KDbField::NoOptions,
        0, 0,
        QVariant(),
        caption, description);

    d->isDBAware = false;
    d->fieldOwned = true;
    d->captionAliasOrName = d->field->captionOrName();
}

// db-aware
KDbTableViewColumn::KDbTableViewColumn(
    const KDbQuerySchema &query, KDbQueryColumnInfo *aColumnInfo,
    KDbQueryColumnInfo *aVisibleLookupColumnInfo)
        : d(new Private)
{
    Q_ASSERT(aColumnInfo);
    d->field = aColumnInfo->field();
    d->columnInfo = aColumnInfo;
    d->visibleLookupColumnInfo = aVisibleLookupColumnInfo;
    d->isDBAware = true;
    d->fieldOwned = false;

    //setup column's caption:
    if (!d->columnInfo->field()->caption().isEmpty()) {
        d->captionAliasOrName = d->columnInfo->field()->caption();
    } else {
        //reuse alias if available:
        d->captionAliasOrName = d->columnInfo->alias();
        //last hance: use field name
        if (d->captionAliasOrName.isEmpty())
            d->captionAliasOrName = d->columnInfo->field()->name();
        //! @todo compute other auto-name?
    }
    //setup column's readonly flag: true, if
    // - it's not from parent table's field, or
    // - if the query itself is coming from read-only connection, or
    // - if the query itself is stored (i.e. has connection) and lookup column is defined
    const bool columnFromMasterTable = query.masterTable() == d->columnInfo->field()->table();
    d->readOnly = !columnFromMasterTable
                 || (query.connection() && query.connection()->options()->isReadOnly());
//! @todo remove this when queries become editable            ^^^^^^^^^^^^^^
// kdbDebug() << "KDbTableViewColumn: query.masterTable()=="
//  << (query.masterTable() ? query.masterTable()->name() : "notable") << ", columnInfo->field->table()=="
//  << (columnInfo->field->table() ? columnInfo->field->table()->name()  : "notable");
}

KDbTableViewColumn::KDbTableViewColumn(bool)
        : d(new Private)
{
    d->isDBAware = false;
}

KDbTableViewColumn::~KDbTableViewColumn()
{
    if (d->fieldOwned)
        delete d->field;
    setValidator(0);
    delete d->relatedData;
    delete d;
}

void KDbTableViewColumn::setValidator(KDbValidator *v)
{
    if (d->validator) {//remove old one
        if (!d->validator->parent()) //destroy if has no parent
            delete d->validator;
    }
    d->validator = v;
}

void KDbTableViewColumn::setData(KDbTableViewData *data)
{
    d->data = data;
}

void KDbTableViewColumn::setRelatedData(KDbTableViewData *data)
{
    if (d->isDBAware)
        return;
    if (d->relatedData)
        delete d->relatedData;
    d->relatedData = 0;
    if (!data)
        return;
    //find a primary key
    const QList<KDbTableViewColumn*> *columns = data->columns();
    int id = -1;
    foreach(KDbTableViewColumn* col, *columns) {
        id++;
        if (col->field()->isPrimaryKey()) {
            //found, remember
            d->relatedDataPKeyID = id;
            d->relatedData = data;
            return;
        }
    }
}

bool KDbTableViewColumn::isReadOnly() const
{
    return d->readOnly || (d->data && d->data->isReadOnly());
}

void KDbTableViewColumn::setReadOnly(bool ro)
{
    d->readOnly = ro;
}

bool KDbTableViewColumn::isVisible() const
{
    return d->columnInfo ? d->columnInfo->isVisible() : d->visible;
}

void KDbTableViewColumn::setVisible(bool v)
{
    bool changed = d->visible != v;
    if (d->columnInfo && d->columnInfo->isVisible() != v) {
        d->columnInfo->setVisible(v);
        changed = true;
    }
    d->visible = v;
    if (changed && d->data) {
        d->data->columnVisibilityChanged(*this);
    }
}

void KDbTableViewColumn::setIcon(const QIcon& icon)
{
    d->icon = icon;
}

QIcon KDbTableViewColumn::icon() const
{
    return d->icon;
}

void KDbTableViewColumn::setHeaderTextVisible(bool visible)
{
    d->headerTextVisible = visible;
}

bool KDbTableViewColumn::isHeaderTextVisible() const
{
    return d->headerTextVisible;
}

QString KDbTableViewColumn::captionAliasOrName() const
{
    return d->captionAliasOrName;
}

KDbValidator* KDbTableViewColumn::validator() const
{
    return d->validator;
}

KDbTableViewData *KDbTableViewColumn::relatedData() const
{
    return d->relatedData;
}

KDbField* KDbTableViewColumn::field() const
{
    return d->field;
}

void KDbTableViewColumn::setRelatedDataEditable(bool set)
{
    d->relatedDataEditable = set;
}

bool KDbTableViewColumn::isRelatedDataEditable() const
{
    return d->relatedDataEditable;
}

KDbQueryColumnInfo* KDbTableViewColumn::columnInfo() const
{
    return d->columnInfo;
}

KDbQueryColumnInfo* KDbTableViewColumn::visibleLookupColumnInfo() const
{
    return d->visibleLookupColumnInfo;
}

bool KDbTableViewColumn::isDBAware() const
{
    return d->isDBAware;
}


bool KDbTableViewColumn::acceptsFirstChar(const QChar &ch) const
{
    // the field we're looking at can be related to "visible lookup column"
    // if lookup column is present
    KDbField *visibleField = d->visibleLookupColumnInfo
                                  ? d->visibleLookupColumnInfo->field() : d->field;
    const KDbField::Type type = visibleField->type(); // cache: evaluating type of expressions can be expensive
    if (KDbField::isNumericType(type)) {
        if (ch == QLatin1Char('.') || ch == QLatin1Char(','))
            return KDbField::isFPNumericType(type);
        if (ch == QLatin1Char('-'))
            return !visibleField->isUnsigned();
        if (ch == QLatin1Char('+') || (ch >= QLatin1Char('0') && ch <= QLatin1Char('9')))
            return true;
        return false;
    }

    switch (type) {
    case KDbField::Boolean:
        return false;
    case KDbField::Date:
    case KDbField::DateTime:
    case KDbField::Time:
        return ch >= QLatin1Char('0') && ch <= QLatin1Char('9');
    default:;
    }
    return true;
}

void KDbTableViewColumn::setWidth(int w)
{
    d->width = w;
}

int KDbTableViewColumn::width() const
{
    return d->width;
}
