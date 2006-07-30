/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/paludis.hh>
#include <paludis/util/log.hh>

#include <libebt/libebt_version.hh>

#include <libwrapiter/libwrapiter_version.hh>

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>

#include <iostream>

#include "command_line.hh"
#include "main_window.hh"

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    struct DoVersion
    {
    };

    struct GtkInitFailed : Exception
    {
        GtkInitFailed() :
            Exception ("Couldn't initialize gtk")
        {
        }
    };

    class TryMain : public Gtk::Main
    {
        static bool _gtkmm_initialized;

    public:
        TryMain(int& argc, char**& argv) :
            Gtk::Main()
        {
            _gtkmm_initialized = gtk_init_check(&argc, &argv);
        }

        bool initialized() const
        {
            return _gtkmm_initialized;
        }

        static void run(Gtk::Window& window)
        {
            if (! _gtkmm_initialized)
                throw GtkInitFailed();
    
            Gtk::Main::run(window);
        }

        static void run()
        {
            if (! _gtkmm_initialized)
                throw GtkInitFailed();
    
            Gtk::Main::run();
        }
    };


    bool TryMain::_gtkmm_initialized;

    void display_version()
    {
        cout << "gtkpaludis " << PALUDIS_VERSION_MAJOR << "."
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
#if HAVE_SANDBOX
        cout << "sandbox:     enabled" << endl;
#else
        cout << "sandbox:     disabled" << endl;
#endif
    }
}

int
main(int argc, char * argv[])
{

    Context context("In main program:");

    TryMain gui_kit(argc, argv);

    try
    {
        {
            Context context_local("When handling command line:");
            CommandLine::get_instance()->run(argc, argv);

            if (CommandLine::get_instance()->a_help.specified())
                throw DoHelp();

            if (CommandLine::get_instance()->a_version.specified())
                throw DoVersion();

            if (! CommandLine::get_instance()->a_log_level.specified())
                Log::get_instance()->set_log_level(ll_qa);
            else if (CommandLine::get_instance()->a_log_level.argument() == "debug")
                Log::get_instance()->set_log_level(ll_debug);
            else if (CommandLine::get_instance()->a_log_level.argument() == "qa")
                Log::get_instance()->set_log_level(ll_qa);
            else if (CommandLine::get_instance()->a_log_level.argument() == "warning")
                Log::get_instance()->set_log_level(ll_warning);
            else if (CommandLine::get_instance()->a_log_level.argument() == "silent")
                Log::get_instance()->set_log_level(ll_silent);
            else
                throw DoHelp("bad value for --log-level");
        }

        {
            Context context_local("When loading configuration:");

            std::string paludis_command(argv[0]);
            std::string::size_type last_slash(paludis_command.rfind('/'));
            if (std::string::npos == last_slash)
                last_slash = 0;
            if (0 == paludis_command.compare(last_slash, 3, "gtk"))
                paludis_command.erase(last_slash, 3);

            if (CommandLine::get_instance()->a_config_suffix.specified())
            {
                DefaultConfig::set_config_suffix(CommandLine::get_instance()->a_config_suffix.argument());
                paludis_command.append(" --config-suffix " +
                        CommandLine::get_instance()->a_config_suffix.argument());
            }

            paludis_command.append(" --log-level " + CommandLine::get_instance()->a_log_level.argument());
            DefaultConfig::get_instance()->set_paludis_command(paludis_command);
        }

        {
            Context context_local("When displaying main window:");

            if (! gui_kit.initialized())
                throw GtkInitFailed();

            TryMain::run(*MainWindow::get_instance());
        }
    }
    catch (const DoVersion &)
    {
        display_version();
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
    catch (const DoHelp & h)
    {
        if (h.message.empty())
        {
            cout << "Usage: " << argv[0] << " [options]" << endl;
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
    catch (GtkInitFailed & e)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * " << e.backtrace("\n  * ")
            << e.message() << " (" << e.what() << ")\n" << endl;

        cerr << "Try adjust DISPLAY environment variable or pass --display option\n" << endl;

        return EXIT_FAILURE;
    }
    catch (const Exception & e)
    {
        if (gui_kit.initialized())
        {
            Gtk::Main::init_gtkmm_internals();
            Gtk::MessageDialog dialog("Unhandled exception", false, Gtk::MESSAGE_ERROR);
            dialog.set_secondary_text(
                    "- " + e.backtrace("\n- ") + e.message() + " (" + e.what() + ")");
            dialog.run();
        }
        else
        {
            cout << endl;
            cerr << "Unhandled exception:" << endl
                << "  * " << e.backtrace("\n  * ")
                << e.message() << " (" << e.what() << ")" << endl;
        }
        return EXIT_FAILURE;
    }
    catch (const std::exception & e)
    {
        if (gui_kit.initialized())
        {
            Gtk::MessageDialog dialog("Unhandled exception", false, Gtk::MESSAGE_ERROR);
            dialog.set_secondary_text("Unhandled exception (" + stringify(e.what()) + ")");
            dialog.run();
        }
        else
        {
            cout << endl;
            cerr << "Unhandled exception:" << endl
                << "  * " << e.what() << endl;
        }
        return EXIT_FAILURE;
    }
    catch (...)
    {
        if (gui_kit.initialized())
        {
            Gtk::MessageDialog dialog("Unhandled exception", false, Gtk::MESSAGE_ERROR);
            dialog.set_secondary_text("Unhandled exception (unknown type)");
            dialog.run();
        }
        else
        {        cout << endl;
                 cerr << "Unhandled exception:" << endl
                     << "  * Unknown exception type. Ouch..." << endl;
        }
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

