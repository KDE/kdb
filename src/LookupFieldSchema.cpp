/* This file is part of the KDE project
   Copyright (C) 2006-2010 Jarosław Staniek <staniek@kde.org>

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

#include "LookupFieldSchema.h"
#include "Utils.h"

#include <Predicate/Tools/Static>

#include <QDomElement>
#include <QVariant>
#include <QStringList>
#include <QHash>

#include <QtDebug>

using namespace Predicate;

//! @internal
class LookupFieldSchema::RecordSource::Private
{
public:
    Private()
            : type(LookupFieldSchema::RecordSource::NoType) {
    }
    LookupFieldSchema::RecordSource::Type type;
    QString name;
    QStringList values;
};

//! @internal
class LookupFieldSchema::Private
{
public:
    Private()
            : boundColumn(-1)
            , maximumListRows(PREDICATE_LOOKUP_FIELD_DEFAULT_LIST_ROWS)
            , displayWidget(PREDICATE_LOOKUP_FIELD_DEFAULT_DISPLAY_WIDGET)
            , columnHeadersVisible(PREDICATE_LOOKUP_FIELD_DEFAULT_HEADERS_VISIBLE)
            , limitToList(PREDICATE_LOOKUP_FIELD_DEFAULT_LIMIT_TO_LIST) {
    }

    RecordSource recordSource;
    int boundColumn;
    QList<uint> visibleColumns;
    QList<int> columnWidths;
    uint maximumListRows;
    DisplayWidget displayWidget;
    bool columnHeadersVisible;
    bool limitToList;
};

//! Cache
class LookupFieldSchemaStatic
{
public:
    LookupFieldSchemaStatic()
     : typeNames((QString[]){
            QString(),
            QLatin1String("table"),
            QLatin1String("query"),
            QLatin1String("sql"),
            QLatin1String("valuelist"),
            QLatin1String("fieldlist")})
    {
        typesForNames.insert(QLatin1String("table"), LookupFieldSchema::RecordSource::Table);
        typesForNames.insert(QLatin1String("query"), LookupFieldSchema::RecordSource::Query);
        typesForNames.insert(QLatin1String("sql"), LookupFieldSchema::RecordSource::SQLStatement);
        typesForNames.insert(QLatin1String("valuelist"), LookupFieldSchema::RecordSource::ValueList);
        typesForNames.insert(QLatin1String("fieldlist"), LookupFieldSchema::RecordSource::FieldList);
    }
    const QString typeNames[];
    QHash<QString, LookupFieldSchema::RecordSource::Type> typesForNames;
};

PREDICATE_GLOBAL_STATIC(LookupFieldSchemaStatic, Predicate_lookupFieldSchemaStatic)

//----------------------------

LookupFieldSchema::RecordSource::RecordSource()
        : d(new Private)
{
}

LookupFieldSchema::RecordSource::RecordSource(const RecordSource& other)
        : d(new Private)
{
    *d = *other.d;
}

LookupFieldSchema::RecordSource::~RecordSource()
{
    delete d;
}

LookupFieldSchema::RecordSource::Type LookupFieldSchema::RecordSource::type() const
{
    return d->type;
}

void LookupFieldSchema::RecordSource::setType(Type type)
{
    d->type = type;
}

QString LookupFieldSchema::RecordSource::name() const
{
    return d->name;
}

void LookupFieldSchema::RecordSource::setName(const QString& name)
{
    d->name = name;
    d->values.clear();
}

QString LookupFieldSchema::RecordSource::typeName() const
{
    Q_ASSERT(d->type < sizeof(Predicate_lookupFieldSchemaStatic->typeNames));
    return Predicate_lookupFieldSchemaStatic->typeNames[d->type];
}

void LookupFieldSchema::RecordSource::setTypeByName(const QString& typeName)
{
    setType(Predicate_lookupFieldSchemaStatic->typesForNames.value(typeName, NoType));
}

QStringList LookupFieldSchema::RecordSource::values() const
{
    return d->values;
}

void LookupFieldSchema::RecordSource::setValues(const QStringList& values)
{
    d->name.clear();
    d->values = values;
}

LookupFieldSchema::RecordSource& LookupFieldSchema::RecordSource::operator=(const RecordSource & other)
{
    if (this != &other) {
        *d = *other.d;
    }
    return *this;
}

QDebug operator<<(QDebug dbg, const LookupFieldSchema::RecordSource& source)
{
    dbg.nospace() << "RecordSource TYPE:";
    dbg.space() << source.typeName();
    dbg.space() << "NAME:";
    dbg.space() << source.name();
    dbg.space() << "VALUES:";
    dbg.space() << source.values().join(QLatin1String("|"));
    return dbg.space();
}

//----------------------------

LookupFieldSchema::LookupFieldSchema()
        : d(new Private)
{
}

LookupFieldSchema::~LookupFieldSchema()
{
    delete d;
}

LookupFieldSchema::RecordSource LookupFieldSchema::recordSource() const
{
    return d->recordSource;
}

void LookupFieldSchema::setRecordSource(const LookupFieldSchema::RecordSource& recordSource)
{
    d->recordSource = recordSource;
}

void LookupFieldSchema::setMaximumListRows(uint rows)
{
    if (rows == 0)
        d->maximumListRows = PREDICATE_LOOKUP_FIELD_DEFAULT_LIST_ROWS;
    else if (rows > PREDICATE_LOOKUP_FIELD_MAX_LIST_ROWS)
        d->maximumListRows = PREDICATE_LOOKUP_FIELD_MAX_LIST_ROWS;
    else
        d->maximumListRows = rows;
}

QDebug operator<<(QDebug dbg, const LookupFieldSchema& lookup)
{
    dbg.nospace() << "LookupFieldSchema(";
    dbg.space() << lookup.recordSource();
    dbg.space() << "boundColumn:";
    dbg.space() << lookup.boundColumn();
    dbg.space() << "visibleColumns:";

    bool first = true;
    foreach(uint visibleColumn, lookup.visibleColumns()) {
        if (first) {
            first = false;
            dbg.nospace();
        }
        else
            dbg.nospace() << ';';
        dbg.nospace() << visibleColumn;
    }
    dbg.space() << "maximumListRows:";
    dbg.space() << lookup.maximumListRows();
    dbg.space() << "displayWidget:";
    dbg.space() << (lookup.displayWidget() == LookupFieldSchema::ComboBox ? "ComboBox" : "ListBox");
    dbg.space() << "columnHeadersVisible:";
    dbg.space() << lookup.columnHeadersVisible();
    dbg.space() << "limitToList:";
    dbg.space() << lookup.limitToList();
    dbg.space() << "columnWidths:";

    first = true;
    for (QList<int>::ConstIterator it = lookup.columnWidths().constBegin();
            it != lookup.columnWidths().constEnd();++it)
    {
        if (first)
            first = false;
        else
            dbg.nospace() << ';';
        dbg.space() << *it;
    }
    dbg.nospace() << ')';
    return dbg.space();
}

/* static */
LookupFieldSchema *LookupFieldSchema::loadFromDom(const QDomElement& lookupEl)
{
    LookupFieldSchema *lookupFieldSchema = new LookupFieldSchema();
    LookupFieldSchema::RecordSource recordSource;
    for (QDomNode node = lookupEl.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement el = node.toElement();
        const QByteArray name(el.tagName().toLatin1());
        if (name == "row-source") {
            /*<row-source>
              empty
              | <type>table|query|sql|valuelist|fieldlist</type>  #required because there can be table and query with the same name
                        "fieldlist" (basically a list of column names of a table/query,
                              "Field List" as in MSA)
              <name>string</name> #table/query name, etc. or KEXISQL SELECT QUERY
              <values><value>...</value> #for "valuelist" type
                <value>...</value>
              </values>
             </row-source> */
            for (el = el.firstChild().toElement(); !el.isNull(); el = el.nextSibling().toElement()) {
                const QByteArray childName(el.tagName().toLatin1());
                if (childName == "type") {
                    recordSource.setTypeByName(el.text());
                }
                else if (childName == "name") {
                    recordSource.setName(el.text());
//! @todo handle fieldlist (retrieve from external table or so?), use RecordSource::setValues()
                }
            }
        } else if (name == "bound-column") {
            /* <bound-column>
                <number>number</number> #in later implementation there can be more columns
               </bound-column> */
            const QVariant val = Predicate::loadPropertyValueFromDom(el.firstChild());
            if (val.type() == QVariant::Int)
                lookupFieldSchema->setBoundColumn(val.toInt());
        } else if (name == "visible-column") {
            /* <visible-column> #a column that has to be visible in the combo box
              <number>number 1</number>
              <number>number 2</number>
              [..]
               </visible-column> */
            QList<uint> list;
            for (QDomNode childNode = el.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling()) {
                const QVariant val = Predicate::loadPropertyValueFromDom(childNode);
                if (val.type() == QVariant::Int)
                    list.append(val.toUInt());
            }
            lookupFieldSchema->setVisibleColumns(list);
        } else if (name == "column-widths") {
            /* <column-widths> #column widths, -1 means 'default'
                <number>int</number>
                ...
                <number>int</number>
               </column-widths> */
            QVariant val;
            QList<int> columnWidths;
            for (el = el.firstChild().toElement(); !el.isNull(); el = el.nextSibling().toElement()) {
                QVariant val = Predicate::loadPropertyValueFromDom(el);
                if (val.type() == QVariant::Int)
                    columnWidths.append(val.toInt());
            }
            lookupFieldSchema->setColumnWidths(columnWidths);
        } else if (name == "show-column-headers") {
            /* <show-column-headers>
                <bool>true/false</bool>
               </show-column-headers> */
            const QVariant val = Predicate::loadPropertyValueFromDom(el.firstChild());
            if (val.type() == QVariant::Bool)
                lookupFieldSchema->setColumnHeadersVisible(val.toBool());
        } else if (name == "list-rows") {
            /* <list-rows>
                <number>1..100</number>
               </list-rows> */
            const QVariant val = Predicate::loadPropertyValueFromDom(el.firstChild());
            if (val.type() == QVariant::Int)
                lookupFieldSchema->setMaximumListRows(val.toUInt());
        } else if (name == "limit-to-list") {
            /* <limit-to-list>
                <bool>true/false</bool>
               </limit-to-list> */
            const QVariant val = Predicate::loadPropertyValueFromDom(el.firstChild());
            if (val.type() == QVariant::Bool)
                lookupFieldSchema->setLimitToList(val.toBool());
        }
        else if (name == "display-widget") {
            const QByteArray displayWidgetName(el.text().toLatin1());
            if (displayWidgetName == "combobox") {
                lookupFieldSchema->setDisplayWidget(LookupFieldSchema::ComboBox);
            }
            else if (displayWidgetName == "listbox") {
                lookupFieldSchema->setDisplayWidget(LookupFieldSchema::ListBox);
            }
        }
    }
    lookupFieldSchema->setRecordSource(recordSource);
    return lookupFieldSchema;
}

/* static */
void LookupFieldSchema::saveToDom(LookupFieldSchema& lookupSchema, QDomDocument& doc, QDomElement& parentEl)
{
    QDomElement lookupColumnEl, rowSourceEl, rowSourceTypeEl, nameEl;
    if (!lookupSchema.recordSource().name().isEmpty()) {
        lookupColumnEl = doc.createElement(QLatin1String("lookup-column"));
        parentEl.appendChild(lookupColumnEl);

        rowSourceEl = doc.createElement(QLatin1String("row-source"));
        lookupColumnEl.appendChild(rowSourceEl);

        rowSourceTypeEl = doc.createElement(QLatin1String("type"));
        rowSourceEl.appendChild(rowSourceTypeEl);
        rowSourceTypeEl.appendChild(doc.createTextNode(lookupSchema.recordSource().typeName()));   //can be empty

        nameEl = doc.createElement(QLatin1String("name"));
        rowSourceEl.appendChild(nameEl);
        nameEl.appendChild(doc.createTextNode(lookupSchema.recordSource().name()));
    }

    const QStringList& values(lookupSchema.recordSource().values());
    if (!values.isEmpty()) {
        QDomElement valuesEl(doc.createElement(QLatin1String("values")));
        rowSourceEl.appendChild(valuesEl);
        for (QStringList::ConstIterator it = values.constBegin(); it != values.constEnd(); ++it) {
            QDomElement valueEl(doc.createElement(QLatin1String("value")));
            valuesEl.appendChild(valueEl);
            valueEl.appendChild(doc.createTextNode(*it));
        }
    }

    if (lookupSchema.boundColumn() >= 0)
        Predicate::saveNumberElementToDom(doc, lookupColumnEl,
                                          QLatin1String("bound-column"),
                                          lookupSchema.boundColumn());

    QList<uint> visibleColumns(lookupSchema.visibleColumns());
    if (!visibleColumns.isEmpty()) {
        QDomElement visibleColumnEl(doc.createElement(QLatin1String("visible-column")));
        lookupColumnEl.appendChild(visibleColumnEl);
        foreach(uint visibleColumn, visibleColumns) {
            QDomElement numberEl(doc.createElement(QLatin1String("number")));
            visibleColumnEl.appendChild(numberEl);
            numberEl.appendChild(doc.createTextNode(QString::number(visibleColumn)));
        }
    }

    const QList<int> columnWidths(lookupSchema.columnWidths());
    if (!columnWidths.isEmpty()) {
        QDomElement columnWidthsEl(doc.createElement(QLatin1String("column-widths")));
        lookupColumnEl.appendChild(columnWidthsEl);
        foreach(int columnWidth, columnWidths) {
            QDomElement columnWidthEl(doc.createElement(QLatin1String("number")));
            columnWidthsEl.appendChild(columnWidthEl);
            columnWidthEl.appendChild(doc.createTextNode(QString::number(columnWidth)));
        }
    }

    if (lookupSchema.columnHeadersVisible() != PREDICATE_LOOKUP_FIELD_DEFAULT_HEADERS_VISIBLE)
        Predicate::saveBooleanElementToDom(doc, lookupColumnEl,
                                           QLatin1String("show-column-headers"),
                                           lookupSchema.columnHeadersVisible());
    if (lookupSchema.maximumListRows() != PREDICATE_LOOKUP_FIELD_DEFAULT_LIST_ROWS)
        Predicate::saveNumberElementToDom(doc, lookupColumnEl,
                                          QLatin1String("list-rows"),
                                          lookupSchema.maximumListRows());
    if (lookupSchema.limitToList() != PREDICATE_LOOKUP_FIELD_DEFAULT_LIMIT_TO_LIST)
        Predicate::saveBooleanElementToDom(doc, lookupColumnEl,
                                           QLatin1String("limit-to-list"),
                                           lookupSchema.limitToList());

    if (lookupSchema.displayWidget() != PREDICATE_LOOKUP_FIELD_DEFAULT_DISPLAY_WIDGET) {
        QDomElement displayWidgetEl(doc.createElement(QLatin1String("display-widget")));
        lookupColumnEl.appendChild(displayWidgetEl);
        displayWidgetEl.appendChild(
            doc.createTextNode(
                QLatin1String((lookupSchema.displayWidget() == ListBox) ? "listbox" : "combobox")));
    }
}

//static
bool LookupFieldSchema::setProperty(
    LookupFieldSchema& lookup, const QByteArray& propertyName, const QVariant& value)
{
    bool ok;
    if ("rowSource" == propertyName
            || "rowSourceType" == propertyName
            || "rowSourceValues" == propertyName) {
        LookupFieldSchema::RecordSource recordSource(lookup.recordSource());
        if ("rowSource" == propertyName)
            recordSource.setName(value.toString());
        else if ("rowSourceType" == propertyName)
            recordSource.setTypeByName(value.toString());
        else if ("rowSourceValues" == propertyName)
            recordSource.setValues(value.toStringList());
        lookup.setRecordSource(recordSource);
    }
    else if ("boundColumn" == propertyName) {
        const int ival = value.toInt(&ok);
        if (!ok)
            return false;
        lookup.setBoundColumn(ival);
    }
    else if ("visibleColumn" == propertyName) {
        QList<QVariant> variantList;
        if (value.type() == QVariant::Int) {
//! @todo Remove this case: it's for backward compatibility with Kexi's 1.1.2 table designer GUI
//!       supporting only single lookup column.
            variantList.append(value.toInt());
        } else {
            variantList = value.toList();
        }
        QList<uint> visibleColumns;
        foreach(const QVariant& variant, variantList) {
            const uint ival = variant.toUInt(&ok);
            if (!ok)
                return false;
            visibleColumns.append(ival);
        }
        lookup.setVisibleColumns(visibleColumns);
    } else if ("columnWidths" == propertyName) {
        QList<QVariant> variantList(value.toList());
        QList<int> widths;
        foreach(const QVariant& variant, variantList) {
            const uint ival = variant.toInt(&ok);
            if (!ok)
                return false;
            widths.append(ival);
        }
        lookup.setColumnWidths(widths);
    } else if ("showColumnHeaders" == propertyName) {
        lookup.setColumnHeadersVisible(value.toBool());
    } else if ("listRows" == propertyName) {
        lookup.setMaximumListRows(value.toBool());
    } else if ("limitToList" == propertyName) {
        lookup.setLimitToList(value.toBool());
    } else if ("displayWidget" == propertyName) {
        const uint ival = value.toUInt(&ok);
        if (!ok || ival > LookupFieldSchema::ListBox)
            return false;
        lookup.setDisplayWidget((LookupFieldSchema::DisplayWidget)ival);
    }
    return true;
}

int LookupFieldSchema::boundColumn() const
{
    return d->boundColumn;
}

void LookupFieldSchema::setBoundColumn(int column)
{
    d->boundColumn = column >= 0 ? column : -1;
}

QList<uint> LookupFieldSchema::visibleColumns() const
{
    return d->visibleColumns;
}

void LookupFieldSchema::setVisibleColumns(const QList<uint>& list)
{
    d->visibleColumns = list;
}

int LookupFieldSchema::visibleColumn(uint fieldsCount) const
{
    if (d->visibleColumns.count() == 1)
        return (d->visibleColumns.first() < fieldsCount) ? (int)d->visibleColumns.first() : -1;
    if (d->visibleColumns.isEmpty())
        return -1;
    return fieldsCount - 1;
}

QList<int> LookupFieldSchema::columnWidths() const
{
    return d->columnWidths;
}

void LookupFieldSchema::setColumnWidths(const QList<int>& widths)
{
    d->columnWidths = widths;
}

bool LookupFieldSchema::columnHeadersVisible() const
{
    return d->columnHeadersVisible;
}

void LookupFieldSchema::setColumnHeadersVisible(bool set)
{
    d->columnHeadersVisible = set;
}

uint LookupFieldSchema::maximumListRows() const
{
    return d->maximumListRows;
}

bool LookupFieldSchema::limitToList() const
{
    return d->limitToList;
}

void LookupFieldSchema::setLimitToList(bool set)
{
    d->limitToList = set;
}

LookupFieldSchema::DisplayWidget LookupFieldSchema::displayWidget() const
{
    return d->displayWidget;
}

void LookupFieldSchema::setDisplayWidget(DisplayWidget widget)
{
    d->displayWidget = widget;
}
