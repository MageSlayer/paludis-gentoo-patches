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

#include <paludis/resolver/get_constraints_for_dependent_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
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
    struct Imp<GetConstraintsForDependentHelper>
    {
        const Environment * const env;
        PackageDepSpecCollection less_restrictive_remove_blockers_specs;

        Imp(const Environment * const e) :
            env(e),
            less_restrictive_remove_blockers_specs(make_null_shared_ptr())
        {
        }
    };
}

GetConstraintsForDependentHelper::GetConstraintsForDependentHelper(const Environment * const e) :
    Pimp<GetConstraintsForDependentHelper>(e)
{
}

GetConstraintsForDependentHelper::~GetConstraintsForDependentHelper() = default;

void
GetConstraintsForDependentHelper::add_less_restrictive_remove_blockers_spec(const PackageDepSpec & spec)
{
    _imp->less_restrictive_remove_blockers_specs.insert(spec);
}

const std::shared_ptr<ConstraintSequence>
GetConstraintsForDependentHelper::operator() (
        const std::shared_ptr<const Resolution> &,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const ChangeByResolventSequence> & dependent_upon_ids) const
{
    auto result(std::make_shared<ConstraintSequence>());

    std::shared_ptr<PackageDepSpec> spec;
    if (_imp->less_restrictive_remove_blockers_specs.match_any(_imp->env, id, { }))
        spec = make_shared_copy(id->uniquely_identifying_spec());
    else
    {
        PartiallyMadePackageDepSpec partial_spec({ });
        partial_spec.package(id->name());
        if (id->slot_key())
            partial_spec.slot_requirement(std::make_shared<ELikeSlotExactRequirement>(id->slot_key()->value(), false));
        spec = std::make_shared<PackageDepSpec>(partial_spec);
    }

    for (auto i(dependent_upon_ids->begin()), i_end(dependent_upon_ids->end()) ;
            i != i_end ; ++i)
    {
        auto reason(std::make_shared<DependentReason>(*i));

        result->push_back(std::make_shared<Constraint>(make_named_values<Constraint>(
                        n::destination_type() = dt_install_to_slash,
                        n::force_unable() = false,
                        n::from_id() = id,
                        n::nothing_is_fine_too() = true,
                        n::reason() = reason,
                        n::spec() = BlockDepSpec("!" + stringify(*spec), *spec, bk_weak),
                        n::untaken() = false,
                        n::use_existing() = ue_if_possible
                        )));
    }

    return result;
}

template class Pimp<GetConstraintsForDependentHelper>;

