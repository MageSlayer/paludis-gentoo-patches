/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_selection.cc "example_selection.cc" .
 *
 * \ingroup g_selection
 */

/** \example example_selection.cc
 *
 * This example demonstrates how to use the standard Selection, Generator and
 * Filter classes.
 **/

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
    /* Run a particular selection, and show its results. */
    void show_selection(const std::shared_ptr<const Environment> & env, const Selection & selection)
    {
        /* Selections support a crude form of stringification. */
        cout << selection << ":" << endl;

        /* Usually the only thing clients will do with a Selection object is pass it
         * to Environment::operator[]. */
        std::shared_ptr<const PackageIDSequence> ids((*env)[selection]);

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
                "example_selection", "EXAMPLE_SELECTION_OPTIONS", "EXAMPLE_SELECTION_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Make some selections, and display what they give. The selection
         * object used determines the number and ordering of results. In the
         * simplest form, it takes a Generator as a parameter. */
        show_selection(env, selection::AllVersionsSorted(
                    generator::Matches(make_package_dep_spec(PartiallyMadePackageDepSpecOptions()).package(
                            QualifiedPackageName("sys-apps/paludis")), MatchPackageOptions())));

        /* Generators can be passed through a Filter. The Selection optimises
         * the code internally to avoid doing excess work. */
        show_selection(env, selection::AllVersionsSorted(
                    generator::Matches(make_package_dep_spec(PartiallyMadePackageDepSpecOptions()).package(
                            QualifiedPackageName("sys-apps/paludis")), MatchPackageOptions()) |
                    filter::InstalledAtRoot(FSEntry("/"))));

        /* Filters can be combined. Usually filter::NotMasked should be combined
         * with filter::SupportsAction<InstallAction>, since installed packages
         * aren't masked. */
        show_selection(env, selection::AllVersionsSorted(
                    generator::Matches(make_package_dep_spec(PartiallyMadePackageDepSpecOptions()).package(
                            QualifiedPackageName("sys-apps/paludis")), MatchPackageOptions()) |
                    filter::SupportsAction<InstallAction>() |
                    filter::NotMasked()));

        /* selection::AllVersionsSorted can be expensive, particularly if there
         * is no metadata cache. Consider using other Selection objects if
         * you only need the best matching or some arbitrary matching ID. */
        show_selection(env, selection::BestVersionOnly(
                    generator::Matches(make_package_dep_spec(PartiallyMadePackageDepSpecOptions()).package(
                            QualifiedPackageName("sys-apps/paludis")), MatchPackageOptions()) |
                    filter::SupportsAction<InstallAction>() |
                    filter::NotMasked()));
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


