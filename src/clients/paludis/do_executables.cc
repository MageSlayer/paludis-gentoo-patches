/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Richard Brown
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

#include "do_executables.hh"
#include "paludis/util/fs_entry-fwd.hh"
#include "paludis/util/log.hh"
#include "paludis/util/tokeniser.hh"
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
    struct ExecutablesDisplayer
    {
        private:
            const std::list<std::string> _paths;

            bool is_file_in_path(FSEntry file)
            {
                try
                {
                    if (file.exists())
                    {
                        if (file.has_permission(fs_ug_others, fs_perm_execute))
                        {
                            FSEntry dirname(file.dirname());
                            for (std::list<std::string>::const_iterator it(_paths.begin()),
                                    it_end(_paths.end()); it_end != it; ++it)
                            {
                                if (dirname == *it)
                                    return true;
                            }
                        }
                    }
                    else
                    {
                        Context context("When checking permissions on '" + stringify(file) + "'");
                        Log::get_instance()->message("do_executables.file_does_not_exist", ll_warning, lc_context)
                            << "'" << stringify(file) << "' is listed as installed but does not exist";
                    }
                    return false;
                }
                catch (const FSError & e)
                {
                    return false;
                }
            }

        public:
            ExecutablesDisplayer(std::list<std::string> p) :
                _paths(p)
            {
            }

            void visit(const ContentsFileEntry & e)
            {
                if (is_file_in_path(e.location_key()->value()))
                    cout << "    " << colour(cl_file, e.location_key()->value()) << endl;
            }

            void visit(const ContentsDirEntry &)
            {
            }

            void visit(const ContentsSymEntry & e)
            {
                FSEntry sym(e.location_key()->value());
                FSEntry real(sym.realpath_if_exists());
                if (sym != real)
                    if (is_file_in_path(sym))
                        cout << "    " << colour(cl_sym, e.location_key()->value()) << endl;
            }

            void visit(const ContentsOtherEntry &)
            {
            }
    };
}

void
do_one_executables_entry(
        const std::tr1::shared_ptr<Environment>,
        const PackageID & e)
{
    cout << "* " << colour(cl_package_name, e) << endl;

    if (e.contents_key())
    {
        std::tr1::shared_ptr<const Contents> contents(e.contents_key()->value());
        std::string path(getenv("PATH"));
        std::list<std::string> paths;
        tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(path, ":", "", std::back_inserter(paths));
        ExecutablesDisplayer d(paths);
        std::for_each(indirect_iterator(contents->begin()), indirect_iterator(contents->end()), accept_visitor(d));
    }
    else
        cout << "    " << colour(cl_error, "(unknown)") << endl;

    cout << endl;
}

void
do_one_executables(
        const std::tr1::shared_ptr<Environment> env,
        const std::string & q)
{
    Context local_context("When handling query '" + q + "':");

    std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(
                parse_user_package_dep_spec(q, env.get(), UserPackageDepSpecOptions() + updso_allow_wildcards,
                    filter::InstalledAtRoot(env->root()))));

    std::tr1::shared_ptr<const PackageIDSequence> entries(
            (*env)[selection::AllVersionsSorted(generator::Matches(*spec, MatchPackageOptions()) | filter::InstalledAtRoot(env->root()))]);

    if (entries->empty())
        throw NoSuchPackageError(q);

    for (PackageIDSequence::ConstIterator i(entries->begin()),
            i_end(entries->end()) ; i != i_end ; ++i)
        do_one_executables_entry(env, **i);
}

int
do_executables(const std::tr1::shared_ptr<Environment> & env)
{
    int return_code(0);

    Context context("When performing executables action from command line:");

    CommandLine::ParametersConstIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());
    for ( ; q != q_end ; ++q)
    {
        try
        {
            do_one_executables(env, *q);
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

                FuzzyCandidatesFinder f(*env, e.name(), filter::InstalledAtRoot(env->root()));

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

