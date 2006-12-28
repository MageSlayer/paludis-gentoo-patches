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

#include "owner.hh"
#include <src/output/colour.hh>
#include "command_line.hh"
#include <paludis/paludis.hh>
#include <paludis/environment/default/default_environment.hh>
#include <iostream>

namespace p = paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    struct ContentsFinder :
        p::ContentsVisitorTypes::ConstVisitor
    {
        bool found;
        const std::string query;
        const bool full;

        ContentsFinder(const std::string & q, bool f) :
            found(false),
            query(q),
            full(f)
        {
        }

        void handle(const std::string & e)
        {
            if (full)
                found |= (e == query);
            else
                found |= (std::string::npos != e.find(query));
        }

        void visit(const p::ContentsFileEntry * const e)
        {
            handle(e->name());
        }

        void visit(const p::ContentsDirEntry * const e)
        {
            handle(e->name());
        }

        void visit(const p::ContentsSymEntry * const e)
        {
            handle(e->name());
        }

        void visit(const p::ContentsMiscEntry * const e)
        {
            handle(e->name());
        }
    };
}

int
do_one_owner(
        const p::Environment * const env,
        const std::string & query)
{
    bool found_owner=false;
    cout << "* " << colour(cl_package_name, query) << endl;

    for (p::PackageDatabase::RepositoryIterator r(env->package_database()->begin_repositories()),
            r_end(env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (*r)->installed_interface)
            continue;

        p::CategoryNamePartCollection::ConstPointer cats((*r)->category_names());
        for (p::CategoryNamePartCollection::Iterator c(cats->begin()),
                c_end(cats->end()) ; c != c_end ; ++c)
        {
            p::QualifiedPackageNameCollection::ConstPointer pkgs((*r)->package_names(*c));
            for (p::QualifiedPackageNameCollection::Iterator p(pkgs->begin()),
                    p_end(pkgs->end()) ; p != p_end ; ++p)
            {
                p::VersionSpecCollection::ConstPointer vers((*r)->version_specs(*p));
                for (p::VersionSpecCollection::Iterator v(vers->begin()),
                        v_end(vers->end()) ; v != v_end ; ++v)
                {
                    p::PackageDatabaseEntry e(*p, *v, (*r)->name());
                    p::Contents::ConstPointer contents((*r)->installed_interface->
                            contents(*p, *v));
                    ContentsFinder d(query, CommandLine::get_instance()->a_full_match.specified());
                    std::for_each(contents->begin(), contents->end(), accept_visitor(&d));
                    if (d.found)
                    {
                        cout << "    " << e << endl;
                        found_owner=true;
                    }
                }
            }
        }
    }

    cout << endl;
    return found_owner ? 0 : 1;
}


int
do_owner()
{
    int return_code(0);
    p::Context context("When performing owner action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());
    for ( ; q != q_end ; ++q)
        return_code |= do_one_owner(env, *q);

    return return_code;
}

