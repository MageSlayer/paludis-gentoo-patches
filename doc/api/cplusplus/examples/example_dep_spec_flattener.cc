/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_dep_spec_flattener.cc "example_dep_spec_flattener.cc" .
 *
 * \ingroup g_dep_spec
 */

/** \example example_dep_spec_flattener.cc
 *
 * This example demonstrates how to use DepSpecFlattener. It extracts various
 * metadata items from a package.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <set>
#include <map>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::setw;
using std::left;

int main(int argc, char * argv[])
{
    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_dep_spec_flattener", "EXAMPLE_DEP_SPEC_FLATTENER_OPTIONS", "EXAMPLE_DEP_SPEC_FLATTENER_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch package IDs for all installed packages. */
        std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                    generator::All() |
                    filter::InstalledAtSlash())]);

        /* For each ID: */
        for (const auto & id : *ids)
        {
            cout << "Information about '" << *id << "':" << endl;

            /* Do we have a homepage? All PackageID key methods may return a
             * null pointer. */
            if (id->homepage_key())
            {
                DepSpecFlattener<SimpleURISpecTree, SimpleURIDepSpec> homepages(env.get(), id);
                id->homepage_key()->parse_value()->top()->accept(homepages);

                cout << "    " << left << setw(24) << "Homepages:" << " "
                    << join(indirect_iterator(homepages.begin()), indirect_iterator(homepages.end()), " ")
                    << endl;
            }

            /* And again for restricts. There's no global restrict key, since
             * it has no meaning outside of the repositories that support it.
             * Instead, we use PackageID::find_metadata to see if the key we
             * want exists, and then visitor_cast<> to see whether it's
             * of a suitable type (the key could be something other than a
             * MetadataSpecTreeKey<PlainTextSpecTree>). */
            if (id->end_metadata() != id->find_metadata("RESTRICT") &&
                    visitor_cast<const MetadataSpecTreeKey<PlainTextSpecTree> >(**id->find_metadata("RESTRICT")))
            {
                DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(env.get(), id);

                visitor_cast<const MetadataSpecTreeKey<PlainTextSpecTree> >(
                        **id->find_metadata("RESTRICT"))->parse_value()->top()->accept(restricts);

                cout << "    " << left << setw(24) << "Restricts:" << " "
                    << join(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()), " ")
                    << endl;
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


