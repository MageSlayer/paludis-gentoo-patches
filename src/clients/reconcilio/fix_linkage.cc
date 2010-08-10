/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#include "command_line.hh"
#include "fix_linkage.hh"
#include "install.hh"

#include <paludis/util/fs_entry.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>

#include <paludis/broken_linkage_finder.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/name.hh>
#include <paludis/package_id.hh>
#include <paludis/version_requirements.hh>
#include <paludis/metadata_key.hh>
#include <paludis/partially_made_package_dep_spec.hh>

#include <src/output/colour.hh>

#include <iostream>

using namespace paludis;

int
do_fix_linkage(const std::shared_ptr<Environment> & env)
{
    Context ctx("When performing the Fix Linkage action:");

    std::string library(CommandLine::get_instance()->a_library.argument());
    auto libraries(std::make_shared<Sequence<std::string>>());
    libraries->push_back(library);

    if (library.empty())
        std::cout << "Searching for broken packages... " << std::flush;
    else
        std::cout << "Searching for packages that depend on " << library << "... " << std::flush;
    BrokenLinkageFinder finder(env.get(), libraries);
    std::cout << std::endl;

    if (finder.begin_broken_packages() == finder.end_broken_packages())
    {
        if (library.empty())
            std::cout << "No broken packages found" << std::endl;
        else
            std::cout << "No packages that depend on " << library << " found" << std::endl;
    }
    else
    {
        if (library.empty())
            std::cout << std::endl << colour(cl_heading, "Broken packages:") << std::endl;
        else
            std::cout << std::endl << colour(cl_heading, "Packages that depend on " +  library + ":") << std::endl;
    }

    std::shared_ptr<Sequence<std::string> > targets(std::make_shared<Sequence<std::string>>());
    for (BrokenLinkageFinder::BrokenPackageConstIterator pkg_it(finder.begin_broken_packages()),
             pkg_it_end(finder.end_broken_packages()); pkg_it_end != pkg_it; ++pkg_it)
    {
        std::cout << std::endl;

        std::string pkgname(stringify((*pkg_it)->name()));
        std::string fullname((*pkg_it)->canonical_form(idcf_full));
        std::string::size_type pos(fullname.find(pkgname));
        if (std::string::npos != pos)
            fullname.replace(pos, pkgname.length(), colour(cl_package_name, pkgname));
        std::cout << "* " << fullname << std::endl;

        for (BrokenLinkageFinder::BrokenFileConstIterator file_it(finder.begin_broken_files(*pkg_it)),
                 file_it_end(finder.end_broken_files(*pkg_it)); file_it_end != file_it; ++file_it)
        {
            std::cout << "    " << *file_it;
            if (library.empty())
                std::cout << " (requires "
                          << join(finder.begin_missing_requirements(*pkg_it, *file_it),
                                  finder.end_missing_requirements(*pkg_it, *file_it),
                                  " ") << ")";
            std::cout << std::endl;
        }

        PartiallyMadePackageDepSpec part_spec({ });
        part_spec.package((*pkg_it)->name());
        if ((*pkg_it)->slot_key())
            part_spec.slot_requirement(std::make_shared<UserSlotExactRequirement>((*pkg_it)->slot_key()->value()));

        if (CommandLine::get_instance()->a_exact.specified())
            part_spec.version_requirement(make_named_values<VersionRequirement>(
                        n::version_operator() = vo_equal,
                        n::version_spec() = (*pkg_it)->version()));

        targets->push_back(stringify(PackageDepSpec(part_spec)));
    }

    std::shared_ptr<const PackageID> orphans;
    if (finder.begin_broken_files(orphans) != finder.end_broken_files(orphans))
    {
        if (library.empty())
            std::cout << std::endl << "The following broken files are not owned by any installed package:" << std::endl;
        else
            std::cout << std::endl << "The following files that depend on " << library << " are not owned by any installed package:" << std::endl;

        for (BrokenLinkageFinder::BrokenFileConstIterator file_it(finder.begin_broken_files(orphans)),
                 file_it_end(finder.end_broken_files(orphans)); file_it_end != file_it; ++file_it)
        {
            std::cout << "    " << *file_it;
            if (library.empty())
                std::cout << " (requires "
                          << join(finder.begin_missing_requirements(orphans, *file_it),
                                  finder.end_missing_requirements(orphans, *file_it),
                                  " ") << ")";
            std::cout << std::endl;
        }
    }

    if (! targets->empty())
        return do_install(env, targets);
    return 0;
}

