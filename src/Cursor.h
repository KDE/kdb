/* This file is part of the KDE project
   Copyright (C) 2003-2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef PREDICATE_CURSOR_H
#define PREDICATE_CURSOR_H

#include <QString>
#include <QVariant>

#include <Predicate/Connection>

namespace Predicate
{

class RecordEditBuffer;

//! Provides database cursor functionality.
/*!
  Cursor can be defined in two ways:

  -# by passing QuerySchema object to Connection::executeQuery() or Connection::prepareQuery();
     then query is defined for in engine-independent way -- this is recommended usage

  -# by passing raw query statement string to Connection::executeQuery() or Connection::prepareQuery();
     then query may be defined for in engine-dependent way -- this is not recommended usage,
     but convenient when we can't or do not want to allocate QuerySchema object, while we
     know that the query statement is syntactically and logically ok in our context.

  You can move cursor to next record with moveNext() and move back with movePrev().
  The cursor is always positioned on record, not between records, with exception that
  after open() it is positioned before the first record (if any) -- then bof() equals true.
  The cursor can also be positioned after the last record (if any) with moveNext() -- then eof() equals true.
  For example, if you have four records, 1, 2, 3, 4, then after calling open(), moveNext(),
  moveNext(), moveNext(), movePrev() you are going through records: 1, 2, 3, 2.

  Cursor can be buffered or unbuferred.
  
  @warning Buffered cursors are not implemented!

  Buffering in this class is not related to any SQL engine capatibilities for server-side cursors
  (eg. like 'DECLARE CURSOR' statement) - buffered data is at client (application) side.
  Any record retrieved in buffered cursor will be stored inside an internal buffer
  and reused when needed. Unbuffered cursor always requires one record fetching from
  db connection at every step done with moveNext(), movePrev(), etc.

  Notes:
  - Do not use delete operator for Cursor objects - this will fail; use Connection::deleteCursor()
  instead.
  - QuerySchema object is not owned by Cursor object that uses it.
*/
class PREDICATE_EXPORT Cursor: public Resultable
{
public:
    //! Cursor options that describes its behaviour
    enum Options {
        NoOptions = 0,
        Buffered = 1
    };

    virtual ~Cursor();

    /*! @return connection used for the cursor */
    inline Connection* connection() const {
        return m_conn;
    }

    /*! Opens the cursor using data provided on creation.
     The data might be either QuerySchema or raw sql statement. */
    bool open();

    /*! Closes and then opens again the same cursor.
     If the cursor is not opened it is just opened and result of this open is returned.
     Otherwise, true is returned if cursor is successfully closed and then opened. */
    bool reopen();

//  /*! Opens the cursor using @a statement.
//   Omit @a statement if cursor is already initialized with statement
//   at creation time. If @a statement is not empty, existing statement
//   (if any) is overwritten. */
//  bool open( const QString& statement = QString() );

    /*! Closes previously opened cursor.
      If the cursor is closed, nothing happens. */
    virtual bool close();

    /*! @return query schema used to define this cursor
     or NULL if the cursor is not defined by a query schema but by a raw statement. */
    inline QuerySchema *query() const {
        return m_query;
    }

    //! @return query parameters assigned to this cursor
    QList<QVariant> queryParameters() const;

    //! Sets query parameters @a params for this cursor.
    void setQueryParameters(const QList<QVariant>& params);

    /*! @return raw query statement used to define this cursor
     or null string if raw statement instead (but QuerySchema is defined instead). */
    inline EscapedString rawStatement() const {
        return m_rawStatement;
    }

    /*! @return logically or'd cursor's options,
      selected from Cursor::Options enum. */
    inline uint options() const {
        return m_options;
    }

    /*! @return true if the cursor is opened. */
    inline bool isOpened() const {
        return m_opened;
    }

    /*! @return true if the cursor is buffered. */
    bool isBuffered() const;

    /*! Sets this cursor to buffered type or not. See description
      of buffered and nonbuffered cursors in class description.
      This method only works if cursor is not opened (isOpened()==false).
      You can close already opened cursor and then switch this option on/off.
    */
    void setBuffered(bool buffered);

    /*! Moves current position to the first record and retrieves it.
      @return true if the first record was retrieved.
      False could mean that there was an error or there is no record available. */
    bool moveFirst();

    /*! Moves current position to the last record and retrieves it.
      @return true if the last record was retrieved.
      False could mean that there was an error or there is no record available. */
    virtual bool moveLast();

    /*! Moves current position to the next record and retrieves it. */
    virtual bool moveNext();

    /*! Moves current position to the next record and retrieves it.
     Currently it's only supported for buffered cursors. */
    virtual bool movePrev();

    /*! @return true if current position is after last record. */
    inline bool eof() const {
        return m_afterLast;
    }

    /*! @return true if current position is before first record. */
    inline bool bof() const {
        return m_at == 0;
    }

    /*! @return current internal position of the cursor's query.
     We are counting records from 0.
     Value -1 means that cursor does not point to any valid record
     (this happens eg. after open(), close(),
     and after moving after last record or before first one. */
    inline qint64 at() const {
        return m_readAhead ? 0 : (m_at - 1);
    }

    /*! @return number of fields available for this cursor.
     This never includes ROWID column or other internal columns (e.g. lookup). */
    inline uint fieldCount() const {
        return m_query ? m_logicalFieldCount : m_fieldCount;
    }

    /*! @return true if ROWID information is available for each record.
     ROWID information is available
     if DriverBehaviour::ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE == false
     for a Predicate database driver and the master table has no primary key defined.
     Phisically, ROWID value is returned after last returned field,
     so data vector's length is expanded by one. */
    inline bool containsRecordIdInfo() const {
        return m_containsRecordIdInfo;
    }

    /*! @return a value stored in column number @a i (counting from 0).
     Is has unspecified behaviour if the cursor is not at valid record.
     Note for driver developers:
     If @a i is >= than m_fieldCount, null QVariant value should be returned.
     To return a value typically you can use a pointer to internal structure
     that contain current record data (buffered or unbuffered). */
    virtual QVariant value(uint i) = 0;

    /*! [PROTOTYPE] @return current record data or NULL if there is no current records. */
    virtual const char ** recordData() const = 0;

    /*! Sets a list of columns for ORDER BY section of the query.
     Only works when the Cursor.has been created using QuerySchema object
     (i.e. when query()!=0; does not work with raw statements).
     Each name on the list must be a field or alias present within the query
     and must not be covered by aliases. If one or more names cannot be found within
     the query, the method will have no effect. Any previous ORDER BY settings will be removed.

     The order list provided here has priority over a list defined in the QuerySchema
     object itseld (using QuerySchema::setOrderByColumnList()).
     The QuerySchema object itself is not modifed by this method: only order of records retrieved
     by this cursor is affected.

     Use this method before calling open(). You can also call reopen() after calling this method
     to see effects of applying records order. */
    void setOrderByColumnList(const QStringList& columnNames);

    /*! Convenience method, similar to setOrderByColumnList(const QStringList&). */
    void setOrderByColumnList(const QString& column1, const QString& column2 = QString(),
                              const QString& column3 = QString(), const QString& column4 = QString(),
                              const QString& column5 = QString());

    /*! @return a list of fields contained in ORDER BY section of the query.
     @see setOrderBy(const QStringList&) */
    QueryColumnInfo::Vector orderByColumnList() const;

    /*! Allocates a new RecordData and stores data in it (makes a deep copy of each field).
     If the cursor is not at valid record, the result is undefined.
     @return newly created record data object or 0 on error. */
    inline RecordData* storeCurrentRecord() const {
        RecordData* data = new RecordData(m_fieldsToStoreInRecord);
        if (!drv_storeCurrentRecord(data)) {
            delete data;
            return 0;
        }
        return data;
    }

    /*! Puts current record's data into @a data (makes a deep copy of each field).
     If the cursor is not at valid record, the result is undefined.
     @return true on success. */
    inline bool storeCurrentRecord(RecordData* data) const {
        data->resize(m_fieldsToStoreInRecord);
        return drv_storeCurrentRecord(data);
    }

    bool updateRecord(RecordData* data, RecordEditBuffer* buf, bool useRecordId = false);

    bool insertRecord(RecordData* data, RecordEditBuffer* buf, bool getRecrordId = false);

    bool deleteRecord(RecordData* data, bool useRecordId = false);

    bool deleteAllRecords();

protected:
    /*! Cursor will operate on @a conn, raw @a statement will be used to execute query. */
    Cursor(Connection* conn, const EscapedString& statement, uint options = NoOptions);

    /*! Cursor will operate on @a conn, @a query schema will be used to execute query. */
    Cursor(Connection* conn, QuerySchema* query, uint options = NoOptions);

    void init();

    /*! Internal: cares about proper flag setting depending on result of drv_getNextRecord()
     and depending on wherher a cursor is buffered. */
    bool getNextRecord();

    /* Note for driver developers: this method should initialize engine-specific cursor's
     resources using m_sql statement. It is not required to store @a sql statement somewhere
     in your Cursor subclass (it is already stored in m_query or m_rawStatement,
     depending query type) - only pass it to proper engine's function. */
    virtual bool drv_open(const EscapedString& sql) = 0;

    virtual bool drv_close() = 0;
//  virtual bool drv_moveFirst() = 0;
    virtual void drv_getNextRecord() = 0;
//unused  virtual bool drv_getPrevRecord() = 0;

    /*! Stores currently fetched record's values in appropriate place of the buffer.
     Note for driver developers:
     This place can be computed using m_at. Do not change value of m_at or any other
     Cursor members, only change your internal structures like pointer to current
     record, etc. If your database engine's API function (for record fetching)
     do not allocates such a space, you want to allocate a space for current
     record. Otherwise, reuse existing structure, what could be more efficient.
     All functions like drv_appendCurrentRecordToBuffer() operates on the buffer,
     i.e. array of stored records. You are not forced to have any particular
     fixed structure for buffer item or buffer itself - the structure is internal and
     only methods like storeCurrentRecord() visible to public.
    */
    virtual void drv_appendCurrentRecordToBuffer() = 0;
    /*! Moves pointer (that points to the buffer) -- to next item in this buffer.
     Note for driver developers: probably just execute "your_pointer++" is enough.
    */
    virtual void drv_bufferMovePointerNext() = 0;
    /*! Like drv_bufferMovePointerNext() but execute "your_pointer--". */
    virtual void drv_bufferMovePointerPrev() = 0;
    /*! Moves pointer (that points to the buffer) to a new place: @a at.
    */
    virtual void drv_bufferMovePointerTo(qint64 at) = 0;

    /*DISABLED: ! This is called only once in open(), after successful drv_open().
      Reimplement this if you need (or not) to do get the first record after drv_open(),
      eg. to know if there are any records in table. Value returned by this method
      will be assigned to m_readAhead.
      Default implementation just calls drv_getNextRecord(). */

    /*! Clears cursor's buffer if this was allocated (only for buffered cursor type).
      Otherwise do nothing. For reimplementing. Default implementation does nothing. */
    virtual void drv_clearBuffer() {}

    //! @internal clears buffer with reimplemented drv_clearBuffer(). */
    void clearBuffer();

    /*! Puts current record's data into @a data (makes a deep copy of each field).
     This method has unspecified behaviour if the cursor is not at valid record.
     @return true on success.
     Note: For reimplementation in driver's code. Shortly, this method translates
     a record data from internal representation (probably also used in buffer)
     to simple public RecordData representation. */
    virtual bool drv_storeCurrentRecord(RecordData* data) const = 0;

//    QPointer<Connection> m_conn;
    Connection *m_conn;
    QuerySchema *m_query;
//  CursorData *m_data;
    EscapedString m_rawStatement;
    bool m_opened;
//js (m_at==0 is enough)  bool m_beforeFirst : 1;
    bool m_atLast;
    bool m_afterLast;
//  bool m_atLast;
    bool m_validRecord; //!< true if valid record is currently retrieved @ current position
    bool m_containsRecordIdInfo; //!< true if result contains extra column for record id;
                                 //!< used only for PostgreSQL now
    qint64 m_at;
    uint m_fieldCount; //!< cached field count information
    uint m_fieldsToStoreInRecord; //!< Used by storeCurrentRecord(), reimplement if needed
                                  //!< (e.g. PostgreSQL driver, when m_containsRecordIdInfo is true
                                  //!< sets m_fieldCount+1 here)
    uint m_logicalFieldCount;  //!< logical field count, i.e. without intrernal values like Record Id or lookup
    uint m_options; //!< cursor options that describes its behaviour

    //! possible results of record fetching, used for m_fetchResult
    enum FetchResult {
        FetchInvalid, //!< used before starting the fetching, result is not known yet
        FetchError, //!< error of fetching
        FetchOK, //!< the data is fetched
        FetchEnd //!< at the end of data
    };

    FetchResult m_fetchResult; //!< result of a record fetching

    //<members related to buffering>
    int m_records_in_buf;         //!< number of records currently stored in the buffer
    bool m_buffering_completed;   //!< true if we already have all records stored in the buffer
    //</members related to buffering>

    //! Useful e.g. for value(int) method when we need access to schema def.
    QueryColumnInfo::Vector* m_fieldsExpanded;

    //! Used by setOrderByColumnList()
    QueryColumnInfo::Vector* m_orderByColumnList;

    QList<QVariant>* m_queryParameters;

private:

    bool m_readAhead;

    //<members related to buffering>
    bool m_at_buffer; //!< true if we already point to the buffer with curr_coldata
    //</members related to buffering>
};

} //namespace Predicate

//! Sends information about object @a cursor to debug output @a dbg.
PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Predicate::Cursor& cursor);

#endif
