/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_query.cc "example_query.cc" .
 *
 * \ingroup g_query
 */

/** \example example_query.cc
 *
 * This example demonstrates how to use the standard Query classes. For custom
 * Query subclasses, see \ref example_query_delegate.cc
 * "example_query_delegate.cc".
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <algorithm>
#include <iterator>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;

namespace
{
    /* Run a particular query, and show its results. */
    void show_query(const std::tr1::shared_ptr<const Environment> & env, const Query & query)
    {
        /* Queries support a crude form of stringification. */
        cout << query << ":" << endl;

        /* Usually the only thing clients will do with a Query object is pass it
         * to PackageDatabase::query. */
        std::tr1::shared_ptr<const PackageIDSequence> ids(env->package_database()->query(query, qo_order_by_version));

        /* Show the results */
        if (! ids->empty())
            std::copy(indirect_iterator(ids->begin()), indirect_iterator(ids->end()),
                    std::ostream_iterator<const PackageID>(cout, "\n"));
        cout << endl;
    }
}

int main(int argc, char * argv[])
{
    int exit_status(0);

    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_action", "EXAMPLE_ACTION_OPTIONS", "EXAMPLE_ACTION_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Make some queries, and display what they give. */
        show_query(env, query::Matches(make_package_dep_spec().package(QualifiedPackageName("sys-apps/paludis"))));

        /* Queries can be combined. The resulting query is optimised internally,
         * potentially giving better performance than doing things by hand. */
        show_query(env,
                query::Matches(make_package_dep_spec().package(QualifiedPackageName("sys-apps/paludis"))) &
                query::SupportsAction<InstalledAction>());

        /* Usually query::NotMasked should be combined with
         * query::SupportsAction<InstallAction>, since installed packages aren't
         * masked. */
        show_query(env,
                query::Matches(make_package_dep_spec().package(QualifiedPackageName("sys-apps/paludis"))) &
                query::SupportsAction<InstallAction>() &
                query::NotMasked());

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


