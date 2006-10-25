/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <paludis/paludis.hh>
#include <paludis/environment/default/default_environment.hh>

#include <iostream>
#include <cstdlib>

using std::cout;
using std::cerr;
using std::endl;

int main(int, char *[])
{
    try
    {
        paludis::PackageDatabaseEntryCollection::ConstPointer packages(
                paludis::DefaultEnvironment::get_instance()->package_database()->query(
                    paludis::PackageDepAtom("app-editors/vim"), paludis::is_installed_only));

        if (packages->empty())
            cout << "Vim is not installed" << endl;
        else
            cout << "Vim " << packages->last()->version << " is installed" << endl;
    }
    catch (const paludis::Exception & e)
    {
        cerr << "Caught exception '" << e.message() << "' ("
            << e.what() << ")" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

