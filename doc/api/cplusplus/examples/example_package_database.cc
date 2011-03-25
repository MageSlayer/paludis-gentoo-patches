/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_package_database.cc "example_package_database.cc" .
 *
 * \ingroup g_package_database
 */

/** \example example_package_database.cc
 *
 * This example demonstrates how to use PackageDatabase.
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
                "example_package_database", "EXAMPLE_PACKAGE_DATABASE_OPTIONS", "EXAMPLE_PACKAGE_DATABASE_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Mostly PackageDatabase is used by Environment. But some methods are useful: */
        if (env->package_database()->has_repository_named(RepositoryName("gentoo")))
        {
            std::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(RepositoryName("gentoo")));
            cout << "Repository 'gentoo' exists, and has format '" <<
                (repo->format_key() ? repo->format_key()->value() : "") << "'" << endl;
        }

        /* Users often expect to be able to refer to a package by its name part
         * only (e.g. 'foo' rather than 'app-misc/foo'). This has to be
         * disambiguated as follows: */
        try
        {
            QualifiedPackageName name(env->package_database()->fetch_unique_qualified_package_name(
                        PackageNamePart("git")));
            cout << "The only package named 'git' is '" << name << "'" << endl;
        }
        catch (const NoSuchPackageError & e)
        {
            cout << "There is no package named 'git'" << endl;
        }
        catch (const AmbiguousPackageNameError & e)
        {
            cout << "There are several packages named 'git':" << endl;
            for (AmbiguousPackageNameError::OptionsConstIterator o(e.begin_options()), o_end(e.end_options()) ;
                    o != o_end ; ++o)
                cout << "    " << *o << endl;
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


