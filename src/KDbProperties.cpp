/* This file is part of the KDE project
   Copyright (C) 2005-2006 Jarosław Staniek <staniek@kde.org>

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

#include "KDbProperties.h"
#include "KDbConnection.h"

//! @todo IMPORTANT: replace QPointer<KDbConnection> m_conn;
KDbProperties::KDbProperties(KDbConnection *conn)
        : m_conn(conn)
{
}

KDbProperties::~KDbProperties()
{
}

bool KDbProperties::setValue(const QString& _name, const QVariant& value)
{
    QString name(_name.trimmed());
    //we need to know whether update or insert
    const tristate result = m_conn->resultExists(
                      KDbEscapedString("SELECT 1 FROM kexi__db WHERE db_property=%1")
                      .arg(m_conn->escapeString(name)));
    if (~result) {
        m_result = m_conn->result();
        m_result.prependMessage(tr("Could not set value of database property \"%1\".").arg(name));
        return false;
    }

    if (result == true) {
        if (!m_conn->executeVoidSQL(
                    KDbEscapedString("UPDATE kexi__db SET db_value=%1 WHERE db_property=%2")
                    .arg(m_conn->escapeString(value.toString()))
                    .arg(m_conn->escapeString(name))))
        {
            m_result = m_conn->result();
            m_result.prependMessage(tr("Could not set value of database property \"%1\".").arg(name));
            return false;
        }
        return true;
    }

    if (!m_conn->executeVoidSQL(
                KDbEscapedString("INSERT INTO kexi__db (db_property, db_value) VALUES (%1, %2)")
                    .arg(m_conn->escapeString(name))
                    .arg(m_conn->escapeString(value.toString()))))
    {
        m_result = m_conn->result();
        m_result.prependMessage(tr("Could not set value of database property \"%1\".").arg(name));
        return false;
    }
    return true;
}

bool KDbProperties::setCaption(const QString& _name, const QString& caption)
{
    QString name(_name.trimmed());
    //captions have ' ' prefix
    name.prepend(QLatin1String(" "));
    //we need to know whether update or insert
    const tristate result = m_conn->resultExists(
                      KDbEscapedString("SELECT 1 FROM kexi__db WHERE db_property=%1")
                        .arg(m_conn->escapeString(name)));
    if (~result) {
        m_result = m_conn->result();
        m_result.prependMessage(tr("Could not set caption for database property \"%1\".").arg(name));
        return false;
    }

    if (result == true) {
        if (!m_conn->executeVoidSQL(
                    KDbEscapedString("UPDATE kexi__db SET db_value=%1 WHERE db_property=%2")
                        .arg(m_conn->escapeString(caption))
                        .arg(m_conn->escapeString(name)))) {
            m_result = m_conn->result();
            m_result.prependMessage(tr("Could not set caption for database property \"%1\".").arg(name));
            return false;
        }
        return true;
    }

    if (!m_conn->executeVoidSQL(
                KDbEscapedString("INSERT INTO kexi__db (db_property, db_value) VALUES (%1, %2)")
                    .arg(m_conn->escapeString(name))
                    .arg(m_conn->escapeString(caption)))) {
        m_result = m_conn->result();
        m_result.prependMessage(tr("Could not set caption for database property \"%1\".").arg(name));
        return false;
    }
    return true;
}

QVariant KDbProperties::value(const QString& _name)
{
    QString result;
    QString name(_name.trimmed());
    if (true != m_conn->querySingleString(
                KDbEscapedString("SELECT db_value FROM kexi__db WHERE db_property=")
                    + m_conn->escapeString(name), &result))
    {
        m_result = m_conn->result();
        m_result.prependMessage(ERR_NO_DB_PROPERTY, tr("Could not read database property \"%1\".").arg(name));
        return QVariant();
    }
    return result;
}

QString KDbProperties::caption(const QString& _name)
{
    QString result;
    QString name(_name.trimmed());
    //captions have ' ' prefix
    name.prepend(QLatin1String(" "));
    if (true != m_conn->querySingleString(
                KDbEscapedString("SELECT db_value FROM kexi__db WHERE db_property=")
                    + m_conn->escapeString(name), &result))
    {
        m_result = m_conn->result();
        m_result.prependMessage(tr("Could not read database property \"%1\".").arg(name));
        return QString();
    }
    return result;
}

QStringList KDbProperties::names()
{
    QStringList result;
    if (true != m_conn->queryStringList(
                KDbEscapedString("SELECT db_property FROM kexi__db WHERE db_property NOT LIKE ")
                + m_conn->escapeString(QString::fromLatin1(" %%")), &result, 0 /*0-th*/))
        //                                                        ^^ exclude captions
    {
        m_result = m_conn->result();
        m_result.prependMessage(tr("Could not read database properties."));
        return QStringList();
    }
    return result;
}
