/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
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

#include <paludis/args/args.hh>
#include <paludis/paludis.hh>
#include <paludis/environment_maker.hh>
#include <paludis/environments/adapted/adapted_environment.hh>
#include <paludis/util/util.hh>
#include <paludis/util/system.hh>

#include <cstdlib>
#include <iostream>
#include <algorithm>

#include "target_config.hh"
#include "command_line.hh"
#include "stage.hh"
#include "config.h"
#include "stage_builder.hh"

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

struct DoVersion
{
};

int main(int argc, char *argv[])
{
    std::string options(paludis::getenv_with_default("CONTRARIUS_OPTIONS", ""));
    if (! options.empty())
        options = "(" + options + ") ";
    options += join(argv + 1, argv + argc, " ");

    Context context(std::string("In program ") + argv[0] + " " + options + ":");

    try
    {
        CommandLine::get_instance()->run(argc, argv, "contrarius", "CONTRARIUS_OPTIONS", "CONTRARIUS_CMDLINE");

        if (CommandLine::get_instance()->a_help.specified())
            throw DoHelp();

        if (CommandLine::get_instance()->a_log_level.specified())
            Log::get_instance()->set_log_level(CommandLine::get_instance()->a_log_level.option());
        else
            Log::get_instance()->set_log_level(ll_qa);

        if (CommandLine::get_instance()->a_version.specified())
            throw DoVersion();

        Log::get_instance()->message("contrarius.sucks", ll_warning, lc_no_context) <<
            "Contrarius is mostly undocumented, unreliable and unsupported. If you're lucky, it might work "
            "exactly as expected. But it probably won't, and when it breaks, we can't really help you. You "
            "are welcome to send patches.";

        if (! CommandLine::get_instance()->a_target.specified())
            throw DoHelp("you need to specify a --target");

        std::string stage;
        if (CommandLine::get_instance()->a_stage.specified())
            stage = CommandLine::get_instance()->a_stage.argument();
        else
            stage = "cxx";

        std::tr1::shared_ptr<AdaptedEnvironment> env(
                new AdaptedEnvironment(EnvironmentMaker::get_instance()->make_from_spec(
                        CommandLine::get_instance()->a_environment.argument())));

        StageOptions stage_opts(CommandLine::get_instance()->a_pretend.specified(),
            CommandLine::get_instance()->a_fetch.specified(),
            CommandLine::get_instance()->a_always_rebuild.specified());

        OurStageBuilderTask builder(stage_opts);

        do
        {
            if (! TargetConfig::get_instance()->aux().empty())
                builder.queue_stage(std::tr1::shared_ptr<const StageBase>(new AuxiliaryStage(env)));

            builder.queue_stage(std::tr1::shared_ptr<const StageBase>(new BinutilsStage(env)));

            if (stage == "binutils")
                break;

            if (CommandLine::get_instance()->a_headers.specified())
            {
                if (TargetConfig::get_instance()->headers().empty())
                    throw DoHelp("--headers specified though CTARGET does not need any headers");
                builder.queue_stage(std::tr1::shared_ptr<const StageBase>(new KernelHeadersStage(env)));
                builder.queue_stage(std::tr1::shared_ptr<const StageBase>(new LibCHeadersStage(env)));
            }

            builder.queue_stage(std::tr1::shared_ptr<const StageBase>(new MinimalStage(env)));

            if (stage == "minimal")
                break;

            if ((! CommandLine::get_instance()->a_headers.specified()) && 
                    (! TargetConfig::get_instance()->headers().empty()))
                builder.queue_stage(std::tr1::shared_ptr<const StageBase>(new KernelHeadersStage(env)));

            if (stage == "headers")
                break;

            builder.queue_stage(std::tr1::shared_ptr<const StageBase>(new LibCStage(env)));

            if (stage == "libc")
                break;

            builder.queue_stage(std::tr1::shared_ptr<const StageBase>(new FullStage(env)));

        } while (false);

        builder.execute();

        return EXIT_SUCCESS;
    }
    catch (const DoVersion &)
    {
        cout << "contrarius, part of " << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
            << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO;
        if (! std::string(PALUDIS_SUBVERSION_REVISION).empty())
            cout << " svn " << PALUDIS_SUBVERSION_REVISION;
        cout << endl << endl;
        cout << "Paludis comes with ABSOLUTELY NO WARRANTY. Paludis is free software, and you" << endl;
        cout << "are welcome to redistribute it under the terms of the GNU General Public" << endl;
        cout << "License, version 2." << endl;

        return EXIT_SUCCESS;
    }
    catch (const paludis::args::ArgsError & e)
    {
        cerr << "Usage error: " << e.message() << endl;
        cerr << "Try " << argv[0] << " --help" << endl;
        return EXIT_FAILURE;
    }
    catch (const DoHelp & h)
    {
        if (h.message.empty())
        {
            cout << "Usage: " << argv[0] << " --target CTARGET [options]" << endl;
            cout << endl;
            cout << *CommandLine::get_instance();
            return EXIT_SUCCESS;
        }
        else
        {
            cerr << "Usage error: " << h.message << endl;
            cerr << "Try " << argv[0] << " --help" << endl;
            return EXIT_FAILURE;
        }
    }
    catch (const Exception & e)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * " << e.backtrace("\n  * ")
            << e.message() << " (" << e.what() << ")" << endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception & e)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * Unknown exception type. Ouch..." << endl;
        return EXIT_FAILURE;
    }
}

