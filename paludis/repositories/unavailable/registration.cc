/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/repository_maker.hh>
#include <paludis/repositories/unavailable/unavailable_repository.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>

using namespace paludis;
using namespace paludis::unavailable_repository;

extern "C"
{
    void PALUDIS_VISIBLE register_repositories(RepositoryMaker * maker);
}

namespace
{
    std::tr1::shared_ptr<Repository>
    make_unavailable_repository(
            Environment * const env,
            const std::tr1::function<std::string (const std::string &)> & f)
    {
        Context context("When making unavailable repository from repo_file '" + f("repo_file") + "':");

        std::string name_str(f("name"));
        if (name_str.empty())
            name_str = "unavailable";

        std::string location(f("location"));
        if (location.empty())
            throw UnavailableRepositoryConfigurationError("Key 'location' not specified or empty");

        std::string sync(f("sync"));

        std::string sync_options(f("sync_options"));

        return std::tr1::shared_ptr<UnavailableRepository>(new UnavailableRepository(
                    make_named_values<UnavailableRepositoryParams>(
                        value_for<n::environment>(env),
                        value_for<n::location>(location),
                        value_for<n::name>(RepositoryName(name_str)),
                        value_for<n::sync>(sync),
                        value_for<n::sync_options>(sync_options)
                    )));
    }
}

void register_repositories(RepositoryMaker * maker)
{
    maker->register_maker("unavailable", &make_unavailable_repository);
}

