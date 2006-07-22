/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "command_line.hh"
#include "config.h"

#include <iostream>
#include <cstdlib>

using std::cout;
using std::endl;

int
main(int, char *[])
{
    cout << ".TH \"" << CommandLine::get_instance()->app_name() << "\" "
        << CommandLine::get_instance()->man_section() << endl;

    cout << ".SH NAME" << endl;
    cout << CommandLine::get_instance()->app_name() << " \\- "
        << CommandLine::get_instance()->app_synopsis() << endl;

    cout << ".SH SYNOPSIS" << endl;

    for (CommandLine::UsageLineIterator u(CommandLine::get_instance()->begin_usage_lines()),
            u_end(CommandLine::get_instance()->end_usage_lines()) ; u != u_end ; ++u)
        cout << ".B " << CommandLine::get_instance()->app_name() << " " << *u << endl << endl;

    cout << ".SH DESCRIPTION" << endl;
    cout << CommandLine::get_instance()->app_description() << endl;

    cout << ".SH OPTIONS" << endl;

    for (CommandLine::ArgsGroupsIterator a(CommandLine::get_instance()->begin_args_groups()),
            a_end(CommandLine::get_instance()->end_args_groups()) ; a != a_end ; ++a)
    {
        cout << ".SS \"" << (*a)->name() << "\"" << endl;
        cout << (*a)->description() << endl;

        for (paludis::args::ArgsGroup::Iterator b((*a)->begin()), b_end((*a)->end()) ;
                b != b_end ; ++b)
        {
            cout << ".TP" << endl;
            cout << ".B \"";
            if ((*b)->short_name())
                cout << "\\-" << (*b)->short_name() << " , ";
            cout << "\\-\\-" << (*b)->long_name() << "\"" << endl;
            cout << (*b)->description() << endl;
        }
    }

    return EXIT_SUCCESS;
}

