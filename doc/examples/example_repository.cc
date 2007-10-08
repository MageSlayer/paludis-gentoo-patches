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
        tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(
                    CommandLine::get_instance()->a_environment.argument()));

        /* For each repository... */
        for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
                r_end(env->package_database()->end_repositories()) ;
                r != r_end ; ++r)
        {
            /* A repository is identified by its name. */
            cout << left << (*r)->name() << ":" << endl;

            /* Configuration and possibly additional information is available
             * via Repository::info. */
            tr1::shared_ptr<const RepositoryInfo> info((*r)->info(true));

            cout << left << setw(30) << "    Info:" << " " << endl;
            /* RepositoryInfo is made up of a number of sections... */
            for (RepositoryInfo::SectionConstIterator s(info->begin_sections()), s_end(info->end_sections()) ;
                    s != s_end ; ++s)
            {
                cout << left << setw(30) << ("        " + (*s)->heading() + ":") << " " << endl;
                /* And a section is made up of key+value pairs */
                for (RepositoryInfoSection::KeyValueConstIterator v((*s)->begin_kvs()), v_end((*s)->end_kvs()) ;
                        v != v_end ; ++v)
                    cout << left << setw(30) << ("            " + v->first + ":") << " " << v->second << endl;
            }

            /* Repositories support various methods for querying categories,
             * packages, IDs and so on. These methods are used by
             * PackageDatabase::query, but are also sometimes of direct use to
             * clients. */
            tr1::shared_ptr<const CategoryNamePartSet> cats((*r)->category_names());
            cout << left << setw(30) << "    Number of categories:" << " " << cats->size() << endl;
            tr1::shared_ptr<const PackageIDSequence> ids((*r)->package_ids(QualifiedPackageName("sys-apps/paludis")));
            cout << left << setw(30) << "    IDs for sys-apps/paludis:" << " " <<
                join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " ") << endl;

            /* Much of the Repository functionality is optional -- for example,
             * not all repositories support use flags as a concept, and not all
             * repositories are syncable, and merging only makes sense to
             * certain repositories. We gain access to optional functionality
             * via interface methods, all of which may return a zero pointer.
             * Many Repository interfaces are of little direct use to clients;
             * we cover only those that are here. */
            if ((*r)->installed_interface)
                cout << left << setw(30) << "    Root:" << " " << (*r)->installed_interface->root() << endl;
            if ((*r)->syncable_interface)
                cout << left << setw(30) << "    Syncable:" << " " << "yes" << endl;

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



