/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

using namespace paludis;
using namespace paludis::erepository;

EChoiceValue::EChoiceValue(const ChoicePrefixName & r, const UnprefixedChoiceName & v, const ChoiceNameWithPrefix & np, const QualifiedPackageName & p,
        const std::shared_ptr<const UseDesc> & d,
        bool b, bool def, bool l, bool x, const std::string & o,
        const std::string & pv) :
    _prefix(r),
    _unprefixed_name(v),
    _name_with_prefix(np),
    _package_name(p),
    _use_desc(d),
    _enabled(b),
    _enabled_by_default(def),
    _locked(l),
    _explicitly_listed(x),
    _override_description(o),
    _parameter(pv)
{
}

const UnprefixedChoiceName
EChoiceValue::unprefixed_name() const
{
    return _unprefixed_name;
}

const ChoiceNameWithPrefix
EChoiceValue::name_with_prefix() const
{
    return _name_with_prefix;
}

const std::string
EChoiceValue::description() const
{
    if (! _override_description.empty())
        return _override_description;
    if (! _use_desc)
        return "";
    return _use_desc->describe(_package_name, _prefix, _unprefixed_name);
}

bool
EChoiceValue::enabled() const
{
    return _enabled;
}

bool
EChoiceValue::enabled_by_default() const
{
    return _enabled_by_default;
}

bool
EChoiceValue::locked() const
{
    return _locked;
}

bool
EChoiceValue::explicitly_listed() const
{
    return _explicitly_listed;
}

const std::string
EChoiceValue::parameter() const
{
    return _parameter;
}

