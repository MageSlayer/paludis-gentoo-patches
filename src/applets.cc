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

#include "src/applets.hh"
#include <functional>
#include <iomanip>
#include <iostream>
#include <paludis/paludis.hh>
#include <string>

/** \file
 * Handle the --has-version and --best-version actions for the main paludis
 * program.
 */

namespace p = paludis;

int do_has_version()
{
    int return_code(0);

    p::Context context("When performing has-version action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    std::string query(*CommandLine::get_instance()->begin_parameters());
    p::PackageDepAtom::Pointer atom(new p::PackageDepAtom(query));
    p::PackageDatabaseEntryCollection::ConstPointer entries(env->package_database()->query(
                atom, p::is_installed_only));
    if (entries->empty())
        return_code = 1;

    return return_code;
}

int do_best_version()
{
    int return_code(0);

    p::Context context("When performing best-version action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    std::string query(*CommandLine::get_instance()->begin_parameters());
    p::PackageDepAtom::Pointer atom(new p::PackageDepAtom(query));
    p::PackageDatabaseEntryCollection::ConstPointer entries(env->package_database()->query(
                atom, p::is_installed_only));
    if (entries->empty())
        return_code = 1;
    else
        std::cout << *(entries->last()) << std::endl;

    return return_code;
}


