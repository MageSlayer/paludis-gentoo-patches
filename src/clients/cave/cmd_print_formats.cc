/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include "cmd_print_formats.hh"
#include <paludis/util/stringify.hh>
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <map>

#include "command_command_line.hh"

#include "config.h"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    namespace c
    {
        const std::string red() { return "${red}"; }
        const std::string bold_red() { return "${bold_red}"; }
        const std::string green() { return "${green}"; }
        const std::string bold_green() { return "${bold_green}"; }
        const std::string yellow() { return "${yellow}"; }
        const std::string bold_yellow() { return "${bold_yellow}"; }
        const std::string blue() { return "${blue}"; }
        const std::string bold_blue() { return "${bold_blue}"; }
        const std::string pink() { return "${pink}"; }
        const std::string bold_pink() { return "${bold_pink}"; }
        const std::string normal() { return "${normal}"; }
        const std::string bold_normal() { return "${bold_normal}"; }

#if PALUDIS_COLOUR_PINK
        const std::string blue_or_pink() { return pink(); }
        const std::string bold_blue_or_pink() { return bold_pink(); }
        const std::string green_or_pink() { return pink(); }
        const std::string bold_green_or_pink() { return bold_pink(); }
#else
        const std::string blue_or_pink() { return blue(); }
        const std::string bold_blue_or_pink() { return bold_blue(); }
        const std::string green_or_pink() { return green(); }
        const std::string bold_green_or_pink() { return bold_green(); }
#endif
    }

    template <char c_>
    std::string param()
    {
        return "%" + std::string(1, c_);
    }

    struct Storer
    {
        std::string user_key;
        int user_key_version;

        std::string value;
    };

    Storer && operator<< (Storer && s, const std::string & t)
    {
        s.value.append(t);
        return s;
    }

    struct GetFormats
    {
        std::map<std::string, Storer> storers;

        Storer & make_format_string_fetcher(const std::string & v, const int vi)
        {
            std::string k(0 == vi ? v : v + "." + stringify(vi));
            auto i(storers.insert(std::make_pair(k, Storer{v, vi, ""})));
            if (! i.second)
                throw InternalError(PALUDIS_HERE, "couldn't insert " + k);

            return i.first->second;
        }

        void collect()
        {
            {
#include "cmd_contents-fmt.hh"
            }{
#include "cmd_executables-fmt.hh"
            }{
#include "cmd_fix_cache-fmt.hh"
            }{
#include "cmd_info-fmt.hh"
            }{
#include "cmd_owner-fmt.hh"
            }{
#include "cmd_report-fmt.hh"
            }{
#include "cmd_sync-fmt.hh"
            }{
#include "cmd_verify-fmt.hh"
            }{
#include "colour_formatter-fmt.hh"
            }
        }
    };
}

namespace
{
    struct PrintFormatsCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-formats";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of known format strings.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of known format strings. No formatting is used, making the output "
                "suitable for parsing by scripts.";
        }

        PrintFormatsCommandLine()
        {
            add_usage_line("");
        }
    };
}

int
PrintFormatsCommand::run(
        const std::shared_ptr<Environment> &,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintFormatsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_FORMATS_OPTIONS", "CAVE_PRINT_FORMATS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-formats takes no parameters");

    cout << "[colours]"   << endl;
    cout << "red"         << " = " << "\\e[0;31m" << endl;
    cout << "bold_red"    << " = " << "\\e[1;31m" << endl;
    cout << "green"       << " = " << "\\e[0;32m" << endl;
    cout << "bold_green"  << " = " << "\\e[1;32m" << endl;
    cout << "yellow"      << " = " << "\\e[0;33m" << endl;
    cout << "bold_yellow" << " = " << "\\e[1;33m" << endl;
    cout << "blue"        << " = " << "\\e[0;34m" << endl;
    cout << "bold_blue"   << " = " << "\\e[1;34m" << endl;
    cout << "pink"        << " = " << "\\e[0;35m" << endl;
    cout << "bold_pink"   << " = " << "\\e[1;35m" << endl;
    cout << "normal"      << " = " << "\\e[0;0m" << endl;
    cout << "bold_normal" << " = " << "\\e[1m" << endl;

    GetFormats get_formats;
    get_formats.collect();

    std::string current_section;
    for (auto i(get_formats.storers.begin()), i_end(get_formats.storers.end()) ;
            i != i_end ; ++i)
    {
        std::string::size_type p(i->first.find("/"));
        if (std::string::npos == p)
            throw InternalError(PALUDIS_HERE, "weird key " + i->first);
        std::string section(i->first.substr(0, p)), key(i->first.substr(p + 1));

        if (current_section != section)
        {
            cout << endl << "[" << section << "]" << endl;
            current_section = section;
        }

        cout << key << " = ";
        if (0 == i->second.value.compare(0, 1, " "))
            cout << "\\";
        cout << i->second.value << endl;
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintFormatsCommand::make_doc_cmdline()
{
    return std::make_shared<PrintFormatsCommandLine>();
}

