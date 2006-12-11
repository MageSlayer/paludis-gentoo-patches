/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "make_gems_repository.hh"
#include <paludis/repositories/gems/gems_repository_exceptions.hh>

using namespace paludis;

Repository::Pointer
paludis::make_gems_repository(const Environment * const env,
        const PackageDatabase * const db,
        AssociativeCollection<std::string, std::string>::ConstPointer m)
{
    std::string repo_file(m->end() == m->find("repo_file") ? std::string("?") :
            m->find("repo_file")->second);

    Context context("When making gems repository from repo_file '" + repo_file + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw GemsRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string distdir;
    if (m->end() == m->find("distdir") || ((distdir = m->find("distdir")->second)).empty())
        distdir = location + "/distfiles";

    std::string sync;
    if (m->end() != m->find("sync"))
        sync = m->find("sync")->second;

    std::string sync_exclude;
    if (m->end() != m->find("sync_exclude"))
        sync_exclude = m->find("sync_exclude")->second;

    std::string root;
    if (m->end() == m->find("root") || ((root = m->find("root")->second)).empty())
        root = "/";

    std::string buildroot;
    if (m->end() == m->find("buildroot") || ((buildroot = m->find("buildroot")->second)).empty())
        buildroot = "/var/tmp/paludis";

    return Repository::Pointer(new GemsRepository(GemsRepositoryParams::create()
                .environment(env)
                .package_database(db)
                .location(location)
                .distdir(distdir)
                .sync(sync)
                .sync_exclude(sync_exclude)
                .root(root)
                .buildroot(buildroot)));

}

