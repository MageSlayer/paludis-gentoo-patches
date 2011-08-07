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

#include <paludis/resolver/order_early_helper.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/decision_utils.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_dep_spec_collection.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<OrderEarlyHelper>
    {
        const Environment * const env;
        PackageDepSpecCollection early_specs;
        PackageDepSpecCollection late_specs;

        Imp(const Environment * const e) :
            env(e),
            early_specs(make_null_shared_ptr()),
            late_specs(make_null_shared_ptr())
        {
        }
    };
}

OrderEarlyHelper::OrderEarlyHelper(const Environment * const e) :
    _imp(e)
{
}

OrderEarlyHelper::~OrderEarlyHelper() = default;

void
OrderEarlyHelper::add_early_spec(const PackageDepSpec & spec)
{
    _imp->early_specs.insert(spec);
}

void
OrderEarlyHelper::add_late_spec(const PackageDepSpec & spec)
{
    _imp->late_specs.insert(spec);
}

Tribool
OrderEarlyHelper::operator() (const std::shared_ptr<const Resolution> & resolution) const
{
    auto id(get_decided_id_or_null(resolution->decision()));

    if (id)
    {
        if (_imp->early_specs.match_any(_imp->env, id, { }))
            return true;
        if (_imp->late_specs.match_any(_imp->env, id, { }))
            return false;
    }

    return indeterminate;
}

namespace paludis
{
    template class Pimp<OrderEarlyHelper>;
}
