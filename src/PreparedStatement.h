/* This file is part of the KDE project
   Copyright (C) 2005-2008 Jarosław Staniek <staniek@kde.org>

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

#ifndef PREDICATE_PREPAREDSTATEMENT_H
#define PREDICATE_PREPAREDSTATEMENT_H

#include <QVariant>
#include <QStringList>
#include <QSharedData>

#include "FieldList.h"

namespace Predicate
{

//class ConnectionInternal;
class TableSchema;
class PreparedStatementInterface;

/*! @short Prepared database command for optimizing sequences of multiple database actions

  Currently INSERT and SELECT statements are supported.
  For example, wher using PreparedStatement for INSERTs,
  you can gain about 30% speedup compared to using multiple
  connection.insertRecord(*tabelSchema, dbRowBuffer).

  To use PreparedStatement, create is using Predicate::Connection:prepareStatement(),
  providing table schema; set up arguments using operator << ( const QVariant& value );
  and call execute() when ready. PreparedStatement objects are accessed
  using KDE shared pointers, i.e Predicate::PreparedStatement, so you do not need
  to remember about destroying them. However, when underlying Connection object
  is destroyed, PreparedStatement should not be used.

  Let's assume tableSchema contains two columns NUMBER integer and TEXT text.
  Following code inserts 10000 records with random numbers and text strings
  obtained elsewhere using getText(i).
  \code
  bool insertMultiple(Predicate::Connection &conn, Predicate::TableSchema& tableSchema)
  {
    Predicate::PreparedStatement statement = conn.prepareStatement(
      Predicate::PreparedStatement::Insert, tableSchema);
    for (i=0; i<10000; i++) {
      prepared << rand() << getText(i);
      if (!prepared.execute())
        return false;
      prepared.clearArguments();
    }
    return true;
  }
  \endcode

  If you do not call clearArguments() after every insert, you can insert
  the same value multiple times using execute() what increases efficiency even more.

  Another use case is inserting large objects (BLOBs or CLOBs).
  Depending on database backend, you can avoid escaping BLOBs.
  See KexiFormView::storeData() for example use.
*/
class PREDICATE_EXPORT PreparedStatement
{
public:
    //! Prepared statement arguments used in execute()
    typedef QList<QVariant> Arguments;

    //! Defines type of the prepared statement.
    enum Type {
        Invalid,//!< Used only in invalid statements
        Select, //!< SELECT statement will be prepared end executed
        Insert  //!< INSERT statement will be prepared end executed
    };

    //! @internal
    class Data : public QSharedData {
    public:
        Data() : type(Invalid), whereFields(0), dirty(true) {}
        Data(Type _type, PreparedStatementInterface& _iface, FieldList& _fields,
             const QStringList& _whereFieldNames)
            : type(_type), fields(_fields), whereFieldNames(_whereFieldNames)
            , fieldsForArguments(0), whereFields(0), dirty(true), iface(&_iface)
        {}
        ~Data();
        Type type;
        FieldList fields;
        QStringList whereFieldNames;
        const Field::List* fieldsForArguments; //!< fields where we'll put the inserted arguments
        Field::List* whereFields; //!< temporary, used for select statements, based on whereFieldNames
        bool dirty : 1; //!< true if the statement has to be internally 
                        //!< prepared (possible again) before calling executeInternal()
        PreparedStatementInterface *iface;
    };

    //! Creates an invalid prepared statement.
    PreparedStatement()
        : d( new Data() )
    {
    }

    virtual ~PreparedStatement();

    bool isValid() const { return d->type == Invalid; }

    Type type() const { return d->type; }
    void setType(Type type) { d->type = type; d->dirty = true; }

    FieldList fields() const { return d->fields; }
    void setFields(FieldList& fields) { d->fields = fields; d->dirty = true; }

    QStringList whereFieldNames() const { return d->whereFieldNames; }
    void setWhereFieldNames(const QStringList& whereFieldNames)
        { d->whereFieldNames = whereFieldNames; d->dirty = true; }

    /*! Executes the prepared statement using @a args arguments.
     A number arguments set up for the statement must be the same as a number of fields
     defined in the underlying database table.
     @return false on failure. Detailed error status can be obtained
     from Predicate::Connection object that was used to create this statement object. */
    bool execute( const Arguments& args );

protected:
    //! Creates a new prepared statement. In your code use 
    //! Users call Predicate::Connection:prepareStatement() instead.
    PreparedStatement(PreparedStatementInterface& iface, Type type, FieldList& fields,
                      const QStringList& whereFieldNames = QStringList())
        : d( new Data(type, iface, fields, whereFieldNames) )
    {
    }

    friend class Connection;

private:
//! @todo is this portable across backends?
    void generateStatementString(QByteArray& s);
    void generateSelectStatementString(QByteArray& s);
    void generateInsertStatementString(QByteArray& s);

    QSharedDataPointer<Data> d;
};

} //namespace Predicate

#endif
