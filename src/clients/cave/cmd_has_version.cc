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

#include "cmd_has_version.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_null_shared_ptr.hh>
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
using std::endl;

namespace
{
    struct HasVersionCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave has-version";
        }

        virtual std::string app_synopsis() const
        {
            return "Returns success if there is an installed package matching a particular spec.";
        }

        virtual std::string app_description() const
        {
            return "Returns success if there is an installed package matching a particular spec. Suitable "
                "for use in scripts.";
        }

        HasVersionCommandLine()
        {
            add_usage_line("spec");
        }
    };
}

int
HasVersionCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    HasVersionCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_HAS_VERSION_OPTIONS", "CAVE_HAS_VERSION_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != std::distance(cmdline.begin_parameters(), cmdline.end_parameters()))
        throw args::DoHelp("has-version takes exactly one parameter");

    auto s(parse_user_package_dep_spec(*cmdline.begin_parameters(), env.get(), { }));
    auto ids((*env)[selection::BestVersionOnly(generator::Matches(s, make_null_shared_ptr(), { }) | filter::InstalledAtRoot(
                    env->preferred_root_key()->value()))]);

    if (ids->empty())
        return EXIT_FAILURE;
    else
        return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
HasVersionCommand::make_doc_cmdline()
{
    return std::make_shared<HasVersionCommandLine>();
}

