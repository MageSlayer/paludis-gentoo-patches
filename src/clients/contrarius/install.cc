/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "install.hh"
#include <src/output/colour.hh>
#include <src/output/console_install_task.hh>

#include <iostream>
#include <limits>
#include <set>

#include <paludis/tasks/install_task.hh>
#include <paludis/util/fd_output_stream.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/sequence.hh>
#include <paludis/environment.hh>
#include <paludis/dep_list/exceptions.hh>
#include <paludis/query.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/action.hh>
#include <paludis/package_id.hh>

/** \file
 * Handle the --install action for the contrarius program.
 */

using namespace paludis;

using std::cerr;
using std::cout;
using std::endl;

namespace
{
    class OurInstallTask :
        public ConsoleInstallTask
    {
        public:
            OurInstallTask(tr1::shared_ptr<Environment> env, const DepListOptions & options) :
                ConsoleInstallTask(env.get(), options, env->default_destinations())
            {
            }

            virtual bool want_full_install_reasons() const
            {
                return "full" == CommandLine::get_instance()->a_show_reasons.argument();
            }

            virtual bool want_tags_summary() const
            {
                return CommandLine::get_instance()->a_pretend.specified();
            }

            virtual bool want_install_reasons() const
            {
                return "full" == CommandLine::get_instance()->a_show_reasons.argument() ||
                    "summary" == CommandLine::get_instance()->a_show_reasons.argument();
            }

            virtual bool want_unchanged_use_flags() const
            {
                return false;
            }

            virtual bool want_changed_use_flags() const
            {
                return false;
            }

            virtual bool want_new_use_flags() const
            {
                return false;
            }

            virtual bool want_use_summary() const
            {
                return false;
            }

            virtual void show_resume_command() const
            {
                if (CommandLine::get_instance()->a_fetch.specified() ||
                        CommandLine::get_instance()->a_pretend.specified())
                    return;

                const tr1::shared_ptr<const PackageIDSequence> p(packages_not_yet_installed_successfully());
                if (! p->empty())
                {
                    std::string resume_command = environment()->paludis_command() + " "
                        "--dl-installed-deps-pre discard "
                        "--dl-installed-deps-runtime discard "
                        "--dl-installed-deps-post discard "
                        "--dl-uninstalled-deps-pre discard "
                        "--dl-uninstalled-deps-runtime discard "
                        "--dl-uninstalled-deps-post discard "
                        "--install --preserve-world";
                    for (PackageIDSequence::ConstIterator i(p->begin()), i_end(p->end()) ; i != i_end ; ++i)
                        resume_command = resume_command + " '=" + stringify(**i) + "'";

                    if (CommandLine::get_instance()->a_resume_command_template.specified())
                    {
                        std::string file_name(CommandLine::get_instance()->a_resume_command_template.argument());
                        char* resume_template = strdup(file_name.c_str());
                        FDOutputStream resume_command_file(mkstemp(resume_template));
                        cerr << endl;
                        cerr << "Resume command saved to file: " << resume_template;
                        cerr << endl;
                        resume_command_file << resume_command << endl;
                        std::free(resume_template);
                    }
                    else
                        cerr << "Resume command: " << resume_command << endl;
                }
            }
    };
}

int
do_install(tr1::shared_ptr<Environment> env, std::string spec_str)
{
    int return_code(0);

    Context context("When performing install action from command line:");

    DepListOptions options;

    options.circular = dl_circular_discard;
    options.circular = dl_circular_error;
    options.installed_deps_pre = dl_deps_discard;
    options.installed_deps_runtime = dl_deps_discard;
    options.installed_deps_post = dl_deps_discard;
    options.uninstalled_deps_pre = dl_deps_discard;
    options.uninstalled_deps_runtime = dl_deps_discard;
    options.uninstalled_deps_post = dl_deps_discard;
    options.reinstall = dl_reinstall_never;
    options.target_type = dl_target_set;

    OurInstallTask task(env, options);
    task.set_fetch_only(CommandLine::get_instance()->a_fetch.specified());
    task.set_pretend(CommandLine::get_instance()->a_pretend.specified());

    if (CommandLine::get_instance()->a_debug_build.specified())
    {
        if (CommandLine::get_instance()->a_debug_build.argument() == "none")
            task.set_debug_mode(iado_none);
        else if (CommandLine::get_instance()->a_debug_build.argument() == "split")
            task.set_debug_mode(iado_split);
        else if (CommandLine::get_instance()->a_debug_build.argument() == "internal")
            task.set_debug_mode(iado_internal);
        else
            throw DoHelp("bad value for --debug-build");
    }

    task.add_target(spec_str);

    task.execute();

    cout << endl;

    if (task.dep_list().has_errors())
        return_code |= 1;

    if (task.had_resolution_failures())
        return_code |= 3;

    if (task.had_action_failures())
        return_code |= 7;

    return return_code;
}

