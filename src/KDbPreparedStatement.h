/* This file is part of the KDE project
   Copyright (C) 2005-2016 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_PREPAREDSTATEMENT_H
#define KDB_PREPAREDSTATEMENT_H

#include <QVariant>
#include <QStringList>
#include <QSharedData>

#include "KDbField.h"
#include "KDbResult.h"

class KDbFieldList;
class KDbPreparedStatementInterface;

//! Prepared statement paraneters used in KDbPreparedStatement::execute()
typedef QList<QVariant> KDbPreparedStatementParameters;

/*! @short Prepared database command for optimizing sequences of multiple database actions

  Currently INSERT and SELECT statements are supported.
  For example when using KDbPreparedStatement for INSERTs,
  you can gain about 30% speedup compared to using multiple
  connection.insertRecord(*tabelSchema, dbRecordBuffer).

  To use KDbPreparedStatement, create is using KDbConnection:prepareStatement(),
  providing table schema; set up parameters using operator << ( const QVariant& value );
  and call execute() when ready. KDbPreparedStatement objects are accessed
  using KDE shared pointers, i.e KDbPreparedStatement, so you do not need
  to remember about destroying them. However, when underlying KDbConnection object
  is destroyed, KDbPreparedStatement should not be used.

  Let's assume tableSchema contains two columns NUMBER integer and TEXT text.
  Following code inserts 10000 records with random numbers and text strings
  obtained elsewhere using getText(i).
  @code
  bool insertMultiple(KDbConnection* conn, KDbTableSchema* tableSchema)
  {
    KDbPreparedStatement statement = conn->prepareStatement(
      KDbPreparedStatement::Insert, tableSchema);
    for (i=0; i<10000; i++) {
      KDbPreparedStatementParameters parameters;
      parameters << qrand() << getText(i);
      if (!statement.execute(parameters))
        return false;
    }
    return true;
  }
  @endcode

  If you do not call clearParameters() after every insert, you can insert
  the same value multiple times using execute() what increases efficiency even more.

  Another use case is inserting large objects (BLOBs or CLOBs).
  Depending on database backend, you can avoid escaping BLOBs.
  See KexiFormView::storeData() for example use.
*/
class KDB_EXPORT KDbPreparedStatement : public KDbResultable
{
public:

    //! Defines type of the prepared statement.
    enum Type {
        InvalidStatement, //!< Used only in invalid statements
        SelectStatement,  //!< SELECT statement will be prepared end executed
        InsertStatement   //!< INSERT statement will be prepared end executed
    };

    //! @internal
    class KDB_EXPORT Data : public QSharedData {
    public:
        Data();
        Data(Type _type, KDbPreparedStatementInterface* _iface, KDbFieldList* _fields,
             const QStringList& _whereFieldNames);
        ~Data();
        Type type;
        KDbFieldList *fields;
        QStringList whereFieldNames;
        const KDbField::List* fieldsForParameters; //!< fields where we'll put the inserted parameters
        KDbField::List* whereFields; //!< temporary, used for select statements, based on whereFieldNames
        bool dirty; //!< true if the statement has to be internally
                    //!< prepared (possible again) before calling executeInternal()
        KDbPreparedStatementInterface *iface;
        quint64 lastInsertRecordId;
    };

    //! Creates an invalid prepared statement.
    KDbPreparedStatement();

    ~KDbPreparedStatement() override;

    bool isValid() const;

    KDbPreparedStatement::Type type() const;

    void setType(KDbPreparedStatement::Type type);

    const KDbFieldList* fields() const;

    //! Sets fields for the statement. Does nothing if @a fields is @c nullptr.
    void setFields(KDbFieldList* fields);

    QStringList whereFieldNames() const;

    void setWhereFieldNames(const QStringList& whereFieldNames);

    /*! Executes the prepared statement using @a parameters parameters.
     A number parameters set up for the statement must be the same as a number of fields
     defined in the underlying database table.
     @return false on failure. Detailed error status can be obtained
     from KDbConnection object that was used to create this statement object. */
    bool execute(const KDbPreparedStatementParameters& parameters);

    /*! @return unique identifier of the most recently inserted record.
     Typically this is just primary key value. This identifier could be reused when we want
     to reference just inserted record. If there was no insertion recently performed,
     std::numeric_limits<quint64>::max() is returned. */
    quint64 lastInsertRecordId() const;

protected:
    //! Creates a new prepared statement. In your code use
    //! Users call KDbConnection:prepareStatement() instead.
    KDbPreparedStatement(KDbPreparedStatementInterface* iface, Type type,
                         KDbFieldList* fields,
                         const QStringList& whereFieldNames = QStringList());

    friend class KDbConnection;

private:
//! @todo is this portable across backends?
    bool generateStatementString(KDbEscapedString* s);
    bool generateSelectStatementString(KDbEscapedString * s);
    bool generateInsertStatementString(KDbEscapedString * s);

    QSharedDataPointer<Data> d;
};

#endif
