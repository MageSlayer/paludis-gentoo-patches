/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2013, 2014 Ciaran McCreesh
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
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_dep_spec_collection.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>

#include <memory>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<FindRepositoryForHelper>
    {
        const Environment * const env;
        std::unique_ptr<const FSPath> chroot_path;
        std::string cross_compile_host;

        Imp(const Environment * const e) :
            env(e)
        {
        }
    };
}

FindRepositoryForHelper::FindRepositoryForHelper(const Environment * const e) :
    _imp(e)
{
}

FindRepositoryForHelper::~FindRepositoryForHelper() = default;

namespace
{
    bool is_fake(const std::shared_ptr<const Repository> & repo)
    {
        return repo->format_key() && repo->format_key()->parse_value() == "installed_fake";
    }
}

const std::shared_ptr<const Repository>
FindRepositoryForHelper::operator() (
        const std::shared_ptr<const Resolution> & resolution,
        const ChangesToMakeDecision & decision) const
{
    std::shared_ptr<const Repository> result;

    for (const auto & repository : _imp->env->repositories())
    {
        switch (resolution->resolvent().destination_type())
        {
            case dt_install_to_slash:
                if (repository->cross_compile_host_key() || (!repository->installed_root_key()) ||
                    (repository->installed_root_key()->parse_value() != _imp->env->system_root_key()->parse_value()))
                    continue;
                break;

            case dt_install_to_chroot:
                if (repository->cross_compile_host_key())
                    continue;

                if (_imp->chroot_path) {
                    if ((!repository->installed_root_key()) || (repository->installed_root_key()->parse_value() != *_imp->chroot_path))
                        continue;
                }
                else {
                    if ((!repository->installed_root_key()) || (repository->installed_root_key()->parse_value() == _imp->env->system_root_key()->parse_value()))
                        continue;
                }
                break;

            case dt_create_binary:
                if (repository->installed_root_key())
                    continue;
                break;

            case dt_cross_compile:
                if (! repository->cross_compile_host_key())
                    continue;

                if (! _imp->cross_compile_host.empty())
                    if (repository->cross_compile_host_key()->parse_value() != _imp->cross_compile_host)
                        continue;

                break;

            case last_dt:
                break;
        }

        if (repository->destination_interface() && repository->destination_interface()->is_suitable_destination_for(decision.origin_id()))
        {
            if (result)
            {
                if (is_fake(repository) && ! is_fake(result))
                    ;
                else if (is_fake(result) && ! is_fake(repository))
                    result = repository;
                else
                    throw ConfigurationError("For '" + stringify(*decision.origin_id()) + "' with destination type " + stringify(resolution->resolvent().destination_type()) +
                                             ", don't know whether to install to ::" + stringify(result->name()) + " or ::" + stringify(repository->name()));
            }
            else
            {
                result = repository;
            }
        }
    }

    if (! result)
        throw ConfigurationError("No repository suitable for '" + stringify(*decision.origin_id())
                + "' with destination type " + stringify(resolution->resolvent().destination_type()) + " has been configured");
    return result;
}

void
FindRepositoryForHelper::set_chroot_path(const FSPath & p)
{
    _imp->chroot_path = std::make_unique<FSPath>(p);
}

void
FindRepositoryForHelper::set_cross_compile_host(const std::string & target)
{
    _imp->cross_compile_host = target;
}

namespace paludis
{
    template class Pimp<FindRepositoryForHelper>;
}

