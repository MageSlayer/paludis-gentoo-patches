/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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

#include "cmd_contents.hh"
#include "format_user_config.hh"
#include "colours.hh"
#include "exceptions.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/environment.hh>
#include <paludis/contents.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/stringify.hh>
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
#include "parse_spec_with_nice_error.hh"

using namespace paludis;
using namespace cave;
using std::cout;

namespace
{
#include "cmd_contents-fmt.hh"

    struct ContentsCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave contents";
        }

        std::string app_synopsis() const override
        {
            return "Prints the contents of a package ID.";
        }

        std::string app_description() const override
        {
            return "Display the contents of (files installed by) a given package ID.";
        }

        ContentsCommandLine()
        {
            add_usage_line("spec");
        }
    };

    std::string stringify_contents_entry(const std::shared_ptr<const ContentsEntry> & entry)
    {
        return entry->make_accept_returning(
            [&] (const ContentsFileEntry & e) {
                return fuc(fs_file(),
                           fv<'p'>(e.part_key()
                                    ? stringify(e.part_key()->parse_value())
                                    : ""),
                           fv<'s'>(stringify(e.location_key()->parse_value())));
            },

            [&] (const ContentsDirEntry & e) {
                return fuc(fs_dir(), fv<'s'>(stringify(e.location_key()->parse_value())));
            },

            [&] (const ContentsSymEntry & e) {
                return fuc(fs_sym(),
                           fv<'p'>(e.part_key()
                                    ? stringify(e.part_key()->parse_value())
                                    : ""),
                           fv<'s'>(stringify(e.location_key()->parse_value())),
                           fv<'t'>(stringify(e.target_key()->parse_value())));
            },

            [&] (const ContentsOtherEntry & e) {
                return fuc(fs_other(), fv<'s'>(stringify(e.location_key()->parse_value())));
            }
            );
    }
}

int
ContentsCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    ContentsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_CONTENTS_OPTIONS", "CAVE_CONTENTS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.parameters().size() != 1)
        throw args::DoHelp("contents takes exactly one parameter");

    PackageDepSpec spec(parse_spec_with_nice_error(*cmdline.begin_parameters(), env.get(),
                { }, filter::InstalledAtRoot(env->preferred_root_key()->parse_value())));

    std::shared_ptr<const PackageIDSequence> entries(
            (*env)[selection::AllVersionsSorted(generator::Matches(spec, nullptr, { })
                | filter::InstalledAtRoot(env->preferred_root_key()->parse_value()))]);

    if (entries->empty())
        nothing_matching_error(env.get(), *cmdline.begin_parameters(), filter::InstalledAtRoot(env->preferred_root_key()->parse_value()));

    const std::shared_ptr<const PackageID> id(*entries->last());
    auto contents(id->contents());
    if (! contents)
        throw BadIDForCommand(spec, id, "does not support listing contents");

    std::transform(
            contents->begin(),
            contents->end(),
            std::ostream_iterator<std::string>(cout, "\n"),
            std::bind(stringify_contents_entry, std::placeholders::_1));

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
ContentsCommand::make_doc_cmdline()
{
    return std::make_shared<ContentsCommandLine>();
}

CommandImportance
ContentsCommand::importance() const
{
    return ci_supplemental;
}

