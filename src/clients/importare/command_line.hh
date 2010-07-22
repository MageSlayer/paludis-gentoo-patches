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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_IMPORTARE_COMMAND_LINE_HH
#define PALUDIS_GUARD_SRC_CLIENTS_IMPORTARE_COMMAND_LINE_HH 1

#include <paludis/args/args.hh>
#include <paludis/util/singleton.hh>
#include <paludis/args/dep_list_args_group.hh>
#include <paludis/args/install_args_group.hh>
#include <paludis/args/log_level_arg.hh>

class CommandLine :
    public paludis::args::ArgsHandler,
    public paludis::Singleton<CommandLine>
{
    friend class paludis::Singleton<CommandLine>;

    private:
        CommandLine();
        ~CommandLine();

    public:
        virtual std::string app_name() const;
        virtual std::string app_synopsis() const;
        virtual std::string app_description() const;

        paludis::args::ArgsGroup action_args;
        paludis::args::SwitchArg a_install;
        paludis::args::SwitchArg a_version;
        paludis::args::SwitchArg a_help;

        paludis::args::ArgsGroup general_args;
        paludis::args::LogLevelArg a_log_level;
        paludis::args::SwitchArg a_no_colour;
        paludis::args::AliasArg a_no_color;
        paludis::args::SwitchArg a_force_colour;
        paludis::args::AliasArg a_force_color;
        paludis::args::StringArg a_environment;
        paludis::args::SwitchArg a_compact;

        paludis::args::ArgsGroup source_args;
        paludis::args::StringArg a_location;
        paludis::args::StringArg a_install_under;
        paludis::args::IntegerArg a_rewrite_ids_over_to_root;

        paludis::args::ArgsGroup metadata_args;
        paludis::args::StringArg a_description;
        paludis::args::StringSetArg a_build_dependency;
        paludis::args::StringSetArg a_run_dependency;
        paludis::args::SwitchArg a_preserve_metadata;

        paludis::args::InstallArgsGroup install_args;
        paludis::args::DepListArgsGroup dl_args;
};

#endif
