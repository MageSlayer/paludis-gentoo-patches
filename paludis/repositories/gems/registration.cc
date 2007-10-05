/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/repositories/gems/gems_repository.hh>
#include <paludis/repositories/gems/installed_gems_repository.hh>
#include <paludis/repositories/gems/params.hh>
#include <paludis/repositories/gems/exceptions.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/map.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

using namespace paludis;

namespace
{
    tr1::shared_ptr<Repository>
    make_gems_repository(
            Environment * const env,
            tr1::shared_ptr<const Map<std::string, std::string> > m)
    {
        std::string location;
        if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
            throw gems::RepositoryConfigurationError("Key 'location' not specified or empty");

        std::string install_dir;
        if (m->end() == m->find("install_dir") || ((install_dir = m->find("install_dir")->second)).empty())
            throw gems::RepositoryConfigurationError("Key 'install_dir' not specified or empty");

        std::string sync;
        if (m->end() != m->find("sync"))
            sync = m->find("sync")->second;

        std::string sync_options;
        if (m->end() != m->find("sync_options"))
            sync_options = m->find("sync_options")->second;

        std::string builddir;
        if (m->end() == m->find("builddir") || ((builddir = m->find("builddir")->second)).empty())
            builddir = DistributionData::get_instance()->distribution_from_string(env->default_distribution())->default_ebuild_builddir;

        return make_shared_ptr(new GemsRepository(gems::RepositoryParams::create()
                    .location(location)
                    .sync(sync)
                    .sync_options(sync_options)
                    .environment(env)
                    .install_dir(install_dir)
                    .builddir(builddir)));
    }

    tr1::shared_ptr<Repository>
    make_installed_gems_repository(
            Environment * const env,
            tr1::shared_ptr<const Map<std::string, std::string> > m)
    {
        std::string install_dir;
        if (m->end() == m->find("install_dir") || ((install_dir = m->find("install_dir")->second)).empty())
            throw gems::RepositoryConfigurationError("Key 'install_dir' not specified or empty");

        std::string builddir;
        if (m->end() == m->find("builddir") || ((builddir = m->find("builddir")->second)).empty())
            builddir = DistributionData::get_instance()->distribution_from_string(env->default_distribution())->default_ebuild_builddir;

        return make_shared_ptr(new InstalledGemsRepository(gems::InstalledRepositoryParams::create()
                    .environment(env)
                    .install_dir(install_dir)
                    .builddir(builddir)));
    }
}

extern "C"
{
    void PALUDIS_VISIBLE register_repositories(RepositoryMaker * maker);
}

void register_repositories(RepositoryMaker * maker)
{
    maker->register_maker("gems", &make_gems_repository);
    maker->register_maker("installed_gems", &make_installed_gems_repository);
}

