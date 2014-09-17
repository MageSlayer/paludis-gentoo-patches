/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_package_id.cc "example_package_id.cc" .
 *
 * \ingroup g_package_id
 */

/** \example example_package_id.cc
 *
 * This example demonstrates how to use PackageID.
 *
 * See \ref example_action.cc "example_action.cc" for more on actions. See \ref
 * example_metadata_key.cc "example_metadata_key.cc" for more on metadata keys.
 * See \ref example_mask.cc "example_mask.cc" for more on masks.
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
using std::left;
using std::setw;
using std::hex;
using std::boolalpha;

int main(int argc, char * argv[])
{
    int exit_status(0);

    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_package_id", "EXAMPLE_PACKAGE_ID_OPTIONS", "EXAMPLE_PACKAGE_ID_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch package IDs for 'sys-apps/paludis'. */
        std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                    generator::Package(QualifiedPackageName("sys-apps/paludis")))]);

        /* For each ID: */
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            /* IDs can be written to an ostream. */
            cout << **i << ":" << endl;

            /* Start by outputting some basic properties: */
            cout << left << setw(30) << "    Name:" << " " << (*i)->name() << endl;
            cout << left << setw(30) << "    Version:" << " " << (*i)->version() << endl;
            cout << left << setw(30) << "    Repository Name:" << " " << (*i)->repository_name() << endl;

            /* The PackageID::canonical_form method should be used when
             * outputting a package (the ostream << operator uses this). */
            cout << left << setw(30) << "    idcf_full:" << " " << (*i)->canonical_form(idcf_full) << endl;
            cout << left << setw(30) << "    idcf_version:" << " " << (*i)->canonical_form(idcf_version) << endl;
            cout << left << setw(30) << "    idcf_no_version:" << " " << (*i)->canonical_form(idcf_no_version) << endl;
            cout << left << setw(30) << "    idcf_no_name:" << " " << (*i)->canonical_form(idcf_no_name) << endl;

            /* Let's see what keys we have. Other examples cover keys in depth,
             * so we'll just use the basic methods here. */
            cout << left << setw(30) << "    Keys:" << " " << endl;
            for (PackageID::MetadataConstIterator k((*i)->begin_metadata()), k_end((*i)->end_metadata()) ;
                    k != k_end ; ++k)
                cout << left << setw(30) << ("        " + (*k)->raw_name() + ":") << " " << (*k)->human_name() << endl;

            /* And what about masks? Again, these are covered in depth
             * elsewhere. */
            if ((*i)->masked())
            {
                cout << left << setw(30) << "    Masks:" << " " << endl;
                for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                        m != m_end ; ++m)
                    cout << left << setw(30) << ("        " + stringify((*m)->key()) + ":") << " " << (*m)->description() << endl;
            }

            /* Let's see which actions we support. There's no particularly nice
             * way of doing this, since it's not something we'd expect to be
             * doing. */
            std::set<std::string> actions;
            {
                SupportsActionTest<InstallAction> install_action;
                if ((*i)->supports_action(install_action))
                    actions.insert("install");

                SupportsActionTest<UninstallAction> uninstall_action;
                if ((*i)->supports_action(uninstall_action))
                    actions.insert("uninstall");

                SupportsActionTest<PretendAction> pretend_action;
                if ((*i)->supports_action(pretend_action))
                    actions.insert("pretend");

                SupportsActionTest<ConfigAction> config_action;
                if ((*i)->supports_action(config_action))
                    actions.insert("config");

                SupportsActionTest<UninstallAction> fetch_action;
                if ((*i)->supports_action(fetch_action))
                    actions.insert("fetch");

                SupportsActionTest<UninstallAction> info_action;
                if ((*i)->supports_action(info_action))
                    actions.insert("info");
            }
            cout << left << setw(30) << "    Actions:" << " " << join(actions.begin(), actions.end(), " ") << endl;

            /* And various misc methods. Clients don't usually use these
             * directly. */
            cout << left << setw(30) << "    extra_hash_value:" << " " << "0x" << hex << (*i)->extra_hash_value() << endl;

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


