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

#include <paludis/resolver/remove_if_dependent_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_dep_spec_collection.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<RemoveIfDependentHelper>
    {
        const Environment * const env;
        PackageDepSpecCollection remove_if_dependent_specs;
        std::string cross_compile_host;

        Imp(const Environment * const e) : env(e), remove_if_dependent_specs(nullptr)
        {
        }
    };
}

RemoveIfDependentHelper::RemoveIfDependentHelper(const Environment * const e) :
    _imp(e)
{
}

RemoveIfDependentHelper::~RemoveIfDependentHelper() = default;

void
RemoveIfDependentHelper::add_remove_if_dependent_spec(const PackageDepSpec & spec)
{
    _imp->remove_if_dependent_specs.insert(spec);
}

void
RemoveIfDependentHelper::set_cross_compile_host(const std::string & host)
{
    _imp->cross_compile_host = host;
}

bool
RemoveIfDependentHelper::operator()(const std::shared_ptr<const PackageID> & id) const
{
    auto repository = _imp->env->fetch_repository(id->repository_name());
    auto cross_compile_host_key = repository->cross_compile_host_key();
    if (_imp->cross_compile_host.empty())
    {
        if (cross_compile_host_key)
            return false;
    }
    else
    {
        if (! cross_compile_host_key || cross_compile_host_key->parse_value() != _imp->cross_compile_host)
            return false;
    }

    return _imp->remove_if_dependent_specs.match_any(_imp->env, id, { });
}

namespace paludis
{
    template class Pimp<RemoveIfDependentHelper>;
}
