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

#include "cmd_print_checksum.hh"
#include "command_command_line.hh"

#include <paludis/util/digest_registry.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/upper_lower.hh>

#include <paludis/args/do_help.hh>

#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace paludis;
using namespace paludis::cave;

namespace
{
    struct PrintChecksumCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_input;
        args::SwitchArg a_stdin;
        args::StringArg a_file;
        args::StringArg a_text;

        PrintChecksumCommandLine() :
            g_input(main_options_section(), "Input Options", "Input Options"),
            a_stdin(&g_input, "stdin", 's', "Calculate the checksum of standard input.", false),
            a_file(&g_input, "file", 'f', "Calculate the checksum of the contents of the specified file."),
            a_text(&g_input, "text", 't', "Calculate the checksum of the specified text.")
        {
            add_usage_line("ALGORITHM --stdin");
            add_usage_line("ALGORITHM --file FILENAME");
            add_usage_line("ALGORITHM --text TEXT");
            add_see_also("cave-print-checksum-algorithms", 1);
        }

        std::string app_name() const override
        {
            return "cave print-checksum";
        }

        std::string app_synopsis() const override
        {
            return "prints cryptographic checksums";
        }

        std::string app_description() const override
        {
            return "Prints the checksum of the specified input using the specified algorithm. No formatting "
                "is used, making the output suitable for parsing by scripts.";
        }
    };
}

int
PrintChecksumCommand::run(const std::shared_ptr<Environment> &,
    const std::shared_ptr<const Sequence<std::string> > & args)
{
    PrintChecksumCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_CHECKSUM_OPTIONS", "CAVE_PRINT_CHECKSUM_CMDLINE");

    if (cmdline.a_help.specified())
    {
        std::cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (std::distance(cmdline.begin_parameters(), cmdline.end_parameters()) != 1)
        throw args::DoHelp("print-checksum takes exactly one parameter");

    std::string algo(toupper(*cmdline.begin_parameters()));
    DigestRegistry::Function f(DigestRegistry::get_instance()->get(algo));
    if (! f)
        throw args::DoHelp("algorithm '" + algo + "' is not supported");

    if (cmdline.a_stdin.specified() + cmdline.a_file.specified() + cmdline.a_text.specified() != 1)
        throw args::DoHelp("print-checksum requires exactly one of --stdin, --file or --text");

    std::string result;
    if (cmdline.a_stdin.specified())
        result = f(std::cin);
    else if (cmdline.a_file.specified())
    {
        SafeIFStream s(FSPath(cmdline.a_file.argument()));
        result = f(s);
    }
    else if (cmdline.a_text.specified())
    {
        std::istringstream s(cmdline.a_text.argument());
        result = f(s);
    }

    std::cout << result << std::endl;
    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintChecksumCommand::make_doc_cmdline()
{
    return std::make_shared<PrintChecksumCommandLine>();
}

CommandImportance
PrintChecksumCommand::importance() const
{
    return ci_scripting;
}

