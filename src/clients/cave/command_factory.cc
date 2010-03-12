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
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <tr1/functional>
#include <map>

#include "cmd_config.hh"
#include "cmd_display_resolution.hh"
#include "cmd_execute_resolution.hh"
#include "cmd_find_candidates.hh"
#include "cmd_fix_cache.hh"
#include "cmd_help.hh"
#include "cmd_import.hh"
#include "cmd_info.hh"
#include "cmd_match.hh"
#include "cmd_perform.hh"
#include "cmd_print_categories.hh"
#include "cmd_print_commands.hh"
#include "cmd_print_environment_metadata.hh"
#include "cmd_print_id_contents.hh"
#include "cmd_print_id_executables.hh"
#include "cmd_print_id_metadata.hh"
#include "cmd_print_ids.hh"
#include "cmd_print_owners.hh"
#include "cmd_print_packages.hh"
#include "cmd_print_repositories.hh"
#include "cmd_print_repository_formats.hh"
#include "cmd_print_sets.hh"
#include "cmd_print_sync_protocols.hh"
#include "cmd_resolve.hh"
#include "cmd_show.hh"
#include "cmd_sync.hh"
#include "cmd_update_world.hh"

using namespace paludis;
using namespace cave;

typedef std::map<std::string, std::tr1::function<const std::tr1::shared_ptr<Command> ()> > Handlers;

namespace paludis
{
    template <>
    struct Implementation<CommandFactory>
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
    const std::tr1::shared_ptr<T_> make_command()
    {
        return make_shared_ptr(new T_);
    }
}

CommandFactory::CommandFactory() :
    PrivateImplementationPattern<CommandFactory>(new Implementation<CommandFactory>)
{
    _imp->handlers.insert(std::make_pair("config", std::tr1::bind(&make_command<ConfigCommand>)));
    _imp->handlers.insert(std::make_pair("display-resolution", std::tr1::bind(&make_command<DisplayResolutionCommand>)));
    _imp->handlers.insert(std::make_pair("execute-resolution", std::tr1::bind(&make_command<ExecuteResolutionCommand>)));
    _imp->handlers.insert(std::make_pair("find-candidates", std::tr1::bind(&make_command<FindCandidatesCommand>)));
    _imp->handlers.insert(std::make_pair("fix-cache", std::tr1::bind(&make_command<FixCacheCommand>)));
    _imp->handlers.insert(std::make_pair("help", std::tr1::bind(&make_command<HelpCommand>)));
    _imp->handlers.insert(std::make_pair("import", std::tr1::bind(&make_command<ImportCommand>)));
    _imp->handlers.insert(std::make_pair("info", std::tr1::bind(&make_command<InfoCommand>)));
    _imp->handlers.insert(std::make_pair("match", std::tr1::bind(&make_command<MatchCommand>)));
    _imp->handlers.insert(std::make_pair("perform", std::tr1::bind(&make_command<PerformCommand>)));
    _imp->handlers.insert(std::make_pair("print-categories", std::tr1::bind(&make_command<PrintCategoriesCommand>)));
    _imp->handlers.insert(std::make_pair("print-commands", std::tr1::bind(&make_command<PrintCommandsCommand>)));
    _imp->handlers.insert(std::make_pair("print-environment-metadata", std::tr1::bind(&make_command<PrintEnvironmentMetadataCommand>)));
    _imp->handlers.insert(std::make_pair("print-id-contents", std::tr1::bind(&make_command<PrintIDContentsCommand>)));
    _imp->handlers.insert(std::make_pair("print-id-executables", std::tr1::bind(&make_command<PrintIDExecutablesCommand>)));
    _imp->handlers.insert(std::make_pair("print-id-metadata", std::tr1::bind(&make_command<PrintIDMetadataCommand>)));
    _imp->handlers.insert(std::make_pair("print-ids", std::tr1::bind(&make_command<PrintIDsCommand>)));
    _imp->handlers.insert(std::make_pair("print-owners", std::tr1::bind(&make_command<PrintOwnersCommand>)));
    _imp->handlers.insert(std::make_pair("print-packages", std::tr1::bind(&make_command<PrintPackagesCommand>)));
    _imp->handlers.insert(std::make_pair("print-repositories", std::tr1::bind(&make_command<PrintRepositoriesCommand>)));
    _imp->handlers.insert(std::make_pair("print-repository-formats", std::tr1::bind(&make_command<PrintRepositoryFormatsCommand>)));
    _imp->handlers.insert(std::make_pair("print-sets", std::tr1::bind(&make_command<PrintSetsCommand>)));
    _imp->handlers.insert(std::make_pair("print-sync-protocols", std::tr1::bind(&make_command<PrintSyncProtocolsCommand>)));
    _imp->handlers.insert(std::make_pair("resolve", std::tr1::bind(&make_command<ResolveCommand>)));
    _imp->handlers.insert(std::make_pair("show", std::tr1::bind(&make_command<ShowCommand>)));
    _imp->handlers.insert(std::make_pair("sync", std::tr1::bind(&make_command<SyncCommand>)));
    _imp->handlers.insert(std::make_pair("update-world", std::tr1::bind(&make_command<UpdateWorldCommand>)));
}

CommandFactory::~CommandFactory()
{
}

const std::tr1::shared_ptr<Command>
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

template class InstantiationPolicy<CommandFactory, instantiation_method::SingletonTag>;
template class PrivateImplementationPattern<CommandFactory>;
template class WrappedForwardIterator<CommandFactory::ConstIteratorTag, const std::string>;

