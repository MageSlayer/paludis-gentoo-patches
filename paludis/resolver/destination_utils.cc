/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2014 Ciaran McCreesh
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

#include <paludis/resolver/destination_utils.hh>

#include <paludis/util/stringify.hh>

#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/generator_handler.hh>
#include <paludis/filter.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>

using namespace paludis;
using namespace paludis::resolver;

bool
paludis::resolver::can_make_binary_for(const std::shared_ptr<const PackageID> & id)
{
    if (! id->behaviours_key())
        return true;
    auto v(id->behaviours_key()->parse_value());
    return v->end() == v->find("unbinaryable");
}

bool
paludis::resolver::is_already_binary(const std::shared_ptr<const PackageID> & id)
{
    if (! id->behaviours_key())
        return false;
    auto v(id->behaviours_key()->parse_value());
    return v->end() != v->find("binary");
}

bool
paludis::resolver::can_chroot(const std::shared_ptr<const PackageID> & id)
{
    if (! id->behaviours_key())
        return true;
    auto v(id->behaviours_key()->parse_value());
    return v->end() == v->find("unchrootable");
}

bool
paludis::resolver::can_cross_compile(const std::shared_ptr<const PackageID> & id)
{
    if (! id->behaviours_key())
        return true;

    auto v(id->behaviours_key()->parse_value());
    return v->end() == v->find("uncrossable");
}

namespace
{
    struct BinaryDestinationGeneratorHandler :
        AllGeneratorHandlerBase
    {
        std::shared_ptr<const RepositoryNameSet>
        repositories(const Environment * const env,
                     const RepositoryContentMayExcludes &) const override
        {
            auto result(std::make_shared<RepositoryNameSet>());

            for (const auto & repository : env->repositories())
                if (! repository->installed_root_key())
                    if (repository->destination_interface())
                        result->insert(repository->name());

            return result;
        }

        std::string as_string() const override
        {
            return "binary destination repositories";
        }
    };

    struct BinaryDestinationGenerator :
        Generator
    {
        BinaryDestinationGenerator() :
            Generator(std::make_shared<BinaryDestinationGeneratorHandler>())
        {
        }
    };

    struct CrossCompiledGeneratorHandler :
        AllGeneratorHandlerBase
    {
        std::shared_ptr<const RepositoryNameSet>
        repositories(const Environment * const env,
                     const RepositoryContentMayExcludes &) const override
        {
            auto result(std::make_shared<RepositoryNameSet>());

            for (const auto & repository : env->repositories())
                if (repository->cross_compile_host_key())
                    result->insert(repository->name());

            return result;
        }

        std::string as_string() const override
        {
            return "cross-compiled destination repositories";
        }
    };

    struct CrossCompiledDestinationGenerator :
        Generator
    {
        CrossCompiledDestinationGenerator()
            : Generator(std::make_shared<CrossCompiledGeneratorHandler>())
        {
        }
    };
}

FilteredGenerator
paludis::resolver::destination_filtered_generator(const Environment * const env,
                                                  const DestinationType t,
                                                  const Generator & g)
{
    switch (t)
    {
        case dt_install_to_slash:
            return g | filter::InstalledAtRoot(env->system_root_key()->parse_value());

        case dt_install_to_chroot:
            return g | filter::InstalledNotAtRoot(env->system_root_key()->parse_value());

        case dt_create_binary:
            return g & BinaryDestinationGenerator();

        case dt_cross_compile:
            return g & CrossCompiledDestinationGenerator();

        case last_dt:
            break;
    }

    throw InternalError(PALUDIS_HERE, "unhandled dt");
}

Filter
paludis::resolver::make_destination_type_filter(const DestinationType t)
{
    switch (t)
    {
        case dt_install_to_slash:
            return filter::All();

        case dt_install_to_chroot:
            return filter::All();

        case dt_create_binary:
            return filter::ByFunction([] (const std::shared_ptr<const PackageID> & id) {
                    return ! can_make_binary_for(id);
                    },
                    "can be made into a binary");

        case dt_cross_compile:
            return filter::All();

        case last_dt:
            break;
    }

    throw InternalError(PALUDIS_HERE, "unhandled dt");
}

