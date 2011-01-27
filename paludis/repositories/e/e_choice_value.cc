/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/e_choice_value.hh>
#include <paludis/repositories/e/use_desc.hh>
#include <paludis/util/make_null_shared_ptr.hh>

using namespace paludis;
using namespace paludis::erepository;

EChoiceValue::EChoiceValue(const EChoiceValueParams & p) :
    _params(p)
{
}

const UnprefixedChoiceName
EChoiceValue::unprefixed_name() const
{
    return _params.unprefixed_choice_name();
}

const ChoiceNameWithPrefix
EChoiceValue::name_with_prefix() const
{
    return _params.choice_name_with_prefix();
}

const std::string
EChoiceValue::description() const
{
    return _params.description();
}

bool
EChoiceValue::enabled() const
{
    return _params.enabled();
}

bool
EChoiceValue::enabled_by_default() const
{
    return _params.enabled_by_default();
}

bool
EChoiceValue::locked() const
{
    return _params.locked();
}

bool
EChoiceValue::explicitly_listed() const
{
    return _params.explicitly_listed();
}

const std::string
EChoiceValue::parameter() const
{
    return "";
}

const std::shared_ptr<const PermittedChoiceValueParameterValues>
EChoiceValue::permitted_parameter_values() const
{
    return make_null_shared_ptr();
}

