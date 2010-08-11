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

#include "cmd_print_id_contents.hh"
#include "exceptions.hh"
#include "format_plain_contents_entry.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/environment.hh>
#include <paludis/contents.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <iostream>
#include <algorithm>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintContentsCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-id-contents";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints the contents of a package ID.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of the contents of a given ID. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }

        args::ArgsGroup g_spec_options;
        args::SwitchArg a_all;
        args::SwitchArg a_best;

        args::ArgsGroup g_filter_options;
        args::EnumArg a_type;

        args::ArgsGroup g_display_options;
        args::StringArg a_format;

        PrintContentsCommandLine() :
            g_spec_options(main_options_section(), "Spec Options", "Alter how the supplied spec is used."),
            a_all(&g_spec_options, "all", 'a', "If the spec matches multiple IDs, display all matches.", true),
            a_best(&g_spec_options, "best", 'b', "If the spec matches multiple IDs, select the best ID rather than giving an error.", true),
            g_filter_options(main_options_section(), "Filter Options", "Alter which contents entries are displayed."),
            a_type(&g_filter_options, "type", 't', "Display only entries of the specified type",
                    args::EnumArg::EnumArgOptions
                    ("all",   'a', "Show all entries")
                    ("file",  'f', "Show only file entries")
                    ("dir",   'd', "Show only directory entries")
                    ("sym",   's', "Show only symlink entries")
                    ("other", 'o', "Show only other entries"),
                    "all"),
            g_display_options(main_options_section(), "Display Options", "Controls the output format."),
            a_format(&g_display_options, "format", '\0', "Select the output format. Special tokens recognised are "
                    "%n for filename, %d for dirname, %b for basename, %t for symlink targets (blank for non-symlinks), "
                    "%a for ' -> ' if we're a symlink and '' otherwise, %/ for '/' if we're a directory and '' otherwise, "
                    "%i for one space for every parent directory, "
                    "\\n for newline, \\t for tab. Default is '%n%a%t\\n'.")
        {
            add_usage_line("spec");
            a_format.set_argument("%n%a%t\\n");
        }
    };

    struct MatchTypeVisitor
    {
        const args::EnumArg & arg;

        bool visit(const ContentsFileEntry &) const
        {
            return arg.argument() == "file";
        }

        bool visit(const ContentsDirEntry &) const
        {
            return arg.argument() == "dir";
        }

        bool visit(const ContentsSymEntry &) const
        {
            return arg.argument() == "sym";
        }

        bool visit(const ContentsOtherEntry &) const
        {
            return arg.argument() == "other";
        }
    };

    bool match_type(
            const args::EnumArg & arg,
            const std::shared_ptr<const ContentsEntry> & entry)
    {
        if (arg.argument() == "all")
            return true;

        return entry->accept_returning<bool>(MatchTypeVisitor{arg});
    }
}

int
PrintIDContentsCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintContentsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_ID_CONTENTS_OPTIONS", "CAVE_PRINT_ID_CONTENTS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != std::distance(cmdline.begin_parameters(), cmdline.end_parameters()))
        throw args::DoHelp("print-id-contents takes exactly one parameter");

    PackageDepSpec spec(parse_user_package_dep_spec(*cmdline.begin_parameters(), env.get(),
                { updso_allow_wildcards }, filter::InstalledAtRoot(env->root())));

    std::shared_ptr<const PackageIDSequence> entries(
            (*env)[selection::AllVersionsSorted(generator::Matches(spec, { }) | filter::InstalledAtRoot(env->root()))]);

    if (entries->empty())
        throw NothingMatching(spec);

    if ((! cmdline.a_best.specified()) && (! cmdline.a_all.specified())
            && (next(entries->begin()) != entries->end()))
        throw BeMoreSpecific(spec, entries);

    for (auto i(cmdline.a_best.specified() ? entries->last() : entries->begin()), i_end(entries->end()) ;
            i != i_end ; ++i)
    {
        if (! (*i)->contents_key())
            throw BadIDForCommand(spec, (*i), "does not support listing contents");

        for (auto c((*i)->contents_key()->value()->begin()), c_end((*i)->contents_key()->value()->end()) ;
                c != c_end ; ++c)
            if (match_type(cmdline.a_type, *c))
                cout << format_plain_contents_entry(*c, cmdline.a_format.argument());
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintIDContentsCommand::make_doc_cmdline()
{
    return std::make_shared<PrintContentsCommandLine>();
}

