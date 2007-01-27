/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "make_ebuild_repository.hh"
#include <paludis/util/log.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/repositories/portage/portage_repository_exceptions.hh>

using namespace paludis;

CountedPtr<PortageRepository>
paludis::make_ebuild_repository(
        Environment * const env,
        AssociativeCollection<std::string, std::string>::ConstPointer m)
{
    std::string repo_file(m->end() == m->find("repo_file") ? std::string("?") :
            m->find("repo_file")->second);

    Context context("When making ebuild repository from repo_file '" + repo_file + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw PortageRepositoryConfigurationError("Key 'location' not specified or empty");

    FSEntryCollection::Pointer profiles(new FSEntryCollection::Concrete);
    if (m->end() != m->find("profiles"))
        WhitespaceTokeniser::get_instance()->tokenise(m->find("profiles")->second,
                create_inserter<FSEntry>(std::back_inserter(*profiles)));
    if (m->end() != m->find("profile") && ! m->find("profile")->second.empty())
    {
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Key 'profile' in '" + repo_file + "' is deprecated, "
                "use 'profiles = " + m->find("profile")->second + "' instead");
        if (profiles->empty())
            profiles->append(m->find("profile")->second);
        else
            throw PortageRepositoryConfigurationError("Both 'profile' and 'profiles' keys are present");
    }
    if (profiles->empty())
        throw PortageRepositoryConfigurationError("No profiles have been specified");

    FSEntryCollection::Pointer eclassdirs(new FSEntryCollection::Concrete);
    if (m->end() != m->find("eclassdirs"))
        WhitespaceTokeniser::get_instance()->tokenise(m->find("eclassdirs")->second,
                create_inserter<FSEntry>(std::back_inserter(*eclassdirs)));
    if (m->end() != m->find("eclassdir") && ! m->find("eclassdir")->second.empty())
    {
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Key 'eclassdir' in '" + repo_file + "' is deprecated, "
                "use 'eclassdirs = " + m->find("eclassdir")->second + "' instead");
        if (eclassdirs->empty())
            eclassdirs->append(m->find("eclassdir")->second);
        else
            throw PortageRepositoryConfigurationError("Both 'eclassdir' and 'eclassdirs' keys are present");
    }
    if (eclassdirs->empty())
        eclassdirs->append(location + "/eclass");

    std::string distdir;
    if (m->end() == m->find("distdir") || ((distdir = m->find("distdir")->second)).empty())
        distdir = location + "/distfiles";

    std::string setsdir;
    if (m->end() == m->find("setsdir") || ((setsdir = m->find("setsdir")->second)).empty())
        setsdir = location + "/sets";

    std::string securitydir;
    if (m->end() == m->find("securitydir") || ((securitydir = m->find("securitydir")->second)).empty())
        securitydir = location + "/metadata/glsa";

    std::string newsdir;
    if (m->end() == m->find("newsdir") || ((newsdir = m->find("newsdir")->second)).empty())
        newsdir = location + "/metadata/news";

    std::string cache;
    if (m->end() == m->find("cache") || ((cache = m->find("cache")->second)).empty())
    {
        cache = location + "/metadata/cache";
        if (! FSEntry(cache).exists())
            cache = "/var/empty";
    }

    std::string write_cache;
    if (m->end() == m->find("write_cache") || ((write_cache = m->find("write_cache")->second)).empty())
        write_cache = "/var/empty";

    std::string names_cache;
    if (m->end() == m->find("names_cache") || ((names_cache = m->find("names_cache")->second)).empty())
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "The names_cache key is not set in '"
                + repo_file + "'. You should read http://paludis.pioto.org/cachefiles.html and select an "
                "appropriate value.");
        names_cache = "/var/empty";
    }

    std::string sync;
    if (m->end() != m->find("sync"))
            sync = m->find("sync")->second;

    std::string sync_options;
    if (m->end() != m->find("sync_options"))
        sync_options = m->find("sync_options")->second;

    if (m->end() != m->find("sync_exclude"))
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "The sync_exclude key in '"
                + repo_file + "' is deprecated in favour of sync_options = --exclude-from=");
        if (! sync_options.empty())
            sync_options += " ";
        sync_options += "--exclude-from='" + m->find("sync_exclude")->second + "'";
    }

    std::string root;
    if (m->end() == m->find("root") || ((root = m->find("root")->second)).empty())
        root = "/";

    std::string buildroot;
    if (m->end() == m->find("buildroot") || ((buildroot = m->find("buildroot")->second)).empty())
        buildroot = "/var/tmp/paludis";

    return CountedPtr<PortageRepository>(new PortageRepository(PortageRepositoryParams::create()
                .entry_format("ebuild")
                .environment(env)
                .location(location)
                .profiles(profiles)
                .cache(cache)
                .write_cache(write_cache)
                .names_cache(names_cache)
                .eclassdirs(eclassdirs)
                .distdir(distdir)
                .pkgdir(FSEntry("/var/empty"))
                .securitydir(securitydir)
                .setsdir(setsdir)
                .newsdir(newsdir)
                .sync(sync)
                .sync_options(sync_options)
                .root(root)
                .buildroot(buildroot)));
}

CountedPtr<Repository>
paludis::make_ebuild_repository_wrapped(
        Environment * const env,
        AssociativeCollection<std::string, std::string>::ConstPointer m)
{
    return make_ebuild_repository(env, m);
}

