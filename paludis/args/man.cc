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

#include "man.hh"
#include <ostream>

using namespace paludis;
using namespace paludis::args;
using std::endl;

void
paludis::args::generate_man(std::ostream & f, const ArgsHandler * const h)
{
    f << ".TH \"" << h->app_name() << "\" "
        << h->man_section() << endl;

    f << ".SH NAME" << endl;
    f << h->app_name() << " \\- "
        << h->app_synopsis() << endl;

    f << ".SH SYNOPSIS" << endl;

    for (ArgsHandler::UsageLineIterator u(h->begin_usage_lines()),
            u_end(h->end_usage_lines()) ; u != u_end ; ++u)
        f << ".B " << h->app_name() << " " << *u << endl << endl;

    f << ".SH DESCRIPTION" << endl;
    f << h->app_description() << endl;

    f << ".SH OPTIONS" << endl;

    for (ArgsHandler::ArgsGroupsIterator a(h->begin_args_groups()),
            a_end(h->end_args_groups()) ; a != a_end ; ++a)
    {
        f << ".SS \"" << (*a)->name() << "\"" << endl;
        f << (*a)->description() << endl;

        for (paludis::args::ArgsGroup::Iterator b((*a)->begin()), b_end((*a)->end()) ;
                b != b_end ; ++b)
        {
            f << ".TP" << endl;
            f << ".B \"";
            if ((*b)->short_name())
                f << "\\-" << (*b)->short_name() << " , ";
            f << "\\-\\-" << (*b)->long_name() << "\"" << endl;
            f << (*b)->description() << endl;
        }
    }

    if (h->begin_environment_lines() !=
            h->end_environment_lines())
    {
        f << ".SH ENVIRONMENT" << endl;

        for (ArgsHandler::EnvironmentLineIterator a(h->begin_environment_lines()),
                a_end(h->end_environment_lines()) ; a != a_end ; ++a)
        {
            f << ".TP" << endl;
            f << ".B \"" << a->first << "\"" << endl;
            f << a->second << endl;
        }

    }
}

