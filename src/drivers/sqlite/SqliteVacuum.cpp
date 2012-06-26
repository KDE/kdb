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

#include "SqliteVacuum.h"

#include <Predicate/Global>

#include <QtDebug>
#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QApplication>
#include <QProcess>
#include <QCursor>
#include <QLocale>
#include <QTemporaryFile>
#include <QProgressDialog>

#ifdef Q_WS_WIN
#include <windows.h>
void usleep(unsigned int usec)
{
	Sleep(usec/1000);
}
#else
#include <unistd.h>
#endif

SQLiteVacuum::SQLiteVacuum(const QString& filePath)
        : m_filePath(filePath)
{
    m_dumpProcess = 0;
    m_sqliteProcess = 0;
    m_percent = 0;
    m_dlg = 0;
    m_result = true;
}

SQLiteVacuum::~SQLiteVacuum()
{
    m_dumpProcess->waitForFinished();
    delete m_dumpProcess;
    m_sqliteProcess->waitForFinished();
    delete m_sqliteProcess;
    if (m_dlg)
        m_dlg->close();
    delete m_dlg;
}

tristate SQLiteVacuum::run()
{
    const QString dump_app(QLatin1String(PREDICATE_SQLITE_DUMP_TOOL));
    PreDrvDbg << dump_app;
    if (dump_app.isEmpty()) {
        PreDrvWarn << "SQLiteVacuum::run(): Could not find tool" << PREDICATE_SQLITE_DUMP_TOOL;
        m_result = false;
        return m_result;
    }
    const QString sqlite_app(QLatin1String("sqlite3"));
    PreDrvDbg << sqlite_app;
    if (sqlite_app.isEmpty()) {
        PreDrvWarn << "SQLiteVacuum::run(): Could not find tool" << sqlite_app;
        m_result = false;
        return m_result;
    }
    
    QFileInfo fi(m_filePath);
    if (!fi.isReadable()) {
        PreDrvWarn << "SQLiteVacuum::run(): No such file" << m_filePath;
        return false;
    }

    PreDrvDbg << "SQLiteVacuum::run():" << m_filePath << QFileInfo(m_filePath).absoluteDir().path();

    delete m_dumpProcess;
    m_dumpProcess = new QProcess(this);
    m_dumpProcess->setWorkingDirectory(QFileInfo(m_filePath).absoluteDir().path());
    m_dumpProcess->setReadChannel(QProcess::StandardError);
    connect(m_dumpProcess, SIGNAL(readyReadStandardError()), this, SLOT(readFromStdErr()));
    connect(m_dumpProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(dumpProcessFinished(int,QProcess::ExitStatus)));
            
    delete m_sqliteProcess;
    m_sqliteProcess = new QProcess(this);
    m_sqliteProcess->setWorkingDirectory(QFileInfo(m_filePath).absoluteDir().path());
    connect(m_sqliteProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(sqliteProcessFinished(int,QProcess::ExitStatus)));

    m_dumpProcess->setStandardOutputProcess(m_sqliteProcess);
    m_dumpProcess->start(dump_app, QStringList() << m_filePath);
    if (!m_dumpProcess->waitForStarted()) {
        delete m_dumpProcess;
        m_dumpProcess = 0;
        m_result = false;
        return m_result;
    }
    
    QTemporaryFile *tempFile = new QTemporaryFile(m_filePath);
    tempFile->open();
    m_tmpFilePath = tempFile->fileName();
    delete tempFile;
    PreDrvDbg << "SQLiteVacuum::run():" << m_tmpFilePath;
    m_sqliteProcess->start(sqlite_app, QStringList() << m_tmpFilePath);
    
    m_dlg = new QProgressDialog(0);
    m_dlg->setWindowTitle(tr("Compacting database"));
    m_dlg->setLabelText(
        QLatin1String("<qt>") + tr("Compacting database \"%1\"...")
            .arg(QLatin1String("<nobr>") + QDir::convertSeparators(QFileInfo(m_filePath).fileName()) + QLatin1String("</nobr>"))
    );
    m_dlg->adjustSize();
    m_dlg->resize(300, m_dlg->height());
    connect(m_dlg, SIGNAL(cancelClicked()), this, SLOT(cancelClicked()));
    m_dlg->setMinimumDuration(1000);
    m_dlg->setAutoClose(true);
    m_dlg->setRange(0, 100);
    m_dlg->exec();
    while (m_dumpProcess->state() == QProcess::Running) {
        readFromStdErr();
        usleep(50000);
    }
    readFromStdErr();

    return m_result;
}

void SQLiteVacuum::readFromStdErr()
{
    while (true) {
        QByteArray s(m_dumpProcess->readLine(1000));
        if (s.isEmpty())
            break;
	PreDrvDbg << s;
        if (s.startsWith("DUMP: ")) {
            //set previously known progress
            m_dlg->setValue(m_percent);
            //update progress info
            if (s.mid(6, 4) == "100%") {
                m_percent = 100;
#warning TODO m_dlg->setAllowCancel(false);
                m_dlg->setCursor(QCursor(Qt::WaitCursor));
            } else if (s.mid(7, 1) == "%") {
                m_percent = s.mid(6, 1).toInt();
            } else if (s.mid(8, 1) == "%") {
                m_percent = s.mid(6, 2).toInt();
            }
            m_dlg->setValue(m_percent);
        }
    }
}

void SQLiteVacuum::dumpProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    PreDrvDbg << exitCode << exitStatus;
    if (m_dlg) {
        m_dlg->close();
        delete m_dlg;
        m_dlg = 0;
    }
}

void SQLiteVacuum::sqliteProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    PreDrvDbg << exitCode << exitStatus;

    const uint origSize = QFileInfo(m_filePath).size();

    if (!QFile::rename(m_tmpFilePath, m_filePath)) {
        PreDrvWarn << "SQLiteVacuum::sqliteProcessFinished(): Rename"
	  << m_tmpFilePath << "to" << m_filePath << "failed.";
    }

    if (m_result == true) {
        const uint newSize = QFileInfo(m_filePath).size();
        const uint decrease = 100 - 100 * newSize / origSize;
        QMessageBox::information(0, QString(),
            QObject::tr("The database has been compacted. Current size decreased by %1% to %2 MB.")
                .arg(decrease).arg(QLocale().toString(double(newSize)/1000000.0, 'f', 2)));
    }
}

void SQLiteVacuum::cancelClicked()
{
    if (!(m_dumpProcess->exitStatus() == QProcess::NormalExit)) {
        m_dumpProcess->write("q"); //quit
        m_result = cancelled;
    }
}
