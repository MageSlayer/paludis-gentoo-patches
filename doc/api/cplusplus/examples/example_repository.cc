/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_repository.cc "example_repository.cc" .
 *
 * \ingroup g_repository
 */

/** \example example_repository.cc
 *
 * This example demonstrates how to use Repository.
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
using std::left;
using std::setw;

int main(int argc, char * argv[])
{
    int exit_status(0);

    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_environment", "EXAMPLE_ENVIRONMENT_OPTIONS", "EXAMPLE_ENVIRONMENT_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* For each repository... */
        for (const auto & repository : env->repositories())
        {
            /* A repository is identified by its name. */
            cout << left << repository->name() << ":" << endl;

            /* Like a PackageID, a Repository has metadata. Usually metadata
             * keys will be available for all of the configuration options for
             * that repository; some repositories also provide more (ebuild
             * format repositories, for example, provide info_pkgs too). See
             * \ref example_metadata_key.cc "example_metadata_key.cc" for how to
             * display a metadata key in detail. */
            cout << left << setw(30) << "    Metadata keys:" << endl;
            for (const auto & key : repository->metadata())
                cout << "        " << key->human_name() << endl;

            /* Repositories support various methods for querying categories,
             * packages, IDs and so on. These methods are used by
             * Environment::operator[], but are also sometimes of direct use to
             * clients. */
            std::shared_ptr<const CategoryNamePartSet> cats(repository->category_names({ }));
            cout << left << setw(30) << "    Number of categories:" << " " << cats->size() << endl;
            std::shared_ptr<const PackageIDSequence> ids(repository->package_ids(QualifiedPackageName("sys-apps/paludis"), { }));
            cout << left << setw(30) << "    IDs for sys-apps/paludis:" << " " <<
                join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " ") << endl;

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



