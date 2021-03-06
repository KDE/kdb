/* This file is part of the KDE project
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_CURSOR_H
#define KDB_CURSOR_H

#include <QString>
#include <QVariant>

#include "KDbResult.h"
#include "KDbQueryColumnInfo.h"

class KDbConnection;
class KDbRecordData;
class KDbQuerySchema;
class KDbRecordEditBuffer;

//! Provides database cursor functionality.
/*!
  Cursor can be defined in two ways:

  -# by passing KDbQuerySchema object to KDbConnection::executeQuery() or KDbConnection::prepareQuery();
     then query is defined for in engine-independent way -- this is recommended usage

  -# by passing raw query statement string to KDbConnection::executeQuery() or KDbConnection::prepareQuery();
     then query may be defined for in engine-dependent way -- this is not recommended usage,
     but convenient when we can't or do not want to allocate KDbQuerySchema object, while we
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
  - Do not use delete operator for KDbCursor objects - this will fail; use KDbConnection::deleteCursor()
  instead.
  - KDbQuerySchema object is not owned by KDbCursor object that uses it.
*/
class KDB_EXPORT KDbCursor: public KDbResultable
{
    Q_DECLARE_TR_FUNCTIONS(KDbCursor)
public:
    //! Options that describe behavior of database cursor
    enum class Option {
        None = 0,
        Buffered = 1
    };
    Q_DECLARE_FLAGS(Options, Option)

    /*! @return connection used for the cursor */
    KDbConnection* connection();

    //! @overload
    //! @since 3.1
    const KDbConnection* connection() const;

    /*! Opens the cursor using data provided on creation.
     The data might be either KDbQuerySchema or a raw SQL statement. */
    bool open();

    /*! Closes and then opens again the same cursor.
     If the cursor is not opened it is just opened and result of this open is returned.
     Otherwise, true is returned if cursor is successfully closed and then opened. */
    bool reopen();

    /*! Closes previously opened cursor.
      If the cursor is closed, nothing happens. */
    virtual bool close();

    /*! @return query schema used to define this cursor
     or 0 if the cursor is not defined by a query schema but by a raw SQL statement. */
    KDbQuerySchema *query() const;

    //! @return query parameters assigned to this cursor
    QList<QVariant> queryParameters() const;

    //! Sets query parameters @a params for this cursor.
    void setQueryParameters(const QList<QVariant>& params);

    /*! @return raw query statement used to define this cursor
     or null string if raw statement instead (but KDbQuerySchema is defined instead). */
    KDbEscapedString rawSql() const;

    /*! @return cursor options */
    Options options() const;

    /*! @return true if the cursor is opened. */
    bool isOpened() const;

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
        return readAhead() ? 0 : (m_at - 1);
    }

    /*! @return number of fields available for this cursor.
     This never includes ROWID column or other internal columns (e.g. lookup). */
    inline int fieldCount() const {
        return m_query ? m_logicalFieldCount : m_fieldCount;
    }

    /*! @return true if ROWID information is available for each record.
     ROWID information is available
     if KDbDriverBehavior::ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE == false
     for a KDb database driver and the master table has no primary key defined.
     Phisically, ROWID value is returned after last returned field,
     so data vector's length is expanded by one. */
    bool containsRecordIdInfo() const;

    /*! @return a value stored in column number @a i (counting from 0).
     It has unspecified behavior if the cursor is not at valid record.
     Note for driver developers:
     If @a i is >= than m_fieldCount, null QVariant value should be returned.
     To return a value typically you can use a pointer to internal structure
     that contain current record data (buffered or unbuffered). */
    virtual QVariant value(int i) = 0;

    /*! [PROTOTYPE] @return current record data or @c nullptr if there is no current records. */
    virtual const char ** recordData() const = 0;

    /*! Sets a list of columns for ORDER BY section of the query.
     Only works when the cursor has been created using KDbQuerySchema object
     (i.e. when query()!=0; does not work with raw statements).
     Each name on the list must be a field or alias present within the query
     and must not be covered by aliases. If one or more names cannot be found within
     the query, the method will have no effect. Any previous ORDER BY settings will be removed.

     The order list provided here has priority over a list defined in the KDbQuerySchema
     object itseld (using KDbQuerySchema::setOrderByColumnList()).
     The KDbQuerySchema object itself is not modifed by this method: only order of records retrieved
     by this cursor is affected.

     Use this method before calling open(). You can also call reopen() after calling this method
     to see effects of applying records order. */
    //! @todo implement this
    void setOrderByColumnList(const QStringList& columnNames);

    /*! Convenience method, similar to setOrderByColumnList(const QStringList&). */
    //! @todo implement this
    void setOrderByColumnList(const QString& column1, const QString& column2 = QString(),
                              const QString& column3 = QString(), const QString& column4 = QString(),
                              const QString& column5 = QString());

    /*! @return a list of fields contained in ORDER BY section of the query.
     @see setOrderBy(const QStringList&) */
    KDbQueryColumnInfo::Vector orderByColumnList() const;

    /*! Allocates a new KDbRecordData and stores data in it (makes a deep copy of each field).
     If the cursor is not at valid record, the result is undefined.
     @return newly created record data object or 0 on error. */
    KDbRecordData* storeCurrentRecord() const;

    /*! Puts current record's data into @a data (makes a deep copy of each field).
     If the cursor is not at valid record, the result is undefined.
     @return true on success.
     @c false is returned if @a data is @c nullptr. */
    bool storeCurrentRecord(KDbRecordData* data) const;

    bool updateRecord(KDbRecordData* data, KDbRecordEditBuffer* buf, bool useRecordId = false);

    bool insertRecord(KDbRecordData* data, KDbRecordEditBuffer* buf, bool getRecrordId = false);

    bool deleteRecord(KDbRecordData* data, bool useRecordId = false);

    bool deleteAllRecords();

protected:
    /*! Cursor will operate on @a conn, raw SQL statement @a sql will be used to execute query. */
    KDbCursor(KDbConnection* conn, const KDbEscapedString& sql, Options options = KDbCursor::Option::None);

    /*! Cursor will operate on @a conn, @a query schema will be used to execute query. */
    KDbCursor(KDbConnection* conn, KDbQuerySchema* query, Options options = KDbCursor::Option::None);

    ~KDbCursor() override;

    void init(KDbConnection* conn);

    /*! Internal: cares about proper flag setting depending on result of drv_getNextRecord()
     and depending on wherher a cursor is buffered. */
    bool getNextRecord();

    /*! Note for driver developers: this method should initialize engine-specific cursor's
     resources using an SQL statement @a sql. It is not required to store @a sql statement somewhere
     in your KDbCursor subclass (it is already stored in m_query or m_rawStatement,
     depending query type) - only pass it to proper engine's function. */
    virtual bool drv_open(const KDbEscapedString& sql) = 0;

    virtual bool drv_close() = 0;
    virtual void drv_getNextRecord() = 0;

    /*! Stores currently fetched record's values in appropriate place of the buffer.
     Note for driver developers:
     This place can be computed using m_at. Do not change value of m_at or any other
     KDbCursor members, only change your internal structures like pointer to current
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

    /*! Clears cursor's buffer if this was allocated (only for buffered cursor type).
      Otherwise do nothing. For reimplementing. Default implementation does nothing. */
    virtual void drv_clearBuffer() {}

    //! @internal clears buffer with reimplemented drv_clearBuffer(). */
    void clearBuffer();

    /*! Puts current record's data into @a data (makes a deep copy of each field).
     This method has unspecified behavior if the cursor is not at valid record.
     @return true on success.
     Note: For reimplementation in driver's code. Shortly, this method translates
     a record data from internal representation (probably also used in buffer)
     to simple public KDbRecordData representation. */
    virtual bool drv_storeCurrentRecord(KDbRecordData* data) const = 0;

    KDbQuerySchema *m_query;
    bool m_afterLast;
    qint64 m_at;
    int m_fieldCount; //!< cached field count information
    int m_fieldsToStoreInRecord; //!< Used by storeCurrentRecord(), reimplement if needed
                                  //!< (e.g. PostgreSQL driver, when m_containsRecordIdInfo is true
                                  //!< sets m_fieldCount+1 here)
    int m_logicalFieldCount;  //!< logical field count, i.e. without internal values like Record Id or lookup
    KDbCursor::Options m_options; //!< cursor options that describes its behavior

    //! Possible results of record fetching, used for m_fetchResult
    enum class FetchResult {
        Invalid, //!< used before starting the fetching, result is not known yet
        Error,   //!< error of fetching
        Ok,      //!< the data is fetched
        End      //!< at the end of data
    };

    FetchResult m_fetchResult; //!< result of a record fetching

    //<members related to buffering>
    int m_records_in_buf;         //!< number of records currently stored in the buffer
    bool m_buffering_completed;   //!< true if we already have all records stored in the buffer
    //</members related to buffering>

    //! Useful e.g. for value(int) method to obtain access to schema definition.
    KDbQueryColumnInfo::Vector* m_visibleFieldsExpanded;

private:
    bool readAhead() const;

    Q_DISABLE_COPY(KDbCursor)
    friend class CursorDeleter;
    class Private;
    Private * const d;
};

//! Sends information about object @a cursor to debug output @a dbg.
//! @since 3.1
KDB_EXPORT QDebug operator<<(QDebug dbg, KDbCursor& cursor);

//! Sends information about object @a cursor to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbCursor& cursor);

Q_DECLARE_OPERATORS_FOR_FLAGS(KDbCursor::Options)

#endif
