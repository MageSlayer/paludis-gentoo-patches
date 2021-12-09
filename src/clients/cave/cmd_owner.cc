/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Alexander Færøy
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

#include "cmd_owner.hh"
#include "command_command_line.hh"
#include "owner_common.hh"
#include "colours.hh"
#include "format_user_config.hh"

#include <paludis/action.hh>
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/stringify.hh>

#include <iostream>
#include <cstdlib>
#include <functional>

using namespace paludis;
using namespace cave;
using std::cout;

namespace
{
#include "cmd_owner-fmt.hh"

    struct OwnerCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave owner";
        }

        std::string app_synopsis() const override
        {
            return "Shows package IDs owning a given file.";
        }

        std::string app_description() const override
        {
            return "Shows package IDs owning a given file.";
        }

        args::ArgsGroup g_owner_options;
        args::EnumArg a_type;
        args::SwitchArg a_dereference;
        args::StringSetArg a_matching;

        OwnerCommandLine() :
            g_owner_options(main_options_section(), "Owner options", "Alter how the search is performed."),
            a_type(&g_owner_options, "type", 't', "Which type of match algorithm to use",
                    args::EnumArg::EnumArgOptions
                    ("auto",          'a', "If pattern starts with a /, full; if it contains a /, partial; otherwise, basename")
                    ("basename",      'b', "Basename match")
                    ("full",          'f', "Full match")
                    ("partial",       'p', "Partial match"),
                    "auto"),
            a_dereference(&g_owner_options, "dereference", 'd', "If the pattern is a path that exists and is a symbolic link, "
                    "dereference it recursively, and then search for the real path.", true),
            a_matching(&g_owner_options, "matching", 'm', "Show only IDs matching this spec. If specified multiple "
                    "times, only IDs matching every spec are selected.",
                    args::StringSetArg::StringSetArgOptions())
        {
            add_usage_line("[ --type algorithm ] [ --matching spec ] pattern");
        }
    };

    void format_id(const std::shared_ptr<const PackageID> & id)
    {
        cout << fuc(fs_id(), fv<'s'>(stringify(*id)));
    }
}

int
OwnerCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    OwnerCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_OWNER_OPTIONS", "CAVE_OWNER_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (std::distance(cmdline.begin_parameters(), cmdline.end_parameters()) != 1)
        throw args::DoHelp("owner takes exactly one parameter");

    Filter matches((filter::All()));
    if (cmdline.a_matching.specified())
    {
        for (args::StringSetArg::ConstIterator m(cmdline.a_matching.begin_args()),
                m_end(cmdline.a_matching.end_args()) ;
                m != m_end ; ++m)
        {
            PackageDepSpec s(parse_user_package_dep_spec(*m, env.get(), { updso_allow_wildcards }));
            matches = filter::And(matches, filter::Matches(s, nullptr, { }));
        }
    }

    return owner_common(env, cmdline.a_type.argument(), matches,
            *cmdline.begin_parameters(), cmdline.a_dereference.specified(), &format_id);
}

std::shared_ptr<args::ArgsHandler>
OwnerCommand::make_doc_cmdline()
{
    return std::make_shared<OwnerCommandLine>();
}

CommandImportance
OwnerCommand::importance() const
{
    return ci_supplemental;
}

