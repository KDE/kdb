/* This file is part of the KDE project
   Copyright (C) 2004, 2006 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDB_TOOLS_VALIDATOR_H
#define KDB_TOOLS_VALIDATOR_H

#include "kdb_export.h"

#include <QValidator>
#include <QVariant>
#include <QString>

//! @short A validator extending QValidator with offline-checking for value's validity
/*!
 The offline-checking for value's validity is provided by @ref KDbValidator::check() method.
 The validator groups two purposes into one container:
 - string validator for line editors (online checking, "on typing");
 - offline-checking for QVariant values, reimplementing validate().

 It also offers error and warning messages for check() method.
 You may need to reimplement:
 -  QValidator::State validate(QString& input, int& pos) const
 -  Result check(const QString &valueName, const QVariant &v, QString *message, QString *details)
 */
class KDB_EXPORT KDbValidator : public QValidator
{
    Q_OBJECT
public:
    enum Result { Error = 0, Ok = 1, Warning = 2 };

    explicit KDbValidator(QObject *parent = nullptr);

    ~KDbValidator() override;

    /*! Sets accepting empty values on (true) or off (false).
     By default the validator does not accepts empty values. */
    void setAcceptsEmptyValue(bool set);

    /*! @return true if the validator accepts empty values
      @see setAcceptsEmptyValue() */
    bool acceptsEmptyValue() const;

    /*! Checks if value @a value is ok and returns one of @a Result value:
     - @a Error is returned on error;
     - @a Ok on success;
     - @a Warning if there is something to warn about.
     In any case except @a Ok, i18n'ed message will be set in @a message
     and (optionally) datails are set in @a details, e.g. for use in a message box.
     @a valueName can be used to contruct @a message as well, for example:
     "[valueName] is not a valid login name".
     Depending on acceptsEmptyValue(), immediately accepts empty values or not. */
    Result check(const QString &valueName, const QVariant& v, QString *message,
                 QString *details);

    /*! This implementation always returns value QValidator::Acceptable. */
    QValidator::State validate(QString &input, int &pos) const override;

    //! A generic error/warning message "... value has to be entered."
    static const QString messageColumnNotEmpty();

    //! Adds a child validator @a v
    void addChildValidator(KDbValidator* v);

protected:
    /* Used by check(), for reimplementation, by default returns @a Error.*/
    virtual Result internalCheck(const QString &valueName, const QVariant& value,
                                 QString *message, QString *details);

    friend class KDbMultiValidator;

private:
    class Private;
    Private* const d;
    Q_DISABLE_COPY(KDbValidator)
};

//! @short A validator groupping multiple QValidators
/*! KDbMultiValidator behaves like normal KDbValidator,
 but it allows to add define more than one different validator.
 Given validation is successful if every subvalidator accepted given value.

 - acceptsEmptyValue() is used globally here
   (no matter what is defined in subvalidators).

 - result of calling check() depends on value of check() returned by subvalidators:
   - Error is returned if at least one subvalidator returned Error;
   - Warning is returned if at least one subvalidator returned Warning and
     no validator returned error;
   - Ok is returned only if exactly all subvalidators returned Ok.
   - If there is no subvalidators defined, Error is always returned.
   - If a given subvalidator is not of class KDbValidator but ust QValidator,
     it's assumed it's check() method returned Ok.

 - result of calling validate() (a method implemented for QValidator)
   depends on value of validate() returned by subvalidators:
   - Invalid is returned if at least one subvalidator returned Invalid
   - Intermediate is returned if at least one subvalidator returned Intermediate
   - Acceptable is returned if exactly all subvalidators returned Acceptable.
   - If there is no subvalidators defined, Invalid is always returned.

 If there are no subvalidators, the multi validator always accepts the input.
*/
class KDB_EXPORT KDbMultiValidator : public KDbValidator
{
    Q_OBJECT
public:
    /*! Constructs multivalidator with no subvalidators defined.
     You can add more validators with addSubvalidator(). */
    explicit KDbMultiValidator(QObject *parent = nullptr);

    /*! Constructs multivalidator with one validator @a validator.
     It will be owned if has no parent defined.
     You can add more validators with addSubvalidator(). */
    explicit KDbMultiValidator(QValidator *validator, QObject *parent = nullptr);

    ~KDbMultiValidator() override;

    /*! Adds validator @a validator as another subvalidator.
     Subvalidator will be owned by this multivalidator if @a owned is true
     and its parent is @c nullptr. */
    void addSubvalidator(QValidator* validator, bool owned = true);

    /*! Reimplemented to call validate() on subvalidators. */
    QValidator::State validate(QString &input, int &pos) const override;

    /*! Calls QValidator::fixup() on every subvalidator.
     This may be senseless to use this methog in certain cases
     (can return weir results), so think twice before.. */
    void fixup(QString &input) const override;

protected:
    KDbValidator::Result internalCheck(const QString &valueName, const QVariant &value,
                                       QString *message, QString *details) override;

private:
    class Private;
    Private* const d;
    Q_DISABLE_COPY(KDbMultiValidator)
};

#endif
