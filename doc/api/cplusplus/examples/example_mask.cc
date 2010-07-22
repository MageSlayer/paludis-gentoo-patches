/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_mask.cc "example_mask.cc" .
 *
 * \ingroup g_mask
 */

/** \example example_mask.cc
 *
 * This example demonstrates how to use Mask. It displays all the
 * mask keys for a particular PackageID.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <set>
#include <time.h>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::left;
using std::setw;

namespace
{
    /* We use this visitor to display extra information about a Mask,
     * depending upon its type. */
    class MaskInformationVisitor
    {
        public:
            void visit(const UserMask &)
            {
                cout << left << setw(30) << "    Class:" << " " << "UserMask" << endl;
            }

            void visit(const UnacceptedMask & mask)
            {
                cout << left << setw(30) << "    Class:" << " " << "UnacceptedMask" << endl;
                if (mask.unaccepted_key())
                    cout << left << setw(30) << "    Unaccepted key:" << " " << mask.unaccepted_key()->raw_name() << endl;
            }

            void visit(const RepositoryMask & mask)
            {
                cout << left << setw(30) << "    Class:" << " " << "RepositoryMask" << endl;
                if (mask.mask_key())
                    cout << left << setw(30) << "    Mask key:" << " " << mask.mask_key()->raw_name() << endl;
            }

            void visit(const UnsupportedMask & mask)
            {
                cout << left << setw(30) << "    Class:" << " " << "UnsupportedMask" << endl;
                cout << left << setw(30) << "    Explanation:" << " " << mask.explanation() << endl;
            }

            void visit(const AssociationMask & mask)
            {
                cout << left << setw(30) << "    Class:" << " " << "AssociationMask" << endl;
                if (mask.associated_package())
                    cout << left << setw(30) << "    Associated package:" << " " << *mask.associated_package() << endl;
            }
    };
}

int main(int argc, char * argv[])
{
    int exit_status(0);

    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_mask", "EXAMPLE_MASK_OPTIONS", "EXAMPLE_MASK_CMDLINE");

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
            cout << **i << ":" << endl;

            /* For each mask key: */
            for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                    m != m_end ; ++m)
            {
                /* All Mask instances have two basic bits of information: a one
                 * character short key, and a longer description. */
                cout << left << setw(30) << "    Key:" << " " << std::string(1, (*m)->key()) << endl;
                cout << left << setw(30) << "    Description:" << " " << (*m)->description() << endl;

                /* To display more information about a Mask we create a visitor
                 * that visits the appropriate subtype. */
                MaskInformationVisitor v;
                (*m)->accept(v);

                cout << endl;
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

    return exit_status;
}

