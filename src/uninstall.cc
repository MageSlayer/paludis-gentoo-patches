/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "src/colour.hh"
#include "src/uninstall.hh"
#include <iostream>
#include <paludis/paludis.hh>

/** \file
 * Handle the --uninstall action for the main paludis program.
 */

namespace p = paludis;

using std::cerr;
using std::cout;
using std::endl;

int
do_uninstall()
{
    int return_code(0);

    p::Context context("When performing uninstall action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    cout << colour(cl_heading, "These packages will be uninstalled:") << endl << endl;

    std::list<p::PackageDepAtom::Pointer> targets;

    CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());

    for ( ; q != q_end ; ++q)
    {
        /* we might have a dep atom, but we might just have a simple package name
         * without a category. either should work. */
        if (std::string::npos != q->find('/'))
            targets.push_back(p::PackageDepAtom::Pointer(new p::PackageDepAtom(*q)));
        else
            targets.push_back(p::PackageDepAtom::Pointer(new p::PackageDepAtom(
                            env->package_database()->fetch_unique_qualified_package_name(
                                p::PackageNamePart(*q)))));
    }

    bool ok(true);
    p::PackageDatabaseEntryCollection::Pointer unmerge(new p::PackageDatabaseEntryCollection);
    for (std::list<p::PackageDepAtom::Pointer>::iterator t(targets.begin()), t_end(targets.end()) ;
            t != t_end ; ++t)
    {
        p::PackageDatabaseEntryCollection::ConstPointer r(
                env->package_database()->query(*t, p::is_installed_only));
        if (r->empty())
        {
            cout << "* No match for " << colour(cl_red, **t) << endl;
            ok = false;
        }
        else if (r->size() > 1)
        {
            cout << "* Multiple matches for " << colour(cl_red, **t) << ":" << endl;
            for (p::PackageDatabaseEntryCollection::Iterator p(r->begin()),
                    p_end(r->end()) ; p != p_end ; ++p)
                cout << "  * " << *p << endl;
            ok = false;
        }
        else
        {
            cout << "* " << colour(cl_package_name, *r->begin()) << endl;
            unmerge->insert(*r->begin());
        }
    }

    int current_count = 0, max_count = std::distance(unmerge->begin(), unmerge->end());

    cout << endl << "Total: " << max_count <<
        (max_count == 1 ? " package" : " packages") << endl << endl;

    if (! ok)
        return 1 | return_code;

    if (CommandLine::get_instance()->a_pretend.specified())
        return return_code;

    if (! CommandLine::get_instance()->a_preserve_world.specified())
    {
        p::AllDepAtom::Pointer all(new p::AllDepAtom);
        for (std::list<p::PackageDepAtom::Pointer>::const_iterator t(targets.begin()),
                t_end(targets.end()) ; t != t_end ; ++t)
            all->add_child(*t);

        env->remove_appropriate_from_world(all);
    }

    p::InstallOptions opts(false, false);
    if (CommandLine::get_instance()->a_no_config_protection.specified())
        opts.set<p::io_noconfigprotect>(true);

    env->perform_hook("uninstall_all_pre");
    for (p::PackageDatabaseEntryCollection::Iterator pkg(unmerge->begin()), pkg_end(unmerge->end()) ;
            pkg != pkg_end ; ++pkg)
    {
        std::string cpv = p::stringify(pkg->get<p::pde_name>()) + "-" +
            p::stringify(pkg->get<p::pde_version>());

        cout << endl << colour(cl_heading,
                "Uninstalling " + cpv) << endl << endl;

        // TODO: some way to reset this properly would be nice.
        cerr << xterm_title("(" + p::stringify(++current_count) + " of " +
                p::stringify(max_count) + ") Uninstalling " + cpv);

        env->perform_hook("uninstall_pre");
        env->package_database()->fetch_repository(pkg->get<p::pde_repository>())->
            uninstall(pkg->get<p::pde_name>(), pkg->get<p::pde_version>(), opts);
        env->perform_hook("uninstall_post");
    }
    env->perform_hook("uninstall_all_post");

    return return_code;
}

