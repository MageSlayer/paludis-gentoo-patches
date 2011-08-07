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

#include <paludis/resolver/can_use_helper.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/package_dep_spec_collection.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<CanUseHelper>
    {
        const Environment * const env;
        PackageDepSpecCollection cannot_use_specs;

        Imp(const Environment * const e) :
            env(e),
            cannot_use_specs(make_null_shared_ptr())
        {
        }
    };
}

CanUseHelper::CanUseHelper(const Environment * const e) :
    _imp(e)
{
}

CanUseHelper::~CanUseHelper() = default;

void
CanUseHelper::add_cannot_use_spec(const PackageDepSpec & spec)
{
    _imp->cannot_use_specs.insert(spec);
}

bool
CanUseHelper::operator() (const std::shared_ptr<const PackageID> & id) const
{
    return ! _imp->cannot_use_specs.match_any(_imp->env, id, { });
}

namespace paludis
{
    template class Pimp<CanUseHelper>;
}
