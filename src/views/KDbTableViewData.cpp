/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003-2017 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2014 Michał Poteralski <michalpoteralskikde@gmail.com>

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

#include "KDbTableViewData.h"
#include "KDbConnection.h"
#include "KDbConnectionOptions.h"
#include "KDbCursor.h"
#include "KDbError.h"
#include "KDb.h"
#include "KDbOrderByColumn.h"
#include "KDbQuerySchema.h"
#include "KDbRecordEditBuffer.h"
#include "KDbTableViewColumn.h"
#include "kdb_debug.h"

#include <QApplication>

#include <unicode/coll.h>

// #define TABLEVIEW_NO_PROCESS_EVENTS

static unsigned short charTable[] = {
#include "chartable.txt"
};

//-------------------------------

//! @internal Unicode-aware collator for comparing strings
class CollatorInstance
{
public:
    CollatorInstance() {
        UErrorCode status = U_ZERO_ERROR;
        m_collator = icu::Collator::createInstance(status);
        if (U_FAILURE(status)) {
            kdbWarning() << "Could not create instance of collator:" << status;
            m_collator = nullptr;
        } else {
            // enable normalization by default
            m_collator->setAttribute(UCOL_NORMALIZATION_MODE, UCOL_ON, status);
            if (U_FAILURE(status)) {
                kdbWarning() << "Could not set collator attribute:" << status;
            }
        }
    }

    icu::Collator* getCollator() {
        return m_collator;
    }

    ~CollatorInstance() {
        delete m_collator;
    }

private:
    icu::Collator *m_collator;
};

Q_GLOBAL_STATIC(CollatorInstance, KDb_collator)

//! @internal A functor used in qSort() in order to sort by a given column
class LessThanFunctor
{
private:
    KDbOrderByColumn::SortOrder m_order;
    QVariant m_leftTmp, m_rightTmp;
    int m_sortColumn;

    bool (*m_lessThanFunction)(const QVariant&, const QVariant&);

#define CAST_AND_COMPARE(casting) \
    return left.casting() < right.casting()

    static bool cmpInt(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toInt);
    }

    static bool cmpUInt(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toUInt);
    }

    static bool cmpLongLong(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toLongLong);
    }

    static bool cmpULongLong(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toULongLong);
    }

    static bool cmpDouble(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toDouble);
    }

    static bool cmpDate(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toDate);
    }

    static bool cmpDateTime(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toDateTime);
    }

    static bool cmpTime(const QVariant& left, const QVariant& right) {
        CAST_AND_COMPARE(toDate);
    }

    static bool cmpString(const QVariant& left, const QVariant& right) {
        const QString &as = left.toString();
        const QString &bs = right.toString();

        const QChar *a = as.isEmpty() ? nullptr : as.unicode();
        const QChar *b = bs.isEmpty() ? nullptr : bs.unicode();

        if (a == nullptr) {
            return b != nullptr;
        }
        if (a == b || b == nullptr) {
            return false;
        }

        int len = qMin(as.length(), bs.length());
        forever {
            unsigned short au = a->unicode();
            unsigned short bu = b->unicode();
            au = (au <= 0x17e ? charTable[au] : 0xffff);
            bu = (bu <= 0x17e ? charTable[bu] : 0xffff);

            if (len <= 0)
                return false;
            len--;

            if (au != bu)
                return au < bu;
            a++;
            b++;
        }
        return false;
    }

    static bool cmpStringWithCollator(const QVariant& left, const QVariant& right) {
        const QString &as = left.toString();
        const QString &bs = right.toString();
        return icu::Collator::LESS == KDb_collator->getCollator()->compare(
                                        (const UChar *)as.constData(), as.size(),
                                        (const UChar *)bs.constData(), bs.size());
    }

    //! Compare function for BLOB data (QByteArray). Uses size as the weight.
    static bool cmpBLOB(const QVariant& left, const QVariant& right) {
        return left.toByteArray().size() < right.toByteArray().size();
    }

public:
    LessThanFunctor()
            : m_order(KDbOrderByColumn::SortOrder::Ascending)
            , m_sortColumn(-1)
            , m_lessThanFunction(nullptr)
    {
    }

    void setColumnType(const KDbField& field) {
        const KDbField::Type t = field.type();
        if (field.isTextType())
            m_lessThanFunction = &cmpString;
        if (KDbField::isFPNumericType(t))
            m_lessThanFunction = &cmpDouble;
        else if (t == KDbField::Integer && field.isUnsigned())
            m_lessThanFunction = &cmpUInt;
        else if (t == KDbField::Boolean || KDbField::isNumericType(t))
            m_lessThanFunction = &cmpInt; //other integers
        else if (t == KDbField::BigInteger) {
            if (field.isUnsigned())
                m_lessThanFunction = &cmpULongLong;
            else
                m_lessThanFunction = &cmpLongLong;
        } else if (t == KDbField::Date)
            m_lessThanFunction = &cmpDate;
        else if (t == KDbField::Time)
            m_lessThanFunction = &cmpTime;
        else if (t == KDbField::DateTime)
            m_lessThanFunction = &cmpDateTime;
        else if (t == KDbField::BLOB)
            //! @todo allow users to define BLOB sorting function?
            m_lessThanFunction = &cmpBLOB;
        else { // anything else
            // check if CollatorInstance is not destroyed and has valid collator
            if (!KDb_collator.isDestroyed() && KDb_collator->getCollator()) {
                m_lessThanFunction = &cmpStringWithCollator;
            }
            else {
                m_lessThanFunction = &cmpString;
            }
        }
    }

    void setSortOrder(KDbOrderByColumn::SortOrder order) {
        m_order = order;
    }

    void setSortColumn(int column) {
        m_sortColumn = column;
    }

#define _IIF(a,b) ((a) ? (b) : !(b))

    //! Main comparison operator that takes column number, type and order into account
    bool operator()(KDbRecordData* record1, KDbRecordData* record2) {
        // compare NULLs : NULL is smaller than everything
        if ((m_leftTmp = record1->at(m_sortColumn)).isNull())
            return _IIF(m_order == KDbOrderByColumn::SortOrder::Ascending,
                        !record2->at(m_sortColumn).isNull());
        if ((m_rightTmp = record2->at(m_sortColumn)).isNull())
            return m_order == KDbOrderByColumn::SortOrder::Descending;

        return _IIF(m_order == KDbOrderByColumn::SortOrder::Ascending,
                    m_lessThanFunction(m_leftTmp, m_rightTmp));
    }
};
#undef _IIF
#undef CAST_AND_COMPARE

//! @internal
class Q_DECL_HIDDEN KDbTableViewData::Private
{
public:
    Private()
            : sortColumn(0)
            , realSortColumn(0)
            , sortOrder(KDbOrderByColumn::SortOrder::Ascending)
            , type(1)
            , pRecordEditBuffer(nullptr)
            , readOnly(false)
            , insertingEnabled(true)
            , containsRecordIdInfo(false)
            , autoIncrementedColumn(-2) {
    }

    ~Private() {
        delete pRecordEditBuffer;
    }

    //! Number of physical columns
    int realColumnCount;

    /*! Columns information */
    QList<KDbTableViewColumn*> columns;

    /*! Visible columns information */
    QList<KDbTableViewColumn*> visibleColumns;

    //! (Logical) sorted column number, set by setSorting().
    //! Can differ from realSortColumn if there's lookup column used.
    int sortColumn;

    //! Real sorted column number, set by setSorting(), used by cmp*() methods
    int realSortColumn;

    //! Specifies sorting order
    KDbOrderByColumn::SortOrder sortOrder;

    LessThanFunctor lessThanFunctor;

    short type;

    KDbRecordEditBuffer *pRecordEditBuffer;

    KDbCursor *cursor;

    KDbResultInfo result;

    QList<int> visibleColumnIDs;
    QList<int> globalColumnIDs;

    bool readOnly;

    bool insertingEnabled;

    //! @see KDbTableViewData::containsRecordIdInfo()
    bool containsRecordIdInfo;

    mutable int autoIncrementedColumn;
};

//-------------------------------

KDbTableViewData::KDbTableViewData()
        : QObject()
        , KDbTableViewDataBase()
        , d(new Private)
{
    d->realColumnCount = 0;
    d->cursor = nullptr;
}

// db-aware ctor
KDbTableViewData::KDbTableViewData(KDbCursor *c)
        : QObject()
        , KDbTableViewDataBase()
        , d(new Private)
{
    d->cursor = c;
    d->containsRecordIdInfo = d->cursor->containsRecordIdInfo();
    if (d->cursor && d->cursor->query()) {
        const KDbQuerySchema::FieldsExpandedOptions fieldsExpandedOptions
        = d->containsRecordIdInfo ? KDbQuerySchema::WithInternalFieldsAndRecordId
          : KDbQuerySchema::WithInternalFields;
        d->realColumnCount = d->cursor->query()->fieldsExpanded(fieldsExpandedOptions).count();
    } else {
        d->realColumnCount = d->columns.count() + (d->containsRecordIdInfo ? 1 : 0);
    }

    // Allocate KDbTableViewColumn objects for each visible query column
    const KDbQueryColumnInfo::Vector fields = d->cursor->query()->fieldsExpanded();
    const int fieldCount = fields.count();
    for (int i = 0;i < fieldCount;i++) {
        KDbQueryColumnInfo *ci = fields[i];
        if (ci->isVisible()) {
            KDbQueryColumnInfo *visibleLookupColumnInfo = nullptr;
            if (ci->indexForVisibleLookupValue() != -1) {
                //Lookup field is defined
                visibleLookupColumnInfo = d->cursor->query()->expandedOrInternalField(ci->indexForVisibleLookupValue());
            }
            KDbTableViewColumn* col = new KDbTableViewColumn(*d->cursor->query(), ci, visibleLookupColumnInfo);
            addColumn(col);
        }
    }
}

KDbTableViewData::KDbTableViewData(const QList<QVariant> &keys, const QList<QVariant> &values,
                                   KDbField::Type keyType, KDbField::Type valueType)
    : KDbTableViewData()
{
    KDbField *keyField = new KDbField(QLatin1String("key"), keyType);
    keyField->setPrimaryKey(true);
    KDbTableViewColumn *keyColumn
        = new KDbTableViewColumn(keyField, KDbTableViewColumn::FieldIsOwned::Yes);
    keyColumn->setVisible(false);
    addColumn(keyColumn);

    KDbField *valueField = new KDbField(QLatin1String("value"), valueType);
    KDbTableViewColumn *valueColumn
        = new KDbTableViewColumn(valueField, KDbTableViewColumn::FieldIsOwned::Yes);
    addColumn(valueColumn);

    int cnt = qMin(keys.count(), values.count());
    QList<QVariant>::ConstIterator it_keys = keys.constBegin();
    QList<QVariant>::ConstIterator it_values = values.constBegin();
    for (;cnt > 0;++it_keys, ++it_values, cnt--) {
        KDbRecordData *record = new KDbRecordData(2);
        (*record)[0] = (*it_keys);
        (*record)[1] = (*it_values);
        append(record);
    }
}

KDbTableViewData::KDbTableViewData(KDbField::Type keyType, KDbField::Type valueType)
    : KDbTableViewData(QList<QVariant>(), QList<QVariant>(), keyType, valueType)
{
}

KDbTableViewData::~KDbTableViewData()
{
    emit destroying();
    clearInternal(false /* !processEvents */);
    qDeleteAll(d->columns);
    delete d;
}

void KDbTableViewData::deleteLater()
{
    d->cursor = nullptr;
    QObject::deleteLater();
}

void KDbTableViewData::addColumn(KDbTableViewColumn* col)
{
    d->columns.append(col);
    col->setData(this);
    if (col->isVisible()) {
        d->visibleColumns.append(col);
        d->visibleColumnIDs.append(d->visibleColumns.count() - 1);
        d->globalColumnIDs.append(d->columns.count() - 1);
    } else {
        d->visibleColumnIDs.append(-1);
    }
    d->autoIncrementedColumn = -2; //clear cache;
    if (!d->cursor || !d->cursor->query()) {
        d->realColumnCount = d->columns.count() + (d->containsRecordIdInfo ? 1 : 0);
    }
}

void KDbTableViewData::columnVisibilityChanged(const KDbTableViewColumn &column)
{
    if (column.isVisible()) { // column made visible
        int indexInGlobal = d->columns.indexOf(const_cast<KDbTableViewColumn*>(&column));
        // find previous column that is visible
        int prevIndexInGlobal = indexInGlobal - 1;
        while (prevIndexInGlobal >= 0 && d->visibleColumnIDs[prevIndexInGlobal] == -1) {
            prevIndexInGlobal--;
        }
        int indexInVisible = prevIndexInGlobal + 1;
        // update
        d->visibleColumns.insert(indexInVisible, const_cast<KDbTableViewColumn*>(&column));
        d->visibleColumnIDs[indexInGlobal] = indexInVisible;
        d->globalColumnIDs.insert(indexInVisible, indexInGlobal);
        for (int i = indexInGlobal + 1; i < d->columns.count(); i++) { // increment ids of the rest
            if (d->visibleColumnIDs[i] >= 0) {
                d->visibleColumnIDs[i]++;
            }
        }
    }
    else { // column made invisible
        int indexInVisible = d->visibleColumns.indexOf(const_cast<KDbTableViewColumn*>(&column));
        d->visibleColumns.removeAt(indexInVisible);
        int indexInGlobal = globalIndexOfVisibleColumn(indexInVisible);
        d->visibleColumnIDs[indexInGlobal] = -1;
        d->globalColumnIDs.removeAt(indexInVisible);
    }
}

int KDbTableViewData::globalIndexOfVisibleColumn(int visibleIndex) const
{
    return d->globalColumnIDs.value(visibleIndex, -1);
}

int KDbTableViewData::visibleColumnIndex(int globalIndex) const
{
    return d->visibleColumnIDs.value(globalIndex, -1);
}

int KDbTableViewData::columnCount() const
{
    return d->columns.count();
}

int KDbTableViewData::visibleColumnCount() const
{
    return d->visibleColumns.count();
}

QList<KDbTableViewColumn*>* KDbTableViewData::columns()
{
    return &d->columns;
}

QList<KDbTableViewColumn*>* KDbTableViewData::visibleColumns()
{
    return &d->visibleColumns;
}

KDbTableViewColumn* KDbTableViewData::column(int index)
{
    return d->columns.value(index);
}

KDbTableViewColumn* KDbTableViewData::visibleColumn(int index)
{
    return d->visibleColumns.value(index);
}

bool KDbTableViewData::isDBAware() const
{
    return d->cursor != nullptr;
}

KDbCursor* KDbTableViewData::cursor() const
{
    return d->cursor;
}

bool KDbTableViewData::isInsertingEnabled() const
{
    return d->insertingEnabled;
}

KDbRecordEditBuffer* KDbTableViewData::recordEditBuffer() const
{
    return d->pRecordEditBuffer;
}

const KDbResultInfo& KDbTableViewData::result() const
{
    return d->result;
}

bool KDbTableViewData::containsRecordIdInfo() const
{
    return d->containsRecordIdInfo;
}

KDbRecordData* KDbTableViewData::createItem() const
{
    return new KDbRecordData(d->realColumnCount);
}

QString KDbTableViewData::dbTableName() const
{
    if (d->cursor && d->cursor->query() && d->cursor->query()->masterTable())
        return d->cursor->query()->masterTable()->name();
    return QString();
}

void KDbTableViewData::setSorting(int column, KDbOrderByColumn::SortOrder order)
{
    d->sortOrder = order;
    if (column < 0 || column >= d->columns.count()) {
        d->sortColumn = -1;
        d->realSortColumn = -1;
        return;
    }
    // find proper column information for sorting (lookup column points to alternate column with visible data)
    const KDbTableViewColumn *tvcol = d->columns.at(column);
    KDbQueryColumnInfo* visibleLookupColumnInfo = tvcol->visibleLookupColumnInfo();
    const KDbField *field = visibleLookupColumnInfo ? visibleLookupColumnInfo->field() : tvcol->field();
    d->sortColumn = column;
    d->realSortColumn = tvcol->columnInfo()->indexForVisibleLookupValue() != -1
                          ? tvcol->columnInfo()->indexForVisibleLookupValue() : d->sortColumn;

    // setup compare functor
    d->lessThanFunctor.setColumnType(*field);
    d->lessThanFunctor.setSortOrder(d->sortOrder);
    d->lessThanFunctor.setSortColumn(column);
}

int KDbTableViewData::sortColumn() const
{
    return d->sortColumn;
}

KDbOrderByColumn::SortOrder KDbTableViewData::sortOrder() const
{
    return d->sortOrder;
}

void KDbTableViewData::sort()
{
    if (d->sortColumn < 0 || d->sortColumn >= d->columns.count()) {
        return;
    }
    qSort(begin(), end(), d->lessThanFunctor);
}

void KDbTableViewData::setReadOnly(bool set)
{
    if (d->readOnly == set)
        return;
    d->readOnly = set;
    if (d->readOnly)
        setInsertingEnabled(false);
}

void KDbTableViewData::setInsertingEnabled(bool set)
{
    if (d->insertingEnabled == set)
        return;
    d->insertingEnabled = set;
    if (d->insertingEnabled)
        setReadOnly(false);
}

void KDbTableViewData::clearRecordEditBuffer()
{
    //init record edit buffer
    if (!d->pRecordEditBuffer)
        d->pRecordEditBuffer = new KDbRecordEditBuffer(isDBAware());
    else
        d->pRecordEditBuffer->clear();
}

bool KDbTableViewData::updateRecordEditBufferRef(KDbRecordData *record,
        int colnum, KDbTableViewColumn* col, QVariant* newval, bool allowSignals,
        QVariant *visibleValueForLookupField)
{
    Q_ASSERT(newval);
    d->result.clear();
    if (allowSignals)
        emit aboutToChangeCell(record, colnum, newval, &d->result);
    if (!d->result.success)
        return false;

    //kdbDebug() << "column #" << colnum << " = " << newval.toString();
    if (!col) {
        kdbWarning() << "column #" << colnum << "not found! col==0";
        return false;
    }
    if (!d->pRecordEditBuffer)
        d->pRecordEditBuffer = new KDbRecordEditBuffer(isDBAware());
    if (d->pRecordEditBuffer->isDBAware()) {
        if (!(col->columnInfo())) {
            kdbWarning() << "column #" << colnum << " not found!";
            return false;
        }
        d->pRecordEditBuffer->insert(col->columnInfo(), *newval);

        if (col->visibleLookupColumnInfo() && visibleValueForLookupField) {
            //this is value for lookup table: update visible value as well
            d->pRecordEditBuffer->insert(col->visibleLookupColumnInfo(), *visibleValueForLookupField);
        }
        return true;
    }
    if (!(col->field())) {
        kdbWarning() << "column #" << colnum << "not found!";
        return false;
    }
    //not db-aware:
    const QString colname = col->field()->name();
    if (colname.isEmpty()) {
        kdbWarning() << "column #" << colnum << "not found!";
        return false;
    }
    d->pRecordEditBuffer->insert(colname, *newval);
    return true;
}

bool KDbTableViewData::updateRecordEditBuffer(KDbRecordData *record, int colnum,
                                              KDbTableViewColumn* col,
                                              const QVariant &newval, bool allowSignals)
{
    QVariant newv(newval);
    return updateRecordEditBufferRef(record, colnum, col, &newv, allowSignals);
}

bool KDbTableViewData::updateRecordEditBuffer(KDbRecordData *record, int colnum,
                                              const QVariant &newval, bool allowSignals)
{
    KDbTableViewColumn* col = d->columns.value(colnum);
    QVariant newv(newval);
    return col ? updateRecordEditBufferRef(record, colnum, col, &newv, allowSignals) : false;
}

//! Get a new value (if present in the buffer), or the old one (taken here for optimization)
static inline void saveRecordGetValue(const QVariant **pval, KDbCursor *cursor,
                                      KDbRecordEditBuffer *pRecordEditBuffer,
                                      QList<KDbTableViewColumn*>::ConstIterator* it_f,
                                      KDbRecordData *record, KDbField *f, QVariant* val, int col)
{
    if (!*pval) {
        *pval = cursor
                ? pRecordEditBuffer->at( (**it_f)->columnInfo(),
                                         record->at(col).isNull() /* useDefaultValueIfPossible */ )
                : pRecordEditBuffer->at( *f );
        *val = *pval ? **pval : record->at(col); /* get old value */
        //kdbDebug() << col << *(**it_f)->columnInfo() << "val:" << *val;
    }
}

//! @todo if there're multiple views for this data, we need multiple buffers!
bool KDbTableViewData::saveRecord(KDbRecordData *record, bool insert, bool repaint)
{
    Q_UNUSED(record);
    if (!d->pRecordEditBuffer)
        return true; //nothing to do

    //check constraints:
    //-check if every NOT NULL and NOT EMPTY field is filled
    QList<KDbTableViewColumn*>::ConstIterator it_f(d->columns.constBegin());
    int col = 0;
    const QVariant *pval = nullptr;
    QVariant val;
    for (;it_f != d->columns.constEnd() && col < record->count();++it_f, ++col) {
        KDbField *f = (*it_f)->field();
        if (f->isNotNull()) {
            saveRecordGetValue(&pval, d->cursor, d->pRecordEditBuffer, &it_f, record, f, &val, col);
            //check it
            if (val.isNull() && !f->isAutoIncrement()) {
                //NOT NULL violated
                d->result.message = tr("\"%1\" column requires a value to be entered.").arg(f->captionOrName())
                                + QLatin1String("\n\n") + KDbTableViewData::messageYouCanImproveData();
                d->result.description = tr("The column's constraint is declared as NOT NULL.");
                d->result.column = col;
                return false;
            }
        }
        if (f->isNotEmpty()) {
            saveRecordGetValue(&pval, d->cursor, d->pRecordEditBuffer, &it_f, record, f, &val, col);
            if (!f->isAutoIncrement() && (val.isNull() || KDb::isEmptyValue(f->type(), val))) {
                //NOT EMPTY violated
                d->result.message = tr("\"%1\" column requires a value to be entered.").arg(f->captionOrName())
                                + QLatin1String("\n\n") + KDbTableViewData::messageYouCanImproveData();
                d->result.description = tr("The column's constraint is declared as NOT EMPTY.");
                d->result.column = col;
                return false;
            }
        }
    }

    if (d->cursor) {//db-aware
        if (insert) {
            if (!d->cursor->insertRecord(record, d->pRecordEditBuffer,
                                         d->containsRecordIdInfo /*also retrieve ROWID*/))
            {
                d->result.message = tr("Record inserting failed.") + QLatin1String("\n\n")
                                + KDbTableViewData::messageYouCanImproveData();
                KDb::getHTMLErrorMesage(*d->cursor, &d->result);

                /*   if (desc)
                      *desc = */
                /*! @todo use KexiMainWindow::showErrorMessage(const QString &title, KDbObject *obj)
                  after it will be moved somewhere to KDb (this will require moving other
                    showErrorMessage() methods from KexiMainWindow to libkexiutils....)
                  then: just call: *desc = KDb::errorMessage(d->cursor); */
                return false;
            }
        } else { // record updating
            if (!d->cursor->updateRecord(static_cast<KDbRecordData*>(record), d->pRecordEditBuffer,
                                         d->containsRecordIdInfo /*use ROWID*/))
            {
                d->result.message = tr("Record changing failed.") + QLatin1String("\n\n")
                                + KDbTableViewData::messageYouCanImproveData();
//! @todo set d->result.column if possible
                KDb::getHTMLErrorMesage(*d->cursor, &d->result.description);
                return false;
            }
        }
    } else {//not db-aware version
        KDbRecordEditBuffer::SimpleMap b = d->pRecordEditBuffer->simpleBuffer();
        for (KDbRecordEditBuffer::SimpleMap::ConstIterator it = b.constBegin();it != b.constEnd();++it) {
            int i = -1;
            foreach(KDbTableViewColumn *col, d->columns) {
                i++;
                if (col->field()->name() == it.key()) {
                    kdbDebug() << col->field()->name() << ": " << record->at(i).toString()
                           << " -> " << it.value().toString();
                    (*record)[i] = it.value();
                }
            }
        }
    }

    d->pRecordEditBuffer->clear();

    if (repaint)
        emit recordRepaintRequested(record);
    return true;
}

bool KDbTableViewData::saveRecordChanges(KDbRecordData *record, bool repaint)
{
    Q_UNUSED(record);
    d->result.clear();
    emit aboutToUpdateRecord(record, d->pRecordEditBuffer, &d->result);
    if (!d->result.success)
        return false;

    if (saveRecord(record, false /*update*/, repaint)) {
        emit recordUpdated(record);
        return true;
    }
    return false;
}

bool KDbTableViewData::saveNewRecord(KDbRecordData *record, bool repaint)
{
    Q_UNUSED(record);
    d->result.clear();
    emit aboutToInsertRecord(record, &d->result, repaint);
    if (!d->result.success)
        return false;

    if (saveRecord(record, true /*insert*/, repaint)) {
        emit recordInserted(record, repaint);
        return true;
    }
    return false;
}

bool KDbTableViewData::deleteRecord(KDbRecordData *record, bool repaint)
{
    Q_UNUSED(record);
    d->result.clear();
    emit aboutToDeleteRecord(record, &d->result, repaint);
    if (!d->result.success)
        return false;

    if (d->cursor) {//db-aware
        d->result.success = false;
        if (!d->cursor->deleteRecord(static_cast<KDbRecordData*>(record), d->containsRecordIdInfo /*use ROWID*/)) {
            d->result.message = tr("Record deleting failed.");
            //! @todo use KDberrorMessage() for description as in KDbTableViewData::saveRecord() */
            KDb::getHTMLErrorMesage(*d->cursor, &d->result);
            d->result.success = false;
            return false;
        }
    }

    int index = indexOf(record);
    if (index == -1) {
        //aah - this shouldn't be!
        kdbWarning() << "!removeRef() - IMPL. ERROR?";
        d->result.success = false;
        return false;
    }
    removeAt(index);
    emit recordDeleted();
    return true;
}

void KDbTableViewData::deleteRecords(const QList<int> &recordsToDelete, bool repaint)
{
    Q_UNUSED(repaint);

    if (recordsToDelete.isEmpty())
        return;
    int last_r = 0;
    KDbTableViewDataIterator it(begin());
    for (QList<int>::ConstIterator r_it = recordsToDelete.constBegin(); r_it != recordsToDelete.constEnd(); ++r_it) {
        for (; last_r < (*r_it); last_r++)
            ++it;
        it = erase(it);   /* this will delete *it */
        last_r++;
    }
//DON'T CLEAR BECAUSE KexiTableViewPropertyBuffer will clear BUFFERS!
//--> emit reloadRequested(); //! \todo more effective?
    emit recordsDeleted(recordsToDelete);
}

void KDbTableViewData::insertRecord(KDbRecordData *record, int index, bool repaint)
{
    Q_UNUSED(record);
    insert(index = qMin(index, count()), record);
    emit recordInserted(record, index, repaint);
}

void KDbTableViewData::clearInternal(bool processEvents)
{
    clearRecordEditBuffer();
//! @todo this is time consuming: find better data model
    const int c = count();
#ifndef TABLEVIEW_NO_PROCESS_EVENTS
    const bool _processEvents = processEvents && !qApp->closingDown();
#endif
    for (int i = 0; i < c; i++) {
        removeLast();
#ifndef TABLEVIEW_NO_PROCESS_EVENTS
        if (_processEvents && i % 1000 == 0)
            qApp->processEvents(QEventLoop::AllEvents, 1);
#endif
    }
}

bool KDbTableViewData::deleteAllRecords(bool repaint)
{
    clearInternal();

    bool res = true;
    if (d->cursor) {
        //db-aware
        res = d->cursor->deleteAllRecords();
    }

    if (repaint)
        emit reloadRequested();
    return res;
}

int KDbTableViewData::autoIncrementedColumn() const
{
    if (d->autoIncrementedColumn == -2) {
        //find such a column
        d->autoIncrementedColumn = -1;
        foreach(KDbTableViewColumn *col, d->columns) {
            d->autoIncrementedColumn++;
            if (col->field()->isAutoIncrement())
                break;
        }
    }
    return d->autoIncrementedColumn;
}

bool KDbTableViewData::preloadAllRecords()
{
    if (!d->cursor)
        return false;
    if (!d->cursor->moveFirst() && d->cursor->result().isError())
        return false;

#ifndef TABLEVIEW_NO_PROCESS_EVENTS
    const bool processEvents = !qApp->closingDown();
#endif

    for (int i = 0;!d->cursor->eof();i++) {
        KDbRecordData *record = d->cursor->storeCurrentRecord();
        if (!record) {
            clear();
            return false;
        }
//  record->debug();
        append(record);
        if (!d->cursor->moveNext() && d->cursor->result().isError()) {
            clear();
            return false;
        }
#ifndef TABLEVIEW_NO_PROCESS_EVENTS
        if (processEvents && (i % 1000) == 0)
            qApp->processEvents(QEventLoop::AllEvents, 1);
#endif
    }
    return true;
}

bool KDbTableViewData::isReadOnly() const
{
    return d->readOnly || (d->cursor && d->cursor->connection()->options()->isReadOnly());
}

// static
QString KDbTableViewData::messageYouCanImproveData()
{
    return tr("Please correct data in this record or use the \"Cancel record changes\" function.");
}

QDebug operator<<(QDebug dbg, const KDbTableViewData &data)
{
    dbg.nospace() << "TableViewData(";
    dbg.space() << "sortColumn:" << data.sortColumn()
                << "sortOrder:" << (data.sortOrder() == KDbOrderByColumn::SortOrder::Ascending
                                    ? "asc" : "desc")
                << "isDBAware:" << data.isDBAware()
                << "dbTableName:" << data.dbTableName()
                << "cursor:" << (data.cursor() ? "yes" : "no")
                << "columnCount:" << data.columnCount()
                << "count:" << data.count()
                << "autoIncrementedColumn:" << data.autoIncrementedColumn()
                << "visibleColumnCount:" << data.visibleColumnCount()
                << "isReadOnly:" << data.isReadOnly()
                << "isInsertingEnabled:" << data.isInsertingEnabled()
                << "containsRecordIdInfo:" << data.containsRecordIdInfo()
                << "result:" << data.result();
    dbg.nospace() << ")";
    return dbg.space();
}
