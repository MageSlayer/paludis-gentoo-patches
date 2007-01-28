/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/environment/default/default_environment.hh>
#include <iostream>
#include <algorithm>

namespace p = paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    struct ContentsDisplayer :
        p::ContentsVisitorTypes::ConstVisitor
    {
        void visit(const p::ContentsFileEntry * const e)
        {
            cout << "    " << colour(cl_file, e->name()) << endl;
        }

        void visit(const p::ContentsDirEntry * const e)
        {
            cout << "    " << colour(cl_dir, e->name()) << endl;
        }

        void visit(const p::ContentsSymEntry * const e)
        {
            cout << "    " << colour(cl_sym, e->name()) << " -> " << e->target() << endl;
        }

        void visit(const p::ContentsMiscEntry * const e)
        {
            cout << "    " << colour(cl_misc, e->name()) << endl;
        }

        void visit(const p::ContentsFifoEntry * const e)
        {
            cout << "    " << colour(cl_fifo, e->name()) << endl;
        }

        void visit(const p::ContentsDevEntry * const e)
        {
            cout << "    " << colour(cl_dev, e->name()) << endl;
        }
    };
}

void
do_one_contents_entry(
        const p::Environment * const env,
        const p::PackageDatabaseEntry & e)
{
    cout << "* " << colour(cl_package_name, e) << endl;

    const p::RepositoryInstalledInterface * const installed_interface(
            env->package_database()->fetch_repository(e.repository)->
            installed_interface);
    if (installed_interface)
    {
        p::Contents::ConstPointer contents(installed_interface->contents(
                    e.name, e.version));
        ContentsDisplayer d;
        std::for_each(contents->begin(), contents->end(), accept_visitor(&d));
    }
    else
        cout << "    " << colour(cl_error, "(unknown)") << endl;

    cout << endl;
}

void
do_one_contents(
        const p::Environment * const env,
        const std::string & q)
{
    p::Context local_context("When handling query '" + q + "':");

    /* we might have a dep atom, but we might just have a simple package name
     * without a category. either should work. */
    p::PackageDepAtom::Pointer atom(std::string::npos == q.find('/') ?
            new p::PackageDepAtom(env->package_database()->fetch_unique_qualified_package_name(
                    p::PackageNamePart(q))) :
            new p::PackageDepAtom(q));

    p::PackageDatabaseEntryCollection::ConstPointer
        entries(env->package_database()->query(*atom, p::is_installed_only, p::qo_order_by_version));

    if (entries->empty())
        throw p::NoSuchPackageError(q);

    for (p::PackageDatabaseEntryCollection::Iterator i(entries->begin()),
            i_end(entries->end()) ; i != i_end ; ++i)
        do_one_contents_entry(env, *i);
}

int
do_contents()
{
    int return_code(0);

    p::Context context("When performing contents action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());
    for ( ; q != q_end ; ++q)
    {
        try
        {
            do_one_contents(env, *q);
        }
        catch (const p::AmbiguousPackageNameError & e)
        {
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ");
            cerr << "Ambiguous package name '" << e.name() << "'. Did you mean:" << endl;
            for (p::AmbiguousPackageNameError::OptionsIterator o(e.begin_options()),
                    o_end(e.end_options()) ; o != o_end ; ++o)
                cerr << "    * " << colour(cl_package_name, *o) << endl;
            cerr << endl;
        }
        catch (const p::NameError & e)
        {
            return_code |= 1;
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ") << e.message() << endl;
            cerr << endl;
        }
        catch (const p::PackageDatabaseLookupError & e)
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

