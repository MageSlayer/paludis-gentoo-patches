/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_stringify_formatter.cc "example_stringify_formatter.cc" .
 *
 * \ingroup g_formatters
 */

/** \example example_stringify_formatter.cc
 *
 * This example demonstrates how to use StringifyFormatter.
 *
 * See \ref example_formatter.cc "example_formatter.cc" for how to create
 * a custom Formatter.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <cstdlib>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;

int main(int argc, char * argv[])
{
    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_stringify_formatter", "EXAMPLE_STRINGIFY_FORMATTER_OPTIONS", "EXAMPLE_STRINGIFY_FORMATTER_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch package IDs for installable 'sys-apps/paludis'. */
        std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                    generator::Package(QualifiedPackageName("sys-apps/paludis")) |
                    filter::SupportsAction<InstallAction>())]);

        /* For each ID: */
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            cout << stringify(**i) << ":" << endl;

            /* Our formatter. It has no saved state, so we can use a single
             * formatter for all of the keys. */
            StringifyFormatter formatter;

            if ((*i)->keywords_key())
            {
                cout << "    " << (*i)->keywords_key()->human_name() << ":" << endl;
                cout << "        " << (*i)->keywords_key()->pretty_print_flat(formatter) << endl;
            }

            if ((*i)->homepage_key())
            {
                cout << "    " << (*i)->homepage_key()->human_name() << ":" << endl;
                cout << "        " << (*i)->homepage_key()->pretty_print_flat(formatter) << endl;
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

    return EXIT_SUCCESS;
}




