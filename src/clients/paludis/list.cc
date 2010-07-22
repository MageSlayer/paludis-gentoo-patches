/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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
#include "list.hh"

#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/log.hh>
#include <paludis/util/set.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/metadata_key.hh>
#include <src/output/colour.hh>

#include <memory>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <algorithm>

using namespace paludis;

int
do_list_repositories(const std::shared_ptr<Environment> & env)
{
    int ret_code(1);

    Context context("When performing list-repositories action from command line:");

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.specified())
            if (CommandLine::get_instance()->a_repository.end_args() == std::find(
                        CommandLine::get_instance()->a_repository.begin_args(),
                        CommandLine::get_instance()->a_repository.end_args(),
                        stringify(r->name())))
                continue;
        if (CommandLine::get_instance()->a_repository_format.specified())
            if (CommandLine::get_instance()->a_repository_format.end_args() == std::find(
                        CommandLine::get_instance()->a_repository_format.begin_args(),
                        CommandLine::get_instance()->a_repository_format.end_args(),
                        r->format_key() ? r->format_key()->value() : "?"))
                continue;

        ret_code = 0;

        std::cout << "* " << colour(cl_repository_name, r->name()) << std::endl;
    }

    return ret_code;
}

int
do_list_categories(const std::shared_ptr<Environment> & env)
{
    int ret_code(1);

    Context context("When performing list-categories action from command line:");

    std::map<CategoryNamePart, std::list<RepositoryName> > cats;

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.specified())
            if (CommandLine::get_instance()->a_repository.end_args() == std::find(
                        CommandLine::get_instance()->a_repository.begin_args(),
                        CommandLine::get_instance()->a_repository.end_args(),
                        stringify(r->name())))
                continue;
        if (CommandLine::get_instance()->a_repository_format.specified())
            if (CommandLine::get_instance()->a_repository_format.end_args() == std::find(
                        CommandLine::get_instance()->a_repository_format.begin_args(),
                        CommandLine::get_instance()->a_repository_format.end_args(),
                        r->format_key() ? r->format_key()->value() : "?"))
                continue;

        std::shared_ptr<const CategoryNamePartSet> cat_names(r->category_names());
        for (CategoryNamePartSet::ConstIterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
            cats[*c].push_back(r->name());
    }

    for (std::map<CategoryNamePart, std::list<RepositoryName > >::const_iterator
            c(cats.begin()), c_end(cats.end()) ; c != c_end ; ++c)
    {
        if (CommandLine::get_instance()->a_category.specified())
            if (CommandLine::get_instance()->a_category.end_args() == std::find(
                        CommandLine::get_instance()->a_category.begin_args(),
                        CommandLine::get_instance()->a_category.end_args(),
                        stringify(c->first)))
                continue;

        ret_code = 0;

        std::cout << "* " << colour(cl_package_name, c->first) << std::endl;
        std::cout << "    " << std::setw(22) << std::left << "found in:" <<
            std::setw(0) << " " << join(c->second.begin(), c->second.end(), ", ") << std::endl;
        std::cout << std::endl;
    }

    return ret_code;
}

int
do_list_packages(const std::shared_ptr<Environment> & env)
{
    int ret_code(1);

    Context context("When performing list-packages action from command line:");

    std::map<QualifiedPackageName, std::list<RepositoryName> > pkgs;

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.specified())
            if (CommandLine::get_instance()->a_repository.end_args() == std::find(
                        CommandLine::get_instance()->a_repository.begin_args(),
                        CommandLine::get_instance()->a_repository.end_args(),
                        stringify(r->name())))
                continue;
        if (CommandLine::get_instance()->a_repository_format.specified())
            if (CommandLine::get_instance()->a_repository_format.end_args() == std::find(
                        CommandLine::get_instance()->a_repository_format.begin_args(),
                        CommandLine::get_instance()->a_repository_format.end_args(),
                        r->format_key() ? r->format_key()->value() : "?"))
                continue;

        std::shared_ptr<const CategoryNamePartSet> cat_names(r->category_names());
        for (CategoryNamePartSet::ConstIterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
        {
            if (CommandLine::get_instance()->a_category.specified())
                if (CommandLine::get_instance()->a_category.end_args() == std::find(
                            CommandLine::get_instance()->a_category.begin_args(),
                            CommandLine::get_instance()->a_category.end_args(),
                            stringify(*c)))
                    continue;

            std::shared_ptr<const QualifiedPackageNameSet> pkg_names(r->package_names(*c));
            for (QualifiedPackageNameSet::ConstIterator p(pkg_names->begin()), p_end(pkg_names->end()) ;
                    p != p_end ; ++p)
                pkgs[*p].push_back(r->name());
        }
    }

    for (std::map<QualifiedPackageName, std::list<RepositoryName > >::const_iterator
            p(pkgs.begin()), p_end(pkgs.end()) ; p != p_end ; ++p)
    {
        if (CommandLine::get_instance()->a_package.specified())
            if (CommandLine::get_instance()->a_package.end_args() == std::find(
                        CommandLine::get_instance()->a_package.begin_args(),
                        CommandLine::get_instance()->a_package.end_args(),
                        stringify(p->first.package())))
                continue;

        ret_code = 0;

        std::cout << "* " << colour(cl_package_name, p->first) << std::endl;
        std::cout << "    " << std::setw(22) << std::left << "found in:" <<
            std::setw(0) << " " << join(p->second.begin(), p->second.end(), ", ") << std::endl;
        std::cout << std::endl;
    }

    return ret_code;
}

int
do_list_sets(const std::shared_ptr<Environment> & env)
{
    int ret_code(1);

    Context context("While performing list-sets action from command line:");

    for (SetNameSet::ConstIterator s(env->set_names()->begin()), s_end(env->set_names()->end()) ;
            s != s_end ; ++s)
    {
        if (CommandLine::get_instance()->a_set.specified())
            if (CommandLine::get_instance()->a_set.end_args() == std::find(
                        CommandLine::get_instance()->a_set.begin_args(),
                        CommandLine::get_instance()->a_set.end_args(),
                        stringify(*s)))
                continue;

        ret_code = 0;

        std::cout << "* " << colour(cl_package_name, *s) << std::endl;
    }

    return ret_code;
}


