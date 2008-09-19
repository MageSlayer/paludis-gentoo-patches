/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
#include <src/output/console_install_task.hh>

#include <iostream>

#include <paludis/install_task.hh>
#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>
#include <paludis/environment.hh>
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
            OurInstallTask(std::tr1::shared_ptr<Environment> env, const DepListOptions & options) :
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

            virtual bool want_new_descriptions() const
            {
                return false;
            }

            virtual bool want_existing_descriptions() const
            {
                return false;
            }

            virtual bool want_compact() const
            {
                return false;
            }

            virtual bool want_suggestions() const
            {
                return false;
            }

            virtual std::string make_resume_command(const bool) const
            {
                return "";
            }

            virtual void show_resume_command() const
            {
            }
    };
}

int
do_install(const std::tr1::shared_ptr<Environment> & env, std::string spec_str)
{
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

    std::tr1::shared_ptr<Sequence<std::string> > specs(new Sequence<std::string>);
    specs->push_back(spec_str);
    if (! task.try_to_set_targets_from_user_specs(specs))
        return task.exit_status();

    task.execute();

    cout << endl;

    return task.exit_status();
}

