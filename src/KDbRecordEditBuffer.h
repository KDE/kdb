/* This file is part of the KDE project
   Copyright (C) 2003, 2011 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_RECORDEDITBUFFER_H
#define KDB_RECORDEDITBUFFER_H

#include <QMap>
#include <QVariant>

#include "kdb_export.h"

class KDbQueryColumnInfo;
class KDbField;

/*!  @short provides data for single edited database record

  KDbRecordEditBuffer provides data for single edited record,
  needed to perform update at the database backend.
  Its advantage over pasing e.g. KDbFieldList object is that
  EditBuffer contains only changed values.

  EditBuffer offers two modes: db-aware and not-db-aware.
  Db-aware buffer addresses a field using references to KDbQueryColumnInfo object,
  while not-db-aware buffer addresses a field using its name.

  Example usage of not-db-aware buffer:
  <code>
  KDbQuerySchema *query = .....
  EditBuffer buf;
  buf.insert("name", "Joe");
  buf.insert("surname", "Black");
  buf.at("name"); //returns "Joe"
  buf.at("surname"); //returns "Black"
  buf.at(query->field("surname")); //returns "Black" too
  // Now you can use buf to add or edit records using
  // KDbConnection::updateRecord(), KDbConnection::insertRecord()
  </code>

  Example usage of db-aware buffer:
  <code>
  KDbQuerySchema *query = .....
  KDbQueryColumnInfo *ci1 = ....... //e.g. can be obtained from QueryScehma::fieldsExpanded()
  KDbQueryColumnInfo *ci2 = .......
  EditBuffer buf;
  buf.insert(*ci1, "Joe");
  buf.insert(*ci2, "Black");
  buf.at(*ci1); //returns "Joe"
  buf.at(*ci2); //returns "Black"
  // Now you can use buf to add or edit records using
  // KDbConnection::updateRecord(), KDbConnection::insertRecord()
  </code>

  You can use QMap::clear() to clear buffer contents,
  QMap::isEmpty() to see if buffer is empty.
  For more, see QMap documentation.

  Notes: added fields should come from the same (common) KDbQuerySchema object.
  However, this isn't checked at QValue& EditBuffer::operator[]( const KDbField& f ) level.
*/
class KDB_EXPORT KDbRecordEditBuffer
{
public:
    typedef QMap<QString, QVariant> SimpleMap;
    typedef QHash<KDbQueryColumnInfo*, QVariant> DbHash;

    explicit KDbRecordEditBuffer(bool dbAwareBuffer);

    ~KDbRecordEditBuffer();

    bool isDBAware() const;

    void clear();

    bool isEmpty() const;

    //! Inserts value @a val for db-aware buffer's column @a ci
    void insert(KDbQueryColumnInfo* ci, const QVariant &val);

    //! Inserts value @a val for not-db-aware buffer's column @a fname
    void insert(const QString &fname, const QVariant &val);

    //! Removes value from db-aware buffer's column @a ci
    void removeAt(const KDbQueryColumnInfo& ci);

    //! Removes value from not-db-aware buffer's column @a fname
    void removeAt(const KDbField& field);

    //! Removes value from not-db-aware buffer's column @a fname
    void removeAt(const QString& fname);

    /*! Useful only for db-aware buffer. @return value for column @a ci
     If there is no value assigned for the buffer, this method tries to remember and return
     default value obtained from @a ci if @a useDefaultValueIfPossible is true.
     Note that if the column is declared as unique (especially: primary key),
     default value will not be used. */
    const QVariant* at(KDbQueryColumnInfo *ci, bool useDefaultValueIfPossible = true) const;

    //! Useful only for not-db-aware buffer. @return value for field @a field
    const QVariant* at(const KDbField &field) const;

    //! Useful only for not-db-aware buffer. @return value for field @a fname
    const QVariant* at(const QString& fname) const;

    //! Useful only for db-aware buffer: @return true if the value available as
    //! at( ci ) is obtained from column's default value
    bool hasDefaultValueAt(const KDbQueryColumnInfo &ci) const;

    KDbRecordEditBuffer::SimpleMap simpleBuffer() const;

    KDbRecordEditBuffer::DbHash dbBuffer() const;

protected:
    SimpleMap *m_simpleBuffer;
    SimpleMap::ConstIterator *m_simpleBufferIt;
    DbHash *m_dbBuffer;
    DbHash::Iterator *m_dbBufferIt;
    QMap<KDbQueryColumnInfo*, bool> *m_defaultValuesDbBuffer;
    QMap<KDbQueryColumnInfo*, bool>::ConstIterator *m_defaultValuesDbBufferIt;

private:
    Q_DISABLE_COPY(KDbRecordEditBuffer)
};

//! Sends information about object @a buffer to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbRecordEditBuffer& buffer);

#endif
