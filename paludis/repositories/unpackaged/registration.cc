/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/attributes.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/repositories/unpackaged/installed_repository.hh>
#include <paludis/repositories/unpackaged/unpackaged_repository.hh>
#include <paludis/repositories/unpackaged/exceptions.hh>

using namespace paludis;

namespace
{
    tr1::shared_ptr<Repository>
    make_unpackaged_repository(
            Environment * const env,
            tr1::shared_ptr<const Map<std::string, std::string> > m)
    {
        Context context("When creating UnpackagedRepository:");

        std::string location;
        if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
            throw unpackaged_repositories::RepositoryConfigurationError("Key 'location' not specified or empty");

        std::string install_under;
        if (m->end() == m->find("install_under") || ((install_under = m->find("install_under")->second)).empty())
            install_under = "/";

        std::string name;
        if (m->end() == m->find("name") || ((name = m->find("name")->second)).empty())
            throw unpackaged_repositories::RepositoryConfigurationError("Key 'name' not specified or empty");

        std::string version;
        if (m->end() == m->find("version") || ((version = m->find("version")->second)).empty())
            throw unpackaged_repositories::RepositoryConfigurationError("Key 'version' not specified or empty");

        std::string slot;
        if (m->end() == m->find("slot") || ((slot = m->find("slot")->second)).empty())
            throw unpackaged_repositories::RepositoryConfigurationError("Key 'slot' not specified or empty");

        std::string build_dependencies;
        if (m->end() != m->find("build_dependencies"))
            build_dependencies = m->find("build_dependencies")->second;

        std::string run_dependencies;
        if (m->end() != m->find("run_dependencies"))
            run_dependencies = m->find("run_dependencies")->second;

        std::string description;
        if (m->end() != m->find("description"))
            description = m->find("description")->second;

        return make_shared_ptr(new UnpackagedRepository(RepositoryName("unpackaged"),
                    unpackaged_repositories::UnpackagedRepositoryParams::named_create()
                    (k::environment(), env)
                    (k::location(), location)
                    (k::install_under(), install_under)
                    (k::name(), QualifiedPackageName(name))
                    (k::version(), VersionSpec(version))
                    (k::slot(), SlotName(slot))
                    (k::build_dependencies(), build_dependencies)
                    (k::run_dependencies(), run_dependencies)
                    (k::description(), description)));
    }

    tr1::shared_ptr<Repository>
    make_installed_unpackaged_repository(
            Environment * const env,
            tr1::shared_ptr<const Map<std::string, std::string> > m)
    {
        Context context("When creating InstalledUnpackagedRepository:");

        std::string location;
        if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
            throw unpackaged_repositories::RepositoryConfigurationError("Key 'location' not specified or empty");

        std::string root;
        if (m->end() == m->find("root") || ((root = m->find("root")->second)).empty())
            throw unpackaged_repositories::RepositoryConfigurationError("Key 'root' not specified or empty");

        return make_shared_ptr(new InstalledUnpackagedRepository(RepositoryName("installed-unpackaged"),
                    unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                    .environment(env)
                    .location(location)
                    .root(root)));
    }
}

extern "C"
{
    void PALUDIS_VISIBLE register_repositories(RepositoryMaker * maker);
}

void register_repositories(RepositoryMaker * maker)
{
    maker->register_maker("unpackaged", &make_unpackaged_repository);
    maker->register_maker("installed_unpackaged", &make_installed_unpackaged_repository);
}

