/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/changed_choices.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/tribool.hh>
#include <paludis/choice.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/elike_use_requirement-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <map>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<ChangedChoices>
    {
        std::map<ChoiceNameWithPrefix, bool> overrides;
    };
}

ChangedChoices::ChangedChoices() :
    _imp()
{
}

ChangedChoices::~ChangedChoices() = default;

bool
ChangedChoices::add_override_if_possible(const ChoiceNameWithPrefix & c, const bool v)
{
    auto r(_imp->overrides.insert(std::make_pair(c, v)));
    return r.second || (r.first->second == v);
}

bool
ChangedChoices::empty() const
{
    return _imp->overrides.empty();
}

void
ChangedChoices::add_constraints_to(PartiallyMadePackageDepSpec & spec) const
{
    for (auto o(_imp->overrides.begin()), o_end(_imp->overrides.end()) ;
            o != o_end ; ++o)
    {
        if (o->second)
            spec.choice_constraint(parse_elike_use_requirement("" + stringify(o->first) + "(-)",
                        { euro_allow_default_values }));
        else
            spec.choice_constraint(parse_elike_use_requirement("-" + stringify(o->first) + "(-)",
                        { euro_allow_default_values }));
    }
}

Tribool
ChangedChoices::overridden_value(const ChoiceNameWithPrefix & c) const
{
    auto i(_imp->overrides.find(c));
    if (i == _imp->overrides.end())
        return Tribool(indeterminate);
    else
        return Tribool(i->second);
}

void
ChangedChoices::serialise(Serialiser & s) const
{
    auto ss(s.object("ChangedChoices"));

    ss.member(SerialiserFlags<>(), "count", stringify(_imp->overrides.size()));

    int n(0);
    for (auto o(_imp->overrides.begin()), o_end(_imp->overrides.end()) ;
            o != o_end ; ++o)
    {
        ++n;
        ss.member(SerialiserFlags<>(), stringify(n) + "a" , stringify(o->first));
        ss.member(SerialiserFlags<>(), stringify(n) + "b" , stringify(o->second));
    }
}

const std::shared_ptr<ChangedChoices>
ChangedChoices::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "ChangedChoices");
    auto result(std::make_shared<ChangedChoices>());

    for (int n(1), n_end(v.member<int>("count") + 1) ; n != n_end ; ++n)
        result->add_override_if_possible(
                ChoiceNameWithPrefix(v.member<std::string>(stringify(n) + "a")),
                v.member<bool>(stringify(n) + "b")
                );

    return result;
}

template class Pimp<ChangedChoices>;

