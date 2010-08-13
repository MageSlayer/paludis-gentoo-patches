/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010 Ciaran McCreesh
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
#include "query.hh"
#include <src/output/console_query_task.hh>
#include <paludis/paludis.hh>
#include <paludis/fuzzy_finder.hh>
#include <paludis/filter.hh>
#include <string>
#include <functional>
#include <iomanip>
#include <iostream>
#include <algorithm>

/** \file
 * Handle the --query action for the main paludis program.
 */

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    class QueryTask :
        public ConsoleQueryTask
    {
        public:
            QueryTask(const std::shared_ptr<Environment> e) :
                ConsoleQueryTask(e.get())
            {
            }

            bool want_deps() const
            {
                return CommandLine::get_instance()->a_show_deps.specified() || want_raw();
            }

            bool want_raw() const
            {
                return CommandLine::get_instance()->a_show_metadata.specified();
            }

            bool want_authors() const
            {
                return CommandLine::get_instance()->a_show_authors.specified();
            }

            bool want_compact() const
            {
                return CommandLine::get_instance()->a_compact.specified();
            }
    };
}

void do_one_package_query(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<Map<char, std::string> > & masks_to_explain,
        std::shared_ptr<PackageDepSpec> spec)
{
    QueryTask query(env);
    query.show(*spec);
    std::copy(query.masks_to_explain()->begin(), query.masks_to_explain()->end(),
            masks_to_explain->inserter());
}

namespace
{
    struct SetPrettyPrinter
    {
        std::ostringstream stream;

        void visit(const SetSpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            stream << "            " << *node.spec() << std::endl;
        }

        void visit(const SetSpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            stream << "            " << *node.spec() << std::endl;
        }

        void visit(const SetSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }
    };
}

void do_one_set_query(
        const std::shared_ptr<Environment> &,
        const std::string & q,
        const std::shared_ptr<Map<char, std::string> > &,
        std::shared_ptr<const SetSpecTree> set)
{
    cout << "* " << colour(cl_package_name, q) << endl;
    SetPrettyPrinter packages;
    set->top()->accept(packages);
    cout << "    " << std::setw(22) << std::left << "Packages:" << std::setw(0)
        << endl << packages.stream.str() << endl;
}

void do_one_query(
        const std::shared_ptr<Environment> & env,
        const std::string & q,
        const std::shared_ptr<Map<char, std::string> > & masks_to_explain)
{
    Context local_context("When handling query '" + q + "':");

    try
    {
        do_one_package_query(env, masks_to_explain, std::make_shared<PackageDepSpec>(
                        parse_user_package_dep_spec(q, env.get(), UserPackageDepSpecOptions() + updso_throw_if_set + updso_allow_wildcards)));
    }
    catch (const GotASetNotAPackageDepSpec &)
    {
        do_one_set_query(env, q, masks_to_explain, env->set(SetName(q)));
    }
}

int do_query(const std::shared_ptr<Environment> & env)
{
    int return_code(0);

    Context context("When performing query action from command line:");

    std::shared_ptr<Map<char, std::string> > masks_to_explain(std::make_shared<Map<char, std::string>>());

    CommandLine::ParametersConstIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());
    for ( ; q != q_end ; ++q)
    {
        try
        {
            do_one_query(env, *q, masks_to_explain);
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

                try
                {
                    FuzzyCandidatesFinder f(*env, e.name(), filter::All());

                    if (f.begin() == f.end())
                        cerr << "No suggestions found." << endl;
                    else
                        cerr << "Suggestions:" << endl;

                    for (FuzzyCandidatesFinder::CandidatesConstIterator c(f.begin()),
                             c_end(f.end()) ; c != c_end ; ++c)
                        cerr << "  * " << colour(cl_package_name, *c) << endl;
                }
                catch (const PackageDepSpecError &)
                {
                    cerr << "Query too complicated or confusing to make suggestions." << endl;
                }
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

    if (! masks_to_explain->empty())
    {
        cout << colour(cl_heading, "Key to mask reasons:") << endl << endl;

        for (Map<char, std::string>::ConstIterator m(masks_to_explain->begin()), m_end(masks_to_explain->end()) ;
                m != m_end ; ++m)
            cout << "* " << colour(cl_masked, m->first) << ": " << m->second << endl;

        cout << endl;
    }

    return return_code;
}

