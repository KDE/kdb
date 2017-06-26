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

class Q_DECL_HIDDEN KDbFieldList::Private
{
public:
    Private()
    {
    }
    ~Private()
    {
        delete autoincFields;
    }

    //! Clear cached autoinc fields list
    void clearAutoincFields()
    {
        delete autoincFields;
        autoincFields = nullptr;
    }

    bool renameFieldInternal(KDbField *field, const QString& newNameLower)
    {
        if (fieldsByName.value(newNameLower)) {
            kdbWarning() << "Field" << newNameLower << "already exists";
            return false;
        }
        fieldsByName.remove(field->name().toLower());
        field->setName(newNameLower);
        fieldsByName.insert(newNameLower, field);
        return true;
    }

    KDbField::List fields;

    //!< Fields collected by name. Not used by KDbQuerySchema.
    QHash<QString, KDbField*> fieldsByName;

    KDbField::List *autoincFields = nullptr;

    //! cached
    KDbEscapedString sqlFields;
};

//-------------------------------------------------------

KDbFieldList::KDbFieldList(bool owner)
        : d(new Private)
{
    d->fields.setAutoDelete(owner);
}

//! @todo IMPORTANT: (API) improve deepCopyFields
KDbFieldList::KDbFieldList(const KDbFieldList& fl, bool deepCopyFields)
        : KDbFieldList(fl.d->fields.autoDelete())
{
    if (deepCopyFields) {
        //deep copy for the fields
        for (KDbField *origField : *fl.fields()) {
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
    delete d;
}

KDbField::List *KDbFieldList::fields()
{
    return &d->fields;
}

const KDbField::List* KDbFieldList::fields() const
{
    return &d->fields;
}

int KDbFieldList::fieldCount() const
{
    return d->fields.count();
}

bool KDbFieldList::isEmpty() const
{
    return d->fields.isEmpty();
}

void KDbFieldList::clear()
{
    d->fieldsByName.clear();
    d->clearAutoincFields();
    d->fields.clear();
    d->sqlFields.clear();
}

bool KDbFieldList::insertField(int index, KDbField *field)
{
    if (!field) {
        return false;
    }
    if (index > d->fields.count()) {
        kdbWarning() << "index (" << index << ") out of range";
        return false;
    }
    d->fields.insert(index, field);
    if (!field->name().isEmpty()) {
        d->fieldsByName.insert(field->name().toLower(), field);
    }
    d->sqlFields.clear();
    d->clearAutoincFields();
    return true;
}

bool KDbFieldList::renameField(const QString& oldName, const QString& newName)
{
    KDbField *field = d->fieldsByName.value(oldName.toLower());
    if (!field) {
        kdbWarning() << "Fiels" << oldName << "not found";
        return false;
    }
    return d->renameFieldInternal(field, newName.toLower());
}

bool KDbFieldList::renameField(KDbField *field, const QString& newName)
{
    if (!field || field != d->fieldsByName.value(field->name().toLower())) {
        kdbWarning() << "No field found"
                      << QString::fromLatin1("\"%1\"").arg(field ? field->name() : QString());
        return false;
    }
    return d->renameFieldInternal(field, newName.toLower());
}

bool KDbFieldList::addField(KDbField *field)
{
    return insertField(d->fields.count(), field);
}

bool KDbFieldList::removeField(KDbField *field)
{
    if (!field) {
        return false;
    }
    if (d->fieldsByName.remove(field->name().toLower()) < 1) {
        return false;
    }
    d->fields.removeAt(d->fields.indexOf(field));
    d->sqlFields.clear();
    d->clearAutoincFields();
    return true;
}

bool KDbFieldList::moveField(KDbField *field, int newIndex)
{
    if (!field || !d->fields.removeOne(field)) {
        return false;
    }
    if (newIndex > d->fields.count()) {
        newIndex = d->fields.count();
    }
    d->fields.insert(newIndex, field);
    d->sqlFields.clear();
    d->clearAutoincFields();
    return true;
}

KDbField* KDbFieldList::field(int id)
{
    return d->fields.value(id);
}

const KDbField* KDbFieldList::field(int id) const
{
    return d->fields.value(id);
}

KDbField* KDbFieldList::field(const QString& name)
{
    return d->fieldsByName.value(name.toLower());
}

const KDbField* KDbFieldList::field(const QString& name) const
{
    return d->fieldsByName.value(name.toLower());
}

bool KDbFieldList::hasField(const KDbField& field) const
{
    return d->fields.contains(const_cast<KDbField*>(&field));
}

int KDbFieldList::indexOf(const KDbField& field) const
{
    return d->fields.indexOf(const_cast<KDbField*>(&field));
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
        KDbField *f = d->fieldsByName.value(fname.toLower()); \
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
        KDbField *f = d->fieldsByName.value(QLatin1String(fname.toLower())); \
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
    for (KDbField *f : d->fields) {
        r += f->name().toLower();
    }
    return r;
}

KDbField::ListIterator KDbFieldList::fieldsIterator() const
{
    return d->fields.constBegin();
}

KDbField::ListIterator KDbFieldList::fieldsIteratorConstEnd() const
{
    return d->fields.constEnd();
}

bool KDbFieldList::isOwner() const
{
    return d->fields.autoDelete();
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
    if (!d->sqlFields.isEmpty())
        return d->sqlFields;

    d->sqlFields = KDbFieldList::sqlFieldsList(d->fields, conn, separator, tableOrAlias, escapingType);
    return d->sqlFields;
}

KDbField::List* KDbFieldList::autoIncrementFields() const
{
    if (d->autoincFields)
        return d->autoincFields;

    d->autoincFields = new KDbField::List(false);
    for (KDbField *f : d->fields) {
        if (f->isAutoIncrement()) {
            d->autoincFields->append(f);
        }
    }
    return d->autoincFields;
}
