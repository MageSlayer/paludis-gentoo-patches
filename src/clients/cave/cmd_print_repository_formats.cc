/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ingmar Vanhassel
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

#include "cmd_print_repository_formats.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/repository_factory.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>

#include <set>
#include <iostream>
#include <cstdlib>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintRepositoryFormatsCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-repository-formats";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of available repository formats.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of available repository formats. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }
    };
}

int
PrintRepositoryFormatsCommand::run(
        const std::shared_ptr<Environment> &,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintRepositoryFormatsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_REPOSITORY_FORMATS_OPTIONS", "CAVE_PRINT_REPOSITORY_FORMATS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-repository-formats takes no parameters");

    std::set<std::string> repository_formats(RepositoryFactory::get_instance()->begin_keys(), RepositoryFactory::get_instance()->end_keys());

    if (! repository_formats.empty())
    {
        std::copy(repository_formats.begin(), repository_formats.end(), std::ostream_iterator<std::string>(cout, "\n"));
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

std::shared_ptr<args::ArgsHandler>
PrintRepositoryFormatsCommand::make_doc_cmdline()
{
    return std::make_shared<PrintRepositoryFormatsCommandLine>();
}
