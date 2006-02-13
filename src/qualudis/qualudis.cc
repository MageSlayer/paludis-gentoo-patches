/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <paludis/paludis.hh>
#include <paludis/args/args.hh>
#include <paludis/qa/qa.hh>

#include <cstdlib>
#include <iostream>
#include <algorithm>

#include "qualudis_command_line.hh"
#include "config.h"

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

struct DoHelp
{
    const std::string message;

    DoHelp(const std::string & m = "") :
        message(m)
    {
    }
};

struct DoVersion
{
};

void
display_errors(const qa::CheckResult & r)
{
    cout << r.item() << ": " << r.rule() << ":" << endl;

    for (qa::CheckResult::Iterator i(r.begin()), i_end(r.end()) ; i != i_end ; ++i)
    {
        do
        {
            switch (i->get<qa::mk_level>())
            {
                case qa::qal_info:
                    cout << "  info:   ";
                    continue;

                case qa::qal_minor:
                    cout << "  minor:  ";
                    continue;

                case qa::qal_major:
                    cout << "  major:  ";
                    continue;

                case qa::qal_fatal:
                    cout << "  fatal:  ";
                    continue;

                case qa::qal_maybe:
                    cout << "  maybe:  ";
                    continue;
            }

            throw InternalError(PALUDIS_HERE, "Bad mk_level");
        }
        while (false);

        cout << i->get<qa::mk_msg>() << endl;
    }
}

bool
do_check()
{
    bool ok(true);
    FSEntry cwd(FSEntry::cwd());

    std::list<std::string> package_dir_checks;
    qa::PackageDirCheckMaker::get_instance()->copy_keys(std::back_inserter(package_dir_checks));
    for (std::list<std::string>::const_iterator i(package_dir_checks.begin()),
            i_end(package_dir_checks.end()) ; i != i_end ; ++i)
    {
        qa::CheckResult r((*qa::PackageDirCheckMaker::get_instance()->find_maker(*i)())(cwd));

        if (r.empty())
            continue;

        display_errors(r);

        do
        {
            switch (r.most_severe_level())
            {
                case qa::qal_info:
                case qa::qal_maybe:
                    continue;

                case qa::qal_minor:
                case qa::qal_major:
                    ok = false;
                    continue;

                case qa::qal_fatal:
                    ok = false;
                    return ok;
            }
            throw InternalError(PALUDIS_HERE, "Bad most_severe_level");
        } while (0);

        ok = false;
    }

    return ok;
}

int main(int argc, char *argv[])
{
    Context context("In main program:");

    try
    {
        QualudisCommandLine::get_instance()->run(argc, argv);

        if (QualudisCommandLine::get_instance()->a_help.specified())
            throw DoHelp();

        if (1 != (QualudisCommandLine::get_instance()->a_check.specified() +
                    QualudisCommandLine::get_instance()->a_version.specified()))
            throw DoHelp("you should specify exactly one action");

        if (QualudisCommandLine::get_instance()->a_version.specified())
            throw DoVersion();

        if (QualudisCommandLine::get_instance()->a_check.specified())
        {
            if (! QualudisCommandLine::get_instance()->empty())
                throw DoHelp("check action takes no parameters");

            return do_check() ? EXIT_SUCCESS : EXIT_FAILURE;
        }

        throw InternalError(__PRETTY_FUNCTION__, "no action?");
    }
    catch (const DoVersion &)
    {
        cout << "qualudis, part of " << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
            << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO;
        if (! std::string(PALUDIS_SUBVERSION_REVISION).empty())
            cout << " svn " << PALUDIS_SUBVERSION_REVISION;
        cout << endl << endl;
        cout << "Built by " << PALUDIS_BUILD_USER << "@" << PALUDIS_BUILD_HOST
            << " on " << PALUDIS_BUILD_DATE << endl;
        cout << "CXX:         " << PALUDIS_BUILD_CXX
#if defined(__ICC)
            << " " << __ICC
#elif defined(__VERSION__)
            << " " << __VERSION__
#endif
            << endl;
        cout << "CXXFLAGS:    " << PALUDIS_BUILD_CXXFLAGS << endl;
        cout << "LDFLAGS:     " << PALUDIS_BUILD_LDFLAGS << endl;
        cout << "SYSCONFDIR:  " << SYSCONFDIR << endl;
        cout << "LIBEXECDIR:  " << LIBEXECDIR << endl;
        cout << "stdlib:      "
#if defined(__GLIBCXX__)
#  define XSTRINGIFY(x) #x
#  define STRINGIFY(x) XSTRINGIFY(x)
            << "GNU libstdc++ " << STRINGIFY(__GLIBCXX__)
#endif
            << endl;

        cout << "libebt:      " << LIBEBT_VERSION_MAJOR << "." << LIBEBT_VERSION_MINOR
            << "." << LIBEBT_VERSION_MICRO << endl;
        cout << endl;
        cout << "Paludis comes with ABSOLUTELY NO WARRANTY. Paludis is free software, and you" << endl;
        cout << "are welcome to redistribute it under the terms of the GNU General Public" << endl;
        cout << "License, version 2." << endl;

        return EXIT_SUCCESS;
    }
    catch (const paludis::args::ArgsError & e)
    {
        cerr << "Usage error: " << e.message() << endl;
        cerr << "Try " << argv[0] << " --help" << endl;
        return EXIT_FAILURE;
    }
    catch (const DoHelp & h)
    {
        if (h.message.empty())
        {
            cout << "Usage: " << argv[0] << " [options]" << endl;
            cout << endl;
            cout << *QualudisCommandLine::get_instance();
            return EXIT_SUCCESS;
        }
        else
        {
            cerr << "Usage error: " << h.message << endl;
            cerr << "Try " << argv[0] << " --help" << endl;
            return EXIT_FAILURE;
        }
    }
    catch (const Exception & e)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * " << e.backtrace("\n  * ")
            << e.message() << " (" << e.what() << ")" << endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception & e)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * Unknown exception type. Ouch..." << endl;
        return EXIT_FAILURE;
    }
}

