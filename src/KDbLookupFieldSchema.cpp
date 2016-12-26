/* This file is part of the KDE project
   Copyright (C) 2006-2015 Jarosław Staniek <staniek@kde.org>

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

#include "KDbLookupFieldSchema.h"
#include "KDb.h"
#include "kdb_debug.h"

#include <QDomElement>
#include <QVariant>
#include <QStringList>
#include <QHash>

#include <vector>

//! @internal
class Q_DECL_HIDDEN KDbLookupFieldSchemaRecordSource::Private
{
public:
    Private()
            : type(KDbLookupFieldSchemaRecordSource::NoType) {
    }
    KDbLookupFieldSchemaRecordSource::Type type;
    QString name;
    QStringList values;
};

//! @internal
class Q_DECL_HIDDEN KDbLookupFieldSchema::Private
{
public:
    Private()
            : boundColumn(-1)
            , maxVisibleRecords(KDB_LOOKUP_FIELD_DEFAULT_MAX_VISIBLE_RECORDS)
            , displayWidget(KDB_LOOKUP_FIELD_DEFAULT_DISPLAY_WIDGET)
            , columnHeadersVisible(KDB_LOOKUP_FIELD_DEFAULT_HEADERS_VISIBLE)
            , limitToList(KDB_LOOKUP_FIELD_DEFAULT_LIMIT_TO_LIST) {
    }

    KDbLookupFieldSchemaRecordSource recordSource;
    int boundColumn;
    QList<int> visibleColumns;
    QList<int> columnWidths;
    int maxVisibleRecords;
    DisplayWidget displayWidget;
    bool columnHeadersVisible;
    bool limitToList;
};

//! Cache
class LookupFieldSchemaStatic
{
public:
    LookupFieldSchemaStatic()
     : typeNames({
            QString(), // no type
            QLatin1String("table"),
            QLatin1String("query"),
            QLatin1String("sql"),
            QLatin1String("valuelist"),
            QLatin1String("fieldlist")})
    {
        typesForNames.insert(QLatin1String("table"), KDbLookupFieldSchemaRecordSource::Table);
        typesForNames.insert(QLatin1String("query"), KDbLookupFieldSchemaRecordSource::Query);
        typesForNames.insert(QLatin1String("sql"), KDbLookupFieldSchemaRecordSource::SQLStatement);
        typesForNames.insert(QLatin1String("valuelist"), KDbLookupFieldSchemaRecordSource::ValueList);
        typesForNames.insert(QLatin1String("fieldlist"), KDbLookupFieldSchemaRecordSource::KDbFieldList);
    }
    const std::vector<QString> typeNames;
    QHash<QString, KDbLookupFieldSchemaRecordSource::Type> typesForNames;
private:
    Q_DISABLE_COPY(LookupFieldSchemaStatic)
};

Q_GLOBAL_STATIC(LookupFieldSchemaStatic, KDb_lookupFieldSchemaStatic)

//----------------------------

KDbLookupFieldSchemaRecordSource::KDbLookupFieldSchemaRecordSource()
        : d(new Private)
{
}

KDbLookupFieldSchemaRecordSource::KDbLookupFieldSchemaRecordSource(const KDbLookupFieldSchemaRecordSource& other)
        : d(new Private)
{
    *d = *other.d;
}

KDbLookupFieldSchemaRecordSource::~KDbLookupFieldSchemaRecordSource()
{
    delete d;
}

KDbLookupFieldSchemaRecordSource::Type KDbLookupFieldSchemaRecordSource::type() const
{
    return d->type;
}

void KDbLookupFieldSchemaRecordSource::setType(Type type)
{
    d->type = type;
}

QString KDbLookupFieldSchemaRecordSource::name() const
{
    return d->name;
}

void KDbLookupFieldSchemaRecordSource::setName(const QString& name)
{
    d->name = name;
    d->values.clear();
}

QString KDbLookupFieldSchemaRecordSource::typeName() const
{
    Q_ASSERT(size_t(d->type) < KDb_lookupFieldSchemaStatic->typeNames.size());
    return KDb_lookupFieldSchemaStatic->typeNames[d->type];
}

void KDbLookupFieldSchemaRecordSource::setTypeByName(const QString& typeName)
{
    setType(KDb_lookupFieldSchemaStatic->typesForNames.value(typeName, NoType));
}

QStringList KDbLookupFieldSchemaRecordSource::values() const
{
    return d->values;
}

void KDbLookupFieldSchemaRecordSource::setValues(const QStringList& values)
{
    d->name.clear();
    d->values = values;
}

KDbLookupFieldSchemaRecordSource& KDbLookupFieldSchemaRecordSource::operator=(const KDbLookupFieldSchemaRecordSource & other)
{
    if (this != &other) {
        *d = *other.d;
    }
    return *this;
}

QDebug operator<<(QDebug dbg, const KDbLookupFieldSchemaRecordSource& source)
{
    dbg.nospace() << "LookupFieldSchemaRecordSource TYPE:";
    dbg.space() << source.typeName();
    dbg.space() << "NAME:";
    dbg.space() << source.name();
    dbg.space() << "VALUES:";
    dbg.space() << source.values().join(QLatin1String("|")) << '\n';
    return dbg.nospace();
}

//----------------------------

KDbLookupFieldSchema::KDbLookupFieldSchema()
        : d(new Private)
{
}

KDbLookupFieldSchema::KDbLookupFieldSchema(const KDbLookupFieldSchema &schema)
: d(new Private)
{
    *d = *schema.d;
}

KDbLookupFieldSchema::~KDbLookupFieldSchema()
{
    delete d;
}

static bool setBoundColumn(KDbLookupFieldSchema *lookup, const QVariant &val)
{
    if (val.isNull()) {
        lookup->setBoundColumn(-1);
    }
    else {
        bool ok;
        const int ival = val.toInt(&ok);
        if (!ok)
            return false;
        lookup->setBoundColumn(ival);
    }
    return true;
}

static bool setVisibleColumns(KDbLookupFieldSchema *lookup, const QVariant &val)
{
    QList<QVariant> variantList;
    if (val.canConvert(QVariant::Int)) {
    //! @todo Remove this case: it's for backward compatibility with Kexi's 1.1.2 table designer GUI
    //!       supporting only single lookup column.
        variantList.append(val);
    }
    else {
        variantList = val.toList();
    }
    QList<int> visibleColumns;
    foreach(const QVariant& variant, variantList) {
        bool ok;
        const int ival = variant.toInt(&ok);
        if (!ok) {
            return false;
        }
        visibleColumns.append(ival);
    }
    lookup->setVisibleColumns(visibleColumns);
    return true;
}

static bool setColumnWidths(KDbLookupFieldSchema *lookup, const QVariant &val)
{
    QList<int> widths;
    foreach(const QVariant& variant, val.toList()) {
        bool ok;
        const int ival = variant.toInt(&ok);
        if (!ok)
            return false;
        widths.append(ival);
    }
    lookup->setColumnWidths(widths);
    return true;
}

static bool setDisplayWidget(KDbLookupFieldSchema *lookup, const QVariant &val)
{
    bool ok;
    const int ival = val.toInt(&ok);
    if (!ok || ival > KDbLookupFieldSchema::ListBox)
        return false;
    lookup->setDisplayWidget(static_cast<KDbLookupFieldSchema::DisplayWidget>(ival));
    return true;
}

KDbLookupFieldSchemaRecordSource KDbLookupFieldSchema::recordSource() const
{
    return d->recordSource;
}

void KDbLookupFieldSchema::setRecordSource(const KDbLookupFieldSchemaRecordSource& recordSource)
{
    d->recordSource = recordSource;
}

void KDbLookupFieldSchema::setMaxVisibleRecords(int count)
{
    if (count == 0)
        d->maxVisibleRecords = KDB_LOOKUP_FIELD_DEFAULT_MAX_VISIBLE_RECORDS;
    else if (count > KDB_LOOKUP_FIELD_LIMIT_MAX_VISIBLE_RECORDS)
        d->maxVisibleRecords = KDB_LOOKUP_FIELD_LIMIT_MAX_VISIBLE_RECORDS;
    else
        d->maxVisibleRecords = count;
}

QDebug operator<<(QDebug dbg, const KDbLookupFieldSchema& lookup)
{
    dbg.nospace() << "LookupFieldSchema(";
    dbg.space() << lookup.recordSource();
    dbg.space() << "boundColumn:";
    dbg.space() << lookup.boundColumn();
    dbg.space() << "visibleColumns:";

    bool first = true;
    foreach(int visibleColumn, lookup.visibleColumns()) {
        if (first) {
            first = false;
            dbg.nospace();
        }
        else
            dbg.nospace() << ';';
        dbg.nospace() << visibleColumn;
    }
    dbg.space() << "maxVisibleRecords:";
    dbg.space() << lookup.maxVisibleRecords();
    dbg.space() << "displayWidget:";
    dbg.space() << (lookup.displayWidget() == KDbLookupFieldSchema::ComboBox ? "ComboBox" : "ListBox");
    dbg.space() << "columnHeadersVisible:";
    dbg.space() << lookup.columnHeadersVisible();
    dbg.space() << "limitToList:";
    dbg.space() << lookup.limitToList();
    dbg.space() << "columnWidths:";

    first = true;
    const QList<int> columnWidths(lookup.columnWidths());
    for (int width : columnWidths) {
        if (first)
            first = false;
        else
            dbg.nospace() << ';';
        dbg.space() << width;
    }
    dbg.nospace() << ')';
    return dbg.space();
}

/* static */
KDbLookupFieldSchema *KDbLookupFieldSchema::loadFromDom(const QDomElement& lookupEl)
{
    KDbLookupFieldSchema *lookupFieldSchema = new KDbLookupFieldSchema();
    KDbLookupFieldSchemaRecordSource recordSource;
    for (QDomNode node = lookupEl.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement el = node.toElement();
        const QByteArray name(el.tagName().toLatin1());
        if (name == "row-source") {
            /*<row-source>
              empty
              | <type>table|query|sql|valuelist|fieldlist</type> #required because there can be
                                                                 #table and query with the same name
                                                                 #"fieldlist" (basically a list of
                                                                 #column names of a table/query,
                                                                 #"Field List" as in MSA)
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
//! @todo handle fieldlist (retrieve from external table or so?), use KDbLookupFieldSchemaRecordSource::setValues()
                }
            }
        } else if (name == "bound-column") {
            /* <bound-column>
                <number>number</number> #in later implementation there can be more columns
               </bound-column> */
            bool ok;
            const QVariant val = KDb::loadPropertyValueFromDom(el.firstChild(), &ok);
            if (!ok || !::setBoundColumn(lookupFieldSchema, val)) {
                delete lookupFieldSchema;
                return 0;
            }
        } else if (name == "visible-column") {
            /* <visible-column> #a column that has to be visible in the combo box
              <number>number 1</number>
              <number>number 2</number>
              [..]
               </visible-column> */
            QVariantList list;
            for (QDomNode childNode = el.firstChild(); !childNode.isNull();
                 childNode = childNode.nextSibling())
            {
                bool ok;
                const QVariant val = KDb::loadPropertyValueFromDom(childNode, &ok);
                if (!ok) {
                    delete lookupFieldSchema;
                    return 0;
                }
                list.append(val);
            }
            if (!::setVisibleColumns(lookupFieldSchema, list)) {
                delete lookupFieldSchema;
                return 0;
            }
        } else if (name == "column-widths") {
            /* <column-widths> #column widths, -1 means 'default'
                <number>int</number>
                ...
                <number>int</number>
               </column-widths> */
            QVariantList columnWidths;
            for (el = el.firstChild().toElement(); !el.isNull(); el = el.nextSibling().toElement()) {
                bool ok;
                QVariant val = KDb::loadPropertyValueFromDom(el, &ok);
                if (!ok) {
                    delete lookupFieldSchema;
                    return 0;
                }
                columnWidths.append(val);
            }
            if (!::setColumnWidths(lookupFieldSchema, columnWidths)) {
                delete lookupFieldSchema;
                return 0;
            }
        } else if (name == "show-column-headers") {
            /* <show-column-headers>
                <bool>true/false</bool>
               </show-column-headers> */
            bool ok;
            const QVariant val = KDb::loadPropertyValueFromDom(el.firstChild(), &ok);
            if (!ok) {
                delete lookupFieldSchema;
                return 0;
            }
            if (val.type() == QVariant::Bool)
                lookupFieldSchema->setColumnHeadersVisible(val.toBool());
        } else if (name == "list-rows") {
            /* <list-rows>
                <number>1..100</number>
               </list-rows> */
            bool ok;
            const QVariant val = KDb::loadPropertyValueFromDom(el.firstChild(), &ok);
            if (!ok) {
                delete lookupFieldSchema;
                return 0;
            }
            if (val.type() == QVariant::Int)
                lookupFieldSchema->setMaxVisibleRecords(val.toInt());
        } else if (name == "limit-to-list") {
            /* <limit-to-list>
                <bool>true/false</bool>
               </limit-to-list> */
            bool ok;
            const QVariant val = KDb::loadPropertyValueFromDom(el.firstChild(), &ok);
            if (!ok) {
                delete lookupFieldSchema;
                return 0;
            }
            if (val.type() == QVariant::Bool)
                lookupFieldSchema->setLimitToList(val.toBool());
        }
        else if (name == "display-widget") {
            const QByteArray displayWidgetName(el.text().toLatin1());
            if (displayWidgetName == "combobox") {
                lookupFieldSchema->setDisplayWidget(KDbLookupFieldSchema::ComboBox);
            }
            else if (displayWidgetName == "listbox") {
                lookupFieldSchema->setDisplayWidget(KDbLookupFieldSchema::ListBox);
            }
        }
    }
    lookupFieldSchema->setRecordSource(recordSource);
    return lookupFieldSchema;
}

void KDbLookupFieldSchema::saveToDom(QDomDocument *doc, QDomElement *parentEl)
{
    Q_ASSERT(doc);
    Q_ASSERT(parentEl);
    QDomElement lookupColumnEl, recordSourceEl, recordSourceTypeEl, nameEl;
    if (!recordSource().name().isEmpty()) {
        lookupColumnEl = doc->createElement(QLatin1String("lookup-column"));
        parentEl->appendChild(lookupColumnEl);

        recordSourceEl = doc->createElement(QLatin1String("row-source"));
        lookupColumnEl.appendChild(recordSourceEl);

        recordSourceTypeEl = doc->createElement(QLatin1String("type"));
        recordSourceEl.appendChild(recordSourceTypeEl);
        recordSourceTypeEl.appendChild(doc->createTextNode(recordSource().typeName()));   //can be empty

        nameEl = doc->createElement(QLatin1String("name"));
        recordSourceEl.appendChild(nameEl);
        nameEl.appendChild(doc->createTextNode(recordSource().name()));
    }

    const QStringList& values(recordSource().values());
    if (!values.isEmpty()) {
        QDomElement valuesEl(doc->createElement(QLatin1String("values")));
        recordSourceEl.appendChild(valuesEl);
        for (QStringList::ConstIterator it = values.constBegin(); it != values.constEnd(); ++it) {
            QDomElement valueEl(doc->createElement(QLatin1String("value")));
            valuesEl.appendChild(valueEl);
            valueEl.appendChild(doc->createTextNode(*it));
        }
    }

    if (boundColumn() >= 0) {
        KDb::saveNumberElementToDom(doc, &lookupColumnEl,
                                          QLatin1String("bound-column"), boundColumn());
    }

    QList<int> visibleColumns(this->visibleColumns());
    if (!visibleColumns.isEmpty()) {
        QDomElement visibleColumnEl(doc->createElement(QLatin1String("visible-column")));
        lookupColumnEl.appendChild(visibleColumnEl);
        foreach(int visibleColumn, visibleColumns) {
            QDomElement numberEl(doc->createElement(QLatin1String("number")));
            visibleColumnEl.appendChild(numberEl);
            numberEl.appendChild(doc->createTextNode(QString::number(visibleColumn)));
        }
    }

    const QList<int> columnWidths(this->columnWidths());
    if (!columnWidths.isEmpty()) {
        QDomElement columnWidthsEl(doc->createElement(QLatin1String("column-widths")));
        lookupColumnEl.appendChild(columnWidthsEl);
        foreach(int columnWidth, columnWidths) {
            QDomElement columnWidthEl(doc->createElement(QLatin1String("number")));
            columnWidthsEl.appendChild(columnWidthEl);
            columnWidthEl.appendChild(doc->createTextNode(QString::number(columnWidth)));
        }
    }

    if (columnHeadersVisible() != KDB_LOOKUP_FIELD_DEFAULT_HEADERS_VISIBLE)
        KDb::saveBooleanElementToDom(doc, &lookupColumnEl,
                                           QLatin1String("show-column-headers"),
                                           columnHeadersVisible());
    if (maxVisibleRecords() != KDB_LOOKUP_FIELD_DEFAULT_MAX_VISIBLE_RECORDS)
        KDb::saveNumberElementToDom(doc, &lookupColumnEl,
                                          QLatin1String("list-rows"),
                                          maxVisibleRecords());
    if (limitToList() != KDB_LOOKUP_FIELD_DEFAULT_LIMIT_TO_LIST)
        KDb::saveBooleanElementToDom(doc, &lookupColumnEl,
                                           QLatin1String("limit-to-list"),
                                           limitToList());

    if (displayWidget() != KDB_LOOKUP_FIELD_DEFAULT_DISPLAY_WIDGET) {
        QDomElement displayWidgetEl(doc->createElement(QLatin1String("display-widget")));
        lookupColumnEl.appendChild(displayWidgetEl);
        displayWidgetEl.appendChild(
            doc->createTextNode(
                QLatin1String((displayWidget() == ListBox) ? "listbox" : "combobox")));
    }
}

void KDbLookupFieldSchema::getProperties(QMap<QByteArray, QVariant> *values) const
{
    values->clear();
    KDb::getProperties(this, values);
}

bool KDbLookupFieldSchema::setProperty(const QByteArray& propertyName, const QVariant& value)
{
    bool ok;
    if (   "rowSource" == propertyName
        || "rowSourceType" == propertyName
        || "rowSourceValues" == propertyName)
    {
        KDbLookupFieldSchemaRecordSource recordSource(this->recordSource());
        if ("rowSource" == propertyName)
            recordSource.setName(value.toString());
        else if ("rowSourceType" == propertyName)
            recordSource.setTypeByName(value.toString());
        else if ("rowSourceValues" == propertyName) {
            recordSource.setValues(value.toStringList());
        } else {
            kdbCritical() << "impl. error: unsupported property" << propertyName;
        }
        this->setRecordSource(recordSource);
    }
    else if ("boundColumn" == propertyName) {
        if (!::setBoundColumn(this, value)) {
            return false;
        }
    }
    else if ("visibleColumn" == propertyName) {
        if (!::setVisibleColumns(this, value)) {
            return false;
        }
    } else if ("columnWidths" == propertyName) {
        if (!::setColumnWidths(this, value)) {
            return false;
        }
    } else if ("showColumnHeaders" == propertyName) {
        setColumnHeadersVisible(value.toBool());
    } else if ("listRows" == propertyName) {
        const int ival = value.toInt(&ok);
        if (!ok)
            return false;
        setMaxVisibleRecords(ival);
    } else if ("limitToList" == propertyName) {
        setLimitToList(value.toBool());
    } else if ("displayWidget" == propertyName) {
        if (!::setDisplayWidget(this, value)) {
            return false;
        }
    }
    return true;
}

bool KDbLookupFieldSchema::setProperties(const QMap<QByteArray, QVariant>& values)
{
    QMap<QByteArray, QVariant>::ConstIterator it;
    KDbLookupFieldSchemaRecordSource recordSource(this->recordSource());
    bool ok;
    bool updateRecordSource = false;
    if ((it = values.find("rowSource")) != values.constEnd()) {
        recordSource.setName(it.value().toString());
        updateRecordSource = true;
    }
    if ((it = values.find("rowSourceType")) != values.constEnd()) {
        recordSource.setTypeByName(it.value().toString());
        updateRecordSource = true;
    }
    if ((it = values.find("rowSourceValues")) != values.constEnd()) {
        if (!it.value().isNull()) {
            recordSource.setValues(it.value().toStringList());
            updateRecordSource = true;
        }
    }
    if (updateRecordSource) {
        setRecordSource(recordSource);
    }
    if ((it = values.find("boundColumn")) != values.constEnd()) {
        if (!::setBoundColumn(this, it.value())) {
            return false;
        }
    }
    if ((it = values.find("visibleColumn")) != values.constEnd()) {
        if (!::setVisibleColumns(this, it.value())) {
            return false;
        }
    }
    if ((it = values.find("columnWidths")) != values.constEnd()) {
        if (!::setColumnWidths(this, it.value())) {
            return false;
        }
    }
    if ((it = values.find("showColumnHeaders")) != values.constEnd()) {
        setColumnHeadersVisible(it.value().toBool());
    }
    if ((it = values.find("listRows")) != values.constEnd()) {
        int ival = it.value().toInt(&ok);
        if (!ok)
            return false;
        setMaxVisibleRecords(ival);
    }
    if ((it = values.find("limitToList")) != values.constEnd()) {
        setLimitToList(it.value().toBool());
    }
    if ((it = values.find("displayWidget")) != values.constEnd()) {
        if (!::setDisplayWidget(this, it.value())) {
            return false;
        }
    }
    return true;
}


int KDbLookupFieldSchema::boundColumn() const
{
    return d->boundColumn;
}

void KDbLookupFieldSchema::setBoundColumn(int column)
{
    d->boundColumn = column >= 0 ? column : -1;
}

QList<int> KDbLookupFieldSchema::visibleColumns() const
{
    return d->visibleColumns;
}

void KDbLookupFieldSchema::setVisibleColumns(const QList<int>& list)
{
    d->visibleColumns = list;
}

int KDbLookupFieldSchema::visibleColumn(int index) const
{
    if (index >= d->visibleColumns.count()) {
        return -1;
    }
    return index;
}

QList<int> KDbLookupFieldSchema::columnWidths() const
{
    return d->columnWidths;
}

void KDbLookupFieldSchema::setColumnWidths(const QList<int>& widths)
{
    d->columnWidths = widths;
}

bool KDbLookupFieldSchema::columnHeadersVisible() const
{
    return d->columnHeadersVisible;
}

void KDbLookupFieldSchema::setColumnHeadersVisible(bool set)
{
    d->columnHeadersVisible = set;
}

int KDbLookupFieldSchema::maxVisibleRecords() const
{
    return d->maxVisibleRecords;
}

bool KDbLookupFieldSchema::limitToList() const
{
    return d->limitToList;
}

void KDbLookupFieldSchema::setLimitToList(bool set)
{
    d->limitToList = set;
}

KDbLookupFieldSchema::DisplayWidget KDbLookupFieldSchema::displayWidget() const
{
    return d->displayWidget;
}

void KDbLookupFieldSchema::setDisplayWidget(DisplayWidget widget)
{
    d->displayWidget = widget;
}
