/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2007 David Faure <faure@kde.org>
   Copyright (C) 2015 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_JSONTRADER_P_H
#define KDB_JSONTRADER_P_H

#include <QList>
#include <QString>

class QPluginLoader;

/**
 *  Support class to fetch a list of relevant plugins
 */
class KDbJsonTrader
{
public:
    KDbJsonTrader();

    ~KDbJsonTrader();

    static KDbJsonTrader *self();

    /**
     * The main function in the KDbJsonTrader class.
     *
     * It will return a list of QPluginLoader objects that match your
     * specifications.  The only required parameter is the @a servicetype.
     * The @a mimetype parameter is used to limit the possible choices
     * returned based on the constraints you give it.
     *
     * The keys used in the query (Type, ServiceType, Exec) are all
     * fields found in the .desktop files.
     *
     * @param servicetype A service type like 'KMyApp/Plugin' or 'KFilePlugin'.
     * @param mimetype    A mimetype constraint to limit the choices returned, QString() to
     *                    get all services of the given @p servicetype.
     *
     * @return A list of QPluginLoader that satisfy the query
     * @see http://techbase.kde.org/Development/Tutorials/Services/Traders#The_KTrader_Query_Language
     *
     * @note Ownership of the QPluginLoader objects is transferred to the caller.
     */
     QList<QPluginLoader *> query(const QString &servicetype, const QString &mimetype = QString());

private:
     Q_DISABLE_COPY(KDbJsonTrader)
     class Private;
     Private * const d;
};

#endif
