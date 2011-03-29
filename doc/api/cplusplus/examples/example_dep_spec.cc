/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_dep_spec.cc "example_dep_spec.cc" .
 *
 * \ingroup g_dep_spec
 */

/** \example example_dep_spec.cc
 *
 * This example demonstrates how to handle dependency specs.
 *
 * See \ref example_dep_label.cc "example_dep_label.cc" for labels.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <list>
#include <map>
#include <sstream>

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
                "example_dep_spec", "EXAMPLE_DEP_SPEC_OPTIONS", "EXAMPLE_DEP_SPEC_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* For each command line parameter... */
        for (CommandLine::ParametersConstIterator q(CommandLine::get_instance()->begin_parameters()),
                q_end(CommandLine::get_instance()->end_parameters()) ; q != q_end ; ++q)
        {
            /* Create a PackageDepSpec from the parameter. For user-inputted
             * data, parse_user_package_dep_spec() should be used. If wildcards
             * are to be permitted, the updso_allow_wildcards option should be
             * included. If you might be getting a set, also include
             * updso_throw_if_set and catch the GotASetNotAPackageDepSpec
             * exception. If data about the spec is known at compile time,
             * make_package_dep_spec() should be used instead. */
            PackageDepSpec spec(parse_user_package_dep_spec(*q, env.get(), { updso_allow_wildcards }));

            /* Display information about the PackageDepSpec. */
            cout << "Information about '" << spec << "':" << endl;

            if (spec.package_name_constraint())
                cout << "    " << left << setw(24) << "Package:" << " " << spec.package_name_constraint()->name() << endl;

            if (spec.category_name_part_constraint())
                cout << "    " << left << setw(24) << "Category part:" << " " << spec.category_name_part_constraint()->name_part() << endl;

            if (spec.package_name_part_constraint())
                cout << "    " << left << setw(24) << "Package part:" << " " << spec.package_name_part_constraint()->name_part() << endl;

            if (spec.version_requirements_ptr() && ! spec.version_requirements_ptr()->empty())
            {
                cout << "    " << left << setw(24) << "Version requirements:" << " ";
                bool need_join(false);
                for (VersionRequirements::ConstIterator r(spec.version_requirements_ptr()->begin()),
                        r_end(spec.version_requirements_ptr()->end()) ; r != r_end ; ++r)
                {
                    if (need_join)
                    {
                        switch (spec.version_requirements_mode())
                        {
                            case vr_and:
                                cout << " and ";
                                break;

                            case vr_or:
                                cout << " or ";
                                break;

                            case last_vr:
                                throw InternalError(PALUDIS_HERE, "Bad version_requirements_mode");
                        }
                    }

                    cout << r->version_operator() << r->version_spec();
                    need_join = true;
                }
                cout << endl;
            }

            if (spec.slot_requirement_ptr())
                cout << "    " << left << setw(24) << "Slot:" << " " << *spec.slot_requirement_ptr() << endl;

            if (spec.in_repository_constraint())
                cout << "    " << left << setw(24) << "In repository:" << " " <<
                    spec.in_repository_constraint()->name() << endl;

            if (spec.from_repository_ptr())
                cout << "    " << left << setw(24) << "From repository:" << " " <<
                    *spec.from_repository_ptr() << endl;

            if (spec.installed_at_path_ptr())
                cout << "    " << left << setw(24) << "Installed at path:" << " " <<
                    *spec.installed_at_path_ptr() << endl;

            if (spec.installable_to_path_ptr())
                cout << "    " << left << setw(24) << "Installable to path:" << " " <<
                    spec.installable_to_path_ptr()->path() << ", " <<
                    spec.installable_to_path_ptr()->include_masked() << endl;

            if (spec.installable_to_repository_ptr())
                cout << "    " << left << setw(24) << "Installable to repository:" << " " <<
                    spec.installable_to_repository_ptr()->repository() << ", " <<
                    spec.installable_to_repository_ptr()->include_masked() << endl;

            if (spec.additional_requirements_ptr() && ! spec.additional_requirements_ptr()->empty())
            {
                cout << "    " << left << setw(24) << "Additional requirements:" << " ";
                bool need_join(false);
                for (AdditionalPackageDepSpecRequirements::ConstIterator u(spec.additional_requirements_ptr()->begin()),
                        u_end(spec.additional_requirements_ptr()->end()) ; u != u_end ; ++u)
                {
                    if (need_join)
                        cout << " and ";

                    cout << (*u)->as_raw_string() + " (meaning: " + (*u)->as_human_string(make_null_shared_ptr()) + ")";
                    need_join = true;
                }
                cout << endl;
            }

            /* And display packages matching that spec */
            cout << "    " << left << setw(24) << "Matches:" << " ";
            std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                        generator::Matches(spec, make_null_shared_ptr(), { }))]);
            bool need_indent(false);
            for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                    i != i_end ; ++i)
            {
                if (need_indent)
                    cout << "    " << left << setw(24) << "" << " ";
                cout << **i << endl;
                need_indent = true;
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

