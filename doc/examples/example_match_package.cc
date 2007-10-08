/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_match_package.cc "example_match_package.cc" .
 *
 * \ingroup g_query
 */

/** \example example_match_package.cc
 *
 * This example demonstrates how to use paludis::match_package and
 * paludis::match_package_in_set.
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
                "example_match_package", "EXAMPLE_MATCH_PACKAGE_OPTIONS", "EXAMPLE_MATCH_PACKAGE_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch all installed packages. */
        tr1::shared_ptr<const PackageIDSequence> ids(env->package_database()->query(
                    query::SupportsAction<InstalledAction>(),
                    qo_order_by_version));

        /* Fetch the 'system' and 'world' sets. Ordinarily we should check for
         * zero pointers here, but these two sets will always exist. */
        tr1::shared_ptr<const SetSpecTree::ConstItem> system(env->set(SetName("system"))),
            world(env->set(SetName("world")));

        /* For each ID: */
        for (PackageIDSet::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            /* Is it paludis? */
            if (match_package(*env, PackageDepSpec("sys-apps/paludis", pds_pm_permissive), **i))
                cout << left << setw(50) << (stringify(**i) + ":") << " " << "paludis" << endl;

            /* No. Is it in system or world? */
            else if (match_package_in_set(*env, *system, **i))
                cout << left << setw(50) << (stringify(**i) + ":") << " " << "system" << endl;
            else if (match_package_in_set(*env, *world, **i))
                cout << left << setw(50) << (stringify(**i) + ":") << " " << "world" << endl;
            else
                cout << left << setw(50) << (stringify(**i) + ":") << " " << "nothing" << endl;
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


