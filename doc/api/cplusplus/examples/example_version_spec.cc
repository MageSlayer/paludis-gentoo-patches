/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_version_spec.cc "example_version_spec.cc" .
 *
 * \ingroup g_names
 */

/** \example example_version_spec.cc
 *
 * This example demonstrates how to use VersionSpec.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <set>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::setw;
using std::left;
using std::boolalpha;
using std::hex;

int main(int argc, char * argv[])
{
    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_version_spec", "EXAMPLE_VERSION_SPEC_OPTIONS", "EXAMPLE_VERSION_SPEC_CMDLINE");

        /* Make a set of versions. Use user_version_spec_options() for the
         * options parameter for VersionSpec's constructor for handling any
         * user-inputted data. */
        std::set<VersionSpec> versions;
        versions.insert(VersionSpec("1.0", user_version_spec_options()));
        versions.insert(VersionSpec("1.1", user_version_spec_options()));
        versions.insert(VersionSpec("1.2", user_version_spec_options()));
        versions.insert(VersionSpec("1.2-r1", user_version_spec_options()));
        versions.insert(VersionSpec("2.0", user_version_spec_options()));
        versions.insert(VersionSpec("2.0-try1", user_version_spec_options()));
        versions.insert(VersionSpec("2.0-scm", user_version_spec_options()));
        versions.insert(VersionSpec("9999", user_version_spec_options()));

        /* For each version... */
        for (const auto & version : versions)
        {
            /* Versions are stringifiable */
            cout << version << ":" << endl;

            /* Show the output of various members. Not all of these are of much
             * direct use. */
            cout << "    " << left << setw(24) << "Hash value:" << " " << "0x" << hex << version.hash() << endl;
            cout << "    " << left << setw(24) << "Remove revision:" << " " << version.remove_revision() << endl;
            cout << "    " << left << setw(24) << "Revision only:" << " " << version.revision_only() << endl;
            cout << "    " << left << setw(24) << "Bump:" << " " << version.bump() << endl;
            cout << "    " << left << setw(24) << "Is scm?" << " " << boolalpha << version.is_scm() << endl;
            cout << "    " << left << setw(24) << "Has -try?" << " " << boolalpha << version.has_try_part() << endl;
            cout << "    " << left << setw(24) << "Has -scm?" << " " << boolalpha << version.has_scm_part() << endl;
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




