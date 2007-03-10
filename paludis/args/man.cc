/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <sstream>

using namespace paludis;
using namespace paludis::args;
using std::endl;

namespace
{
    struct ExtraText :
        ArgsVisitorTypes::ConstVisitor
    {
        std::stringstream s;
        ManFormat mf;

        ExtraText(ManFormat m) :
            mf(m)
        {
        }

        void visit(const ArgsOption * const)
        {
        }

        void visit(const StringArg * const)
        {
        }

        void visit(const AliasArg * const)
        {
        }

        void visit(const SwitchArg * const)
        {
        }

        void visit(const IntegerArg * const)
        {
        }

        void visit(const EnumArg * const e)
        {
            if (e->begin_allowed_args() == e->end_allowed_args())
                return;

            switch (mf)
            {
                case mf_man:
                    break;

                case mf_html:
                    s << "<dl>" << endl;
                    break;
            }

            for (EnumArg::AllowedArgIterator a(e->begin_allowed_args()), a_end(e->end_allowed_args()) ;
                    a != a_end ; ++a)
            {
                switch (mf)
                {
                    case mf_man:
                        s << ".RS" << endl;
                        s << ".TP" << endl;
                        s << ".B \"" << a->first << "\"" << endl;
                        s << a->second << endl;
                        s << ".RE" << endl;
                        break;

                    case mf_html:
                        s << "<dt>" << a->first << "</dt>" << endl;
                        s << "<dd>" << a->second << "</dd>" << endl;
                        break;
                }
            }

            switch (mf)
            {
                case mf_man:
                    break;

                case mf_html:
                    s << "</dl>" << endl;
                    break;
            }
        }

        void visit(const StringSetArg * const e)
        {
            if (e->begin_allowed_args() == e->end_allowed_args())
                return;

            switch (mf)
            {
                case mf_man:
                    break;

                case mf_html:
                    s << "<dl>" << endl;
                    break;
            }

            for (EnumArg::AllowedArgIterator a(e->begin_allowed_args()), a_end(e->end_allowed_args()) ;
                    a != a_end ; ++a)
            {
                switch (mf)
                {
                    case mf_man:
                        s << ".RS" << endl;
                        s << ".TP" << endl;
                        s << ".B \"" << a->first << "\"" << endl;
                        s << a->second << endl;
                        s << ".RE" << endl;
                        break;

                    case mf_html:
                        s << "<dt>" << a->first << "</dt>" << endl;
                        s << "<dd>" << a->second << "</dd>" << endl;
                        break;
                }
            }

            switch (mf)
            {
                case mf_man:
                    break;

                case mf_html:
                    s << "</dl>" << endl;
                    break;
            }
        }
    };

    std::ostream &
    operator<< (std::ostream & s, const ExtraText & t)
    {
        s << t.s.str();
        return s;
    }
}

void
paludis::args::generate_man(std::ostream & f, const ArgsHandler * const h)
{
    generate_man(f, h, mf_man);
}

void
paludis::args::generate_man(std::ostream & f, const ArgsHandler * const h,
        const ManFormat mf)
{
    switch (mf)
    {
        case mf_man:
            f << ".TH \"" << h->app_name() << "\" " << h->man_section() << endl;
            f << ".SH NAME" << endl;
            f << h->app_name() << " \\- " << h->app_synopsis() << endl;
            f << ".SH SYNOPSIS" << endl;
            break;

        case mf_html:
            f << "<h1>" << h->app_name() << "(" << h->man_section() << ")" << "</h1>" << endl;
            f << "<h2>Name</h2>" << endl;
            f << "<p>" << h->app_name() << " - " << h->app_synopsis() << "</p>" << endl;
            f << "<h2>Synopsis</h2>" << endl;
            break;
    }

    for (ArgsHandler::UsageLineIterator u(h->begin_usage_lines()),
            u_end(h->end_usage_lines()) ; u != u_end ; ++u)
    {
        switch (mf)
        {
            case mf_man:
                f << ".B " << h->app_name() << " " << *u << endl << endl;
                break;

            case mf_html:
                f << "<p>" << h->app_name() << " " << *u << "</p>" << endl;
                break;
        }
    }

    switch (mf)
    {
        case mf_man:
            f << ".SH DESCRIPTION" << endl;
            f << h->app_description() << endl;
            f << ".SH OPTIONS" << endl;
            break;

        case mf_html:
            f << "<h2>Description</h2>" << endl;
            f << "<p>" << h->app_description() << "</p>" << endl;
            f << "<h2>Options</h2>" << endl;
            break;
    }

    for (ArgsHandler::ArgsGroupsIterator a(h->begin_args_groups()),
            a_end(h->end_args_groups()) ; a != a_end ; ++a)
    {
        switch (mf)
        {
            case mf_man:
                f << ".SS \"" << (*a)->name() << "\"" << endl;
                f << (*a)->description() << endl;
                break;

            case mf_html:
                f << "<h3>" << (*a)->name() << "</h3>" << endl;
                f << "<p>" << (*a)->description() << "</p>" << endl;
                f << "<dl>";
                break;
        }

        for (paludis::args::ArgsGroup::Iterator b((*a)->begin()), b_end((*a)->end()) ;
                b != b_end ; ++b)
        {
            switch (mf)
            {
                case mf_man:
                    f << ".TP" << endl;
                    f << ".B \"";
                    if ((*b)->short_name())
                        f << "\\-" << (*b)->short_name() << " , ";
                    f << "\\-\\-" << (*b)->long_name() << "\"" << endl;
                    f << (*b)->description() << endl;
                    break;

                case mf_html:
                    f << "<dt>";
                    if ((*b)->short_name())
                        f << "-" << (*b)->short_name() << ", ";
                    f << "--" << (*b)->long_name() << "</dt>" << endl;
                    f << "<dd>" << (*b)->description() << endl;

                    break;
            }

            ExtraText t(mf);
            (*b)->accept(&t);
            f << t;

            switch (mf)
            {
                case mf_man:
                    break;

                case mf_html:
                    f << "</dd>" << endl;
                    break;
            }
        }

        switch (mf)
        {
            case mf_man:
                break;

            case mf_html:
                f << "</dl>";
                break;
        }
    }

    if (h->begin_environment_lines() != h->end_environment_lines())
    {
        switch (mf)
        {
            case mf_man:
                f << ".SH ENVIRONMENT" << endl;
                break;

            case mf_html:
                f << "<h2>Environment</h2>" << endl;
                f << "<dl>" << endl;
                break;
        }

        for (ArgsHandler::EnvironmentLineIterator a(h->begin_environment_lines()),
                a_end(h->end_environment_lines()) ; a != a_end ; ++a)
        {
            switch (mf)
            {
                case mf_man:
                    f << ".TP" << endl;
                    f << ".B \"" << a->first << "\"" << endl;
                    f << a->second << endl;
                    break;

                case mf_html:
                    f << "<dt>" << a->first << "</dt>" << endl;
                    f << "<dd>" << a->second << "</dd>" << endl;
                    break;
            }
        }

        switch (mf)
        {
            case mf_man:
                break;

            case mf_html:
                f << "</dl>" << endl;
                break;
        }
    }
}

