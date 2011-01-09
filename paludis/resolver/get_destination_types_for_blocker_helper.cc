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

#include <paludis/resolver/get_destination_types_for_blocker_helper.hh>
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
    struct Imp<GetDestinationTypesForBlockerHelper>
    {
        const Environment * const env;
        DestinationType target_destination_type;

        Imp(const Environment * const e) :
            env(e),
            target_destination_type(dt_install_to_slash)
        {
        }
    };
}

GetDestinationTypesForBlockerHelper::GetDestinationTypesForBlockerHelper(const Environment * const e) :
    _imp(e)
{
}

GetDestinationTypesForBlockerHelper::~GetDestinationTypesForBlockerHelper() = default;

void
GetDestinationTypesForBlockerHelper::set_target_destination_type(const DestinationType d)
{
    _imp->target_destination_type = d;
}

namespace
{
    struct DestinationTypesFinder
    {
        const DestinationType target_destination_type;

        DestinationTypes visit(const TargetReason &) const
        {
            return { target_destination_type };
        }

        DestinationTypes visit(const DependentReason &) const
        {
            return { dt_install_to_slash };
        }

        DestinationTypes visit(const ViaBinaryReason &) const
        {
            return { };
        }

        DestinationTypes visit(const WasUsedByReason &) const
        {
            return { dt_install_to_slash };
        }

        DestinationTypes visit(const DependencyReason &) const
        {
            return { dt_install_to_slash };
        }

        DestinationTypes visit(const PresetReason &) const
        {
            return { };
        }

        DestinationTypes visit(const LikeOtherDestinationTypeReason & r) const
        {
            return r.reason_for_other()->accept_returning<DestinationTypes>(*this);
        }

        DestinationTypes visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<DestinationTypes>(*this);
        }
    };
}

DestinationTypes
GetDestinationTypesForBlockerHelper::operator() (
        const BlockDepSpec &,
        const std::shared_ptr<const Reason> & reason) const
{
    return reason->accept_returning<DestinationTypes>(DestinationTypesFinder{_imp->target_destination_type});
}

template class Pimp<GetDestinationTypesForBlockerHelper>;

