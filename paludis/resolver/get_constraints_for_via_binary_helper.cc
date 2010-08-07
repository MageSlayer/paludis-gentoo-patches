/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/resolver/get_constraints_for_via_binary_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/stringify.hh>
#include <paludis/dep_spec.hh>
#include <paludis/partially_made_package_dep_spec.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<GetConstraintsForViaBinaryHelper>
    {
        const Environment * const env;

        Imp(const Environment * const e) :
            env(e)
        {
        }
    };
}

GetConstraintsForViaBinaryHelper::GetConstraintsForViaBinaryHelper(const Environment * const e) :
    Pimp<GetConstraintsForViaBinaryHelper>(e)
{
}

GetConstraintsForViaBinaryHelper::~GetConstraintsForViaBinaryHelper() = default;

const std::shared_ptr<ConstraintSequence>
GetConstraintsForViaBinaryHelper::operator() (
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const Resolution> & other_resolution) const
{
    auto result(std::make_shared<ConstraintSequence>());

    PartiallyMadePackageDepSpec partial_spec({ });
    partial_spec.package(resolution->resolvent().package());
    PackageDepSpec spec(partial_spec);

    auto reason(std::make_shared<ViaBinaryReason>(other_resolution->resolvent()));

    result->push_back(std::make_shared<Constraint>(make_named_values<Constraint>(
                    n::destination_type() = resolution->resolvent().destination_type(),
                    n::nothing_is_fine_too() = false,
                    n::reason() = reason,
                    n::spec() = spec,
                    n::untaken() = true,
                    n::use_existing() = ue_never
                    )));

    return result;
}

template class Pimp<GetConstraintsForViaBinaryHelper>;

