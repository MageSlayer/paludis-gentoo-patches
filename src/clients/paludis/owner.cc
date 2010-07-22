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

#include "owner.hh"
#include <paludis/util/set.hh>
#include <src/output/colour.hh>
#include "command_line.hh"
#include <paludis/paludis.hh>
#include <iostream>
#include <algorithm>
#include <set>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    struct ContentsFinder
    {
        std::set<std::string> matches;
        const std::string query;
        const bool full;

        ContentsFinder(const std::string & q, bool f) :
            query(q),
            full(f)
        {
        }

        void handle(const std::string & e)
        {
            if (full)
            {
                if (e == query)
                    matches.insert(e);
            }
            else
            {
                if (std::string::npos != e.find(query))
                    matches.insert(e);
            }
        }

        void visit(const ContentsFileEntry & e)
        {
            handle(stringify(e.location_key()->value()));
        }

        void visit(const ContentsDirEntry & e)
        {
            handle(stringify(e.location_key()->value()));
        }

        void visit(const ContentsSymEntry & e)
        {
            handle(stringify(e.location_key()->value()));
        }

        void visit(const ContentsOtherEntry & e)
        {
            handle(stringify(e.location_key()->value()));
        }
    };
}

int
do_one_owner(
        const std::shared_ptr<Environment> env,
        const std::string & query)
{
    bool found_owner=false;
    cout << "* " << colour(cl_package_name, query) << endl;

    for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
            r_end(env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (*r)->installed_root_key())
            continue;

        std::shared_ptr<const CategoryNamePartSet> cats((*r)->category_names());
        for (CategoryNamePartSet::ConstIterator c(cats->begin()),
                c_end(cats->end()) ; c != c_end ; ++c)
        {
            std::shared_ptr<const QualifiedPackageNameSet> pkgs((*r)->package_names(*c));
            for (QualifiedPackageNameSet::ConstIterator p(pkgs->begin()),
                    p_end(pkgs->end()) ; p != p_end ; ++p)
            {
                std::shared_ptr<const PackageIDSequence> ids((*r)->package_ids(*p));
                for (PackageIDSequence::ConstIterator v(ids->begin()), v_end(ids->end()) ; v != v_end ; ++v)
                {
                    if (! (*v)->contents_key())
                        continue;

                    std::shared_ptr<const Contents> contents((*v)->contents_key()->value());
                    ContentsFinder d(query, CommandLine::get_instance()->a_full_match.specified());
                    std::for_each(indirect_iterator(contents->begin()), indirect_iterator(contents->end()), accept_visitor(d));
                    if (! d.matches.empty())
                    {
                        cout << "    " << **v << endl;
                        if (! CommandLine::get_instance()->a_full_match.specified())
                        {
                            for (std::set<std::string>::const_iterator f(d.matches.begin()), f_end(d.matches.end()) ;
                                    f != f_end ; ++f)
                                cout << "        " << *f << endl;
                        }

                        found_owner=true;
                    }

                    (*v)->can_drop_in_memory_cache();
                }
            }
        }
    }

    cout << endl;
    return found_owner ? 0 : 1;
}

int
do_owner(const std::shared_ptr<Environment> & env)
{
    int return_code(0);
    Context context("When performing owner action from command line:");

    CommandLine::ParametersConstIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());
    for ( ; q != q_end ; ++q)
        return_code |= do_one_owner(env, *q);

    return return_code;
}

