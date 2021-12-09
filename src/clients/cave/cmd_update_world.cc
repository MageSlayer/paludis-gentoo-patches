/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include "cmd_update_world.hh"
#include "format_user_config.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/stringify.hh>
#include <paludis/environment.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/metadata_key.hh>
#include <iostream>
#include <cstdlib>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;

namespace
{
#include "cmd_update_world-fmt.hh"

    struct UpdateWorldCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave update-world";
        }

        std::string app_synopsis() const override
        {
            return "Updates the world set.";
        }

        std::string app_description() const override
        {
            return "Adds or removes items from the world set. This command is only for changing the contents "
                "of the world set; it does not install or uninstall anything.";
        }

        args::ArgsGroup g_output_options;
        args::SwitchArg a_verbose;

        args::ArgsGroup g_update_options;
        args::SwitchArg a_remove;
        args::SwitchArg a_if_nothing_left;
        args::SwitchArg a_set;

        UpdateWorldCommandLine() :
            g_output_options(main_options_section(), "Output Options", "Alter the output."),
            a_verbose(&g_output_options, "verbose", '\0', "Produce verbose output", true),

            g_update_options(main_options_section(), "Update Options", "Alter how updates are performed."),
            a_remove(&g_update_options, "remove", 'r', "Remove the specified items instead of adding them", true),
            a_if_nothing_left(&g_update_options, "if-nothing-left", 'l', "Skip any removes where versions remain", true),
            a_set(&g_update_options, "set", 's', "The parameters are set names, not package names", true)
        {
            add_usage_line("[ --remove [ --if-nothing-left ] ] cat/pkg ...");
            add_usage_line("[ --remove [ --if-nothing-left ] ] --set setname ...");
        }
    };
}

int
UpdateWorldCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    UpdateWorldCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_UPDATE_WORLD_OPTIONS", "CAVE_UPDATE_WORLD_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() == cmdline.end_parameters())
        throw args::DoHelp("update-world requires at least one parameter");

    for (UpdateWorldCommandLine::ParametersConstIterator p(cmdline.begin_parameters()),
            p_end(cmdline.end_parameters()) ;
            p != p_end ; ++p)
    {
        bool result(false);
        std::string name;

        if (cmdline.a_set.specified())
        {
            name = *p;
            if (cmdline.a_remove.specified())
                result = env->remove_from_world(SetName(*p));
            else
                result = env->add_to_world(SetName(*p));
        }
        else
        {
            QualifiedPackageName q("x/x");

            if (std::string::npos == p->find('/'))
                q = env->fetch_unique_qualified_package_name(PackageNamePart(*p));
            else
                q = QualifiedPackageName(*p);

            name = stringify(q);

            if (cmdline.a_remove.specified())
            {
                if (cmdline.a_if_nothing_left.specified())
                {
                    auto ids((*env)[selection::SomeArbitraryVersion(generator::Package(q) |
                                filter::InstalledAtRoot(env->preferred_root_key()->parse_value()))]);
                    if (ids->empty())
                        result = env->remove_from_world(q);
                }
                else
                    result = env->remove_from_world(q);
            }
            else
                result = env->add_to_world(q);
        }

        if (cmdline.a_verbose.specified())
        {
            if (cmdline.a_remove.specified())
            {
                if (result)
                    cout << fuc(fs_removed(), fv<'n'>(stringify(name)));
                else
                    cout << fuc(fs_did_not_remove(), fv<'n'>(stringify(name)));
            }
            else
            {
                if (result)
                    cout << fuc(fs_added(), fv<'n'>(stringify(name)));
                else
                    cout << fuc(fs_did_not_add(), fv<'n'>(stringify(name)));
            }
        }
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
UpdateWorldCommand::make_doc_cmdline()
{
    return std::make_shared<UpdateWorldCommandLine>();
}

CommandImportance
UpdateWorldCommand::importance() const
{
    return ci_supplemental;
}

