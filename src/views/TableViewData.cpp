/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003-2013 Jarosław Staniek <staniek@kde.org>

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

#include "TableViewData.h"

#include "Field.h"
#include "RecordEditBuffer.h"
#include "Cursor.h"
#include "Utils.h"
#include "Error.h"

#include <QtDebug>
#include <QApplication>
#include <QVector>

// #define TABLEVIEW_NO_PROCESS_EVENTS

using namespace Predicate;

unsigned short charTable[] = {
#include "chartable.txt"
};

//-------------------------------

//! @internal A functor used in qSort() in order to sort by a given column
class LessThanFunctor
{
private:
    bool m_ascendingOrder;
    QVariant m_leftTmp, m_rightTmp;
    int m_sortedColumn;

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

        const QChar *a = as.isEmpty() ? 0 : as.unicode();
        const QChar *b = bs.isEmpty() ? 0 : bs.unicode();

        if (a == 0) {
            return b != 0;
        }
        if (a == b || b == 0) {
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

    //! Compare function for BLOB data (QByteArray). Uses size as the weight.
    static bool cmpBLOB(const QVariant& left, const QVariant& right) {
        return left.toByteArray().size() < right.toByteArray().size();
    }

public:
    LessThanFunctor()
            : m_ascendingOrder(true)
            , m_sortedColumn(-1)
            , m_lessThanFunction(0)
    {
    }

    void setColumnType(const Field& field) {
        const Field::Type t = field.type();
        if (field.isTextType())
            m_lessThanFunction = &cmpString;
        if (Field::isFPNumericType(t))
            m_lessThanFunction = &cmpDouble;
        else if (t == Field::Integer && field.isUnsigned())
            m_lessThanFunction = &cmpUInt;
        else if (t == Field::Boolean || Field::isNumericType(t))
            m_lessThanFunction = &cmpInt; //other integers
        else if (t == Field::BigInteger) {
            if (field.isUnsigned())
                m_lessThanFunction = &cmpULongLong;
            else
                m_lessThanFunction = &cmpLongLong;
        } else if (t == Field::Date)
            m_lessThanFunction = &cmpDate;
        else if (t == Field::Time)
            m_lessThanFunction = &cmpTime;
        else if (t == Field::DateTime)
            m_lessThanFunction = &cmpDateTime;
        else if (t == Field::BLOB)
            //! @todo allow users to define BLOB sorting function?
            m_lessThanFunction = &cmpBLOB;
        else
            m_lessThanFunction = &cmpString; //anything else
    }

    void setAscendingOrder(bool ascending) {
        m_ascendingOrder = ascending;
    }

    void setSortedColumn(int column) {
        m_sortedColumn = column;
    }

#define _IIF(a,b) ((a) ? (b) : !(b))

    //! Main comparison operator that takes column number, type and order into account
    bool operator()(RecordData* record1, RecordData* record2) {
        // compare NULLs : NULL is smaller than everything
        if ((m_leftTmp = record1->at(m_sortedColumn)).isNull())
            return _IIF(m_ascendingOrder, !record2->at(m_sortedColumn).isNull());
        if ((m_rightTmp = record2->at(m_sortedColumn)).isNull())
            return !m_ascendingOrder;

        return _IIF(m_ascendingOrder, m_lessThanFunction(m_leftTmp, m_rightTmp));
    }
};
#undef _IIF
#undef CAST_AND_COMPARE

//! @internal
class TableViewData::Private
{
public:
    Private()
            : sortedColumn(0)
            , realSortedColumn(0)
            , type(1)
            , pRecordEditBuffer(0)
            , visibleColumnsCount(0)
            , visibleColumnsIDs(100)
            , globalColumnsIDs(100)
            , readOnly(false)
            , insertingEnabled(true)
            , containsRecordIdInfo(false)
            , ascendingOrder(false)
            , descendingOrder(false)
            , autoIncrementedColumn(-2) {
    }

    ~Private() {
        delete pRecordEditBuffer;
    }

    //! (logical) sorted column number, set by setSorting()
    //! can differ from realSortedColumn if there's lookup column used
    int sortedColumn;

    //! real sorted column number, set by setSorting(), used by cmp*() methods
    int realSortedColumn;

    LessThanFunctor lessThanFunctor;

    short type;

    RecordEditBuffer *pRecordEditBuffer;

    Cursor *cursor;

    ResultInfo result;

    uint visibleColumnsCount;

    QVector<int> visibleColumnsIDs, globalColumnsIDs;

    bool readOnly;

    bool insertingEnabled;

    //! @see TableViewData::containsRecordIdInfo()
    bool containsRecordIdInfo;

    //! true if ascending sort order is set
    bool ascendingOrder;

    //! true if descending sort order is set
    bool descendingOrder;

    int autoIncrementedColumn;
};

//-------------------------------

TableViewData::TableViewData()
        : QObject()
        , TableViewDataBase()
        , d(new Private)
{
    init();
    d->cursor = 0;
}

// db-aware ctor
TableViewData::TableViewData(Cursor *c)
        : QObject()
        , TableViewDataBase()
        , d(new Private)
{
    init();
    d->cursor = c;
    d->containsRecordIdInfo = d->cursor->containsRecordIdInfo();
    if (d->cursor && d->cursor->query()) {
        const QuerySchema::FieldsExpandedOptions fieldsExpandedOptions
        = d->containsRecordIdInfo ? QuerySchema::WithInternalFieldsAndRecordId
          : QuerySchema::WithInternalFields;
        m_itemSize = d->cursor->query()->fieldsExpanded(fieldsExpandedOptions).count();
    } else
        m_itemSize = m_columns.count() + (d->containsRecordIdInfo ? 1 : 0);

    // Allocate TableViewColumn objects for each visible query column
    const QueryColumnInfo::Vector fields = d->cursor->query()->fieldsExpanded();
    const uint fieldsCount = fields.count();
    for (uint i = 0;i < fieldsCount;i++) {
        QueryColumnInfo *ci = fields[i];
        if (ci->visible) {
            QueryColumnInfo *visibleLookupColumnInfo = 0;
            if (ci->indexForVisibleLookupValue() != -1) {
                //Lookup field is defined
                visibleLookupColumnInfo = d->cursor->query()->expandedOrInternalField(ci->indexForVisibleLookupValue());
            }
            TableViewColumn* col = new TableViewColumn(*d->cursor->query(), ci, visibleLookupColumnInfo);
            addColumn(col);
        }
    }
}

TableViewData::TableViewData(
    const QList<QVariant> &keys, const QList<QVariant> &values,
    Field::Type keyType, Field::Type valueType)
        : QObject()
        , TableViewDataBase()
        , d(new Private)
{
    init(keys, values, keyType, valueType);
}

TableViewData::TableViewData(
    Field::Type keyType, Field::Type valueType)
        : QObject()
        , TableViewDataBase()
        , d(new Private)
{
    const QList<QVariant> empty;
    init(empty, empty, keyType, valueType);
}

TableViewData::~TableViewData()
{
    emit destroying();
    clearInternal(false /* !processEvents */);
    qDeleteAll(m_columns);
    delete d;
}

void TableViewData::init(
    const QList<QVariant> &keys, const QList<QVariant> &values,
    Field::Type keyType, Field::Type valueType)
{
    init();
    Field *keyField = new Field(QLatin1String("key"), keyType);
    keyField->setPrimaryKey(true);
    TableViewColumn *keyColumn = new TableViewColumn(keyField, true);
    keyColumn->setVisible(false);
    addColumn(keyColumn);

    Field *valueField = new Field(QLatin1String("value"), valueType);
    TableViewColumn *valueColumn = new TableViewColumn(valueField, true);
    addColumn(valueColumn);

    uint cnt = qMin(keys.count(), values.count());
    QList<QVariant>::ConstIterator it_keys = keys.constBegin();
    QList<QVariant>::ConstIterator it_values = values.constBegin();
    for (;cnt > 0;++it_keys, ++it_values, cnt--) {
        RecordData *record = new RecordData(2);
        (*record)[0] = (*it_keys);
        (*record)[1] = (*it_values);
        append(record);
    }
}

void TableViewData::init()
{
    m_itemSize = 0;
}

void TableViewData::deleteLater()
{
    d->cursor = 0;
    QObject::deleteLater();
}

void TableViewData::addColumn(TableViewColumn* col)
{
    m_columns.append(col);
    col->setData(this);
    if (d->globalColumnsIDs.size() < (int)m_columns.count()) {//sanity
        d->globalColumnsIDs.resize(d->globalColumnsIDs.size()*2);
    }
    if (col->isVisible()) {
        d->visibleColumnsCount++;
        if ((uint)d->visibleColumnsIDs.size() < d->visibleColumnsCount) {//sanity
            d->visibleColumnsIDs.resize(d->visibleColumnsIDs.size()*2);
        }
        d->visibleColumnsIDs[ m_columns.count()-1 ] = d->visibleColumnsCount - 1;
        d->globalColumnsIDs[ d->visibleColumnsCount-1 ] = m_columns.count() - 1;
    } else {
        d->visibleColumnsIDs[ m_columns.count()-1 ] = -1;
    }
    d->autoIncrementedColumn = -2; //clear cache;
    if (!d->cursor || !d->cursor->query())
        m_itemSize = m_columns.count() + (d->containsRecordIdInfo ? 1 : 0);
}

int TableViewData::globalColumnID(int visibleID) const
{
    return d->globalColumnsIDs.value(visibleID, -1);
}

int TableViewData::visibleColumnID(int globalID) const
{
    return d->visibleColumnsIDs.value(globalID, -1);
}

bool TableViewData::isDBAware() const
{
    return d->cursor != 0;
}

Cursor* TableViewData::cursor() const
{
    return d->cursor;
}

bool TableViewData::isInsertingEnabled() const
{
    return d->insertingEnabled;
}

RecordEditBuffer* TableViewData::recordEditBuffer() const
{
    return d->pRecordEditBuffer;
}

const ResultInfo& TableViewData::result() const
{
    return d->result;
}

bool TableViewData::containsRecordIdInfo() const
{
    return d->containsRecordIdInfo;
}

QString TableViewData::dbTableName() const
{
    if (d->cursor && d->cursor->query() && d->cursor->query()->masterTable())
        return d->cursor->query()->masterTable()->name();
    return QString();
}

void TableViewData::setSorting(int column, bool ascending)
{
    if (column >= 0 && column < (int)m_columns.count()) {
        d->ascendingOrder = ascending;
        d->descendingOrder = !ascending;
    } else {
        d->ascendingOrder = false;
        d->descendingOrder = false;
        d->sortedColumn = -1;
        d->realSortedColumn = -1;
        return;
    }
    // find proper column information for sorting (lookup column points to alternate column with visible data)
    const TableViewColumn *tvcol = m_columns.at(column);
    QueryColumnInfo* visibleLookupColumnInfo = tvcol->visibleLookupColumnInfo();
    const Field *field = visibleLookupColumnInfo ? visibleLookupColumnInfo->field : tvcol->field();
    d->sortedColumn = column;
    d->realSortedColumn = tvcol->columnInfo()->indexForVisibleLookupValue() != -1
                          ? tvcol->columnInfo()->indexForVisibleLookupValue() : d->sortedColumn;

    // setup compare functor
    d->lessThanFunctor.setColumnType(*field);
    d->lessThanFunctor.setAscendingOrder(ascending);
    d->lessThanFunctor.setSortedColumn(column);
}

int TableViewData::sortedColumn() const
{
    return d->sortedColumn;
}

int TableViewData::sortingOrder() const
{
    return d->ascendingOrder ? 1 : (d->descendingOrder ? -1 : 0);
}

void TableViewData::sort()
{
    if (0 != sortingOrder())
        qSort(begin(), end(), d->lessThanFunctor);
}

void TableViewData::setReadOnly(bool set)
{
    if (d->readOnly == set)
        return;
    d->readOnly = set;
    if (d->readOnly)
        setInsertingEnabled(false);
}

void TableViewData::setInsertingEnabled(bool set)
{
    if (d->insertingEnabled == set)
        return;
    d->insertingEnabled = set;
    if (d->insertingEnabled)
        setReadOnly(false);
}

void TableViewData::clearRecordEditBuffer()
{
    //init record edit buffer
    if (!d->pRecordEditBuffer)
        d->pRecordEditBuffer = new RecordEditBuffer(isDBAware());
    else
        d->pRecordEditBuffer->clear();
}

bool TableViewData::updateRecordEditBufferRef(RecordData *record,
        int colnum, TableViewColumn* col, QVariant& newval, bool allowSignals,
        QVariant *visibleValueForLookupField)
{
    d->result.clear();
    if (allowSignals)
        emit aboutToChangeCell(record, colnum, newval, &d->result);
    if (!d->result.success)
        return false;

    //PreDbg << "column #" << colnum << " = " << newval.toString();
    if (!col) {
        PreWarn << "column #" << colnum << "not found! col==0";
        return false;
    }
    if (!d->pRecordEditBuffer)
        d->pRecordEditBuffer = new RecordEditBuffer(isDBAware());
    if (d->pRecordEditBuffer->isDBAware()) {
        if (!(col->columnInfo())) {
            PreWarn << "column #" << colnum << " not found!";
            return false;
        }
        d->pRecordEditBuffer->insert(*col->columnInfo(), newval);

        if (col->visibleLookupColumnInfo() && visibleValueForLookupField) {
            //this is value for lookup table: update visible value as well
            d->pRecordEditBuffer->insert(*col->visibleLookupColumnInfo(), *visibleValueForLookupField);
        }
        return true;
    }
    if (!(col->field())) {
        PreWarn << "column #" << colnum << "not found!";
        return false;
    }
    //not db-aware:
    const QString colname = col->field()->name();
    if (colname.isEmpty()) {
        PreWarn << "column #" << colnum << "not found!";
        return false;
    }
    d->pRecordEditBuffer->insert(colname, newval);
    return true;
}

//get a new value (if present in the buffer), or the old one, otherwise
//(taken here for optimization)
#define GET_VALUE if (!pval) { \
        pval = d->cursor \
        ? d->pRecordEditBuffer->at( *(*it_f)->columnInfo(), record->at(col).isNull() /* useDefaultValueIfPossible */ ) \
              : d->pRecordEditBuffer->at( *f ); \
        val = pval ? *pval : record->at(col); /* get old value */ \
        /*PreDbg << col << (*it_f)->columnInfo()->debugString() << "val:" << val;*/ \
    }

//! @todo if there're multiple views for this data, we need multiple buffers!
bool TableViewData::saveRecord(RecordData *record, bool insert, bool repaint)
{
    Q_UNUSED(record);
    if (!d->pRecordEditBuffer)
        return true; //nothing to do

    //check constraints:
    //-check if every NOT NULL and NOT EMPTY field is filled
    TableViewColumnListIterator it_f(m_columns.constBegin());
    int col = 0;
    const QVariant *pval = 0;
    QVariant val;
    for (;it_f != m_columns.constEnd() && col < record->count(); ++it_f, col++) {
        Field *f = (*it_f)->field();
        if (f->isNotNull()) {
            GET_VALUE;
            //check it
            if (val.isNull() && !f->isAutoIncrement()) {
                //NOT NULL violated
                d->result.msg = tr("\"%1\" column requires a value to be entered.").arg(f->captionOrName())
                                + QLatin1String("\n\n") + TableViewData::messageYouCanImproveData();
                d->result.desc = tr("The column's constraint is declared as NOT NULL.");
                d->result.column = col;
                return false;
            }
        }
        if (f->isNotEmpty()) {
            GET_VALUE;
            if (!f->isAutoIncrement() && (val.isNull() || Predicate::isEmptyValue(f, val))) {
                //NOT EMPTY violated
                d->result.msg = tr("\"%1\" column requires a value to be entered.").arg(f->captionOrName())
                                + QLatin1String("\n\n") + TableViewData::messageYouCanImproveData();
                d->result.desc = tr("The column's constraint is declared as NOT EMPTY.");
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
                d->result.msg = tr("Record inserting failed.") + QLatin1String("\n\n")
                                + TableViewData::messageYouCanImproveData();
                Predicate::getHTMLErrorMesage(*d->cursor, &d->result);

                /*   if (desc)
                      *desc =
                js: TODO: use KexiMainWindow::showErrorMessage(const QString &title, Object *obj)
                  after it will be moved somewhere to kexidb (this will require moving other
                    showErrorMessage() methods from KexiMainWindow to libkexiutils....)
                  then: just call: *desc = KexiDB::errorMessage(d->cursor);
                */
                return false;
            }
        } else { // record updating
            if (!d->cursor->updateRecord(static_cast<RecordData*>(record), d->pRecordEditBuffer,
                                         d->containsRecordIdInfo /*use ROWID*/))
            {
                d->result.msg = tr("Record changing failed.") + QLatin1String("\n\n")
                                + TableViewData::messageYouCanImproveData();
//! @todo set d->result.column if possible
                Predicate::getHTMLErrorMesage(*d->cursor, d->result.desc);
                return false;
            }
        }
    } else {//not db-aware version
        RecordEditBuffer::SimpleMap b = d->pRecordEditBuffer->simpleBuffer();
        for (RecordEditBuffer::SimpleMap::ConstIterator it = b.constBegin();it != b.constEnd();++it) {
            uint i = -1;
            foreach(TableViewColumn *col, m_columns) {
                i++;
                if (col->field()->name() == it.key()) {
                    PreDbg << col->field()->name() << ": " << record->at(i).toString()
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

bool TableViewData::saveRecordChanges(RecordData *record, bool repaint)
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

bool TableViewData::saveNewRecord(RecordData *record, bool repaint)
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

bool TableViewData::deleteRecord(RecordData *record, bool repaint)
{
    Q_UNUSED(record);
    d->result.clear();
    emit aboutToDeleteRecord(record, &d->result, repaint);
    if (!d->result.success)
        return false;

    if (d->cursor) {//db-aware
        d->result.success = false;
        if (!d->cursor->deleteRecord(static_cast<RecordData*>(record), d->containsRecordIdInfo /*use ROWID*/)) {
            d->result.msg = tr("Record deleting failed.");
            /*js: TODO: use Predicate::errorMessage() for description (desc) as in TableViewData::saveRecord() */
            Predicate::getHTMLErrorMesage(*d->cursor, &d->result);
            d->result.success = false;
            return false;
        }
    }

    int index = indexOf(record);
    if (index == -1) {
        //aah - this shouldn't be!
        PreWarn << "!removeRef() - IMPL. ERROR?";
        d->result.success = false;
        return false;
    }
    removeAt(index);
    emit recordDeleted();
    return true;
}

void TableViewData::deleteRecords(const QList<int> &recordsToDelete, bool repaint)
{
    Q_UNUSED(repaint);

    if (recordsToDelete.isEmpty())
        return;
    int last_r = 0;
    TableViewDataIterator it(begin());
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

void TableViewData::insertRecord(RecordData *record, uint index, bool repaint)
{
    Q_UNUSED(record);
    insert(index = qMin(index, count()), record);
    emit recordInserted(record, index, repaint);
}

void TableViewData::clearInternal(bool processEvents)
{
    clearRecordEditBuffer();
//! @todo this is time consuming: find better data model
    const uint c = count();
#ifndef TABLEVIEW_NO_PROCESS_EVENTS
    const bool _processEvents = processEvents && !qApp->closingDown();
#endif
    for (uint i = 0; i < c; i++) {
        removeLast();
#ifndef TABLEVIEW_NO_PROCESS_EVENTS
        if (_processEvents && i % 1000 == 0)
            qApp->processEvents(QEventLoop::AllEvents, 1);
#endif
    }
}

bool TableViewData::deleteAllRecords(bool repaint)
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

int TableViewData::autoIncrementedColumn()
{
    if (d->autoIncrementedColumn == -2) {
        //find such a column
        d->autoIncrementedColumn = -1;
        foreach(TableViewColumn *col, m_columns) {
            d->autoIncrementedColumn++;
            if (col->field()->isAutoIncrement())
                break;
        }
    }
    return d->autoIncrementedColumn;
}

bool TableViewData::preloadAllRecords()
{
    if (!d->cursor)
        return false;
    if (!d->cursor->moveFirst() && d->cursor->result().isError())
        return false;

#ifndef TABLEVIEW_NO_PROCESS_EVENTS
    const bool processEvents = !qApp->closingDown();
#endif

    for (int i = 0;!d->cursor->eof();i++) {
        RecordData *record = d->cursor->storeCurrentRecord();
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

bool TableViewData::isReadOnly() const
{
    return d->readOnly || (d->cursor && d->cursor->connection()->isReadOnly());
}

// static
QString TableViewData::messageYouCanImproveData()
{
    return tr("You can correct data in this record or use \"Cancel record changes\" function.");
}
