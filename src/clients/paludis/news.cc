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

#include <src/output/colour.hh>
#include "news.hh"
#include <functional>
#include <iomanip>
#include <iostream>
#include <paludis/paludis.hh>
#include <paludis/environment/default/default_environment.hh>
#include <string>

/** \file
 * Handle the --update-news action for the main paludis program.
 */

namespace p = paludis;

int
do_update_news()
{
    int return_code(0);

    p::Context context("When performing update-news action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    for (p::PackageDatabase::RepositoryIterator r(env->package_database()->begin_repositories()),
            r_end(env->package_database()->end_repositories()) ; r != r_end ; ++r)
        if ((*r)->news_interface)
            (*r)->news_interface->update_news();

    return return_code;
}



