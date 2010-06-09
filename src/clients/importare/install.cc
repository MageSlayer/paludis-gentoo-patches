/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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
#include "command_line.hh"
#include <src/output/console_install_task.hh>
#include <paludis/args/do_help.hh>

#include <iostream>
#include <cstdlib>
#include <cstring>

#include <paludis/install_task.hh>
#include <paludis/tasks_exceptions.hh>

#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>

#include <paludis/hook.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/action.hh>
#include <paludis/name.hh>
#include <paludis/choice.hh>

using namespace paludis;

using std::cerr;
using std::cout;
using std::endl;

namespace
{
    class OurInstallTask :
        public ConsoleInstallTask
    {
        private:
            std::tr1::shared_ptr<Environment> _env;

        public:
            OurInstallTask(std::tr1::shared_ptr<Environment> env, const DepListOptions & options,
                    std::tr1::shared_ptr<const DestinationsSet> destinations) :
                ConsoleInstallTask(env.get(), options, destinations),
                _env(env)
            {
            }

            virtual bool want_full_install_reasons() const
            {
                return CommandLine::get_instance()->install_args.want_full_install_reasons();
            }

            virtual bool want_tags_summary() const
            {
                return CommandLine::get_instance()->install_args.want_tags_summary();
            }

            virtual bool want_install_reasons() const
            {
                return CommandLine::get_instance()->install_args.want_install_reasons();
            }

            virtual bool want_unchanged_use_flags() const
            {
                return CommandLine::get_instance()->install_args.want_unchanged_use_flags();
            }

            virtual bool want_changed_use_flags() const
            {
                return CommandLine::get_instance()->install_args.want_changed_use_flags();
            }

            virtual bool want_new_use_flags() const
            {
                return CommandLine::get_instance()->install_args.want_new_use_flags();
            }

            virtual bool want_use_summary() const
            {
                return CommandLine::get_instance()->install_args.want_use_summary();
            }

            virtual bool want_compact() const
            {
                return CommandLine::get_instance()->a_compact.specified();
            }

            virtual bool want_suggestions() const
            {
                return false;
            }

            virtual bool want_new_descriptions() const
            {
                return CommandLine::get_instance()->install_args.want_new_descriptions();
            }

            virtual bool want_existing_descriptions() const
            {
                return CommandLine::get_instance()->install_args.want_existing_descriptions();
            }

            virtual std::string make_resume_command(const bool) const
            {
                return "";
            }

            void show_resume_command() const
            {
            }
    };
}

int
do_install(const std::tr1::shared_ptr<Environment> & env, const std::tr1::shared_ptr<const PackageID> & target)
{
    Context context("When performing install action from command line:");

    CommandLine::get_instance()->install_args.a_add_to_world_spec.set_specified(true);
    CommandLine::get_instance()->install_args.a_add_to_world_spec.set_argument(stringify(target->name()));

    DepListOptions options;
    CommandLine::get_instance()->dl_args.populate_dep_list_options(env.get(), options);
    CommandLine::get_instance()->install_args.populate_dep_list_options(env.get(), options);

    OurInstallTask task(env, options, CommandLine::get_instance()->install_args.destinations(env.get()));
    CommandLine::get_instance()->install_args.populate_install_task(env.get(), task);
    CommandLine::get_instance()->dl_args.populate_install_task(env.get(), task);

    std::tr1::shared_ptr<PackageIDSequence> targets(new PackageIDSequence);
    targets->push_back(target);
    task.set_targets_from_exact_packages(targets);
    task.execute();

    cout << endl;

    return task.exit_status();
}


