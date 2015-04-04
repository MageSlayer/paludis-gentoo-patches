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
#include <cstdlib>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;

namespace
{
    /* Some actions need an OutputManager, but to avoid chicken / egg problems
     * they take a function that creates an OutputManager as a parameter. Here
     * we just use a StandardOutputManager, which sticks everything to stdout /
     * stderr. More complex clients may use Environment::create_output_manager
     * to use the user's preferences for logging etc. */
    std::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return std::make_shared<StandardOutputManager>();
    }

    /* We just want to run all phases for actions. */
    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
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
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch package IDs for 'sys-apps/paludis'. */
        std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                    generator::Package(QualifiedPackageName("sys-apps/paludis")))]);

        /* For each ID: */
        for (const auto & id : *ids)
        {
            /* Failures go here: */
            const std::shared_ptr<Sequence<FetchActionFailure> > failures(std::make_shared<Sequence<FetchActionFailure>>());

            /* Do we support a FetchAction? We find out by creating a
             * SupportsActionTest<FetchAction> object, and querying via the
             * PackageID::supports_action method. */
            SupportsActionTest<FetchAction> supports_fetch_action;
            if (! id->supports_action(supports_fetch_action))
            {
                cout << "ID '" << *id << "' does not support the fetch action." << endl;
            }
            else
            {
                cout << "ID '" << *id << "' supports the fetch action, trying to fetch:" << endl;

                /* Carry out a FetchAction. We need to specify various options when
                 * creating a FetchAction, controlling whether safe resume is used
                 * and whether unneeded (e.g. due to disabled USE flags) and
                 * unmirrorable source files should still be fetched. */
                FetchAction fetch_action(make_named_values<FetchActionOptions>(
                            n::cross_compile_host() = "",
                            n::errors() = failures,
                            n::exclude_unmirrorable() = false,
                            n::fetch_parts() = FetchParts() + fp_regulars + fp_extras,
                            n::ignore_not_in_manifest() = false,
                            n::ignore_unfetched() = false,
                            n::make_output_manager() = &make_standard_output_manager,
                            n::safe_resume() = true,
                            n::tool_prefix() = "",
                            n::want_phase() = &want_all_phases
                            ));
                try
                {
                    id->perform_action(fetch_action);
                }
                catch (const ActionFailedError & e)
                {
                    exit_status |= 1;

                    cout << "Caught FetchActionError, with the following details:" << endl;

                    /* We might get detailed information about individual fetch
                     * failures.  */
                    for (const auto & failure : *failures)
                    {
                        cout << "  * File '" << failure.target_file() << "': ";

                        bool need_comma(false);
                        if (failure.requires_manual_fetching())
                        {
                            cout << "requires manual fetching";
                            need_comma = true;
                        }

                        if (failure.failed_automatic_fetching())
                        {
                            if (need_comma)
                                cout << ", ";
                            cout << "failed automatic fetching";
                            need_comma = true;
                        }

                        if (! failure.failed_integrity_checks().empty())
                        {
                            if (need_comma)
                                cout << ", ";
                            cout << "failed integrity checks: " << failure.failed_integrity_checks();
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

