/* This file is part of the KDE project
   Copyright (C) 2006 Jarosław Staniek <staniek@kde.org>

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

#include "KDbFieldValidator.h"
#include "KDbField.h"
#include "KDbLongLongValidator.h"

#include <QIntValidator>
#include <QDoubleValidator>
#include <QWidget>

using namespace KDbUtils;

KDbFieldValidator::KDbFieldValidator(const KDbField &field, QWidget * parent)
        : KDbMultiValidator(parent)
{
//! @todo merge this code with KexiTableEdit code!
//! @todo set maximum length validator
//! @todo handle input mask (via QLineEdit::setInputMask()
    const KDbField::Type t = field.type(); // cache: evaluating type of expressions can be expensive
    if (KDbField::isIntegerType(t)) {
        QValidator *validator = nullptr;
        const bool u = field.isUnsigned();
        int bottom = 0, top = 0;
        if (t == KDbField::Byte) {
            bottom = u ? 0 : -0x80;
            top = u ? 0xff : 0x7f;
        } else if (t == KDbField::ShortInteger) {
            bottom = u ? 0 : -0x8000;
            top = u ? 0xffff : 0x7fff;
        } else if (t == KDbField::Integer) {
            bottom = u ? 0 : -0x7fffffff - 1;
            top = u ? 0xffffffff : 0x7fffffff;
            validator = new KDbLongLongValidator(bottom, top, nullptr);
        } else if (t == KDbField::BigInteger) {
//! @todo handle unsigned (using ULongLongValidator)
            validator = new KDbLongLongValidator(nullptr);
        }

        if (!validator)
            validator = new QIntValidator(bottom, top, nullptr); //the default
        addSubvalidator(validator);
    } else if (KDbField::isFPNumericType(t)) {
        QValidator *validator;
        if (t == KDbField::Float) {
            if (field.isUnsigned()) //ok?
                validator = new QDoubleValidator(0, 3.4e+38, field.scale(), nullptr);
            else
                validator = new QDoubleValidator((QObject*)nullptr);
        } else {//double
            if (field.isUnsigned()) //ok?
                validator = new QDoubleValidator(0, 1.7e+308, field.scale(), nullptr);
            else
                validator = new QDoubleValidator((QObject*)nullptr);
        }
        addSubvalidator(validator);
    } else if (t == KDbField::Date) {
//! @todo add validator
//  QValidator *validator = new KDateValidator(this);
//  setValidator( validator );
    } else if (t == KDbField::Time) {
//! @todo add validator
    } else if (t == KDbField::DateTime) {
    } else if (t == KDbField::Boolean) {
//! @todo add BooleanValidator
        addSubvalidator(new QIntValidator(0, 1, nullptr));
    }
}

KDbFieldValidator::~KDbFieldValidator()
{
}
