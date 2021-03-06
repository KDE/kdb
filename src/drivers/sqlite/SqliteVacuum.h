/* This file is part of the KDE project
   Copyright (C) 2006-2012 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_SQLITEVACUUM_H
#define KDB_SQLITEVACUUM_H

#include <QObject>
#include <QString>
#include <QProcess>

#include "KDbTristate.h"
#include "KDbResult.h"

class QProgressDialog;

//! @short Helper class performing interactive compacting (VACUUM) of the SQLite database
/*! Proved SQLite database filename in the constructor.
 Then execute run() should be executed.

 QProgressDialog will be displayed. Its progress bar will be updated whenever another
 table's data compacting is performed. User can click "Cancel" button in any time
 (except the final committing) to cancel the operation. In this case,
 it's guaranteed that the original file remains unchanged.

 This is possible because we rely on SQLite's VACUUM SQL command, which itself temporarily
 creates a copy of the original database file, and replaces the orginal with the new only
 on success.
*/
class SqliteVacuum : public QObject, public KDbResultable
{
    Q_OBJECT
public:
    explicit SqliteVacuum(const QString& filePath);
    ~SqliteVacuum() override;

    /*! Performs compacting procedure.
     @return true on success, false on failure and cancelled if user
     clicked "Cancel" button in the progress dialog. */
    tristate run();

public Q_SLOTS:
    void readFromStdErr();
    void dumpProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void sqliteProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void cancelClicked();

private:
    QString m_filePath;
    QString m_tmpFilePath;
    QProcess *m_dumpProcess;
    QProcess *m_sqliteProcess;
    QProgressDialog* m_dlg; // krazy:exclude=qclasses
    int m_percent;
    bool m_canceled;
    Q_DISABLE_COPY(SqliteVacuum)
};

#endif
