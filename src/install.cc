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

    CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());
    for ( ; q != q_end ; ++q)
    {
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

    p::DepList dep_list(env);
    dep_list.set_drop_self_circular(CommandLine::get_instance()->a_dl_drop_self_circular.specified());
    dep_list.set_dont_ignore_patch_dep(CommandLine::get_instance()->a_dl_patch_dep.specified());
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
                cout << colour(cl_slot, " {:" + p::stringify(dep->get<p::dle_metadata>()->get(p::vmk_slot)) + "}");

            /* fetch db entry */
            p::PackageDatabaseEntry p(p::PackageDatabaseEntry(dep->get<p::dle_name>(),
                        dep->get<p::dle_version>(), dep->get<p::dle_repository>()));

            /* display USE flags */
            for (p::VersionMetadata::IuseIterator i(dep->get<p::dle_metadata>()->begin_iuse()),
                    i_end(dep->get<p::dle_metadata>()->end_iuse()) ; i != i_end ; ++i)
            {
                if (env->query_use(*i, &p))
                    cout << " " << colour(cl_flag_on, *i);
                else if (env->package_database()->fetch_repository(dep->get<p::dle_repository>())->query_use_mask(*i))
                    cout << " " << colour(cl_flag_off, "(-" + p::stringify(*i) + ")");
                else
                    cout << " " << colour(cl_flag_off, "-" + p::stringify(*i));
            }

            cout << endl;
        }

        if (CommandLine::get_instance()->a_pretend.specified())
            return return_code;

        int current_count = 0, max_count = std::distance(dep_list.begin(), dep_list.end());
        
        for (p::DepList::Iterator dep(dep_list.begin()), dep_end(dep_list.end()) ;
                dep != dep_end ; ++dep)
        {
            std::string cpv = p::stringify(dep->get<p::dle_name>()) + "-" + p::stringify(dep->get<p::dle_version>());

            cout << endl << colour(cl_heading,
                    "Installing " + cpv) << endl << endl;

            // TODO: some way to reset this properly would be nice.
            cerr << xterm_title("(" + p::stringify(++current_count) + " of " + p::stringify(max_count) + ") Installing " + cpv);


            env->package_database()->fetch_repository(dep->get<p::dle_repository>())->
                install(dep->get<p::dle_name>(), dep->get<p::dle_version>());
        }
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
