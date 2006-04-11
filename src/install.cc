/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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
#include "src/install.hh"
#include <iostream>
#include <paludis/paludis.hh>

/** \file
 * Handle the --install action for the main paludis program.
 */

namespace p = paludis;

using std::cerr;
using std::cout;
using std::endl;

int
do_install()
{
    int return_code(0);

    p::Context context("When performing install action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    cout << colour(cl_heading, "These packages will be installed:") << endl << endl;

    p::CompositeDepAtom::Pointer targets(new p::AllDepAtom);

    p::DepList dep_list(env);

    try
    {
        CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
            q_end(CommandLine::get_instance()->end_parameters());

        bool had_set_targets(false), had_pkg_targets(false);
        for ( ; q != q_end ; ++q)
        {
            if (*q == "system" || *q == "everything")
            {
                if (had_set_targets)
                    throw DoHelp("You should not specify more than one set target.");

                had_set_targets = true;

                for (p::PackageDatabase::RepositoryIterator r(
                            env->package_database()->begin_repositories()),
                        r_end(env->package_database()->end_repositories()) ; r != r_end ; ++r)
                    targets->add_child((*r)->package_set(*q));
            }
            else
            {
                had_pkg_targets = true;

                /* we might have a dep atom, but we might just have a simple package name
                 * without a category. either should work. also allow full atoms, to make
                 * it easy to test things like '|| ( foo/bar foo/baz )'. */
                if (std::string::npos != q->find('/'))
                    targets->add_child(p::DepParser::parse(*q));
                else
                    targets->add_child(p::DepAtom::Pointer(new p::PackageDepAtom(
                                    env->package_database()->fetch_unique_qualified_package_name(
                                        p::PackageNamePart(*q)))));
            }
        }

        if (had_set_targets && had_pkg_targets)
            throw DoHelp("You should not specify set and package targets at the same time.");

        if (had_set_targets)
            dep_list.set_reinstall(false);
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
        return 1;
    }

    dep_list.set_drop_self_circular(CommandLine::get_instance()->a_dl_drop_self_circular.specified());
    dep_list.set_drop_circular(CommandLine::get_instance()->a_dl_drop_circular.specified());
    dep_list.set_drop_all(CommandLine::get_instance()->a_dl_drop_all.specified());
    dep_list.set_ignore_installed(CommandLine::get_instance()->a_dl_ignore_installed.specified());
    dep_list.set_recursive_deps(CommandLine::get_instance()->a_dl_recursive_deps.specified());
    dep_list.set_max_stack_depth(CommandLine::get_instance()->a_dl_max_stack_depth.argument());

    if (CommandLine::get_instance()->a_dl_rdepend_post.argument() == "always")
        dep_list.set_rdepend_post(p::dlro_always);
    else if (CommandLine::get_instance()->a_dl_rdepend_post.argument() == "never")
        dep_list.set_rdepend_post(p::dlro_never);
    else
        dep_list.set_rdepend_post(p::dlro_as_needed);

    try
    {
        dep_list.add(targets);

        for (p::DepList::Iterator dep(dep_list.begin()), dep_end(dep_list.end()) ;
                dep != dep_end ; ++dep)
        {
            Context loop_context("When displaying DepList entry '" + stringify(*dep) + "':");

            /* display name */
            cout << "* " << colour(cl_package_name, dep->get<p::dle_name>());

            /* display version, unless it's 0 and our category is "virtual" */
            if ((p::VersionSpec("0") != dep->get<p::dle_version>()) ||
                    p::CategoryNamePart("virtual") != dep->get<p::dle_name>().get<p::qpn_category>())
                cout << "-" << dep->get<p::dle_version>();

            /* display repository, unless it's our main repository */
            if (env->package_database()->favourite_repository() != dep->get<p::dle_repository>())
                cout << "::" << dep->get<p::dle_repository>();

            /* display slot name, unless it's 0 */
            if ("0" != dep->get<p::dle_metadata>()->get(p::vmk_slot))
                cout << colour(cl_slot, " {:" + p::stringify(
                            dep->get<p::dle_metadata>()->get(p::vmk_slot)) + "}");

            /* indicate [U], [S] or [N]. display existing version, if we're
             * already installed */
            p::PackageDatabaseEntryCollection::Pointer existing(env->package_database()->
                    query(p::PackageDepAtom::Pointer(new p::PackageDepAtom(p::stringify(
                                    dep->get<p::dle_name>()))), p::is_installed_only));

            if (existing->empty())
                cout << colour(cl_updatemode, " [N]");
            else
            {
                existing = env->package_database()->query(p::PackageDepAtom::Pointer(
                            new p::PackageDepAtom(p::stringify(dep->get<p::dle_name>()) + ":" +
                                dep->get<p::dle_metadata>()->get(vmk_slot))),
                        p::is_installed_only);
                if (existing->empty())
                    cout << colour(cl_updatemode, " [S]");
                else if (existing->last()->get<p::pde_version>() < dep->get<p::dle_version>())
                    cout << colour(cl_updatemode, " [U " + stringify(
                                existing->last()->get<pde_version>()) + "]");
                else if (existing->last()->get<p::pde_version>() > dep->get<p::dle_version>())
                    cout << colour(cl_updatemode, " [D " + stringify(
                                existing->last()->get<pde_version>()) + "]");
                else
                    cout << colour(cl_updatemode, " [R]");
            }

            /* fetch db entry */
            p::PackageDatabaseEntry p(p::PackageDatabaseEntry(dep->get<p::dle_name>(),
                        dep->get<p::dle_version>(), dep->get<p::dle_repository>()));

            /* display USE flags */
            for (p::VersionMetadata::IuseIterator i(dep->get<p::dle_metadata>()->begin_iuse()),
                    i_end(dep->get<p::dle_metadata>()->end_iuse()) ; i != i_end ; ++i)
            {
                if (env->query_use(*i, &p))
                    cout << " " << colour(cl_flag_on, *i);
                else if (env->package_database()->fetch_repository(
                            dep->get<p::dle_repository>())->query_use_mask(*i))
                    cout << " " << colour(cl_flag_off, "(-" + p::stringify(*i) + ")");
                else
                    cout << " " << colour(cl_flag_off, "-" + p::stringify(*i));
            }

            cout << endl;
        }

        int current_count = 0, max_count = std::distance(dep_list.begin(), dep_list.end());

        cout << endl << "Total: " << max_count <<
            (max_count == 1 ? " package" : " packages") << endl << endl;

        if (CommandLine::get_instance()->a_pretend.specified())
            return return_code;

        for (p::DepList::Iterator dep(dep_list.begin()), dep_end(dep_list.end()) ;
                dep != dep_end ; ++dep)
        {
            std::string cpv = p::stringify(dep->get<p::dle_name>()) + "-" +
                p::stringify(dep->get<p::dle_version>());

            cout << endl << colour(cl_heading,
                    "Installing " + cpv) << endl << endl;

            // TODO: some way to reset this properly would be nice.
            cerr << xterm_title("(" + p::stringify(++current_count) + " of " +
                    p::stringify(max_count) + ") Installing " + cpv);


            env->package_database()->fetch_repository(dep->get<p::dle_repository>())->
                install(dep->get<p::dle_name>(), dep->get<p::dle_version>());
        }
    }
    catch (const p::NoSuchPackageError & e)
    {
        cout << endl;
        cerr << "Query error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ");
        cerr << "No such package '" << e.name() << "'" << endl;
        return 1;
    }
    catch (const p::AllMaskedError & e)
    {
        try
        {
            p::PackageDatabaseEntryCollection::ConstPointer p(env->package_database()->query(
                        p::PackageDepAtom::ConstPointer(new p::PackageDepAtom(e.query())),
                        p::is_uninstalled_only));
            if (p->empty())
            {
                cout << endl;
                cerr << "Query error:" << endl;
                cerr << "  * " << e.backtrace("\n  * ");
                cerr << "All versions of '" << e.query() << "' are masked" << endl;
            }
            else
            {
                cout << endl;
                cerr << "Query error:" << endl;
                cerr << "  * " << e.backtrace("\n  * ");
                cerr << "All versions of '" << e.query() << "' are masked. Candidates are:" << endl;
                for (p::PackageDatabaseEntryCollection::Iterator pp(p->begin()), pp_end(p->end()) ;
                        pp != pp_end ; ++pp)
                {
                    cerr << "    * " << colour(cl_package_name, *pp) << ": Masked by ";

                    bool need_comma(false);
                    p::MaskReasons m(env->mask_reasons(*pp));
                    for (unsigned mm = 0 ; mm < m.size() ; ++mm)
                        if (m[mm])
                        {
                            if (need_comma)
                                cerr << ", ";
                            cerr << p::MaskReason(mm);
                            need_comma = true;
                        }
                    cerr << endl;
                }
            }
        }
        catch (...)
        {
            throw e;
        }

        return 1;
    }

    catch (const p::DepListStackTooDeepError & e)
    {
        cout << endl;
        cerr << "DepList stack too deep error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ") << e.message() << endl;
        cerr << endl;
        cerr << "Try '--dl-max-stack-depth " << std::max(
                CommandLine::get_instance()->a_dl_max_stack_depth.argument() * 2, 100)
            << "'." << endl << endl;
    }
    catch (const p::DepListError & e)
    {
        cout << endl;
        cerr << "Dependency error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ") << e.message() << " ("
            << e.what() << ")" << endl;
        cerr << endl;

        return_code |= 1;
    }

    return return_code;
}
