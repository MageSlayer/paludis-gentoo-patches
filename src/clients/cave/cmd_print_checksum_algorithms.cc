/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2012 David Leverton
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

#include "cmd_print_checksum_algorithms.hh"
#include "command_command_line.hh"

#include <paludis/util/digest_registry.hh>

#include <paludis/args/do_help.hh>

#include <iostream>

using namespace paludis;
using namespace paludis::cave;

namespace
{
    struct PrintChecksumAlgorithmsCommandLine :
        CaveCommandCommandLine
    {
        PrintChecksumAlgorithmsCommandLine()
        {
            add_usage_line("");
            add_see_also("cave-print-checksum", 1);
        }

        virtual std::string app_name() const
        {
            return "cave print-checksum-algorithms";
        }

        virtual std::string app_synopsis() const
        {
            return "prints supported cryptographic checksum algorithms";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of supported checksum algorithms. No formatting "
                "is used, making the output suitable for parsing by scripts.";
        }
    };
}

int
PrintChecksumAlgorithmsCommand::run(const std::shared_ptr<Environment> &,
    const std::shared_ptr<const Sequence<std::string> > & args)
{
    PrintChecksumAlgorithmsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_CHECKSUM_ALGORITHMS_OPTIONS", "CAVE_PRINT_CHECKSUM_ALGORITHMS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        std::cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (std::distance(cmdline.begin_parameters(), cmdline.end_parameters()) != 0)
        throw args::DoHelp("print-checksum-algorithms takes no parameters");

    auto reg(DigestRegistry::get_instance());
    for (auto it(reg->begin_algorithms()), it_end(reg->end_algorithms()); it_end != it; ++it)
        std::cout << it->first << std::endl;
    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintChecksumAlgorithmsCommand::make_doc_cmdline()
{
    return std::make_shared<PrintChecksumAlgorithmsCommandLine>();
}

CommandImportance
PrintChecksumAlgorithmsCommand::importance() const
{
    return ci_scripting;
}

