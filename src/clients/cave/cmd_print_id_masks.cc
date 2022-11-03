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

#include "cmd_print_id_masks.hh"
#include "exceptions.hh"
#include "format_plain_metadata_key.hh"
#include "format_string.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/map.hh>
#include <paludis/util/stringify.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <iostream>
#include <algorithm>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;

namespace
{
    struct PrintIDMasksCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave print-id-masks";
        }

        std::string app_synopsis() const override
        {
            return "Prints ID masks.";
        }

        std::string app_description() const override
        {
            return "Prints ID masks. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }

        args::ArgsGroup g_spec_options;
        args::SwitchArg a_all;
        args::SwitchArg a_best;

        args::ArgsGroup g_filters;
        args::SwitchArg a_overridden;
        args::SwitchArg a_no_active;

        args::ArgsGroup g_display_options;
        args::StringArg a_format;

        PrintIDMasksCommandLine() :
            g_spec_options(main_options_section(), "Spec Options", "Alter how the supplied spec is used."),
            a_all(&g_spec_options, "all", 'a', "If the spec matches multiple IDs, display all matches.", true),
            a_best(&g_spec_options, "best", 'b', "If the spec matches multiple IDs, select the best ID rather than giving an error.", true),
            g_filters(main_options_section(), "Filters", "Filter the output."),
            a_overridden(&g_filters, "overridden", '\0', "Show overridden masks", true),
            a_no_active(&g_filters, "no-active", '\0', "No not show active (non-overridden) masks", true),
            g_display_options(main_options_section(), "Display Options", "Controls the output format."),
            a_format(&g_display_options, "format", '\0', "Select the output format. Special tokens recognised are "
                    "%k for mask key, %d for mask description, %r for associated key's raw name, "
                    "%= for an = sign if %r is not blank, %h for associated key's human name, "
                    "%v for associated key or id's value or any additional explanation, "
                    "%( and %) for ( and ) if the mask is overridden, %o for the reason a mask is overridden, "
                    "\\n for newline, \\t for tab. Default is '%(%k %d %r%=%v%) %o\\n'.")
        {
            a_format.set_argument("%(%k %d %r%=%v%) %o\\n");
            add_usage_line("[ --format format ] spec");
        }
    };

    struct GetInfo
    {
        const std::pair<std::string, std::string> visit(const UserMask &) const
        {
            return std::make_pair("", "");
        }

        const std::pair<std::string, std::string> visit(const UnacceptedMask & m) const
        {
            return std::make_pair(m.unaccepted_key_name(), "");
        }

        const std::pair<std::string, std::string> visit(const RepositoryMask &) const
        {
            return std::make_pair("", "");
        }

        const std::pair<std::string, std::string> visit(const UnsupportedMask &) const
        {
            return std::make_pair("", "");
        }
    };

    void do_one_mask(
            const std::shared_ptr<const PackageID> & id,
            const std::shared_ptr<const Mask> & mask,
            const MaskOverrideReason & override,
            const PrintIDMasksCommandLine & cmdline
            )
    {
        std::shared_ptr<Map<char, std::string> > m(std::make_shared<Map<char, std::string>>());
        m->insert('k', std::string(1, mask->key()));
        m->insert('d', mask->description());

        auto info(mask->accept_returning<std::pair<std::string, std::string> >(GetInfo()));
        std::shared_ptr<const MetadataKey> info_key;
        if (! info.first.empty())
            info_key = *id->find_metadata(info.first);

        m->insert('r', info_key ? info_key->raw_name() : "");
        m->insert('=', info_key ? "=" : "");
        m->insert('h', info_key ? info_key->human_name() : "");
        m->insert('v', info_key ? format_plain_metadata_key_value(info_key) : info.second);
        m->insert('(', last_mro == override ? "" : "(");
        m->insert(')', last_mro == override ? "" : ")");

        switch (override)
        {
            case mro_accepted_unstable:
                m->insert('o', "accepted unstable");
                break;
            case mro_overridden_by_user:
                m->insert('o', "overridden by user");
                break;
            case last_mro:
                m->insert('o', "");
                break;
        }

        cout << format_string(cmdline.a_format.argument(), m);
    }
}

int
PrintIDMasksCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintIDMasksCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_ID_MASKS_OPTIONS", "CAVE_PRINT_ID_MASKS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != cmdline.parameters().size())
        throw args::DoHelp("print-id-masks takes exactly one parameter");

    PackageDepSpec spec(parse_user_package_dep_spec(*cmdline.begin_parameters(), env.get(), { updso_allow_wildcards }));

    std::shared_ptr<const PackageIDSequence> entries(
            (*env)[selection::AllVersionsSorted(generator::Matches(spec, nullptr, { }))]);

    if (entries->empty())
        throw NothingMatching(spec);

    if ((! cmdline.a_best.specified()) && (! cmdline.a_all.specified())
            && (next(entries->begin()) != entries->end()))
        throw BeMoreSpecific(spec, entries);

    for (auto i(cmdline.a_best.specified() ? entries->last() : entries->begin()), i_end(entries->end()) ;
            i != i_end ; ++i)
    {
        if (! cmdline.a_no_active.specified())
            for (const auto & mask : (*i)->masks())
                do_one_mask(*i, mask, last_mro, cmdline);

        if (cmdline.a_overridden.specified())
            for (const auto & overridden_mask : (*i)->overridden_masks())
                do_one_mask(*i, overridden_mask->mask(), overridden_mask->override_reason(), cmdline);
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintIDMasksCommand::make_doc_cmdline()
{
    return std::make_shared<PrintIDMasksCommandLine>();
}

CommandImportance
PrintIDMasksCommand::importance() const
{
    return ci_scripting;
}

