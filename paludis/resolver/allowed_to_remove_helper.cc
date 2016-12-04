/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/resolver/allowed_to_remove_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_dep_spec_collection.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<AllowedToRemoveHelper>
    {
        const Environment * const env;
        PackageDepSpecCollection allowed_to_remove_specs;

        Imp(const Environment * const e) :
            env(e),
            allowed_to_remove_specs(nullptr)
        {
        }
    };
}

AllowedToRemoveHelper::AllowedToRemoveHelper(const Environment * const e) :
    _imp(e)
{
}

AllowedToRemoveHelper::~AllowedToRemoveHelper() = default;

void
AllowedToRemoveHelper::add_allowed_to_remove_spec(const PackageDepSpec & spec)
{
    _imp->allowed_to_remove_specs.insert(spec);
}

bool
AllowedToRemoveHelper::operator() (
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id) const
{
    auto v = make_visitor(
            [&] (const DependentReason &)  { return true; },
            [&] (const TargetReason &)     { return true; },
            [&] (const DependencyReason &) { return false; },
            [&] (const WasUsedByReason &)  { return true; },
            [&] (const ViaBinaryReason &)  { return false; },
            [&] (const PresetReason &)     { return false; },
            [&] (const SetReason & r, const Revisit<bool, Reason> & revisit) {
                return revisit(*r.reason_for_set());
            },
            [&] (const LikeOtherDestinationTypeReason & r, const Revisit<bool, Reason> & revisit) {
                return revisit(*r.reason_for_other());
            }
            );

    for (const auto & constraint : *resolution->constraints())
        if (constraint->reason()->accept_returning<bool>(v))
            return true;

    return _imp->allowed_to_remove_specs.match_any(_imp->env, id, { });
}

namespace paludis
{
    template class Pimp<AllowedToRemoveHelper>;
}

