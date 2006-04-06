/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

int do_sync()
{
    int return_code(0);

    p::Context context("When performing sync action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    for (p::PackageDatabase::RepositoryIterator r(env->package_database()->begin_repositories()),
            r_end(env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        std::cout << colour(cl_heading, "Sync " + p::stringify((*r)->name())) << std::endl;

        try
        {
            if ((*r)->sync())
            {
                std::cout << "Sync " << (*r)->name() << " completed" << std::endl;
            }
            else
            {
                std::cout << "Sync " << (*r)->name() << " skipped" << std::endl;
            }
        }
        catch (const p::SyncFailedError & e)
        {
            return_code |= 1;
            std::cout << std::endl;
            std::cerr << "Sync error:" << std::endl;
            std::cerr << "  * " << e.backtrace("\n  * ") << e.message() << std::endl;
            std::cerr << std::endl;
            std::cout << "Sync " << (*r)->name() << " failed" << std::endl;
        }
    }

    return return_code;
}


