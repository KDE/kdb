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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KDB_DATETIME_H
#define KDB_DATETIME_H

#include "kdb_export.h"

#include <QDateTime>
#include <QDebug>

/**
 * Generic year constant based on extended ISO 8601 specification
 *
 * This class as well as KDbDate, KDbTime and KDbDateTime is used as a replacement for Qt date/time
 * value classes in case when invalid values have to be preserved. Without this, SQL parsers would
 * not function properly because they would loose information about mistyped constants.
 * The KDb* make it possible for the user to store the constants in SQL expressions and fix them
 * at later time.
 *
 * See this page for specifications of the date/time formats supported:
 * https://community.kde.org/Kexi/Plugins/Queries/SQL_Constants#Date_constants
 *
 * @since 3.2.0
 */
class KDB_EXPORT KDbYear
{
public:
    /**
     * Specifies sign which is used to annotate year
     */
    enum class Sign {
        None,
        Plus,
        Minus
    };

    /**
     * Constructs year based on given sign and string
     *
     * Resulting year can be invalid but the string is always preserved.
     */
    KDbYear(Sign sign, const QByteArray &string) : m_sign(sign), m_string(string)
    {
    }

    /**
     * Constructs yea based on given string, with sign set to Sign::None
     *
     * Resulting year can be invalid but the string is always preserved.
     */
    explicit KDbYear(const QByteArray &string) : KDbYear(Sign::None, string)
    {
    }

    /**
     * Contructs a null year
     */
    KDbYear() : KDbYear(QByteArray())
    {
    }

    bool operator==(const KDbYear &other) const;

    inline bool operator!=(const KDbYear &other) const { return !operator==(other); }

    bool operator<(const KDbYear &other) const;

    inline bool operator<=(const KDbYear &other) const { return operator<(other) || operator==(other); }

    inline bool operator>=(const KDbYear &other) const { return !operator<(other); }

    inline bool operator>(const KDbYear &other) const { return !operator<=(other); }

    /**
     * Returns @c true if the year is valid
     *
     * Year is invalid if it is null or if the supplied string is invalid according to specification.
     */
    bool isValid() const;

    /**
     * Returns @c true if the year is null
     *
     * A year is null if its sign is equal to Sign::None and string is empty.
     */
    bool isNull() const;

    /**
     * Returns the sign which is used to annotate year
     */
    Sign sign() const { return m_sign; }

    QByteArray signString() const;

    /**
     * Returns the string representation of year value even if it is invalid
     */
    QByteArray yearString() const { return m_string; }

    /**
     * Returns entire year value (with sign) converted to string even if it is invalid
     */
    QByteArray toString() const;

    /**
     * Returns the integer year value as defined by extended ISO 8601
     *
     * 0 is returned for invalid year.
     */
    int toIsoValue() const;

    /**
     * Returns the integer year value as defined by the QDate API
     *
     * 0 is returned for invalid year.
     */
    int toQDateValue() const;

private:
    std::tuple<int, bool> intValue() const;

    const Sign m_sign;
    const QByteArray m_string;
    int m_isoValue = -1; //!< Cached value for intValue(), -1 if the cache is missing
};

Q_DECLARE_METATYPE(KDbYear)

//! Sends information about value @a sign to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, KDbYear::Sign sign);

//! Sends information about value @a year to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbYear &year);

/**
 * Generic date constant
 *
 * @since 3.2.0
 */
class KDB_EXPORT KDbDate
{
public:
    KDbDate(const KDbYear &year, const QByteArray &monthString, const QByteArray &dayString)
        : m_year(year), m_monthString(monthString), m_dayString(dayString)
    {
    }

    /**
     * Constructs a null date
     */
    KDbDate() = default;

    bool operator==(const KDbDate &other) const;

    inline bool operator!=(const KDbDate &other) const { return !operator==(other); }

    bool operator<(const KDbDate &other) const;

    inline bool operator<=(const KDbDate &other) const { return operator<(other) || operator==(other); }

    inline bool operator>=(const KDbDate &other) const { return !operator<(other); }

    inline bool operator>(const KDbDate &other) const { return !operator<=(other); }

    /**
     * Returns @c true if the date is valid
     *
     * Validation is performed by converting to QDate (toQDate()) and checking if it is valid.
     */
    bool isValid() const;

    /**
     * Returns @c true if the date is null
     *
     * A date is null if its year is null, month string is empty and date string is empty.
     */
    bool isNull() const;

    /**
     * Returns the date converted to QDate value
     *
     * Invalid QDate is returned if the KDbDate is invalid.
     */
    QDate toQDate() const;

    /**
     * Returns the date value converted to string even if it is invalid
     *
     * For null dates empty string is returned.
     */
    QByteArray toString() const;

    /**
     * Returns the year part of the date
     */
    KDbYear year() const { return m_year; }

    /**
     * Returns the month part of the date converted to integer
     *
     * Correct values are in range 1..12, -1 is returned for invalid month.
     */
    int month() const;

    /**
     * Returns the month part of the date
     *
     * It is original string passed to the KDbDate object and may be invalid.
     */
    QByteArray monthString() const { return m_monthString; }

    /**
     * Returns the day part of the date converted to integer
     *
     * Correct values are in range 1..31, -1 is returned for invalid day.
     * THe date can be still invalid for valid integer, e.g. February 31st.
     */
    int day() const;

    /**
     * Returns the day part of the date
     *
     * It is original string passed to the KDbDate object and may be invalid.
     */
    QByteArray dayString() const { return m_dayString; }

private:
    const KDbYear m_year;
    const QByteArray m_monthString;
    const QByteArray m_dayString;
};

Q_DECLARE_METATYPE(KDbDate)

//! Sends information about value @a date to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbDate &date);

/**
 * Generic time constant
 *
 * @since 3.2.0
 */
class KDB_EXPORT KDbTime
{
public:
    /**
     * Specifies hour period
     */
    enum class Period {
        None, //!< 2-hour time
        Am,   //!< AM, before noon
        Pm    //!< PM, after noon, before midnight
    };

    KDbTime(const QByteArray &hourString, const QByteArray &minuteString,
            const QByteArray &secondString = QByteArray(),
            const QByteArray &msecString = QByteArray(), Period period = Period::None)
        : m_hourString(hourString)
        , m_minuteString(minuteString)
        , m_secondString(secondString)
        , m_msecString(msecString)
        , m_period(period)
    {
    }

    /**
     * Constructs a null time
     */
    KDbTime() = default;

    bool operator==(const KDbTime &other) const;

    inline bool operator!=(const KDbTime &other) const { return !operator==(other); }

    bool operator<(const KDbTime &other) const;

    inline bool operator<=(const KDbTime &other) const { return operator<(other) || operator==(other); }

    inline bool operator>=(const KDbTime &other) const { return !operator<(other); }

    inline bool operator>(const KDbTime &other) const { return !operator<=(other); }

    /**
     * Returns @c true if the time is valid
     *
     * Validation is performed by converting to QTime (toQTime()) and checking if it is valid.
     */
    bool isValid() const;

    /**
     * Returns @c true if the time is null
     *
     * A time is null if its hour string or minute string is empty.
     */
    bool isNull() const;

    /**
     * Returns the time value converted to QTime type
     *
     * Invalid QTime is returned if the KDbTime is invalid.
     */
    QTime toQTime() const;

    /**
     * Returns the time value converted to string even if it is invalid
     *
     * For null times empty string is returned.
     */
    QByteArray toString() const;

    /**
     * Returns the hour part of the time converted to integer
     *
     * Correct values are in range 0..23 for None period, and 1..12 for Am and Pm periods.
     * -1 is returned for invalid hour.
     */
    int hour() const;

    /**
     * Returns the hour part of the date
     *
     * It is original string passed to the KDbTime object and may be invalid.
     */
    QByteArray hourString() const { return m_hourString; }

    /**
     * Returns the minute part of the time converted to integer
     *
     * Correct values are in range 0..59. -1 is returned for invalid minute.
     */
    int minute() const;

    /**
     * Returns the minute part of the date
     *
     * It is original string passed to the KDbTime object and may be invalid.
     */
    QByteArray minuteString() const { return m_minuteString; }

    /**
     * Returns the second part of the time converted to integer
     *
     * Correct values are in range 0..59. -1 is returned for invalid second.
     */
    int second() const;

    /**
     * Returns the second part of the date
     *
     * It is original string passed to the KDbTime object and may be invalid.
     */
    QByteArray secondString() const { return m_secondString; }

    int msec() const;

    /**
     * Returns the milliseconds part of the date
     *
     * It is original string passed to the KDbTime object and may be invalid.
     */
    QByteArray msecString() const { return m_msecString; }

    /**
     * Specifies hour period
     */
    Period period() const { return m_period; }

private:
    const QByteArray m_hourString;
    const QByteArray m_minuteString;
    const QByteArray m_secondString;
    const QByteArray m_msecString;
    const Period m_period = Period::None;
};

Q_DECLARE_METATYPE(KDbTime)

//! Sends information about value @a time to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbTime &time);

/**
 * Generic date/time constant
 *
 * @since 3.2.0
 */
class KDB_EXPORT KDbDateTime
{
public:
    KDbDateTime(const KDbDate &date, const KDbTime &time) : m_date(date), m_time(time)
    {
    }

    /**
     * Constructs a null date/time
     */
    KDbDateTime() : KDbDateTime(KDbDate(), KDbTime())
    {
    }

    bool operator==(const KDbDateTime &other) const;

    inline bool operator!=(const KDbDateTime &other) const { return !operator==(other); }

    bool operator<(const KDbDateTime &other) const;

    inline bool operator<=(const KDbDateTime &other) const { return operator<(other) || operator==(other); }

    inline bool operator>=(const KDbDateTime &other) const { return !operator<(other); }

    inline bool operator>(const KDbDateTime &other) const { return !operator<=(other); }

    /**
     * Returns @c true if the date/time is valid
     *
     * Validation is performed by converting to QDateTime (toQDateTime()) and checking if it is valid.
     */
    bool isValid() const;

    /**
     * Returns @c true if the date/time is null
     *
     * A time is null if its date or time is null.
     */
    bool isNull() const;

    /**
     * Returns the date/time converted to QDateTime value
     *
     * Invalid QDateTime is returned if the KDbDateTime is invalid.
     */
    QDateTime toQDateTime() const;

    /**
     * Returns the date/time value converted to string even if it is invalid
     *
     * For null date/times empty string is returned.
     */
    QByteArray toString() const;

    /**
     * Returns the date part of the date/time
     */
    KDbDate date() const { return m_date; }

    /**
     * Returns the time part of the date/time
     */
    KDbTime time() const { return m_time; }

private:
    const KDbDate m_date;
    const KDbTime m_time;
};

Q_DECLARE_METATYPE(KDbDateTime)

//! Sends information about value @a dateTime to debug output @a dbg.
KDB_EXPORT QDebug operator<<(QDebug dbg, const KDbDateTime &dateTime);

#endif
