/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_version_operator.cc "example_version_operator.cc" .
 *
 * \ingroup g_names
 */

/** \example example_version_operator.cc
 *
 * This example demonstrates how to use VersionOperator.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <set>
#include <list>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::setw;
using std::left;
using std::boolalpha;

int main(int argc, char * argv[])
{
    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_version_operator", "EXAMPLE_VERSION_OPERATOR_OPTIONS", "EXAMPLE_VERSION_OPERATOR_CMDLINE");

        /* Make a set of versions. Use user_version_spec_options() for the
         * options parameter for VersionSpec's constructor for handling any
         * user-inputted data. */
        std::set<VersionSpec> versions;
        versions.insert(VersionSpec("1.0", user_version_spec_options()));
        versions.insert(VersionSpec("1.1", user_version_spec_options()));
        versions.insert(VersionSpec("1.2", user_version_spec_options()));
        versions.insert(VersionSpec("1.2-r1", user_version_spec_options()));
        versions.insert(VersionSpec("2.0", user_version_spec_options()));

        /* And a list of operators */
        std::list<VersionOperator> operators;
        operators.push_back(VersionOperator("="));
        operators.push_back(VersionOperator(">="));
        operators.push_back(VersionOperator("~"));
        operators.push_back(VersionOperator("<"));
        operators.push_back(VersionOperator("~>"));

        /* Display a header */
        cout << " " << left << setw(8) << "LHS" << " | " << left << setw(8) << "RHS";
        for (const auto & op : operators)
            cout << " | " << setw(8) << op;
        cout << endl << std::string(10, '-');
        for (unsigned x(0) ; x <= operators.size() ; ++x)
            cout << "+" << std::string(10, '-');
        cout << endl;

        /* For each pair of versions... */
        for (const auto & version_lhs : versions)
        {
            for (const auto & version_rhs : versions)
            {
                cout << " " << left << setw(8) << version_lhs << " | " << left << setw(8) << version_rhs;

                /* Apply all of our operators, and show the results */
                for (const auto & op : operators)
                {
                    /* VersionOperator::as_version_spec_comparator returns a
                     * binary boolean functor. */
                    cout << " | " << left << setw(8) << boolalpha << (op.as_version_spec_comparator()(version_lhs, version_rhs));
                }

                cout << endl;
            }
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



