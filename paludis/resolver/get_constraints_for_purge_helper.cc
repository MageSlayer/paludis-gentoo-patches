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

#include <paludis/resolver/get_constraints_for_purge_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/make_uninstall_blocker.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/package_dep_spec_collection.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/elike_slot_requirement.hh>
#include <paludis/metadata_key.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<GetConstraintsForPurgeHelper>
    {
        const Environment * const env;
        PackageDepSpecCollection purge_specs;

        Imp(const Environment * const e) :
            env(e),
            purge_specs(make_null_shared_ptr())
        {
        }
    };
}

GetConstraintsForPurgeHelper::GetConstraintsForPurgeHelper(const Environment * const e) :
    _imp(e)
{
}

GetConstraintsForPurgeHelper::~GetConstraintsForPurgeHelper() = default;

void
GetConstraintsForPurgeHelper::add_purge_spec(const PackageDepSpec & spec)
{
    _imp->purge_specs.insert(spec);
}

const std::shared_ptr<ConstraintSequence>
GetConstraintsForPurgeHelper::operator() (
        const std::shared_ptr<const Resolution> &,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const ChangeByResolventSequence> & was_used_by_ids) const
{
    const std::shared_ptr<ConstraintSequence> result(std::make_shared<ConstraintSequence>());

    PartiallyMadePackageDepSpec partial_spec({ });
    partial_spec.package(id->name());
    if (id->slot_key())
        partial_spec.slot_requirement(std::make_shared<ELikeSlotExactRequirement>(id->slot_key()->parse_value(), false));
    PackageDepSpec spec(partial_spec);

    const std::shared_ptr<WasUsedByReason> reason(std::make_shared<WasUsedByReason>(was_used_by_ids));

    result->push_back(std::make_shared<Constraint>(make_named_values<Constraint>(
                    n::destination_type() = dt_install_to_slash,
                    n::force_unable() = false,
                    n::from_id() = id,
                    n::nothing_is_fine_too() = true,
                    n::reason() = reason,
                    n::spec() = make_uninstall_blocker(spec),
                    n::untaken() = ! _imp->purge_specs.match_any(_imp->env, id, { }),
                    n::use_existing() = ue_if_possible
                    )));

    return result;
}

template class Pimp<GetConstraintsForPurgeHelper>;

