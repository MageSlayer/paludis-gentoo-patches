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
#include "command_line.hh"
#include "list.hh"
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <paludis/paludis.hh>
#include <paludis/environment/default/default_environment.hh>
#include <paludis/util/log.hh>
#include <paludis/util/visitor.hh>

namespace p = paludis;

int
do_list_repositories()
{
    int ret_code(1);

    p::Context context("When performing list-repositories action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    for (p::IndirectIterator<p::PackageDatabase::RepositoryIterator, const p::Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.specified())
            if (CommandLine::get_instance()->a_repository.args_end() == std::find(
                        CommandLine::get_instance()->a_repository.args_begin(),
                        CommandLine::get_instance()->a_repository.args_end(),
                        stringify(r->name())))
                continue;

        ret_code = 0;

        std::cout << "* " << colour(cl_repository_name, r->name()) << std::endl;

        p::RepositoryInfo::ConstPointer ii(r->info(false));
        for (p::RepositoryInfo::SectionIterator i(ii->begin_sections()),
                i_end(ii->end_sections()) ; i != i_end ; ++i)
        {
            std::cout << "    " << colour(cl_heading, (*i)->heading() + ":") << std::endl;
            for (p::RepositoryInfoSection::KeyValueIterator k((*i)->begin_kvs()),
                    k_end((*i)->end_kvs()) ; k != k_end ; ++k)
                std::cout << "        " << std::setw(22) << std::left << (p::stringify(k->first) + ":")
                    << std::setw(0) << " " << k->second << std::endl;
            std::cout << std::endl;
        }
    }

    return ret_code;
}

int
do_list_categories()
{
    int ret_code(1);

    p::Context context("When performing list-categories action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    std::map<p::CategoryNamePart, std::list<p::RepositoryName> > cats;

    for (p::IndirectIterator<p::PackageDatabase::RepositoryIterator, const p::Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.specified())
            if (CommandLine::get_instance()->a_repository.args_end() == std::find(
                        CommandLine::get_instance()->a_repository.args_begin(),
                        CommandLine::get_instance()->a_repository.args_end(),
                        stringify(r->name())))
                continue;

        p::CategoryNamePartCollection::ConstPointer cat_names(r->category_names());
        for (p::CategoryNamePartCollection::Iterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
            cats[*c].push_back(r->name());
    }

    for (std::map<p::CategoryNamePart, std::list<p::RepositoryName > >::const_iterator
            c(cats.begin()), c_end(cats.end()) ; c != c_end ; ++c)
    {
        if (CommandLine::get_instance()->a_category.specified())
            if (CommandLine::get_instance()->a_category.args_end() == std::find(
                        CommandLine::get_instance()->a_category.args_begin(),
                        CommandLine::get_instance()->a_category.args_end(),
                        stringify(c->first)))
                continue;

        ret_code = 0;

        std::cout << "* " << colour(cl_package_name, c->first) << std::endl;
        std::cout << "    " << std::setw(22) << std::left << "found in:" <<
            std::setw(0) << " " << p::join(c->second.begin(), c->second.end(), ", ") << std::endl;
        std::cout << std::endl;
    }

    return ret_code;
}

int
do_list_packages()
{
    int ret_code(1);

    p::Context context("When performing list-packages action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    std::map<p::QualifiedPackageName, std::list<p::RepositoryName> > pkgs;

    for (p::IndirectIterator<p::PackageDatabase::RepositoryIterator, const p::Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.specified())
            if (CommandLine::get_instance()->a_repository.args_end() == std::find(
                        CommandLine::get_instance()->a_repository.args_begin(),
                        CommandLine::get_instance()->a_repository.args_end(),
                        stringify(r->name())))
                continue;

        p::CategoryNamePartCollection::ConstPointer cat_names(r->category_names());
        for (p::CategoryNamePartCollection::Iterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
        {
            if (CommandLine::get_instance()->a_category.specified())
                if (CommandLine::get_instance()->a_category.args_end() == std::find(
                            CommandLine::get_instance()->a_category.args_begin(),
                            CommandLine::get_instance()->a_category.args_end(),
                            stringify(*c)))
                    continue;

            p::QualifiedPackageNameCollection::ConstPointer pkg_names(r->package_names(*c));
            for (p::QualifiedPackageNameCollection::Iterator p(pkg_names->begin()), p_end(pkg_names->end()) ;
                    p != p_end ; ++p)
                pkgs[*p].push_back(r->name());
        }
    }

    for (std::map<p::QualifiedPackageName, std::list<p::RepositoryName > >::const_iterator
            p(pkgs.begin()), p_end(pkgs.end()) ; p != p_end ; ++p)
    {
        if (CommandLine::get_instance()->a_package.specified())
            if (CommandLine::get_instance()->a_package.args_end() == std::find(
                        CommandLine::get_instance()->a_package.args_begin(),
                        CommandLine::get_instance()->a_package.args_end(),
                        stringify(p->first.package)))
                continue;

        ret_code = 0;

        std::cout << "* " << colour(cl_package_name, p->first) << std::endl;
        std::cout << "    " << std::setw(22) << std::left << "found in:" <<
            std::setw(0) << " " << p::join(p->second.begin(), p->second.end(), ", ") << std::endl;
        std::cout << std::endl;
    }

    return ret_code;
}

int
do_list_sets()
{
    int ret_code(1);

    p::Context context("While performing list-sets action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    std::map<p::SetName, std::list<std::string> > sets;

    for (p::IndirectIterator<p::PackageDatabase::RepositoryIterator, const p::Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (r->sets_interface == 0)
            continue;

        if (CommandLine::get_instance()->a_repository.specified())
            if (CommandLine::get_instance()->a_repository.args_end() == std::find(
                        CommandLine::get_instance()->a_repository.args_begin(),
                        CommandLine::get_instance()->a_repository.args_end(),
                        stringify(r->name())))
                continue;

        p::SetsCollection::ConstPointer set_names(r->sets_interface->sets_list());
        for (p::SetsCollection::Iterator s(set_names->begin()), s_end(set_names->end()) ;
                s != s_end ; ++s)
            sets[*s].push_back(stringify(r->name()));
    }

    if (! CommandLine::get_instance()->a_repository.specified())
    {
        p::SetsCollection::ConstPointer set_names(env->sets_list());
        for (p::SetsCollection::Iterator s(set_names->begin()), s_end(set_names->end()) ;
                s != s_end ; ++s)
            sets[*s].push_back("environment");
    }

    for (std::map<p::SetName, std::list<std::string> >::const_iterator
            s(sets.begin()), s_end(sets.end()) ; s != s_end ; ++s)
    {
        if (CommandLine::get_instance()->a_set.specified())
            if (CommandLine::get_instance()->a_set.args_end() == std::find(
                        CommandLine::get_instance()->a_set.args_begin(),
                        CommandLine::get_instance()->a_set.args_end(),
                        stringify(s->first)))
                continue;

        ret_code = 0;

        std::cout << "* " << colour(cl_package_name, s->first) << std::endl;
        std::cout << "    " << std::setw(22) << std::left << "found in:" <<
            std::setw(0) << " " << p::join(s->second.begin(), s->second.end(), ", ") << std::endl;
        std::cout << std::endl;
    }

    return ret_code;
}


