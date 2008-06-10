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
            std::tr1::shared_ptr<const Map<std::string, std::string> > m)
    {
        std::string repo_file(m->end() == m->find("repo_file") ? std::string("?") :
                m->find("repo_file")->second);

        Context context("When making unavailable repository from repo_file '" + repo_file + "':");

        std::string name_str;
        RepositoryName name(m->end() == m->find("name") || (name_str = m->find("name")->second).empty()
                            ? "unavailable" : name_str);

        std::string location;
        if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
            throw UnavailableRepositoryConfigurationError("Key 'location' not specified or empty");

        std::string sync;
        if (m->end() != m->find("sync"))
            sync = m->find("sync")->second;

        std::string sync_options;
        if (m->end() != m->find("sync_options"))
            sync_options = m->find("sync_options")->second;

        return std::tr1::shared_ptr<UnavailableRepository>(new UnavailableRepository(
                    UnavailableRepositoryParams::named_create()
                    (k::name(), name)
                    (k::location(), location)
                    (k::sync(), sync)
                    (k::sync_options(), sync_options)
                    (k::environment(), env)
                    ));
    }
}

void register_repositories(RepositoryMaker * maker)
{
    maker->register_maker("unavailable", &make_unavailable_repository);
}

