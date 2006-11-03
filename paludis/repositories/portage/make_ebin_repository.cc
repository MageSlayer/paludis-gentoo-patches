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

#include "make_ebin_repository.hh"
#include <paludis/util/log.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/repositories/portage/portage_repository_exceptions.hh>

using namespace paludis;

CountedPtr<Repository>
paludis::make_ebin_repository(
        const Environment * const env,
        const PackageDatabase * const db,
        AssociativeCollection<std::string, std::string>::ConstPointer m)
{
    std::string repo_file(m->end() == m->find("repo_file") ? std::string("?") :
            m->find("repo_file")->second);

    Context context("When making ebin repository from repo_file '" + repo_file + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw PortageRepositoryConfigurationError("Key 'location' not specified or empty");

    FSEntryCollection::Pointer profiles(new FSEntryCollection::Concrete);
    if (m->end() != m->find("profiles"))
        WhitespaceTokeniser::get_instance()->tokenise(m->find("profiles")->second,
                create_inserter<FSEntry>(std::back_inserter(*profiles)));
    if (profiles->empty())
        throw PortageRepositoryConfigurationError("No profiles have been specified");

    FSEntryCollection::Pointer eclassdirs(new FSEntryCollection::Concrete);

    std::string pkgdir;
    if (m->end() == m->find("pkgdir") || ((pkgdir = m->find("pkgdir")->second)).empty())
        pkgdir = location + "/packages";

    std::string setsdir;
    if (m->end() == m->find("setsdir") || ((setsdir = m->find("setsdir")->second)).empty())
        setsdir = location + "/sets";

    std::string securitydir;
    if (m->end() == m->find("securitydir") || ((securitydir = m->find("securitydir")->second)).empty())
        securitydir = location + "/metadata/glsa";

    std::string newsdir;
    if (m->end() == m->find("newsdir") || ((newsdir = m->find("newsdir")->second)).empty())
        newsdir = location + "/metadata/news";

    std::string sync;
    if (m->end() == m->find("sync"))
        sync = m->find("sync")->second;

    std::string sync_exclude;
    if (m->end() == m->find("sync_exclude"))
        sync_exclude = m->find("sync_exclude")->second;

    std::string root;
    if (m->end() == m->find("root") || ((root = m->find("root")->second)).empty())
        root = "/";

    std::string buildroot;
    if (m->end() == m->find("buildroot") || ((buildroot = m->find("buildroot")->second)).empty())
        buildroot = "/var/tmp/paludis";

    return CountedPtr<Repository>(new PortageRepository(PortageRepositoryParams::create()
                .entry_format("ebin")
                .environment(env)
                .package_database(db)
                .location(location)
                .profiles(profiles)
                .cache(FSEntry("/var/empty"))
                .eclassdirs(eclassdirs)
                .distdir(FSEntry("/var/empty"))
                .pkgdir(pkgdir)
                .securitydir(securitydir)
                .setsdir(setsdir)
                .newsdir(newsdir)
                .sync(sync)
                .sync_exclude(sync_exclude)
                .root(root)
                .buildroot(buildroot)));
}

#ifdef PALUDIS_ENABLE_VISIBILITY
#  pragma GCC visibility push(default)
#endif
namespace
{
    const RepositoryMaker::RegisterMaker register_portage_ebin_repository PALUDIS_ATTRIBUTE((used)) (
            "ebin", &make_ebin_repository);
}
#ifdef PALUDIS_ENABLE_VISIBILITY
#  pragma GCC visibility pop
#endif

