/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include "do_contents.hh"
#include <src/output/colour.hh>
#include "command_line.hh"
#include <paludis/paludis.hh>
#include <paludis/fuzzy_finder.hh>
#include <iostream>
#include <algorithm>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    struct ContentsDisplayer
    {
        void visit(const ContentsFileEntry & e)
        {
            cout << "    " << colour(cl_file, stringify(e.location_key()->value())) << endl;
        }

        void visit(const ContentsDirEntry & e)
        {
            cout << "    " << colour(cl_dir, stringify(e.location_key()->value()) )<< endl;
        }

        void visit(const ContentsSymEntry & e)
        {
            cout << "    " << colour(cl_sym, stringify(e.location_key()->value())) << " -> " << e.target_key()->value() << endl;
        }

        void visit(const ContentsOtherEntry & e)
        {
            cout << "    " << colour(cl_other, stringify(e.location_key()->value())) << endl;
        }
    };
}

void
do_one_contents_entry(
        const std::shared_ptr<Environment>,
        const PackageID & e)
{
    cout << "* " << colour(cl_package_name, e) << endl;

    if (e.contents_key())
    {
        std::shared_ptr<const Contents> contents(e.contents_key()->value());
        ContentsDisplayer d;
        std::for_each(indirect_iterator(contents->begin()), indirect_iterator(contents->end()), accept_visitor(d));
    }
    else
        cout << "    " << colour(cl_error, "(unknown)") << endl;

    cout << endl;
}

void
do_one_contents(
        const std::shared_ptr<Environment> env,
        const std::string & q)
{
    Context local_context("When handling query '" + q + "':");

    std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(
                parse_user_package_dep_spec(q, env.get(), UserPackageDepSpecOptions() + updso_allow_wildcards,
                    filter::InstalledAtRoot(env->preferred_root_key()->value()))));

    std::shared_ptr<const PackageIDSequence> entries(
            (*env)[selection::AllVersionsSorted(generator::Matches(*spec, make_null_shared_ptr(), { })
                | filter::InstalledAtRoot(env->preferred_root_key()->value()))]);

    if (entries->empty())
        throw NoSuchPackageError(q);

    for (PackageIDSequence::ConstIterator i(entries->begin()),
            i_end(entries->end()) ; i != i_end ; ++i)
        do_one_contents_entry(env, **i);
}

int
do_contents(const std::shared_ptr<Environment> & env)
{
    int return_code(0);

    Context context("When performing contents action from command line:");

    CommandLine::ParametersConstIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());
    for ( ; q != q_end ; ++q)
    {
        try
        {
            do_one_contents(env, *q);
        }
        catch (const AmbiguousPackageNameError & e)
        {
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ");
            cerr << "Ambiguous package name '" << e.name() << "'. Did you mean:" << endl;
            for (AmbiguousPackageNameError::OptionsConstIterator o(e.begin_options()),
                    o_end(e.end_options()) ; o != o_end ; ++o)
                cerr << "    * " << colour(cl_package_name, *o) << endl;
            cerr << endl;
        }
        catch (const NameError & e)
        {
            return_code |= 1;
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ") << e.message() << endl;
            cerr << endl;
        }
        catch (const NoSuchPackageError & e)
        {
            return_code |= 1;
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ");
            cerr << "Could not find '" << e.name() << "'.";

            if (! CommandLine::get_instance()->a_no_suggestions.specified())
            {
                cerr << " Looking for suggestions:" << endl;

                FuzzyCandidatesFinder f(*env, e.name(), filter::InstalledAtRoot(env->preferred_root_key()->value()));

                if (f.begin() == f.end())
                    cerr << "No suggestions found." << endl;
                else
                    cerr << "Suggestions:" << endl;

                for (FuzzyCandidatesFinder::CandidatesConstIterator c(f.begin()),
                         c_end(f.end()) ; c != c_end ; ++c)
                    cerr << "  * " << colour(cl_package_name, *c) << endl;
            }

            cerr << endl;
        }
        catch (const PackageDatabaseLookupError & e)
        {
            return_code |= 1;
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ") << e.message() << endl;
            cerr << endl;
        }
    }

    return return_code;
}

