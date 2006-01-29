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

#include "list.hh"
#include "colour.hh"
#include <paludis/paludis.hh>
#include <iostream>
#include <iomanip>
#include <map>
#include <list>

namespace p = paludis;

int
do_list_repositories()
{
    p::Context context("When performing list-repositories action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    for (p::IndirectIterator<p::PackageDatabase::RepositoryIterator, const p::Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        std::cout << "* " << colour(cl_package_name, r->name()) << std::endl;

        for (p::Repository::InfoIterator i(r->begin_info()), i_end(r->end_info()) ; i != i_end ; ++i)
            std::cout << "    " << std::setw(22) << std::left << (p::stringify(i->first) + ":")
                << std::setw(0) << " " << i->second << std::endl;

        std::cout << std::endl;
    }

    return 0;
}

int
do_list_categories()
{
    p::Context context("When performing list-categories action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    std::map<p::CategoryNamePart, std::list<p::RepositoryName> > cats;

    for (p::IndirectIterator<p::PackageDatabase::RepositoryIterator, const p::Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        p::CategoryNamePartCollection::ConstPointer cat_names(r->category_names());
        for (p::CategoryNamePartCollection::Iterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
            cats[*c].push_back(r->name());
    }

    for (std::map<p::CategoryNamePart, std::list<p::RepositoryName > >::const_iterator
            c(cats.begin()), c_end(cats.end()) ; c != c_end ; ++c)
    {
        std::cout << "* " << colour(cl_package_name, c->first) << std::endl;
        std::cout << "    " << std::setw(22) << std::left << "found in:" <<
            std::setw(0) << " " << p::join(c->second.begin(), c->second.end(), ", ") << std::endl;
        std::cout << std::endl;
    }

    return 0;
}
