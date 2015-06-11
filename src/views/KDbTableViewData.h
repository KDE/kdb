/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003-2014 Jarosław Staniek <staniek@kde.org>
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

#ifndef KDB_TABLEVIEWDATA_H
#define KDB_TABLEVIEWDATA_H

#include "KDbUtils.h"
#include "KDbRecordData.h"
#include "KDbTableViewColumn.h"

class KDbRecordEditBuffer;
class KDbCursor;

typedef KDbUtils::AutodeletedList<KDbRecordData*> KDbTableViewDataBase;
typedef KDbTableViewDataBase::ConstIterator KDbTableViewDataConstIterator;
typedef KDbTableViewDataBase::Iterator KDbTableViewDataIterator;

//! A list of records to allow configurable sorting and more.
/*! @todo improve API */
class KDB_EXPORT KDbTableViewData : public QObject, protected KDbTableViewDataBase
{
    Q_OBJECT

public:
    //! Non-db-aware version
    KDbTableViewData();

    //! Db-aware version. The cursor is not owned by the data.
    explicit KDbTableViewData(KDbCursor *c);

    /*! Defines two-column table usually used with comboboxes.
     First column is invisible and contains key values.
     Second column and contains user-visible value.
     @param keys a list of keys
     @param values a list of text values (must be of the same length as keys list)
     @param keyType a type for keys
     @param valueType a type for values

     @todo make this more generic: allow to add more columns! */
    KDbTableViewData(
        const QList<QVariant> &keys, const QList<QVariant> &values,
        KDbField::Type keyType = KDbField::Text, KDbField::Type valueType = KDbField::Text);

    /*! Like above constructor, but keys and values are not provided.
     You can do this later by calling append(KDbRecordData*) method.
     (KDbRecordData object must have exactly two columns) */
    KDbTableViewData(KDbField::Type keyType, KDbField::Type valueType);

    virtual ~KDbTableViewData();

    /*! Preloads all records provided by cursor (only for db-aware version). */
    bool preloadAllRecords();

    /*! Sets sorting for @a column. If @a column is -1, sorting is disabled. */
    void setSorting(int column, Qt::SortOrder OrderByColumn = Qt::AscendingOrder);

    /*! @return the column number by which the data is sorted,
     or -1 if sorting is disabled.
     Initial sorted column number for data after instantiating object is -1. */
    int sortColumn() const;

    /*! @return sorting order. This is independent of whether the data is actually sorted.
     sortColumn() should be checked first to see if sorting for any column is enabled
     (by default it is not). */
    Qt::SortOrder sortOrder() const;

    //! Sorts this data using previously set order.
    void sort();

    /*! Adds column @a col.
     Warning: @a col will be owned by this object, and deleted on its destruction. */
    void addColumn(KDbTableViewColumn* col);

    //! @return Index of visible column @a visibleIndex on global list.
    int globalIndexOfVisibleColumn(int visibleIndex) const;

    //! @return Index on list of visible columns for column @a globalIndex
    //!         or -1 if column at @a globalIndex is not visible.
    int visibleColumnIndex(int globalIndex) const;

    /*! @return true if this db-aware data set. */
    /*! @todo virtual? */
    bool isDBAware() const;

    /*! For db-aware data set only: table name is returned;
     equivalent to cursor()->query()->parentTable()->name(). */
    QString dbTableName() const;

    KDbCursor* cursor() const;

    uint columnCount() const;

    //! @return number of visible columns
    uint visibleColumnCount() const;

    //! @return column at index @a index (visible or not)
    KDbTableViewColumn* column(uint c);

    //! @return visible column at index @a index
    KDbTableViewColumn* visibleColumn(uint index);

    //! @return list of all columns
    QList<KDbTableViewColumn*>* columns();

    //! @return list of visible columns
    QList<KDbTableViewColumn*>* visibleColumns();

    /*! @return true if data is not editable. Can be set using setReadOnly()
     but it's still true if database cursor returned by cursor()
     is not 0 and has read-only connection. */
    virtual bool isReadOnly() const;

    /*! Sets readOnly flag for this data.
     If @a set is true, insertingEnabled flag will be cleared automatically.
     @see isInsertingEnabled() */
    virtual void setReadOnly(bool set);

    /*! @return true if data inserting is enabled (the default). */
    virtual bool isInsertingEnabled() const;

    /*! Sets insertingEnabled flag. If true, empty record is available
     If @a set is true, read-only flag will be cleared automatically.
     @see setReadOnly() */
    virtual void setInsertingEnabled(bool set);

    /*! Clears and initializes internal record edit buffer for incoming editing.
     Creates buffer using recordEditBuffer(false) (false means not db-aware type)
     if our data is not db-aware,
     or db-aware buffer if data is db-aware (isDBAware()==true).
     @see KDbRecordEditBuffer */
    void clearRecordEditBuffer();

    /*! Updates internal record edit buffer: currently edited column @a col (number @a colnum)
     has now assigned new value of @a newval.
     Uses column's caption to address the column in buffer
     if the buffer is of simple type, or db-aware buffer if (isDBAware()==true).
     (then fields are addressed with KDbField, instead of caption strings).
     If @a allowSignals is true (the default), aboutToChangeCell() signal is emitted.
     @a visibleValueForLookupField allows to pass visible value (usually a text)
     for a lookup field (only reasonable if col->visibleLookupColumnInfo != 0).
     Note that @a newval may be changed in aboutToChangeCell() signal handler.
     @see KDbRecordEditBuffer */
    bool updateRecordEditBufferRef(KDbRecordData *record,
                                   int colnum, KDbTableViewColumn* col, QVariant* newval,
                                   bool allowSignals = true,
                                   QVariant *visibleValueForLookupField = 0);

    /*! Added for convenience. Like above but @a newval is passed by value. */
    bool updateRecordEditBuffer(KDbRecordData *record, int colnum, KDbTableViewColumn* col,
                                const QVariant &newval, bool allowSignals = true);

    /*! Added for convenience. Like above but it's assumed that @a record record's columns
     are ordered like in table view, not like in form view. Don't use this with form views. */
    bool updateRecordEditBuffer(KDbRecordData *record, int colnum,
                                const QVariant &newval, bool allowSignals = true);

    //! @return record edit buffer for currently edited record. Can be 0 or empty.
    KDbRecordEditBuffer* recordEditBuffer() const;

    /*! @return last operation's result information (always not null). */
    const KDbResultInfo& result() const;

    bool saveRecordChanges(KDbRecordData *record, bool repaint = false);

    bool saveNewRecord(KDbRecordData *record, bool repaint = false);

    bool deleteRecord(KDbRecordData *record, bool repaint = false);

    /*! Deletes records (by number) passed with @a recordsToDelete.
     Currently, this method is only for non data-aware tables. */
    void deleteRecords(const QList<int> &recordsToDelete, bool repaint = false);

    /*! Deletes all records. Works either for db-aware and non db-aware tables.
     Column's definition is not changed.
     For db-aware version, all records are removed from a database.
     Record-edit buffer is cleared.

     If @a repaint is true, reloadRequested() signal
     is emitted after deleting (if at least one record was deleted),
     so presenters can repaint their contents.

     @return true on success. */
    virtual bool deleteAllRecords(bool repaint = false);

    /*! @internal method, used mostly by specialized classes like KexiTableView.
     Clears internal record structures. Record-edit buffer is cleared.
     Does not touch data @ database backend.
     Use deleteAllRecords() to safely delete all records. */
    virtual void clearInternal(bool processEvents = true);

    /*! Inserts new @a record at index @a index.
     @a record will be owned by this data object.
     Note: Reasonable only for not not-db-aware version. */
    void insertRecord(KDbRecordData *record, uint index, bool repaint = false);

    //! @todo add this as well? void insertRecord(KDbRecordData *record, KDbRecordData *aboveRecord)

    //! @return index of autoincremented column. The result is cached.
    //! @todo what about multiple autoinc columns?
    //! @todo what about changing column order?
    int autoIncrementedColumn();

    //! Emits reloadRequested() signal to reload presenters.
    void reload() {
        emit reloadRequested();
    }

    inline KDbRecordData* at(uint index) {
        return KDbTableViewDataBase::at(index);
    }
    inline virtual uint count() const {
        return KDbTableViewDataBase::count();
    }
    inline bool isEmpty() const {
        return KDbTableViewDataBase::isEmpty();
    }
    inline KDbRecordData* first() {
        return KDbTableViewDataBase::first();
    }
    inline KDbRecordData* last() {
        return KDbTableViewDataBase::last();
    }
    inline int indexOf(const KDbRecordData* record, int from = 0) const {
        return KDbTableViewDataBase::indexOf(const_cast<KDbRecordData*>(record), from);
    }
    inline void removeFirst() {
        KDbTableViewDataBase::removeFirst();
    }
    inline void removeLast() {
        KDbTableViewDataBase::removeLast();
    }
    inline void append(KDbRecordData* record) {
        KDbTableViewDataBase::append(record);
    }
    inline void prepend(KDbRecordData* record) {
        KDbTableViewDataBase::prepend(record);
    }
    inline KDbTableViewDataConstIterator constBegin() const {
        return KDbTableViewDataBase::constBegin();
    }
    inline KDbTableViewDataConstIterator constEnd() const {
        return KDbTableViewDataBase::constEnd();
    }
    inline KDbTableViewDataIterator begin() {
        return KDbTableViewDataBase::begin();
    }
    inline KDbTableViewDataIterator end() {
        return KDbTableViewDataBase::end();
    }

    /*! @return true if ROWID information is stored within every record.
     Only reasonable for db-aware version. ROWID information is available
     if KDbDriverBehaviour::ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE == false
     for a KDb database driver and a table has no primary key defined.
     Phisically, ROWID information is stored after last KDbRecordData's element,
     so every KDbRecordData's length is expanded by one. */
    bool containsRecordIdInfo() const;

    //! Creates a single record data with proper number of columns.
    KDbRecordData* createItem() const;

    //! @return reusable i18n'd message
    //!         "You can correct data in this record or use 'Cancel record changes' function."
    static QString messageYouCanImproveData();

public Q_SLOTS:
    //! @internal Clean up.
    void deleteLater();

Q_SIGNALS:
    void destroying();

    /*! Emitted before change of the single, currently edited cell.
     Connect this signal to your slot and set @a result->success to false
     to disallow this change. You can also change @a newValue to other value,
     or change other columns in @a record. */
    void aboutToChangeCell(KDbRecordData *record, int colnum, QVariant* newValue,
                           KDbResultInfo* result);

    /*! Emitted before inserting of a new, current record.
     Connect this signal to your slot and set @a result->success to false
     to disallow this inserting. You can also change columns in @a record. */
    void aboutToInsertRecord(KDbRecordData *record, KDbResultInfo* result, bool repaint);

    /*! Emitted before changing of an edited, current record.
     Connect this signal to your slot and set @a result->success to false
     to disallow this change. You can also change columns in @a record. */
    void aboutToUpdateRecord(KDbRecordData *record, KDbRecordEditBuffer* buffer,
                             KDbResultInfo* result);

    void recordUpdated(KDbRecordData*); //!< Current record has been updated

    void recordInserted(KDbRecordData*, bool repaint); //!< A record has been inserted

    //! A record has been inserted at @a index position (not db-aware data only)
    void recordInserted(KDbRecordData*, uint index, bool repaint);

    /*! Emitted before deleting of a current record.
     Connect this signal to your slot and set @a result->success to false
     to disallow this deleting. */
    void aboutToDeleteRecord(KDbRecordData *record, KDbResultInfo* result, bool repaint);

    //! Current record has been deleted
    void recordDeleted();

    //! Records have been deleted
    void recordsDeleted(const QList<int> &recordsToDelete);

    //! Displayed data needs to be reloaded in all presenters.
    void reloadRequested();

    void recordRepaintRequested(KDbRecordData*);

protected:
    //! Used by KDbTableViewColumn::setVisible()
    void columnVisibilityChanged(const KDbTableViewColumn &column);

private:
    void init();
    void init(const QList<QVariant> &keys, const QList<QVariant> &values,
              KDbField::Type keyType, KDbField::Type valueType);

    //! @internal for saveRecordChanges() and saveNewRecord()
    bool saveRecord(KDbRecordData *record, bool insert, bool repaint);

    friend class KDbTableViewColumn;

    class Private;
    Private * const d;
};

#endif
