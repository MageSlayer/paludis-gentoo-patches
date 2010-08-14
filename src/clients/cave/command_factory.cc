/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#include "command_factory.hh"
#include "script_command.hh"

#include <paludis/util/singleton-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/options.hh>
#include <paludis/util/system.hh>
#include <functional>
#include <vector>
#include <map>

#include "cmd_config.hh"
#include "cmd_contents.hh"
#include "cmd_display_resolution.hh"
#include "cmd_executables.hh"
#include "cmd_execute_resolution.hh"
#include "cmd_find_candidates.hh"
#include "cmd_fix_cache.hh"
#include "cmd_fix_linkage.hh"
#include "cmd_help.hh"
#include "cmd_import.hh"
#include "cmd_info.hh"
#include "cmd_manage_search_index.hh"
#include "cmd_match.hh"
#include "cmd_owner.hh"
#include "cmd_perform.hh"
#include "cmd_print_categories.hh"
#include "cmd_print_commands.hh"
#include "cmd_print_environment_metadata.hh"
#include "cmd_print_id_actions.hh"
#include "cmd_print_id_contents.hh"
#include "cmd_print_id_environment_variable.hh"
#include "cmd_print_id_executables.hh"
#include "cmd_print_id_masks.hh"
#include "cmd_print_id_metadata.hh"
#include "cmd_print_ids.hh"
#include "cmd_print_owners.hh"
#include "cmd_print_packages.hh"
#include "cmd_print_repositories.hh"
#include "cmd_print_repository_formats.hh"
#include "cmd_print_repository_metadata.hh"
#include "cmd_print_set.hh"
#include "cmd_print_sets.hh"
#include "cmd_print_sync_protocols.hh"
#include "cmd_purge.hh"
#include "cmd_report.hh"
#include "cmd_resolve.hh"
#include "cmd_resume.hh"
#include "cmd_search.hh"
#include "cmd_show.hh"
#include "cmd_sync.hh"
#include "cmd_uninstall.hh"
#include "cmd_update_world.hh"
#include "cmd_verify.hh"

using namespace paludis;
using namespace cave;

typedef std::map<std::string, std::function<const std::shared_ptr<cave::Command> ()> > Handlers;

namespace paludis
{
    template <>
    struct Imp<CommandFactory>
    {
        Handlers handlers;
    };

    template <>
    struct WrappedForwardIteratorTraits<CommandFactory::ConstIteratorTag>
    {
        typedef FirstIteratorTypes<Handlers::const_iterator>::Type UnderlyingIterator;
    };
}

namespace
{
    template <typename T_>
    const std::shared_ptr<T_> make_command()
    {
        return std::make_shared<T_>();
    }

    const std::shared_ptr<ScriptCommand> make_script_command(const std::string & s, const FSEntry & f)
    {
        return std::make_shared<ScriptCommand>(s, f);
    }
}

CommandFactory::CommandFactory() :
    Pimp<CommandFactory>()
{
    std::vector<std::string> paths;
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(getenv_with_default("CAVE_COMMANDS_PATH", LIBEXECDIR "/cave/commands"),
            ":", "", std::back_inserter(paths));
    for (std::vector<std::string>::const_iterator t(paths.begin()), t_end(paths.end()) ;
            t != t_end ; ++t)
    {
        FSEntry path(*t);
        if (! path.exists())
            continue;

        for (DirIterator s(path, { dio_inode_sort }), s_end ;
                s != s_end ; ++s)
        {
            if (s->is_regular_file_or_symlink_to_regular_file() && s->has_permission(fs_ug_others, fs_perm_execute))
            {
                std::string command_name(s->basename());
                std::string::size_type p(command_name.rfind('.'));
                if (std::string::npos != p)
                    command_name.erase(p);

                _imp->handlers.erase(command_name);
                _imp->handlers.insert(std::make_pair(command_name, std::bind(&make_script_command, command_name, *s)));
            }
        }
    }

    _imp->handlers.insert(std::make_pair("config", std::bind(&make_command<ConfigCommand>)));
    _imp->handlers.insert(std::make_pair("contents", std::bind(&make_command<ContentsCommand>)));
    _imp->handlers.insert(std::make_pair("display-resolution", std::bind(&make_command<DisplayResolutionCommand>)));
    _imp->handlers.insert(std::make_pair("executables", std::bind(&make_command<ExecutablesCommand>)));
    _imp->handlers.insert(std::make_pair("execute-resolution", std::bind(&make_command<ExecuteResolutionCommand>)));
    _imp->handlers.insert(std::make_pair("find-candidates", std::bind(&make_command<FindCandidatesCommand>)));
    _imp->handlers.insert(std::make_pair("fix-cache", std::bind(&make_command<FixCacheCommand>)));
    _imp->handlers.insert(std::make_pair("fix-linkage", std::bind(&make_command<FixLinkageCommand>)));
    _imp->handlers.insert(std::make_pair("help", std::bind(&make_command<HelpCommand>)));
    _imp->handlers.insert(std::make_pair("import", std::bind(&make_command<ImportCommand>)));
    _imp->handlers.insert(std::make_pair("info", std::bind(&make_command<InfoCommand>)));
    _imp->handlers.insert(std::make_pair("manage-search-index", std::bind(&make_command<ManageSearchIndexCommand>)));
    _imp->handlers.insert(std::make_pair("match", std::bind(&make_command<MatchCommand>)));
    _imp->handlers.insert(std::make_pair("owner", std::bind(&make_command<OwnerCommand>)));
    _imp->handlers.insert(std::make_pair("perform", std::bind(&make_command<PerformCommand>)));
    _imp->handlers.insert(std::make_pair("purge", std::bind(&make_command<PurgeCommand>)));
    _imp->handlers.insert(std::make_pair("print-categories", std::bind(&make_command<PrintCategoriesCommand>)));
    _imp->handlers.insert(std::make_pair("print-commands", std::bind(&make_command<PrintCommandsCommand>)));
    _imp->handlers.insert(std::make_pair("print-environment-metadata", std::bind(&make_command<PrintEnvironmentMetadataCommand>)));
    _imp->handlers.insert(std::make_pair("print-id-actions", std::bind(&make_command<PrintIDActionsCommand>)));
    _imp->handlers.insert(std::make_pair("print-id-contents", std::bind(&make_command<PrintIDContentsCommand>)));
    _imp->handlers.insert(std::make_pair("print-id-environment-variable", std::bind(&make_command<PrintIDEnvironmentVariableCommand>)));
    _imp->handlers.insert(std::make_pair("print-id-executables", std::bind(&make_command<PrintIDExecutablesCommand>)));
    _imp->handlers.insert(std::make_pair("print-id-masks", std::bind(&make_command<PrintIDMasksCommand>)));
    _imp->handlers.insert(std::make_pair("print-id-metadata", std::bind(&make_command<PrintIDMetadataCommand>)));
    _imp->handlers.insert(std::make_pair("print-ids", std::bind(&make_command<PrintIDsCommand>)));
    _imp->handlers.insert(std::make_pair("print-owners", std::bind(&make_command<PrintOwnersCommand>)));
    _imp->handlers.insert(std::make_pair("print-packages", std::bind(&make_command<PrintPackagesCommand>)));
    _imp->handlers.insert(std::make_pair("print-repositories", std::bind(&make_command<PrintRepositoriesCommand>)));
    _imp->handlers.insert(std::make_pair("print-repository-formats", std::bind(&make_command<PrintRepositoryFormatsCommand>)));
    _imp->handlers.insert(std::make_pair("print-repository-metadata", std::bind(&make_command<PrintRepositoryMetadataCommand>)));
    _imp->handlers.insert(std::make_pair("print-set", std::bind(&make_command<PrintSetCommand>)));
    _imp->handlers.insert(std::make_pair("print-sets", std::bind(&make_command<PrintSetsCommand>)));
    _imp->handlers.insert(std::make_pair("print-sync-protocols", std::bind(&make_command<PrintSyncProtocolsCommand>)));
    _imp->handlers.insert(std::make_pair("report", std::bind(&make_command<ReportCommand>)));
    _imp->handlers.insert(std::make_pair("resolve", std::bind(&make_command<ResolveCommand>)));
    _imp->handlers.insert(std::make_pair("resume", std::bind(&make_command<ResumeCommand>)));
    _imp->handlers.insert(std::make_pair("search", std::bind(&make_command<SearchCommand>)));
    _imp->handlers.insert(std::make_pair("show", std::bind(&make_command<ShowCommand>)));
    _imp->handlers.insert(std::make_pair("sync", std::bind(&make_command<SyncCommand>)));
    _imp->handlers.insert(std::make_pair("uninstall", std::bind(&make_command<UninstallCommand>)));
    _imp->handlers.insert(std::make_pair("update-world", std::bind(&make_command<UpdateWorldCommand>)));
    _imp->handlers.insert(std::make_pair("verify", std::bind(&make_command<VerifyCommand>)));
}

CommandFactory::~CommandFactory()
{
}

const std::shared_ptr<cave::Command>
CommandFactory::create(const std::string & s) const
{
    Handlers::const_iterator i(_imp->handlers.find(s));
    if (i == _imp->handlers.end())
        throw UnknownCommand(s);
    else
        return i->second();
}

CommandFactory::ConstIterator
CommandFactory::begin() const
{
    return first_iterator(_imp->handlers.begin());
}

CommandFactory::ConstIterator
CommandFactory::end() const
{
    return first_iterator(_imp->handlers.end());
}

UnknownCommand::UnknownCommand(const std::string & s) throw () :
    Exception("Unknown command '" + s + "'")
{
}

template class Singleton<CommandFactory>;
template class Pimp<CommandFactory>;
template class WrappedForwardIterator<CommandFactory::ConstIteratorTag, const std::string>;

