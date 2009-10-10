/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#include <paludis/util/attributes.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/args/do_help.hh>
#include <paludis/environment_factory.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>
#include <iostream>
#include <cstdlib>
#include <string>

#include "command_factory.hh"
#include "command_line.hh"

using namespace paludis;
using std::endl;
using std::cout;
using std::cerr;

int main(int argc, char * argv[])
{
    Context context(std::string("In program ") + argv[0] + " " + join(argv + 1, argv + argc, " ") + ":");

    try
    {
        cave::CaveCommandLine cmdline;
        cmdline.run(argc, argv, "CAVE", "CAVE_OPTIONS", "CAVE_CMDLINE", args::ArgsHandlerOptions() + args::aho_stop_on_first_parameter);

        if (cmdline.begin_parameters() == cmdline.end_parameters())
            throw args::DoHelp();

        std::string cave_var(argv[0]);
        if (cmdline.a_environment.specified())
            cave_var = cave_var + " --" + cmdline.a_environment.long_name() + " " + cmdline.a_environment.argument();
        if (cmdline.a_log_level.specified())
            cave_var = cave_var + " --" + cmdline.a_log_level.long_name() + " " + cmdline.a_log_level.argument();
        setenv("CAVE", cave_var.c_str(), 1);

        Log::get_instance()->set_program_name(argv[0]);
        Log::get_instance()->set_log_level(cmdline.a_log_level.option());
        std::tr1::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(cmdline.a_environment.argument()));

        env->set_paludis_command(std::string(BINDIR"/paludis")
               + " --" + cmdline.a_log_level.long_name() + " " + cmdline.a_log_level.argument()
               + " --" + cmdline.a_environment.long_name() + " " + (cmdline.a_environment.argument().empty() ? ":" : cmdline.a_environment.argument()));

        std::tr1::shared_ptr<Sequence<std::string> > seq(new Sequence<std::string>);
        std::copy(next(cmdline.begin_parameters()), cmdline.end_parameters(), seq->back_inserter());

        return cave::CommandFactory::get_instance()->create(*cmdline.begin_parameters())->run(env, seq);
    }
    catch (const args::DoHelp & h)
    {
        if (h.message.empty())
            cout << "Usage: " << argv[0] << " COMMAND [ARGS]" << endl;
        else
            cerr << "Usage error: " << h.message << endl;

        return EXIT_FAILURE;
    }
    catch (const ActionAbortedError & e)
    {
        cout << endl;
        cerr << "Action aborted:" << endl
            << "  * " << e.backtrace("\n  * ")
            << e.message() << " (" << e.what() << ")" << endl;
        return 42;
    }
    catch (const Exception & e)
    {
        cerr << endl;
        cerr << "Error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ") << e.message() << " (" << e.what() << ")" << endl;
        cerr << endl;
        return EXIT_FAILURE;
    }
}

