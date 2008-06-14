/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_about.cc "example_about.cc" .
 *
 * \ingroup g_about
 */

/** \example example_about.cc
 *
 * A simple example showing how to use Paludis version macros.
 */

#include <paludis/paludis.hh>
#include <iostream>
#include <cstdlib>

using std::cout;
using std::endl;

int main(int, char *[])
{
    cout << "Built using Paludis " << PALUDIS_VERSION_MAJOR << "." << PALUDIS_VERSION_MINOR
        << "." << PALUDIS_VERSION_MICRO << PALUDIS_VERSION_SUFFIX;

    if (! std::string(PALUDIS_GIT_HEAD).empty())
        cout << " " << PALUDIS_GIT_HEAD;

    cout << endl;

    return EXIT_SUCCESS;
}

