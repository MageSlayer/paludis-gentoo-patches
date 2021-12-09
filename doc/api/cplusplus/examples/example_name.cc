/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_name.cc "example_name.cc" .
 *
 * \ingroup g_names
 */

/** \example example_name.cc
 *
 * This example demonstrates how to use name classes.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <cstdlib>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::setw;
using std::left;

namespace
{
    /* Try to create a name and show some information about it */
    template <typename NameType_>
    void show_name(const std::string & type_string, const std::string & value)
    {
        cout << type_string << " " << value << ":" << endl;
        try
        {
            /* Names can be explicitly created from a string. If an invalid
             * value is given, an exception is thrown. */
            NameType_ name(value);

            /* Names can be written to a stream. */
            cout << "    " << left << setw(24) << "To stream:" << " " << name << endl;

            /* Names can be stringified if a raw string is needed. */
            cout << "    " << left << setw(24) << "stringify(n).length:"
                << " " << stringify(name).length() << endl;
        }
        catch (const NameError & e)
        {
            cout << "    " << left << setw(24) << "Exception:" << " '" <<
                e.message() << "' (" << e.what() << ")" << endl;
        }
        cout << endl;
    }
}

int main(int argc, char * argv[])
{
    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_name", "EXAMPLE_NAME_OPTIONS", "EXAMPLE_NAME_CMDLINE");

        /* Most names are simply validated string classes. There is no implicit
         * conversion to or from strings to increase static checking. The
         * IUseFlag and QualifiedPackageName names are special -- they are
         * composites, rather than simply validated strings. */
        show_name<SetName>("SetName", "world");
        show_name<SetName>("SetName", "not*valid");
        show_name<PackageNamePart>("PackageNamePart", "fred");
        show_name<PackageNamePart>("PackageNamePart", "not/valid");
        show_name<QualifiedPackageName>("QualifiedPackageName", "cat/pkg");
        show_name<QualifiedPackageName>("QualifiedPackageName", "not-valid");

        QualifiedPackageName q1("cat/pkg");
        QualifiedPackageName q2(CategoryNamePart("cat") + PackageNamePart("pkg"));
        cout << q1 << " " << (q1 == q2 ? "==" : "!=") << " " << q2 << endl;
        cout << q1 << " has category '" << q1.category() << "' and package part '" << q1.package() << "'" << endl;
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

