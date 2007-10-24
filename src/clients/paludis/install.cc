/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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
#include <paludis/query.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/action.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

/** \file
 * Handle the --install action for the main paludis program.
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
        private:
            tr1::shared_ptr<Environment> _env;

        public:
            OurInstallTask(tr1::shared_ptr<Environment> env, const DepListOptions & options,
                    tr1::shared_ptr<const DestinationsSet> destinations) :
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

            virtual std::string make_resume_command(const PackageIDSequence & s) const
            {
                std::string resume_command = environment()->paludis_command()
                    + " --" + CommandLine::get_instance()->a_install.long_name();

                resume_command = resume_command + CommandLine::get_instance()->install_args.resume_command_fragment(*this);
                resume_command = resume_command + CommandLine::get_instance()->dl_args.resume_command_fragment(*this);

                for (PackageIDSequence::ConstIterator i(s.begin()), i_end(s.end()) ;
                        i != i_end ; ++i)
                    resume_command = resume_command + " '=" + stringify(**i) + "'";

                return resume_command;
            }

            void show_resume_command() const
            {
                if (CommandLine::get_instance()->install_args.a_fetch.specified() ||
                        CommandLine::get_instance()->install_args.a_pretend.specified())
                    return;

                ConsoleInstallTask::show_resume_command(CommandLine::get_instance()->a_resume_command_template.argument());
            }
    };
}

int
do_install(tr1::shared_ptr<Environment> env)
{
    Context context("When performing install action from command line:");

    DepListOptions options;
    CommandLine::get_instance()->dl_args.populate_dep_list_options(env.get(), options);
    CommandLine::get_instance()->install_args.populate_dep_list_options(env.get(), options);

    OurInstallTask task(env, options, CommandLine::get_instance()->install_args.destinations(env.get()));
    CommandLine::get_instance()->install_args.populate_install_task(env.get(), task);
    CommandLine::get_instance()->dl_args.populate_install_task(env.get(), task);

    cout << "Building target list... " << std::flush;
    for (CommandLine::ParametersConstIterator q(CommandLine::get_instance()->begin_parameters()),
            q_end(CommandLine::get_instance()->end_parameters()) ; q != q_end ; ++q)
        if (! task.try_to_add_target(*q))
            return task.exit_status();

    cout << endl;

    task.execute();

    cout << endl;

    return task.exit_status();
}

