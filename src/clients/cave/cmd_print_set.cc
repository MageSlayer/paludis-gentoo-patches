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

#include "cmd_print_set.hh"
#include "format_string.hh"
#include "exceptions.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/spec_tree.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/map.hh>
#include <cstdlib>
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
    struct PrintSetCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-set";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of known sets.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of known sets. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }

        args::ArgsGroup g_display_options;
        args::SwitchArg a_expand;

        PrintSetCommandLine() :
            g_display_options(main_options_section(), "Display Options", "Controls the output format."),
            a_expand(&g_display_options, "expand", 'x', "Expand set contents recursively", true)
        {
            add_usage_line("[ --expand ] set");
        }
    };

    struct SetPrinter
    {
        const std::tr1::shared_ptr<const Environment> env;
        const PrintSetCommandLine & cmdline;
        std::set<SetName> & recursing;

        SetPrinter(const std::tr1::shared_ptr<const Environment> & e, const PrintSetCommandLine & c, std::set<SetName> & r) :
            env(e),
            cmdline(c),
            recursing(r)
        {
        }

        void visit(const SetSpecTree::NodeType<PackageDepSpec>::Type & node) const
        {
            cout << *node.spec() << endl;
        }

        void visit(const SetSpecTree::NodeType<NamedSetDepSpec>::Type & node) const
        {
            if (cmdline.a_expand.specified())
            {
                SetName name(node.spec()->name());
                if (! recursing.insert(name).second)
                    throw RecursivelyDefinedSetError(stringify(name));

                const std::tr1::shared_ptr<const SetSpecTree> set(env->set(name));
                if (! set)
                    throw NoSuchSetError(stringify(name));

                set->root()->accept(*this);

                recursing.erase(name);
            }
            else
                cout << *node.spec() << endl;
        }

        void visit(const SetSpecTree::NodeType<AllDepSpec>::Type & node) const
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }
    };
}

int
PrintSetCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintSetCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_IDS_OPTIONS", "CAVE_PRINT_IDS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != std::distance(cmdline.begin_parameters(), cmdline.end_parameters()))
        throw args::DoHelp("print-ids takes exactly one parameter");

    const std::tr1::shared_ptr<const SetSpecTree> set(env->set(SetName(*cmdline.begin_parameters())));
    if (! set)
        throw NothingMatching(*cmdline.begin_parameters());

    std::set<SetName> recursing;
    set->root()->accept(SetPrinter(env, cmdline, recursing));

    return EXIT_SUCCESS;
}

std::tr1::shared_ptr<args::ArgsHandler>
PrintSetCommand::make_doc_cmdline()
{
    return make_shared_ptr(new PrintSetCommandLine);
}

