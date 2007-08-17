/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/qa.hh>
#include <paludis/util/join.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/virtual_constructor-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/environments/no_config/no_config_environment.hh>

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <set>

#include <libebt/libebt.hh>
#include <libwrapiter/libwrapiter.hh>

#include "qualudis_command_line.hh"
#include <src/output/colour.hh>
#include <src/common_args/do_help.hh>

#include "config.h"

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    struct DoVersion
    {
    };

    struct QualudisReporter :
        QAReporter
    {
        FSEntry previous_entry;
        std::string previous_name;

        QualudisReporter() :
            previous_entry("/NONE"),
            previous_name("NONE")
        {
        }

        void message(const QAMessage & msg)
        {
            if (previous_entry != msg.entry)
            {
                std::cout << colour(cl_package_name, strip_leading_string(stringify(msg.entry.strip_leading(FSEntry::cwd())), "/"))
                    << ":" << std::endl;
                previous_entry = msg.entry;
                previous_name = "NONE";
            }

            if (previous_name != msg.name)
            {
                std::cout << "  " << msg.name << ":" << std::endl;
                previous_name = msg.name;
            }

            std::cout << "    [";
            switch (msg.level)
            {
                case qaml_maybe:
                    std::cout << "?";
                    break;

                case qaml_debug:
                    std::cout << "-";
                    break;

                case qaml_minor:
                    std::cout << "*";
                    break;

                case qaml_normal:
                    std::cout << colour(cl_error, "*");
                    break;

                case qaml_severe:
                    std::cout << colour(cl_error, "!");
                    break;

                case last_qaml:
                    break;
            }

            std::cout << "]: " << msg.message << std::endl;
        }
    };
}

int main(int argc, char *argv[])
{
    Context context("In main program:");

    try
    {
        QualudisCommandLine::get_instance()->run(argc, argv, "qualudis", "QUALUDIS_OPTIONS",
                "QUALUDIS_CMDLINE");

        if (QualudisCommandLine::get_instance()->a_help.specified())
            throw args::DoHelp();

        if (QualudisCommandLine::get_instance()->a_log_level.specified())
            Log::get_instance()->set_log_level(QualudisCommandLine::get_instance()->a_log_level.option());
        else
            Log::get_instance()->set_log_level(ll_qa);

        if (! QualudisCommandLine::get_instance()->a_message_level.specified())
            QualudisCommandLine::get_instance()->message_level = qaml_maybe;
        else if (QualudisCommandLine::get_instance()->a_message_level.argument() == "debug")
            QualudisCommandLine::get_instance()->message_level = qaml_debug;
        else if (QualudisCommandLine::get_instance()->a_message_level.argument() == "maybe")
            QualudisCommandLine::get_instance()->message_level = qaml_maybe;
        else if (QualudisCommandLine::get_instance()->a_message_level.argument() == "minor")
            QualudisCommandLine::get_instance()->message_level = qaml_minor;
        else if (QualudisCommandLine::get_instance()->a_message_level.argument() == "normal")
            QualudisCommandLine::get_instance()->message_level = qaml_normal;
        else if (QualudisCommandLine::get_instance()->a_message_level.argument() == "severe")
            QualudisCommandLine::get_instance()->message_level = qaml_severe;
        else
            throw args::DoHelp("bad value for --message-level");

        if (QualudisCommandLine::get_instance()->a_version.specified())
            throw DoVersion();

        if (! QualudisCommandLine::get_instance()->a_write_cache_dir.specified())
            QualudisCommandLine::get_instance()->a_write_cache_dir.set_argument("/var/empty");

        if (! QualudisCommandLine::get_instance()->a_master_repository_dir.specified())
            QualudisCommandLine::get_instance()->a_master_repository_dir.set_argument("/var/empty");

        tr1::shared_ptr<NoConfigEnvironment> env(new NoConfigEnvironment(no_config_environment::Params::create()
                    .repository_dir(FSEntry::cwd())
                    .write_cache(QualudisCommandLine::get_instance()->a_write_cache_dir.argument())
                    .accept_unstable(false)
                    .repository_type(no_config_environment::ncer_ebuild)
                    .master_repository_dir(QualudisCommandLine::get_instance()->a_master_repository_dir.argument())
                    .disable_metadata_cache(! QualudisCommandLine::get_instance()->a_use_repository_cache.specified())
                    ));

        if (! env->main_repository()->qa_interface)
            throw ConfigurationError("Repository '" + stringify(env->main_repository()->name()) + "' does not support QA checks");

        QualudisReporter r;
        env->main_repository()->qa_interface->check_qa(
                r,
                QACheckProperties(),
                QACheckProperties(),
                QualudisCommandLine::get_instance()->message_level,
                env->main_repository_dir());
    }
    catch (const DoVersion &)
    {
        cout << "qualudis, part of " << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
            << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO;
        if (! std::string(PALUDIS_SUBVERSION_REVISION).empty())
            cout << " svn " << PALUDIS_SUBVERSION_REVISION;
        cout << endl << endl;
        cout << "Built by " << PALUDIS_BUILD_USER << "@" << PALUDIS_BUILD_HOST
            << " on " << PALUDIS_BUILD_DATE << endl;
        cout << "CXX:         " << PALUDIS_BUILD_CXX
#if defined(__ICC)
            << " " << __ICC
#elif defined(__VERSION__)
            << " " << __VERSION__
#endif
            << endl;
        cout << "CXXFLAGS:    " << PALUDIS_BUILD_CXXFLAGS << endl;
        cout << "LDFLAGS:     " << PALUDIS_BUILD_LDFLAGS << endl;
        cout << "SYSCONFDIR:  " << SYSCONFDIR << endl;
        cout << "LIBEXECDIR:  " << LIBEXECDIR << endl;
        cout << "BIGTEMPDIR:  " << BIGTEMPDIR << endl;
        cout << "stdlib:      "
#if defined(__GLIBCXX__)
#  define XSTRINGIFY(x) #x
#  define STRINGIFY(x) XSTRINGIFY(x)
            << "GNU libstdc++ " << STRINGIFY(__GLIBCXX__)
#endif
            << endl;

        cout << "libebt:      " << LIBEBT_VERSION_MAJOR << "." << LIBEBT_VERSION_MINOR
            << "." << LIBEBT_VERSION_MICRO << endl;
        cout << "libwrapiter: " << LIBWRAPITER_VERSION_MAJOR << "." << LIBWRAPITER_VERSION_MINOR
            << "." << LIBWRAPITER_VERSION_MICRO << endl;

        cout << endl;
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
    catch (const args::DoHelp & h)
    {
        if (h.message.empty())
        {
            cout << "Usage: " << argv[0] << " [options]" << endl;
            cout << "   or: " << argv[0] << " [package/category ..]" << endl;
            cout << endl;
            cout << *QualudisCommandLine::get_instance();
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

