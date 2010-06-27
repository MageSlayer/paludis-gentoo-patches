/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_CMD_RESOLVE_CMDLINE_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_CMD_RESOLVE_CMDLINE_HH 1

#include "command_command_line.hh"
#include <paludis/environment-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace cave
    {
        struct ResolveCommandLineResolutionOptions :
            args::ArgsSection
        {
            ResolveCommandLineResolutionOptions(args::ArgsHandler * const);

            args::ArgsGroup g_execution_options;
            args::SwitchArg a_execute;

            args::ArgsGroup g_convenience_options;
            args::SwitchArg a_lazy;
            args::SwitchArg a_complete;
            args::SwitchArg a_everything;

            args::ArgsGroup g_resolution_options;
//            args::SwitchArg a_permit_older_slot_uninstalls;
            args::StringSetArg a_permit_uninstall;
            args::StringSetArg a_permit_downgrade;
            args::StringSetArg a_permit_old_version;
            args::StringSetArg a_purge;

            args::ArgsGroup g_dependent_options;
            args::StringSetArg a_uninstalls_may_break;
            args::StringSetArg a_remove_if_dependent;
            args::StringSetArg a_less_restrictive_remove_blockers;

            args::ArgsGroup g_keep_options;
            args::EnumArg a_keep_targets;
            args::EnumArg a_keep;
            args::EnumArg a_reinstall_scm;
            args::StringSetArg a_with;
            args::StringSetArg a_without;

            args::ArgsGroup g_slot_options;
            args::EnumArg a_target_slots;
            args::EnumArg a_slots;

            args::ArgsGroup g_dependency_options;
            args::SwitchArg a_follow_installed_build_dependencies;
            args::SwitchArg a_no_follow_installed_dependencies;
            args::StringSetArg a_no_dependencies_from;
            args::StringSetArg a_no_blockers_from;

            args::ArgsGroup g_suggestion_options;
            args::EnumArg a_suggestions;
            args::EnumArg a_recommendations;
            args::StringSetArg a_take;
            args::StringSetArg a_take_from;
            args::StringSetArg a_ignore;
            args::StringSetArg a_ignore_from;

            args::ArgsGroup g_package_options;
            args::StringSetArg a_favour;
            args::StringSetArg a_avoid;

//            args::ArgsGroup g_ordering_options;
//            args::StringSetArg a_early;
//            args::StringSetArg a_late;
//
            args::ArgsGroup g_preset_options;
            args::StringSetArg a_preset;

            args::ArgsGroup g_destination_options;
//            args::SwitchArg a_fetch;
            args::SwitchArg a_create_binaries_for_targets;
//            args::StringSetArg a_install_via_binary;
//            args::StringSetArg a_no_binaries_for;

//
//            args::ArgsGroup g_query_options;
//            args::SwitchArg a_query;
//            args::SwitchArg a_query_slots;
//            args::SwitchArg a_query_decisions;
//            args::SwitchArg a_query_order;

            args::ArgsGroup g_dump_options;
            args::SwitchArg a_dump;
            args::SwitchArg a_dump_restarts;

            void apply_shortcuts();
            void verify(const std::tr1::shared_ptr<const Environment> & env);
        };

        struct ResolveCommandLineExecutionOptions :
            args::ArgsSection
        {
            ResolveCommandLineExecutionOptions(args::ArgsHandler * const);

            args::ArgsGroup g_world_options;
            args::SwitchArg a_preserve_world;

            args::ArgsGroup g_failure_options;
            args::EnumArg a_continue_on_failure;
            args::StringArg a_resume_file;

            args::ArgsGroup g_phase_options;
            args::StringSetArg a_skip_phase;
            args::StringSetArg a_abort_at_phase;
            args::StringSetArg a_skip_until_phase;
            args::EnumArg a_change_phases_for;
        };

        struct ResolveCommandLineDisplayOptions :
            args::ArgsSection
        {
            ResolveCommandLineDisplayOptions(args::ArgsHandler * const);

            args::ArgsGroup g_display_options;
            args::EnumArg a_show_option_descriptions;
            args::EnumArg a_show_descriptions;

            args::ArgsGroup g_explanations;
            args::StringSetArg a_explain;
        };

        struct ResolveCommandLineProgramOptions :
            args::ArgsSection
        {
            ResolveCommandLineProgramOptions(args::ArgsHandler * const);

            args::ArgsGroup g_program_options;
            args::StringArg a_display_resolution_program;
            args::StringArg a_execute_resolution_program;
            args::StringArg a_perform_program;
            args::StringArg a_update_world_program;
        };

        struct ResolveCommandLineImportOptions :
            args::ArgsSection
        {
            ResolveCommandLineImportOptions(args::ArgsHandler * const);

            args::ArgsGroup g_import_options;
            args::StringSetArg a_unpackaged_repository_params;

            void apply(const std::tr1::shared_ptr<Environment> & env) const;
        };
    }
}

#endif
