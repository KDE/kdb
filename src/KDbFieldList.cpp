/* This file is part of the KDE project
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

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

#include "KDbFieldList.h"
#include "KDbConnection.h"
#include "kdb_debug.h"

KDbFieldList::KDbFieldList(bool owner)
        : m_fields(owner)
        , m_autoinc_fields(nullptr)
{
}

//! @todo IMPORTANT: (API) improve deepCopyFields
KDbFieldList::KDbFieldList(const KDbFieldList& fl, bool deepCopyFields)
        : m_fields(fl.m_fields.autoDelete())
        , m_autoinc_fields(nullptr)
{
    if (deepCopyFields) {
        //deep copy for the fields
        foreach(KDbField *origField, fl.m_fields) {
            KDbField *f = origField->copy();
            if (origField->parent() == &fl) {
                f->setParent(this);
            }
            const bool addFieldOk = addField(f);
            Q_ASSERT(addFieldOk);
        }
    }
}

KDbFieldList::~KDbFieldList()
{
    delete m_autoinc_fields;
}

int KDbFieldList::fieldCount() const
{
    return m_fields.count();
}

bool KDbFieldList::isEmpty() const
{
    return m_fields.isEmpty();
}

void KDbFieldList::clear()
{
    m_fields_by_name.clear();
    delete m_autoinc_fields;
    m_autoinc_fields = nullptr;
    m_fields.clear();
    m_sqlFields.clear();
}

bool KDbFieldList::insertField(int index, KDbField *field)
{
    Q_ASSERT(field);
    if (!field) {
        return false;
    }
    if (index > m_fields.count()) {
        kdbCritical() << "index (" << index << ") out of range";
        return false;
    }
    m_fields.insert(index, field);
    if (!field->name().isEmpty())
        m_fields_by_name.insert(field->name().toLower(), field);
    m_sqlFields.clear();
    delete m_autoinc_fields;
    m_autoinc_fields = nullptr;
    return true;
}

void KDbFieldList::renameField(const QString& oldName, const QString& newName)
{
    KDbField *field = m_fields_by_name.value(oldName.toLower());
    if (!field) {
        kdbCritical() << "no field found" << QString::fromLatin1("\"%1\"").arg(oldName);
        return;
    }
    renameFieldInternal(field, newName.toLower());
}

void KDbFieldList::renameField(KDbField *field, const QString& newName)
{
    if (!field || field != m_fields_by_name.value(field->name().toLower())) {
        kdbCritical() << "no field found"
                      << QString::fromLatin1("\"%1\"").arg(field ? field->name() : QString());
        return;
    }
    renameFieldInternal(field, newName.toLower());
}

void KDbFieldList::renameFieldInternal(KDbField *field, const QString& newNameLower)
{
    m_fields_by_name.remove(field->name().toLower());
    field->setName(newNameLower);
    m_fields_by_name.insert(newNameLower, field);
}


bool KDbFieldList::addField(KDbField *field)
{
    return insertField(m_fields.count(), field);
}

bool KDbFieldList::removeField(KDbField *field)
{
    Q_ASSERT(field);
    if (!field)
        return false;
    if (m_fields_by_name.remove(field->name().toLower()) < 1)
        return false;
    m_fields.removeAt(m_fields.indexOf(field));
    m_sqlFields.clear();
    delete m_autoinc_fields;
    m_autoinc_fields = nullptr;
    return true;
}

bool KDbFieldList::moveField(KDbField *field, int newIndex)
{
    Q_ASSERT(field);
    if (!field || !m_fields.removeOne(field)) {
        return false;
    }
    if (newIndex > m_fields.count()) {
        newIndex = m_fields.count();
    }
    m_fields.insert(newIndex, field);
    m_sqlFields.clear();
    delete m_autoinc_fields;
    m_autoinc_fields = nullptr;
    return true;
}

KDbField* KDbFieldList::field(int id)
{
    return m_fields.value(id);
}

const KDbField* KDbFieldList::field(int id) const
{
    return m_fields.value(id);
}

KDbField* KDbFieldList::field(const QString& name)
{
    return m_fields_by_name.value(name.toLower());
}

const KDbField* KDbFieldList::field(const QString& name) const
{
    return m_fields_by_name.value(name.toLower());
}

bool KDbFieldList::hasField(const KDbField& field) const
{
    return m_fields.contains(const_cast<KDbField*>(&field));
}

int KDbFieldList::indexOf(const KDbField& field) const
{
    return m_fields.indexOf(const_cast<KDbField*>(&field));
}

KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbFieldList& list)
{
    if (list.fields()->isEmpty())
        dbg.nospace() << "<NO FIELDS>";
    bool start = true;
    foreach(const KDbField *field, *list.fields()) {
        if (!start)
            dbg.nospace() << '\n';
        else
            start = false;
        dbg.nospace() << " - " << *field;
    }
    return dbg.space();
}

#define _ADD_FIELD(fname) \
    { \
        if (fname.isEmpty()) return fl; \
        KDbField *f = m_fields_by_name.value(fname.toLower()); \
        if (!f || !fl->addField(f)) { kdbWarning() << subListWarning1(fname); delete fl; return 0; } \
    }

static QString subListWarning1(const QString& fname)
{
    return QString::fromLatin1("could not find field \"%1\"").arg(fname);
}

KDbFieldList* KDbFieldList::subList(const QString& n1, const QString& n2,
                              const QString& n3, const QString& n4,
                              const QString& n5, const QString& n6,
                              const QString& n7, const QString& n8,
                              const QString& n9, const QString& n10,
                              const QString& n11, const QString& n12,
                              const QString& n13, const QString& n14,
                              const QString& n15, const QString& n16,
                              const QString& n17, const QString& n18)
{
    if (n1.isEmpty())
        return nullptr;
    KDbFieldList *fl = new KDbFieldList(false);
    _ADD_FIELD(n1);
    _ADD_FIELD(n2);
    _ADD_FIELD(n3);
    _ADD_FIELD(n4);
    _ADD_FIELD(n5);
    _ADD_FIELD(n6);
    _ADD_FIELD(n7);
    _ADD_FIELD(n8);
    _ADD_FIELD(n9);
    _ADD_FIELD(n10);
    _ADD_FIELD(n11);
    _ADD_FIELD(n12);
    _ADD_FIELD(n13);
    _ADD_FIELD(n14);
    _ADD_FIELD(n15);
    _ADD_FIELD(n16);
    _ADD_FIELD(n17);
    _ADD_FIELD(n18);
    return fl;
}

KDbFieldList* KDbFieldList::subList(const QStringList& list)
{
    KDbFieldList *fl = new KDbFieldList(false);
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        _ADD_FIELD((*it));
    }
    return fl;
}

#undef _ADD_FIELD

#define _ADD_FIELD(fname) \
    { \
        if (fname.isEmpty()) return fl; \
        KDbField *f = m_fields_by_name.value(QLatin1String(fname.toLower())); \
        if (!f || !fl->addField(f)) { kdbWarning() << subListWarning1(QLatin1String(fname)); delete fl; return 0; } \
    }

KDbFieldList* KDbFieldList::subList(const QList<QByteArray>& list)
{
    KDbFieldList *fl = new KDbFieldList(false);
    for (QList<QByteArray>::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        _ADD_FIELD((*it));
    }
    return fl;
}

#undef _ADD_FIELD

KDbFieldList* KDbFieldList::subList(const QList<int>& list)
{
    QScopedPointer<KDbFieldList> fl(new KDbFieldList(false));
    foreach(int index, list) {
        KDbField *f = field(index);
        if (!f) {
            kdbWarning() << QString::fromLatin1("could not find field at position %1").arg(index);
            return nullptr;
        }
        if (!fl->addField(f)) {
            kdbWarning() << QString::fromLatin1("could not add field at position %1").arg(index);
            return nullptr;
        }
    }
    return fl.take();
}

QStringList KDbFieldList::names() const
{
    QStringList r;
    foreach(KDbField *f, m_fields) {
        r += f->name().toLower();
    }
    return r;
}

KDbField::ListIterator KDbFieldList::fieldsIterator() const
{
    return m_fields.constBegin();
}

KDbField::ListIterator KDbFieldList::fieldsIteratorConstEnd() const
{
    return m_fields.constEnd();
}

const KDbField::List* KDbFieldList::fields() const
{
    return &m_fields;
}

bool KDbFieldList::isOwner() const
{
    return m_fields.autoDelete();
}

//static
KDbEscapedString KDbFieldList::sqlFieldsList(const KDbField::List& list, KDbConnection *conn,
                                       const QString& separator, const QString& tableOrAlias,
                                       KDb::IdentifierEscapingType escapingType)
{
    KDbEscapedString result;
    result.reserve(256);
    bool start = true;
    QString tableOrAliasAndDot;
    if (!tableOrAlias.isEmpty()) {
        tableOrAliasAndDot
                 = ((conn && escapingType == KDb::DriverEscaping)
                        ? conn->escapeIdentifier(tableOrAlias)
                        : KDb::escapeIdentifier(tableOrAlias))
                   + QLatin1Char('.');
    }
    foreach(KDbField *f, list) {
        if (!start)
            result.append(separator);
        else
            start = false;
        result = (result + tableOrAliasAndDot +
                   ((conn && escapingType == KDb::DriverEscaping)
                        ? conn->escapeIdentifier(f->name())
                        : KDb::escapeIdentifier(f->name()))
                 );
    }
    return result;
}

KDbEscapedString KDbFieldList::sqlFieldsList(KDbConnection *conn,
                                 const QString& separator, const QString& tableOrAlias,
                                 KDb::IdentifierEscapingType escapingType) const
{
    if (!m_sqlFields.isEmpty())
        return m_sqlFields;

    m_sqlFields = KDbFieldList::sqlFieldsList(m_fields, conn, separator, tableOrAlias, escapingType);
    return m_sqlFields;
}

KDbField::List* KDbFieldList::autoIncrementFields() const
{
    if (m_autoinc_fields)
        return m_autoinc_fields;

    m_autoinc_fields = new KDbField::List();
    m_autoinc_fields->setAutoDelete(false);
    foreach(KDbField *f, m_fields) {
        if (f->isAutoIncrement()) {
            m_autoinc_fields->append(f);
        }
    }
    return m_autoinc_fields;
}
