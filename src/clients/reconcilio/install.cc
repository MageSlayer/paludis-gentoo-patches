/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
 * Copyright (c) 2007 David Leverton <levertond@googlemail.com>
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

#include <paludis/util/sequence-impl.hh>
#include <paludis/util/tr1_functional.hh>

#include <src/output/console_install_task.hh>

#include <algorithm>
#include <iostream>

using namespace paludis;

namespace
{
    class OurInstallTask :
        public ConsoleInstallTask
    {
        private:
            tr1::shared_ptr<Environment> _env;

        public:
            OurInstallTask(const tr1::shared_ptr<Environment> & env, const DepListOptions & options,
                    const tr1::shared_ptr<const DestinationsSet> & destinations) :
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
                std::string resume_command = environment()->paludis_command() + " --install";

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
do_install(const tr1::shared_ptr<Environment> & env, const tr1::shared_ptr<const Sequence<std::string> > & targets)
{
    using namespace tr1::placeholders;

    DepListOptions options;
    CommandLine::get_instance()->dl_args.populate_dep_list_options(env.get(), options);
    CommandLine::get_instance()->install_args.populate_dep_list_options(env.get(), options);

    OurInstallTask task(env, options, CommandLine::get_instance()->install_args.destinations(env.get()));
    CommandLine::get_instance()->install_args.populate_install_task(env.get(), task);
    CommandLine::get_instance()->dl_args.populate_install_task(env.get(), task);

    std::for_each(targets->begin(), targets->end(), tr1::bind(&InstallTask::add_target, tr1::ref(task), _1));

    std::cout << std::endl;
    task.execute();
    std::cout << std::endl;

    return task.exit_status();
}

