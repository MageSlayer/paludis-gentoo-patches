/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include "cmd_print_ids.hh"
#include "format_package_id.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/stringify.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/metadata_key.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;

namespace
{
    struct PrintIDsCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave print-ids";
        }

        std::string app_synopsis() const override
        {
            return "Prints a list of known IDs.";
        }

        std::string app_description() const override
        {
            return "Prints a list of known IDs. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }

        args::ArgsGroup g_filters;
        args::StringSetArg a_matching;
        args::StringSetArg a_supporting;
        args::StringSetArg a_with_mask;

        args::ArgsGroup g_display_options;
        args::StringArg a_format;

        PrintIDsCommandLine() :
            g_filters(main_options_section(), "Filters", "Filter the output. Each filter may be specified more than once."),
            a_matching(&g_filters, "matching", 'm', "Show only IDs matching this spec. If specified multiple "
                    "times, only IDs matching every spec are selected.",
                    args::StringSetArg::StringSetArgOptions()),
            a_supporting(&g_filters, "supporting", 's', "Show only IDs supporting this action. If specified "
                    "multiple times, all listed actions must be supported.",
                    args::StringSetArg::StringSetArgOptions
                    ("install",       "able to be installed")
                    ("uninstall",     "able to be uninstalled")
                    ("pretend",       "has pretend-install-time checks")
                    ("config",        "supports post-install configuration")
                    ("fetch",         "able to have sources fetched")
                    ("pretend-fetch", "able to have sources pretend-fetched")
                    ("info",          "provides extra pre- or post-install information")
                    ),
            a_with_mask(&g_filters, "with-mask", 'M', "Show only IDs with this kind of mask. If specified "
                    "multiple times, all listed masks must be present.",
                    args::StringSetArg::StringSetArgOptions
                    ("none",          "no mask")
                    ("any",           "any kind of mask")
                    ("user",          "masked by user")
                    ("unaccepted",    "masked by unaccepted key")
                    ("repository",    "masked by repository")
                    ("unsupported",   "masked because it is unsupported")
                   ),
            g_display_options(main_options_section(), "Display Options", "Controls the output format."),
            a_format(&g_display_options, "format", 'f', format_package_id_help)
        {
            add_usage_line("[ --matching spec ] [ --supporting action ] [ --with-mask mask-kind ]");
            a_format.set_argument("%F\\n");
        }
    };

    struct WithMaskFilterHandler :
        FilterHandler
    {
        const PrintIDsCommandLine & cmdline;
        const std::string mask;

        WithMaskFilterHandler(const PrintIDsCommandLine & c, const std::string & f) :
            cmdline(c),
            mask(f)
        {
        }

        const RepositoryContentMayExcludes may_excludes() const override
        {
            return { };
        }

        std::string as_string() const override
        {
            return "with mask '" + mask + "'";
        }

        std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const,
                const std::shared_ptr<const RepositoryNameSet> & r) const override
        {
            return r;
        }

        std::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const,
                const std::shared_ptr<const RepositoryNameSet> &,
                const std::shared_ptr<const CategoryNamePartSet> & c) const override
        {
            return c;
        }

        std::shared_ptr<const QualifiedPackageNameSet> packages(
                const Environment * const,
                const std::shared_ptr<const RepositoryNameSet> &,
                const std::shared_ptr<const QualifiedPackageNameSet> & q) const override
        {
            return q;
        }

        std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const PackageIDSet> & c) const override
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (PackageIDSet::ConstIterator i(c->begin()), i_end(c->end()) ;
                    i != i_end ; ++i)
            {
                if (mask == "none")
                {
                    if (! (*i)->masked())
                        result->insert(*i);
                }
                else if (mask == "any")
                {
                    if ((*i)->masked())
                        result->insert(*i);
                }
                else if (mask == "user")
                {
                    for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                            m != m_end ; ++m)
                        if (visitor_cast<const UserMask>(**m))
                        {
                            result->insert(*i);
                            break;
                        }
                }
                else if (mask == "unaccepted")
                {
                    for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                            m != m_end ; ++m)
                        if (visitor_cast<const UnacceptedMask>(**m))
                        {
                            result->insert(*i);
                            break;
                        }
                }
                else if (mask == "repository")
                {
                    for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                            m != m_end ; ++m)
                        if (visitor_cast<const RepositoryMask>(**m))
                        {
                            result->insert(*i);
                            break;
                        }
                }
                else if (mask == "unsupported")
                {
                    for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                            m != m_end ; ++m)
                        if (visitor_cast<const UnsupportedMask>(**m))
                        {
                            result->insert(*i);
                            break;
                        }
                }
                else
                    throw args::DoHelp("Unknown --" + cmdline.a_with_mask.long_name() + " value '" + mask + "'");
            }

            return result;
        }
    };

    struct WithMaskFilter :
        Filter
    {
        WithMaskFilter(const PrintIDsCommandLine & c, const std::string & f) :
            Filter(std::make_shared<WithMaskFilterHandler>(c, f))
        {
        }
    };
}

int
PrintIDsCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintIDsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_IDS_OPTIONS", "CAVE_PRINT_IDS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-ids takes no parameters");

    Generator g((generator::All()));
    if (cmdline.a_matching.specified())
    {
        for (const auto & matching_spec : cmdline.a_matching.args())
        {
            PackageDepSpec s(parse_user_package_dep_spec(matching_spec, env.get(), { updso_allow_wildcards }));
            g = g & generator::Matches(s, nullptr, { });
        }
    }

    FilteredGenerator fg(g);

    if (cmdline.a_supporting.specified())
    {
        for (const auto & action : cmdline.a_supporting.args())
        {
            if (action == "install")
                fg = fg | filter::SupportsAction<InstallAction>();
            else if (action == "uninstall")
                fg = fg | filter::SupportsAction<UninstallAction>();
            else if (action == "pretend")
                fg = fg | filter::SupportsAction<PretendAction>();
            else if (action == "config")
                fg = fg | filter::SupportsAction<ConfigAction>();
            else if (action == "fetch")
                fg = fg | filter::SupportsAction<FetchAction>();
            else if (action == "pretend-fetch")
                fg = fg | filter::SupportsAction<PretendFetchAction>();
            else if (action == "info")
                fg = fg | filter::SupportsAction<InfoAction>();
            else
                throw args::DoHelp("Unknown --" + cmdline.a_supporting.long_name() + " value '" + action + "'");
        }
    }

    if (cmdline.a_with_mask.specified())
    {
        for (const auto & mask : cmdline.a_with_mask.args())
        {
            fg = fg | WithMaskFilter(cmdline, mask);
        }
    }

    const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(fg)]);
    for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
            i != i_end ; ++i)
        cout << format_package_id(*i, cmdline.a_format.argument());

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintIDsCommand::make_doc_cmdline()
{
    return std::make_shared<PrintIDsCommandLine>();
}

CommandImportance
PrintIDsCommand::importance() const
{
    return ci_scripting;
}

