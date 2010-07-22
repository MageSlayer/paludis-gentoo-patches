/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Danny van Dyk
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

#include "find_unused_packages.hh"
#include "command_line.hh"

#include <paludis/find_unused_packages_task.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/package_database.hh>

#include <set>
#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

void do_find_unused_packages(const NoConfigEnvironment & env)
{
    Context context("When performing find-unused-packages action:");

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
            r(env.package_database()->begin_repositories()),
            r_end(env.package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (r->name() == RepositoryName("virtuals"))
            continue;
        if (env.master_repository() && r->name() == env.master_repository()->name())
            continue;

        Context repo_context("When searching for unused packages in repository '" + stringify(r->name()) + "':");
        FindUnusedPackagesTask task(&env, &(*r));

        cout << "Searching for unused packages in repository " << stringify(r->name()) << endl;

        std::shared_ptr<const CategoryNamePartSet> categories(r->category_names());
        for (CategoryNamePartSet::ConstIterator c(categories->begin()), c_end(categories->end()) ;
                c != c_end ; ++c)
        {
            Context cat_context("When searching for unused packages in category '" + stringify(*c) + "':");

            if (CommandLine::get_instance()->a_category.specified())
                if (CommandLine::get_instance()->a_category.end_args() == std::find(
                            CommandLine::get_instance()->a_category.begin_args(),
                            CommandLine::get_instance()->a_category.end_args(),
                            stringify(*c)))
                    continue;

            cout << " In category " << stringify(*c) << ":" << endl;
            std::shared_ptr<const QualifiedPackageNameSet> packages(r->package_names(*c));

            for (QualifiedPackageNameSet::ConstIterator p(packages->begin()), p_end(packages->end()) ;
                    p != p_end ; ++p)
            {
                Context pkg_context("When searching for unused packages with package name '" + stringify(*p) + "':");

                if (CommandLine::get_instance()->a_category.specified())
                    if (CommandLine::get_instance()->a_category.end_args() == std::find(
                                CommandLine::get_instance()->a_category.begin_args(),
                                CommandLine::get_instance()->a_category.end_args(),
                                stringify(*c)))
                        continue;

                std::shared_ptr<const PackageIDSequence> unused(task.execute(*p));
                for (IndirectIterator<PackageIDSequence::ConstIterator> u(unused->begin()), u_end(unused->end()) ; 
                        u != u_end ; ++u)
                    cout << stringify(*u) << endl;
            }

            cout << endl;
        }
    }
}

