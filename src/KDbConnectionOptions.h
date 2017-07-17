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

#ifndef KDB_CONNECTIONOPTIONS_H
#define KDB_CONNECTIONOPTIONS_H

#include <QCoreApplication>
#include "KDbUtils.h"

class KDbConnectionPrivate;
class KDbConnection;

/*! @brief Generic options for a single connection.
    The options are accessible using key/value pairs. This enables extensibility
    depending on driver's type and version.
    @see KDbDriver::createConnection(const KDbConnectionData&, const KDbConnectionOptions&)
    @see KDbConnection::options()
*/
class KDB_EXPORT KDbConnectionOptions : public KDbUtils::PropertySet
{
    Q_DECLARE_TR_FUNCTIONS(KDbConnectionOptions)
public:
    KDbConnectionOptions();

    KDbConnectionOptions(const KDbConnectionOptions &other);

    ~KDbConnectionOptions();

    KDbConnectionOptions& operator=(const KDbConnectionOptions &other);

    //! @return true if these options have exactly the same values as @a other
    //! @since 3.1
    bool operator==(const KDbConnectionOptions &other) const;

    //! @return true if these options differs in at least one value from @a other
    //! @since 3.1
    bool operator!=(const KDbConnectionOptions &other) const { return !operator==(other); }

    /*! @return true for read-only connection. Used especially for file-based drivers.
     Can be implemented in a driver to provide real read-only flag of the connection
     (sqlite driver does this). */
    bool isReadOnly() const;

    /*! @internal used by KDbDriver::createConnection().
     Only works if connection is not yet established. */
    void setReadOnly(bool set);

    //! Inserts option with a given @a name, @a value and @a caption.
    //! If such option exists, value is updated but caption only if existing caption is empty.
    //! @a name must be a valid identifier (see KDb::isIdentifier()).
    void insert(const QByteArray &name, const QVariant &value, const QString &caption = QString());

    //! Sets caption for option @a name to @a caption.
    //! If such option does not exist, does nothing.
    void setCaption(const QByteArray &name, const QString &caption);

    //! Sets value for option @a name to @a value.
    //! If such option does not exist, does nothing.
    //! @since 3.1
    void setValue(const QByteArray &name, const QVariant &value);

    //! Removes option with a given @a name if exists.
    void remove(const QByteArray &name);

private:
    void setConnection(KDbConnection *connection);

    friend class KDbConnectionPrivate;

    class Private;
    Private * const d;
};

#endif
