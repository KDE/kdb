/* This file is part of the KDE project
   Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003   Daniel Molkentin <molkentin@kde.org>
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

   Original Author:  Till Busch <till@bux.at>
   Original Project: buX (www.bux.at)
*/

#ifndef PREDICATE_RECORDDATA_H
#define PREDICATE_RECORDDATA_H

#include <QVector>
#include <QVariant>
#include <QtDebug>
#include <Predicate/predicate_export.h>

namespace Predicate
{

//! @short Structure for storing single record with type information.
//! @todo consider forking QVariant to a non-shared Variant class, with type information stored elsewhere.
//! @todo Variant should have toQVariant() method
//! @todo look if we can have zero-copy strategy for SQLite and other backends
//! @todo look if we can use memory allocator for strings, etc.
class PREDICATE_EXPORT RecordData
{
public:
    /*! Creates a new empty record. */
    inline RecordData() : m_data(0), m_numCols(0) {}

    /*! Creates a new record data with \a numCols columns.
     Values are initialized to null. */
    inline explicit RecordData(int numCols) { init(numCols); }

    ~RecordData() {
        if (m_numCols > 0) {
            for (int i = 0; i < m_numCols; i++)
                delete m_data[i];
            free(m_data);
        }
    }

    inline bool isEmpty() const { return m_numCols == 0; }

    inline int size() const { return m_numCols; }

    inline int count() const { return m_numCols; }

    /*! @return the value at position @a i.
     @a i must be a valid index. i.e. 0 <= i < size().
     @see value(), operator[](). */
    inline const QVariant& at(int i) const {
        if (!m_data[i])
            return s_null;
        return *m_data[i];
    }

    /*! @return the value at position @a i as a modifiable reference.
     @a i must be a valid index, i.e. 0 <= i < size().
     @see at(), value(). */
    inline QVariant& operator[](int i) {
        if (!m_data[i]) {
            return *(m_data[i] = new QVariant);
        }
        return *m_data[i];
    }

    /*! @return true id value at position @a i is null.
     @a i must be a valid index, i.e. 0 <= i < size().
     @see at(), value(). */
    inline bool isNull(int i) const { return !m_data[i] || m_data[i]->isNull(); }

    /*! Overloaded function.*/
    inline const QVariant& operator[](int i) const {
        if (!m_data[i])
            return s_null;
        return *m_data[i];
    }

    /*! @return the value at index position i in the vector.
     If the index @a i is out of bounds, the function returns a default-constructed value.
     If you are certain that i is within bounds, you can use at() instead, which is slightly faster. */
    inline QVariant value(int i) const {
        if (!m_data || i < 0 || i >= m_numCols || !m_data[i])
            return QVariant();
        return *m_data[i];
    }

    /*! @return the value at index position i in the vector.
     This is an overloaded function.
     If the index @a i is out of bounds, the function returns a @a defaultValue.
     If you are certain that i is within bounds, you can use at() instead, which is slightly faster. */
    QVariant value(int i, const QVariant& defaultValue) const {
        if (!m_data || i < 0 || i >= m_numCols)
            return defaultValue;
        if (!m_data[i])
            return QVariant();
        return *m_data[i];
    }

    /*! Sets existing column values to null, current number of columns is preserved. */
    inline void clearValues() {
        for (int i = 0; i < m_numCols; i++) {
            delete m_data[i];
            m_data[i] = 0;
        }
    }

    /*! Clears all columns, the record is set empty. */
    void clear();

    /*! Resize existing record to @a numCols.
     If @a numCols differ from size(), new with record with all values set to null is created.
     If @a numCols equals size() nothing is performed. */
    inline void resize(int numCols) {
        if (m_numCols == numCols)
            return;
        else if (m_numCols < numCols) { // grow
            m_data = (QVariant**)realloc(m_data, numCols * sizeof(QVariant*));
            memset(m_data + m_numCols * sizeof(QVariant*), 0, (numCols - m_numCols) * sizeof(QVariant*));
            m_numCols = numCols;
        }
        else { // shrink
            for (int i = numCols; i < m_numCols; i++)
                delete m_data[i];
            m_data = (QVariant**)realloc(m_data, numCols * sizeof(QVariant*));
            m_numCols = numCols;
        }
    }

private:
    Q_DISABLE_COPY(RecordData)

    inline void init(int numCols)
    {
        m_numCols = numCols;
        if (m_numCols > 0) {
            m_data = (QVariant**)malloc(m_numCols * sizeof(QVariant*));
            memset(m_data, 0, m_numCols * sizeof(QVariant*));
        }
        else
            m_data = 0;
    }

    QVariant **m_data;
    int m_numCols;
    static QVariant s_null;
};

}

//! Sends information about record data @a data to debug output @a dbg.
PREDICATE_EXPORT QDebug operator<<(QDebug dbg, const Predicate::RecordData& data);

#endif
