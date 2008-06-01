/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_contents.cc "example_contents.cc" .
 *
 * \ingroup g_contents
 */

/** \example example_contents.cc
 *
 * This example demonstrates how to use contents. It displays details about
 * the files installed by 'sys-apps/paludis'.
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

namespace
{
    /* This visitor displays information about ContentsEntry subclasses. */
    class ContentsPrinter :
        public ConstVisitor<ContentsVisitorTypes>
    {
        public:
            void visit(const ContentsDevEntry & d)
            {
                cout << left << setw(10) << "device" << d.name() << endl;
            }

            void visit(const ContentsMiscEntry & d)
            {
                cout << left << setw(10) << "misc" << d.name() << endl;
            }

            void visit(const ContentsFileEntry & d)
            {
                cout << left << setw(10) << "file" << d.name() << endl;
            }

            void visit(const ContentsDirEntry & d)
            {
                cout << left << setw(10) << "dir" << d.name() << endl;
            }

            void visit(const ContentsFifoEntry & d)
            {
                cout << left << setw(10) << "fifo" << d.name() << endl;
            }

            void visit(const ContentsSymEntry & d)
            {
                cout << left << setw(10) << "sym" << d.name() << " -> " << d.target() << endl;
            }
    };
}

int main(int argc, char * argv[])
{
    int exit_status(0);

    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_contents", "EXAMPLE_CONTENTS_OPTIONS", "EXAMPLE_CONTENTS_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch package IDs for installed 'sys-apps/paludis'. */
        std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                    generator::Matches(make_package_dep_spec().package(QualifiedPackageName("sys-apps/paludis"))) |
                    filter::SupportsAction<InstalledAction>())]);

        /* For each ID: */
        for (PackageIDSet::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            /* Do we have a contents key? PackageID _key() methods can return
             * a zero pointer. */
            if (! (*i)->contents_key())
            {
                cout << "ID '" << **i << "' does not provide a contents key." << endl;
            }
            else
            {
                cout << "ID '" << **i << "' provides contents key:" << endl;

                /* Visit the contents key's value's entries with our visitor. We use
                 * indirect_iterator because value()->begin() and ->end() return
                 * iterators to std::tr1::shared_ptr<>s rather than raw objects. */
                ContentsPrinter p;
                std::for_each(
                        indirect_iterator((*i)->contents_key()->value()->begin()),
                        indirect_iterator((*i)->contents_key()->value()->end()),
                        accept_visitor(p));
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

