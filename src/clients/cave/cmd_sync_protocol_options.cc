/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include "cmd_sync_protocol_options.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>

#include <paludis/environment.hh>
#include <paludis/repository.hh>

#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/log.hh>
#include <paludis/util/process.hh>
#include <paludis/util/system.hh>
#include <paludis/util/env_var_names.hh>
#include <paludis/util/join.hh>

#include <cstdlib>
#include <iostream>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct SyncProtocolOptionsCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave sync-protocol-options";
        }

        virtual std::string app_synopsis() const
        {
            return "Displays options for a given sync protocol.";
        }

        virtual std::string app_description() const
        {
            return "Displays options for a given sync protocol.";
        }

        SyncProtocolOptionsCommandLine()
        {
            add_usage_line("protocol");
        }
    };
}

int
SyncProtocolOptionsCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    SyncProtocolOptionsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_SYNC_PROTOCOL_OPTIONS_OPTIONS", "CAVE_SYNC_PROTOCOL_OPTIONS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != std::distance(cmdline.begin_parameters(), cmdline.end_parameters()))
        throw args::DoHelp("sync-protocol-options takes exactly one parameter");
    std::string format(*cmdline.begin_parameters());

    auto syncer_dirs(env->syncers_dirs());
    FSPath syncer("/dev/null");
    bool ok(false);
    for (auto d(syncer_dirs->begin()), d_end(syncer_dirs->end()) ;
            d != d_end && ! ok; ++d)
    {
        syncer = FSPath(*d) / ("do" + format);
        FSStat syncer_stat(syncer);
        if (syncer_stat.exists() && 0 != (syncer_stat.permissions() & S_IXUSR))
            ok = true;
    }

    if (! ok)
        throw args::DoHelp("syncer '" + format + "' does not exist");

    Process process(ProcessCommand({stringify(syncer), "--help"}));
    process
        .setenv("PALUDIS_FETCHERS_DIRS", join(syncer_dirs->begin(), syncer_dirs->end(), " "))
        .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"));

    if (0 != process.run().wait())
        Log::get_instance()->message("paludis.syncer_help.failure", ll_warning, lc_context)
            << "Syncer help command '" << syncer << " --help' failed";
    cout << endl;

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
SyncProtocolOptionsCommand::make_doc_cmdline()
{
    return std::make_shared<SyncProtocolOptionsCommandLine>();
}

CommandImportance
SyncProtocolOptionsCommand::importance() const
{
    return ci_supplemental;
}

