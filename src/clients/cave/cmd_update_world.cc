/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <iostream>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct UpdateWorldCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave update-world";
        }

        virtual std::string app_synopsis() const
        {
            return "Updates the world set.";
        }

        virtual std::string app_description() const
        {
            return "Adds or removes items from the world set.";
        }

        args::ArgsGroup g_update_options;
        args::SwitchArg a_remove;
        args::SwitchArg a_set;

        UpdateWorldCommandLine() :
            g_update_options(this, "Update Options", "Alter how updates are performed."),
            a_remove(&g_update_options, "remove", 'r', "Remove the specified items instead of adding them", true),
            a_set(&g_update_options, "set", 's', "The parameters are set names, not package names", true)
        {
            add_usage_line("[ --remove ] cat/pkg ...");
            add_usage_line("[ --remove ] --set setname ...");
        }
    };
}

int
UpdateWorldCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
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
        if (cmdline.a_set.specified())
        {
            if (cmdline.a_remove.specified())
                env->remove_from_world(SetName(*p));
            else
                env->add_to_world(SetName(*p));
        }
        else
        {
            QualifiedPackageName q("x/x");

            if (std::string::npos == p->find('/'))
                q = env->package_database()->fetch_unique_qualified_package_name(PackageNamePart(*p));
            else
                q = QualifiedPackageName(*p);

            if (cmdline.a_remove.specified())
                env->remove_from_world(q);
            else
                env->add_to_world(q);
        }
    }

    return EXIT_SUCCESS;
}

std::tr1::shared_ptr<args::ArgsHandler>
UpdateWorldCommand::make_doc_cmdline()
{
    return make_shared_ptr(new UpdateWorldCommandLine);
}


