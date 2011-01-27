/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <paludis/permitted_choice_value_parameter_values.hh>

using namespace paludis;

PermittedChoiceValueParameterValues::~PermittedChoiceValueParameterValues() = default;

PermittedChoiceValueParameterIntegerValue::PermittedChoiceValueParameterIntegerValue(const int min, const int max) :
    _min(min),
    _max(max)
{
}

int
PermittedChoiceValueParameterIntegerValue::minimum_allowed_value() const
{
    return _min;
}

int
PermittedChoiceValueParameterIntegerValue::maximum_allowed_value() const
{
    return _max;
}

PermittedChoiceValueParameterEnumValue::PermittedChoiceValueParameterEnumValue(
        const std::shared_ptr<const Map<std::string, std::string> > & a) :
    _allowed(a)
{
}

const std::shared_ptr<const Map<std::string, std::string> >
PermittedChoiceValueParameterEnumValue::allowed_values_and_descriptions() const
{
    return _allowed;
}

