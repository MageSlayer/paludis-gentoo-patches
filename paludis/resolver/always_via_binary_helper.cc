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

#include <paludis/resolver/always_via_binary_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_dep_spec_collection.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<AlwaysViaBinaryHelper>
    {
        const Environment * const env;
        PackageDepSpecCollection always_via_binary_specs;

        Imp(const Environment * const e) :
            env(e)
        {
        }
    };
}

AlwaysViaBinaryHelper::AlwaysViaBinaryHelper(const Environment * const e) :
    Pimp<AlwaysViaBinaryHelper>(e)
{
}

AlwaysViaBinaryHelper::~AlwaysViaBinaryHelper() = default;

void
AlwaysViaBinaryHelper::add_always_via_binary_spec(const PackageDepSpec & spec)
{
    _imp->always_via_binary_specs.insert(spec);
}

namespace
{
    bool can_make_binary_for(const std::shared_ptr<const PackageID> & id)
    {
        if (! id->behaviours_key())
            return true;
        return id->behaviours_key()->value()->end() == id->behaviours_key()->value()->find("unbinaryable");
    }
}

bool
AlwaysViaBinaryHelper::operator() (const std::shared_ptr<const Resolution> & resolution) const
{
    const ChangesToMakeDecision * changes_decision(simple_visitor_cast<const ChangesToMakeDecision>(*resolution->decision()));
    if (! changes_decision)
        return false;

    return can_make_binary_for(changes_decision->origin_id()) &&
        _imp->always_via_binary_specs.match_any(_imp->env, changes_decision->origin_id(), { });
}

template class Pimp<AlwaysViaBinaryHelper>;

