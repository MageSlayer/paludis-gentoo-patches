/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Alexander Færøy
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

#include "cmd_print_sync_protocols.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>

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
    struct PrintSyncProtocolsCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-sync-protocols";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of sync protocols.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of sync protocols. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }
    };
}

int
PrintSyncProtocolsCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintSyncProtocolsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_SYNC_PROTOCOLS_OPTIONS", "CAVE_PRINT_SYNC_PROTOCOLS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-sync-protocols takes no parameters");

    std::set<std::string> syncers;

    std::shared_ptr<const FSEntrySequence> fes(env->syncers_dirs());

    for (FSEntrySequence::ConstIterator s(fes->begin()), s_end(fes->end());
            s != s_end; ++s)
    {
        FSEntry dir(*s);

        if (! dir.is_directory())
            continue;

        for (DirIterator f(dir), f_end; f != f_end; ++f)
        {
            std::string name(f->basename());

            if (f->has_permission(fs_ug_owner, fs_perm_execute) && name.compare(0, 2, "do", 0, 2) == 0)
            {
                name.erase(0, 2);

                syncers.insert(name);
            }
        }
    }

    std::copy(syncers.begin(), syncers.end(), std::ostream_iterator<std::string>(cout, "\n"));

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintSyncProtocolsCommand::make_doc_cmdline()
{
    return std::make_shared<PrintSyncProtocolsCommandLine>();
}

