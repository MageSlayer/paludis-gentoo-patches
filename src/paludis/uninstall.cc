/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "colour.hh"
#include "uninstall.hh"

#include <paludis/environment/default/default_environment.hh>
#include <paludis/tasks/uninstall_task.hh>
#include <paludis/dep_list/uninstall_list.hh>

#include <iostream>

/** \file
 * Handle the --uninstall action for the main paludis program.
 */

using namespace paludis;

using std::cerr;
using std::cout;
using std::endl;

namespace
{
    class OurUninstallTask :
        public UninstallTask
    {
        private:
            int _count, _current_count;

        public:
            OurUninstallTask() :
                UninstallTask(DefaultEnvironment::get_instance()),
                _count(0),
                _current_count(0)
            {
            };

            virtual void on_build_unmergelist_pre()
            {
                cout << "Building unmerge list... " << std::flush;
            }

            virtual void on_build_unmergelist_post()
            {
                cout << "done" << endl;
            }

            virtual void on_display_unmerge_list_pre()
            {
                cout << endl << colour(cl_heading, "These packages will be uninstalled:")
                    << endl << endl;
            }

            virtual void on_display_unmerge_list_post()
            {
                cout << endl << endl <<
                    "Total: " << _count << (_count == 1 ? " package" : " packages") << endl;
            }

            virtual void on_display_unmerge_list_entry(const UninstallListEntry & d)
            {
                if (d.skip_uninstall)
                    if (CommandLine::get_instance()->a_show_install_reasons.argument() != "full")
                        return;

                cout << "* " << colour(d.skip_uninstall ? cl_unimportant : cl_package_name, stringify(d.package));
                ++_count;

                if ((CommandLine::get_instance()->a_show_install_reasons.argument() == "summary") ||
                        (CommandLine::get_instance()->a_show_install_reasons.argument() == "full"))
                {
                    std::string deps;
                    unsigned count(0), max_count;
                    if (CommandLine::get_instance()->a_show_install_reasons.argument() == "summary")
                        max_count = 3;
                    else
                        max_count = std::numeric_limits<long>::max();

                    for (SortedCollection<DepTag::Pointer>::Iterator
                            tag(d.tags->begin()),
                            tag_end(d.tags->end()) ;
                            tag != tag_end ; ++tag)
                    {
                        if ((*tag)->category() != "dependency")
                            continue;

                        if (++count < max_count)
                        {
                            deps.append((*tag)->short_text());
                            deps.append(", ");
                        }
                    }
                    if (! deps.empty())
                    {
                        if (count >= max_count)
                            deps.append(stringify(count - max_count + 1) + " more, ");

                        deps.erase(deps.length() - 2);
                        cout << " " << colour(d.skip_uninstall ? cl_unimportant : cl_tag,
                                "<" + deps + ">");
                    }
                }

                cout << endl;
            }

            virtual void on_uninstall_all_pre()
            {
            }

            virtual void on_uninstall_pre(const UninstallListEntry & d)
            {
                cout << endl << colour(cl_heading, "Uninstalling " +
                        stringify(d.package.name) + "-" + stringify(d.package.version) +
                        "::" + stringify(d.package.repository)) << endl << endl;

                cerr << xterm_title("(" + stringify(++_current_count) + " of " +
                        stringify(_count) + ") Uninstalling " +
                        stringify(d.package.name) + "-" + stringify(d.package.version) +
                        "::" + stringify(d.package.repository));
            }

            virtual void on_uninstall_post(const UninstallListEntry &)
            {
            }

            virtual void on_uninstall_all_post()
            {
            }

            virtual void on_update_world_pre()
            {
                cout << endl << colour(cl_heading, "Updating world file") << endl << endl;
            }

            virtual void on_update_world(const PackageDepAtom & a)
            {
                cout << "* removing " << colour(cl_package_name, a.package()) << endl;
            }

            virtual void on_update_world_post()
            {
                cout << endl;
            }

            virtual void on_preserve_world()
            {
                cout << endl << colour(cl_heading, "Updating world file") << endl << endl;
                cout << "* --preserve-world was specified, skipping world changes" << endl;
                cout << endl;
            }
    };

    int real_uninstall(bool unused)
    {
        int return_code(0);

        Context context(unused ?
                "When performing uninstall-unused action from command line:" :
                "When performing uninstall action from command line:");

        OurUninstallTask task;

        task.set_pretend(CommandLine::get_instance()->a_pretend.specified());
        task.set_no_config_protect(CommandLine::get_instance()->a_no_config_protection.specified());
        task.set_preserve_world(CommandLine::get_instance()->a_preserve_world.specified());
        task.set_with_unused_dependencies(CommandLine::get_instance()->a_with_unused_dependencies.specified());
        task.set_with_dependencies(CommandLine::get_instance()->a_with_dependencies.specified());

        try
        {
            if (unused)
                task.add_unused();
            else
                for (CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
                        q_end(CommandLine::get_instance()->end_parameters()) ; q != q_end ; ++q)
                    task.add_target(*q);

            task.execute();

            cout << endl;
        }
        catch (const AmbiguousUnmergeTargetError & e)
        {
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ");
            cerr << "Ambiguous unmerge target '" << e.target() << "'. Did you mean:" << endl;
            for (AmbiguousUnmergeTargetError::Iterator o(e.begin()),
                    o_end(e.end()) ; o != o_end ; ++o)
                cerr << "    * =" << colour(cl_package_name, *o) << endl;
            cerr << endl;
            return 1;
        }
        catch (const PackageUninstallActionError & e)
        {
            cout << endl;
            cerr << "Uninstall error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ");
            cerr << e.message() << endl;

            return_code |= 1;
        }
        catch (const NoSuchPackageError & e)
        {
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ");
            cerr << "No such package '" << e.name() << "'" << endl;
            return 1;
        }

        return return_code;
    }
}

int
do_uninstall()
{
    return real_uninstall(false);
}

int
do_uninstall_unused()
{
    return real_uninstall(true);
}

