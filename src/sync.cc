/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "src/colour.hh"
#include "src/sync.hh"
#include <functional>
#include <iomanip>
#include <iostream>
#include <paludis/paludis.hh>
#include <string>

/** \file
 * Handle the --sync action for the main paludis program.
 */

namespace p = paludis;

namespace
{
    int do_one_sync(p::Repository::ConstPointer r)
    {
        int return_code(0);

        std::cout << colour(cl_heading, "Sync " + p::stringify(r->name())) << std::endl;
        try
        {
            if (r->sync())
                std::cout << "Sync " << r->name() << " completed" << std::endl;
            else
                std::cout << "Sync " << r->name() << " skipped" << std::endl;
        }
        catch (const p::SyncFailedError & e)
        {
            return_code |= 1;
            std::cout << std::endl;
            std::cerr << "Sync error:" << std::endl;
            std::cerr << "  * " << e.backtrace("\n  * ") << e.message() << std::endl;
            std::cerr << std::endl;
            std::cout << "Sync " << r->name() << " failed" << std::endl;
        }

        return return_code;
    }
}

int do_sync()
{
    int return_code(0);

    p::Context context("When performing sync action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    if (CommandLine::get_instance()->empty())
        for (p::PackageDatabase::RepositoryIterator r(env->package_database()->begin_repositories()),
                r_end(env->package_database()->end_repositories()) ; r != r_end ; ++r)
            return_code |= do_one_sync(*r);
    else
    {
        std::set<p::RepositoryName> repo_names;
        std::copy(CommandLine::get_instance()->begin_parameters(),
                CommandLine::get_instance()->end_parameters(),
                p::create_inserter<p::RepositoryName>(std::inserter(
                        repo_names, repo_names.begin())));

        env->perform_hook("sync_all_pre");
        for (std::set<p::RepositoryName>::iterator r(repo_names.begin()), r_end(repo_names.end()) ;
                r != r_end ; ++r)
        {
            try
            {
                env->perform_hook("sync_pre");
                return_code |= do_one_sync(env->package_database()->fetch_repository(*r));
                env->perform_hook("sync_post");
            }
            catch (const p::NoSuchRepositoryError & e)
            {
                return_code |= 1;
                std::cerr << "No such repository '" << *r << "'" << std::endl;
                std::cout << "Sync " << *r << " failed" << std::endl;
            }
        }
        env->perform_hook("sync_all_post");
    }

    return return_code;
}


