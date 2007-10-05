/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_environment.cc "example_environment.cc" .
 *
 * \ingroup g_environment
 */

/** \example example_environment.cc
 *
 * This example demonstrates how to use EnvironmentMaker and the resultant
 * Environment.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::left;
using std::setw;

int main(int argc, char * argv[])
{
    int exit_status(0);

    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_environment", "EXAMPLE_ENVIRONMENT_OPTIONS", "EXAMPLE_ENVIRONMENT_CMDLINE");

        /* We use EnvironmentMaker to construct an environment from the user's
         * --environment commandline choice. With an empty string, this uses the
         * distribution-defined default environment. With a non-empty string, it
         * is split into two parts upon the first colon (if there is no colon,
         * the second part is considered empty). The first part is the name of
         * the environment class to use (e.g. 'paludis', 'portage') and the
         * second part is passed as parameters to be handled by that
         * environment's constructor. */
        tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(
                    CommandLine::get_instance()->a_environment.argument()));

        /* A lot of the Environment members aren't very useful to clients. The
         * mask related methods are used by PackageID, and shouldn't usually be
         * called directly from clients. The system information and mirror
         * functions are mostly for use by Repository subclasses. That leaves
         * the package database, sets and (currently, although this may well
         * change in the future) use flag queries. The package database has its
         * own examples, so we'll start with sets: */

        tr1::shared_ptr<SetSpecTree::ConstItem> world(env->set(SetName("world")));
        if (world)
        {
            /* See \ref example_dep_tree.cc "example_dep_tree.cc" for how to
             * make use of this set. */
            cout << "World set exists" << endl;
        }
        else
            cout << "No world set defined" << endl;

        /* And use flags, for which we need a package IDs: */
        tr1::shared_ptr<const PackageIDSequence> ids(env->package_database()->query(
                    query::Matches(PackageDepSpec("sys-apps/paludis", pds_pm_permissive)) &
                    query::SupportsAction<InstalledAction>(),
                    qo_order_by_version));

        if (! ids->empty())
        {
            UseFlagName u("ruby");
            cout << "Use flag '" << u << "' for ID '" << **ids->rbegin() << "' is "
                << (env->query_use(u, **ids->rbegin()) ? "enabled" : "disabled") << endl;
        }
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


