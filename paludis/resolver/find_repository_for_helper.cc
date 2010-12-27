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

#include <paludis/resolver/find_repository_for_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_dep_spec_collection.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<FindRepositoryForHelper>
    {
        const Environment * const env;

        Imp(const Environment * const e) :
            env(e)
        {
        }
    };
}

FindRepositoryForHelper::FindRepositoryForHelper(const Environment * const e) :
    Pimp<FindRepositoryForHelper>(e)
{
}

FindRepositoryForHelper::~FindRepositoryForHelper() = default;

namespace
{
    bool is_fake(const std::shared_ptr<const Repository> & repo)
    {
        return repo->format_key() && repo->format_key()->value() == "installed_fake";
    }
}

const std::shared_ptr<const Repository>
FindRepositoryForHelper::operator() (
        const std::shared_ptr<const Resolution> & resolution,
        const ChangesToMakeDecision & decision) const
{
    std::shared_ptr<const Repository> result;

    for (auto r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        switch (resolution->resolvent().destination_type())
        {
            case dt_install_to_slash:
                if ((! (*r)->installed_root_key()) || ((*r)->installed_root_key()->value() != _imp->env->system_root_key()->value()))
                    continue;
                break;

            case dt_install_to_chroot:
                if ((! (*r)->installed_root_key()) || ((*r)->installed_root_key()->value() == _imp->env->system_root_key()->value()))
                    continue;
                break;

            case dt_create_binary:
                if ((*r)->installed_root_key())
                    continue;
                break;

            case last_dt:
                break;
        }

        if ((*r)->destination_interface() &&
                (*r)->destination_interface()->is_suitable_destination_for(decision.origin_id()))
        {
            if (result)
            {
                if (is_fake(*r) && ! is_fake(result))
                {
                }
                else if (is_fake(result) && ! is_fake(*r))
                {
                    result = *r;
                }
                else
                    throw ConfigurationError("For '" + stringify(*decision.origin_id())
                            + "' with destination type " + stringify(resolution->resolvent().destination_type())
                            + ", don't know whether to install to ::" + stringify(result->name())
                            + " or ::" + stringify((*r)->name()));
            }
            else
                result = *r;
        }
    }

    if (! result)
        throw ConfigurationError("No repository suitable for '" + stringify(*decision.origin_id())
                + "' with destination type " + stringify(resolution->resolvent().destination_type()) + " has been configured");
    return result;
}

template class Pimp<FindRepositoryForHelper>;

