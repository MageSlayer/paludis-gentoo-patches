/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_environment.cc "example_environment.cc" .
 *
 * \ingroup g_environment
 */

/** \example example_environment.cc
 *
 * This example demonstrates how to use EnvironmentFactory and the resultant
 * Environment.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <cstdlib>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;

int main(int argc, char * argv[])
{
    int exit_status(0);

    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_environment", "EXAMPLE_ENVIRONMENT_OPTIONS", "EXAMPLE_ENVIRONMENT_CMDLINE");

        /* We use EnvironmentFactory to construct an environment from the user's
         * --environment commandline choice. With an empty string, this uses the
         * distribution-defined default environment. With a non-empty string, it
         * is split into two parts upon the first colon (if there is no colon,
         * the second part is considered empty). The first part is the name of
         * the environment class to use (e.g. 'paludis', 'portage') and the
         * second part is passed as parameters to be handled by that
         * environment's constructor. */
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* A lot of the Environment members aren't very useful to clients. The
         * mask related methods are used by PackageID, and shouldn't usually be
         * called directly from clients. The system information and mirror
         * functions are mostly for use by Repository subclasses. The []
         * operator is covered in \ref example_selection.cc
         * "example_selection.cc". That leaves the package database and sets.
         * The package database has its own examples, so we'll start with sets:
         * */

        std::shared_ptr<const SetSpecTree> world(env->set(SetName("world")));
        if (world)
        {
            cout << "World set exists" << endl;
        }
        else
            cout << "No world set defined" << endl;
    }
    catch (const Exception & e)
    {
        /* Paludis exceptions can provide a handy human-readable backtrace and
         * an explanation message. Where possible, these should be displayed. */
        cout << endl;
        cout << "Unhandled exception:" << endl
            << "  * " << e.backtrace("\n  * ")
            << e.message() << " (" << e.what() << ")" << endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception & e)
    {
        cout << endl;
        cout << "Unhandled exception:" << endl
            << "  * " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cout << endl;
        cout << "Unhandled exception:" << endl
            << "  * Unknown exception type. Ouch..." << endl;
        return EXIT_FAILURE;
    }

    return exit_status;
}


