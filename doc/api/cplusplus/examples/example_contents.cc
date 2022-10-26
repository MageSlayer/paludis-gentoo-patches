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
#include <algorithm>
#include <cstdlib>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::left;
using std::setw;

namespace
{
    /* This visitor displays information about ContentsEntry subclasses. */
    class ContentsPrinter
    {
        public:
            void visit(const ContentsFileEntry & d)
            {
                cout << left << setw(10) << "file" << d.location_key()->parse_value() << endl;
            }

            void visit(const ContentsDirEntry & d)
            {
                cout << left << setw(10) << "dir" << d.location_key()->parse_value() << endl;
            }

            void visit(const ContentsSymEntry & d)
            {
                cout << left << setw(10) << "sym" << d.location_key()->parse_value()
                    << " -> " << d.target_key()->parse_value() << endl;
            }

            void visit(const ContentsOtherEntry & d)
            {
                cout << left << setw(10) << "other" << d.location_key()->parse_value() << endl;
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
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch package IDs for installed 'sys-apps/paludis'. */
        std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                    generator::Package(QualifiedPackageName("sys-apps/paludis")) |
                    filter::InstalledAtSlash())]);

        /* For each ID: */
        for (const auto & id : *ids)
        {
            /* Do we have contents key?*/
            auto contents(id->contents());
            if (! contents)
            {
                cout << "ID '" << *id << "' does not provide a contents key." << endl;
            }
            else
            {
                cout << "ID '" << *id << "' provides contents key:" << endl;

                /* Visit the contents key's value's entries with our visitor. We use
                 * indirect_iterator because choices->begin() and ->end() return
                 * iterators to std::shared_ptr<>s rather than raw objects. */
                ContentsPrinter p;
                std::for_each(
                        indirect_iterator(contents->begin()),
                        indirect_iterator(contents->end()),
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

