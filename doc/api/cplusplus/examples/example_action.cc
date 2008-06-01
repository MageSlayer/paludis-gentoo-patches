/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_action.cc "example_action.cc" .
 *
 * \ingroup g_actions
 */

/** \example example_action.cc
 *
 * This example demonstrates how to use actions. It uses FetchAction to fetch
 * source files for all versions of sys-apps/paludis that support fetching.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>

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
                "example_action", "EXAMPLE_ACTION_OPTIONS", "EXAMPLE_ACTION_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch package IDs for 'sys-apps/paludis'. */
        std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                    generator::Matches(make_package_dep_spec().package(QualifiedPackageName("sys-apps/paludis"))))]);

        /* For each ID: */
        for (PackageIDSet::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            /* Do we support a FetchAction? We find out by creating a
             * SupportsActionTest<FetchAction> object, and querying via the
             * PackageID::supports_action method. */
            SupportsActionTest<FetchAction> supports_fetch_action;
            if (! (*i)->supports_action(supports_fetch_action))
            {
                cout << "ID '" << **i << "' does not support the fetch action." << endl;
            }
            else
            {
                cout << "ID '" << **i << "' supports the fetch action, trying to fetch:" << endl;

                /* Carry out a FetchAction. We need to specify various options when
                 * creating a FetchAction, controlling whether safe resume is used
                 * and whether unneeded (e.g. due to disabled USE flags) source
                 * files should still be fetched. */
                FetchAction fetch_action(FetchActionOptions::named_create()
                        (k::fetch_unneeded(), false)
                        (k::safe_resume(), true)
                        );
                try
                {
                    (*i)->perform_action(fetch_action);
                }
                catch (const FetchActionError & e)
                {
                    exit_status |= 1;

                    cout << "Caught FetchActionError, with the following details:" << endl;

                    /* We might get detailed information about individual fetch
                     * failures.  */
                    for (Sequence<FetchActionFailure>::ConstIterator f(e.failures()->begin()), f_end(e.failures()->end()) ;
                            f != f_end ; ++f)
                    {
                        cout << "  * File '" << (*f)[k::target_file()] << "': ";

                        bool need_comma(false);
                        if ((*f)[k::requires_manual_fetching()])
                        {
                            cout << "requires manual fetching";
                            need_comma = true;
                        }

                        if ((*f)[k::failed_automatic_fetching()])
                        {
                            if (need_comma)
                                cout << ", ";
                            cout << "failed automatic fetching";
                            need_comma = true;
                        }

                        if (! (*f)[k::failed_integrity_checks()].empty())
                        {
                            if (need_comma)
                                cout << ", ";
                            cout << "failed integrity checks: " << (*f)[k::failed_integrity_checks()];
                            need_comma = true;
                        }
                    }

                    cout << endl;
                }
            }

            cout << endl;
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

