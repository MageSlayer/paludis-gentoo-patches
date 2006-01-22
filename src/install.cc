/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#include "src/install.hh"
#include "src/colour.hh"
#include <paludis/paludis.hh>
#include <iostream>

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

    p::CompositeDepAtom::Pointer targets(new p::AllDepAtom);

    CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());
    for ( ; q != q_end ; ++q)
    {
        /* we might have a dep atom, but we might just have a simple package name
         * without a category. either should work. also allow full atoms, to make
         * it easy to test things like '|| ( foo/bar foo/baz )'. */
        if (std::string::npos != q->find_first_of(" \t\n"))
            targets->add_child(p::DepParser::parse(*q));
        else if (std::string::npos == q->find('/'))
            targets->add_child(p::DepAtom::Pointer(new p::PackageDepAtom(
                            env->package_database()->fetch_unique_qualified_package_name(
                                p::PackageNamePart(*q)))));
        else
            targets->add_child(p::DepAtom::Pointer(new p::PackageDepAtom(*q)));
    }

    p::DepList dep_list(env);
    dep_list.set_drop_self_circular(CommandLine::get_instance()->a_dl_drop_self_circular.specified());
    dep_list.set_drop_circular(CommandLine::get_instance()->a_dl_drop_circular.specified());
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
            if (p::SlotName("0") != dep->get<p::dle_slot>())
                cout << colour(cl_slot, " {:" + stringify(dep->get<p::dle_slot>()) + "}");

            /* fetch metadata */
            p::PackageDatabaseEntry p(p::PackageDatabaseEntry(dep->get<p::dle_name>(),
                        dep->get<p::dle_version>(), dep->get<p::dle_repository>()));
            p::VersionMetadata::ConstPointer metadata(env->package_database()->fetch_metadata(p));

            /* display USE flags */
            for (p::VersionMetadata::IuseIterator i(metadata->begin_iuse()),
                    i_end(metadata->end_iuse()) ; i != i_end ; ++i)
            {
                if (env->query_use(*i, p))
                    cout << " " << colour(cl_flag_on, *i);
                else if (env->package_database()->fetch_repository(dep->get<p::dle_repository>())->query_use_mask(*i))
                    cout << " " << colour(cl_flag_off, "(-" + p::stringify(*i) + ")");
                else
                    cout << " " << colour(cl_flag_off, "-" + p::stringify(*i));
            }

            cout << endl;
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

        dep_list.set_drop_self_circular(true);
        try
        {
            dep_list.add(targets);
            cerr << "Adding --dl-drop-self-circular will resolve this, but may omit some genuine"
                << endl << "dependencies." << endl << endl;
        }
        catch (...)
        {
            dep_list.set_drop_circular(true);
            try
            {
                dep_list.add(targets);
                cerr << "Adding --dl-drop-circular will resolve this, but may omit some genuine" << endl
                    << "dependencies." << endl << endl;
            }
            catch (...)
            {
            }
        }

        return_code |= 1;
    }

    return return_code;
}
