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

#include <paludis/resolver/allowed_to_restart_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision_utils-fwd.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_dep_spec_collection.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<AllowedToRestartHelper>
    {
        const Environment * const env;
        PackageDepSpecCollection no_restarts_for_specs;

        Imp(const Environment * const e) :
            env(e),
            no_restarts_for_specs(nullptr)
        {
        }
    };
}

AllowedToRestartHelper::AllowedToRestartHelper(const Environment * const e) :
    _imp(e)
{
}

AllowedToRestartHelper::~AllowedToRestartHelper() = default;

void
AllowedToRestartHelper::add_no_restarts_for_spec(const PackageDepSpec & spec)
{
    _imp->no_restarts_for_specs.insert(spec);
}

bool
AllowedToRestartHelper::operator() (
        const std::shared_ptr<const Resolution> & resolution) const
{
    if (! resolution->decision())
        return true;

    auto id(get_decided_id_or_null(resolution->decision()));
    if (! id)
        return true;

    return ! _imp->no_restarts_for_specs.match_any(_imp->env, id, { });
}

namespace paludis
{
    template class Pimp<AllowedToRestartHelper>;
}
