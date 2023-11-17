/* This file is part of the KDE project
   Copyright (C) 2018 Jarosław Staniek <staniek@kde.org>

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
 * Boston, MA 02110-1301, USA.
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 */

#include "KDbDateTime.h"

#include <QRegularExpression>

const int UNCACHED_YEAR = -1;
const int INVALID_YEAR = -2;

namespace {
template <typename T>
std::function<QString(const T&)> byteArrayToString()
{
    return [](const T &v) { return QString::fromLatin1(v.toString()); };
}

struct KDbDateTimeMetatypeInitializer {
    KDbDateTimeMetatypeInitializer()
    {
        using namespace std::placeholders;
        QMetaType::registerConverter<KDbYear, QString>(byteArrayToString<KDbYear>());
        QMetaType::registerConverter<KDbYear, int>(std::bind(&KDbYear::toIsoValue, _1));
        QMetaType::registerConverter<KDbDate, QString>(byteArrayToString<KDbDate>());
        QMetaType::registerConverter<KDbDate, QDate>(std::bind(&KDbDate::toQDate, _1));
        QMetaType::registerConverter<KDbTime, QString>(byteArrayToString<KDbTime>());
        QMetaType::registerConverter<KDbTime, QTime>(std::bind(&KDbTime::toQTime, _1));
        QMetaType::registerConverter<KDbDateTime, QString>(byteArrayToString<KDbDateTime>());
        QMetaType::registerConverter<KDbDateTime, QDateTime>(std::bind(&KDbDateTime::toQDateTime, _1));
#if QT_VERSION <= QT_VERSION_CHECK(6, 0, 0)
        QMetaType::registerComparators<KDbYear>();
        QMetaType::registerComparators<KDbDate>();
        QMetaType::registerComparators<KDbTime>();
        QMetaType::registerComparators<KDbDateTime>();
#endif
    }
};

KDbDateTimeMetatypeInitializer s_init;
}

bool KDbYear::operator==(const KDbYear &other) const
{
    return m_sign == other.sign() && m_string == other.yearString();
}

bool KDbYear::operator<(const KDbYear &other) const
{
    return toQDateValue() < other.toQDateValue();
}

bool KDbYear::isValid() const
{
    return std::get<1>(intValue());
}

bool KDbYear::isNull() const
{
    return m_sign == Sign::None && m_string.isEmpty();
}

QByteArray KDbYear::signString() const
{
    QByteArray result;
    switch (m_sign) {
    case Sign::Plus:
        result = QByteArrayLiteral("+");
        break;
    case Sign::Minus:
        result = QByteArrayLiteral("-");
        break;
    default:
        break;
    }
    return result;
}

KDB_EXPORT QDebug operator<<(QDebug dbg, KDbYear::Sign sign)
{
    QDebugStateSaver saver(dbg);
    switch (sign) {
    case KDbYear::Sign::None:
        break;
    case KDbYear::Sign::Plus:
        dbg.nospace() << '+';
        break;
    case KDbYear::Sign::Minus:
        dbg.nospace() << '-';
        break;
    }
    return dbg.maybeSpace();
}

KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbYear& year)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace().noquote() << "KDbYear(" << year.sign() << year.yearString();
    if (!year.isValid()) {
        dbg.nospace() << " INVALID";
    }
    dbg.nospace() << ")";
    return dbg.maybeSpace();
}

QByteArray KDbYear::toString() const
{
    QByteArray result;
    if (isNull()) {
        result = QByteArrayLiteral("<NULL_YEAR>");
    } else { // can be invalid, that's OK
        result = signString() + m_string;
    }
    return result;
}

int KDbYear::toIsoValue() const
{
    return std::get<0>(intValue());
}

int KDbYear::toQDateValue() const
{
    int result;
    bool ok;
    std::tie(result, ok) = intValue();
    if (!ok) {
        return 0;
    }
    if (result > 0) {
        return result;
    }
    return result - 1;
}

namespace {
int intValueInternal(KDbYear::Sign sign, const QByteArray &string)
{
    const int length = string.length();
    if (length < 4) {
        // TODO message: at least 4 digits required
        return INVALID_YEAR;
    } else if (length > 4) {
        if (sign == KDbYear::Sign::None) {
            // TODO message: more than 4 digits, sign required
            return INVALID_YEAR;
        }
    }

    static const QRegularExpression digitsRegExp(QStringLiteral("^\\d+$"));
    if (!digitsRegExp.match(QString::fromLatin1(string)).hasMatch()) {
        // TODO message: only digits are accepted for year
        return INVALID_YEAR;
    }

    bool ok;
    int result = string.toInt(&ok);
    if (!ok || result < 0) {
        // TODO message: failed to convert year to integer >= 0
        return INVALID_YEAR;
    }
    int qDateYear;
    if (result == 0) {
        if (sign != KDbYear::Sign::Plus) {
            // TODO message: + required for 0000
            return INVALID_YEAR;
        }
        qDateYear = -1;
    } else if (sign == KDbYear::Sign::Minus) {
        qDateYear = - result - 1;
    } else { // Plus or None
        qDateYear = result;
    }
    // verify if this year is within the limits of QDate (see QDate::minJd(), QDate::maxJd())
    if (!QDate(qDateYear, 1, 1).isValid()) {
        // TODO message: year is not within limits
        return INVALID_YEAR;
    }
    return result;
}
}

std::tuple<int, bool> KDbYear::intValue() const
{
    if (m_isoValue == UNCACHED_YEAR) {
        const_cast<int&>(m_isoValue) = intValueInternal(m_sign, m_string); // cache
    }
    if (m_isoValue == INVALID_YEAR) {
        return std::make_tuple(0, false);
    }
    return std::make_tuple(m_sign == Sign::Minus ? -m_isoValue : m_isoValue, true);
}

bool KDbDate::operator==(const KDbDate &other) const
{
    return m_year == other.year() && m_monthString == other.monthString()
        && m_dayString == other.dayString();
}

bool KDbDate::operator<(const KDbDate &other) const
{
    return toQDate() < other.toQDate();
}

bool KDbDate::isValid() const
{
    return toQDate().isValid();
}

bool KDbDate::isNull() const
{
    return m_year.isNull() && m_monthString.isEmpty() && m_dayString.isEmpty();
}

QDate KDbDate::toQDate() const
{
    return { m_year.toQDateValue(), month(), day() };
}

namespace {
int toInt(const QByteArray &string, int min, int max, int minLength, int maxLength)
{
    if (string.length() < minLength || string.length() > maxLength) {
        // TODO message: invalid length
        return -1;
    }
    bool ok = true;
    const int result = string.isEmpty() ? 0 : string.toInt(&ok);
    if (!ok || result < min || result > max) {
        // TODO message: could not convert string to integer
        return -1;
    }
    return result;
}
}

int KDbDate::month() const
{
    return toInt(m_monthString, 1, 12, 1, 2);
}

int KDbDate::day() const
{
    return toInt(m_dayString, 1, 31, 1, 2);
}

QByteArray KDbDate::toString() const
{
    QByteArray result;
    if (isNull()) {
        result = QByteArrayLiteral("<NULL_DATE>");
    } else { // can be invalid, that's OK
        result = m_year.toString() + '-' + m_monthString + '-' + m_dayString;
    }
    return result;
}

KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbDate &date)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace().noquote() << "KDbDate(" << date.toString();
    if (!date.isValid()) {
        dbg.nospace() << " INVALID";
    }
    dbg.nospace() << ")";
    return dbg.maybeSpace();
}

bool KDbTime::operator==(const KDbTime &other) const
{
    return m_hourString == other.hourString() && m_minuteString == other.minuteString()
        && m_secondString == other.secondString() && m_msecString == other.msecString()
        && m_period == other.period();
}

bool KDbTime::operator<(const KDbTime &other) const
{
    return toQTime() < other.toQTime();
}

QTime KDbTime::toQTime() const
{
    // Rules for hours based on https://www.timeanddate.com/time/am-and-pm.html#converting
    int h = hour();
    if (h == -1) {
        return {};
    }
    const int m = minute();
    if (m == -1) {
        return {};
    }
    const int s = second();
    if (s == -1) {
        return {};
    }
    const int ms = msec();
    if (ms == -1) {
        return {};
    }
    if (m_period == Period::None) {
        return { h, m, s, ms };
    }
    return QTime::fromString(
        QStringLiteral("%1:%2:%3.%4 %5")
            .arg(h)
            .arg(m)
            .arg(s)
            .arg(ms)
            .arg(m_period == Period::Am ? QLatin1String("AM") : QLatin1String("PM")),
        QStringLiteral("h:m:s.z AP"));
}

int KDbTime::hour() const
{
    switch (m_period) {
    case Period::None:
        return toInt(m_hourString, 0, 23, 1, 2);
    case Period::Am:
    case Period::Pm:
        return toInt(m_hourString, 1, 12, 1, 2);
    }
    return -1;
}

int KDbTime::minute() const
{
    return toInt(m_minuteString, 0, 59, 1, 2);
}

int KDbTime::second() const
{
    return toInt(m_secondString, 0, 59, 0, 2);
}

int KDbTime::msec() const
{
    return toInt(m_msecString, 0, 999, 0, 3);
}

bool KDbTime::isValid() const
{
    return toQTime().isValid();
}

bool KDbTime::isNull() const
{
    return m_hourString.isEmpty() || m_minuteString.isEmpty();
}

QByteArray KDbTime::toString() const
{
    QByteArray result;
    if (isNull()) {
        result = QByteArrayLiteral("<NULL_TIME>");
    } else if (m_msecString.isEmpty()) { // can be invalid, that's OK
        if (m_secondString.isEmpty()) {
            result = m_hourString + ':' + m_minuteString;
        } else {
            result = m_hourString + ':' + m_minuteString + ':' + m_secondString;
        }
    } else { // can be invalid, that's OK
        result = m_hourString + ':' + m_minuteString + ':' + m_secondString + '.' + m_msecString;
    }
    switch (m_period) {
    case KDbTime::Period::Am:
        result += " AM";
        break;
    case KDbTime::Period::Pm:
        result += " PM";
        break;
    default:
        break;
    }
    return result;
}

KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbTime &time)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace().noquote() << "KDbTime(" << time.toString();
    if (!time.isValid()) {
        dbg.nospace() << " INVALID";
    }
    dbg.nospace() << ")";
    return dbg.maybeSpace();
}

bool KDbDateTime::operator==(const KDbDateTime &other) const
{
    return date() == other.date() && time() == other.time();
}

bool KDbDateTime::operator<(const KDbDateTime &other) const
{
    return toQDateTime() < other.toQDateTime();
}

bool KDbDateTime::isValid() const
{
    return m_date.isValid() && m_time.isValid();
}

bool KDbDateTime::isNull() const
{
    return m_date.isNull() || m_time.isNull();
}

QDateTime KDbDateTime::toQDateTime() const
{
    return { m_date.toQDate(), m_time.toQTime() };
}

QByteArray KDbDateTime::toString() const
{
    QByteArray result;
    if (isNull()) {
        result = QByteArrayLiteral("<NULL_DATETIME>");
    } else {
        result = m_date.toString() + ' ' + m_time.toString(); // can be invalid, that's OK
    }
    return result;
}

KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbDateTime &dateTime)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace().noquote() << "KDbDateTime(" << dateTime.toString();
    if (!dateTime.isValid()) {
        dbg.nospace() << "INVALID";
    }
    dbg.nospace() << ")";
    return dbg.maybeSpace();
}
