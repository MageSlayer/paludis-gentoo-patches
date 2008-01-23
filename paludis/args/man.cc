/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <ostream>
#include <sstream>
#include <algorithm>

using namespace paludis;
using namespace paludis::args;
using std::endl;

namespace
{
    struct ExtraText :
        ConstVisitor<ArgsVisitorTypes>
    {
        DocWriter & _dw;

        ExtraText(DocWriter & dw) :
            _dw(dw)
        {
        }

        void visit(const StringArg &)
        {
        }

        void visit(const AliasArg &)
        {
        }

        void visit(const SwitchArg &)
        {
        }

        void visit(const IntegerArg &)
        {
        }

        void visit(const EnumArg & e)
        {
            if (e.begin_allowed_args() == e.end_allowed_args())
                return;

            _dw.start_extra_arg();

            for (EnumArg::AllowedArgConstIterator a(e.begin_allowed_args()), a_end(e.end_allowed_args()) ;
                    a != a_end ; ++a)
            {
                _dw.extra_arg_enum(a->first, a->second, e.default_arg());
            }

            _dw.end_extra_arg();
        }

        void visit(const StringSetArg & e)
        {
            if (e.begin_allowed_args() == e.end_allowed_args())
                return;

            _dw.start_extra_arg();

            for (EnumArg::AllowedArgConstIterator a(e.begin_allowed_args()), a_end(e.end_allowed_args()) ;
                    a != a_end ; ++a)
            {
                _dw.extra_arg_string_set(a->first, a->second);
            }

            _dw.end_extra_arg();
        }
    };
}


void
paludis::args::generate_doc(DocWriter & dw, const ArgsHandler * const h)
{
    using namespace tr1::placeholders;

    dw.heading(h->app_name(), h->man_section(), h->app_synopsis());

    for (ArgsHandler::UsageLineConstIterator u(h->begin_usage_lines()),
            u_end(h->end_usage_lines()) ; u != u_end ; ++u)
    {
        dw.usage_line(h->app_name(), *u);
    }

    dw.start_description(h->app_description());

    for (ArgsHandler::ArgsGroupsConstIterator a(h->begin_args_groups()),
            a_end(h->end_args_groups()) ; a != a_end ; ++a)
    {
        dw.start_arg_group((*a)->name(), (*a)->description());

        for (paludis::args::ArgsGroup::ConstIterator b((*a)->begin()), b_end((*a)->end()) ;
                b != b_end ; ++b)
        {
            dw.arg_group_item((*b)->short_name(), (*b)->long_name(),
                    (*b)->can_be_negated() ? "no-" + (*b)->long_name() : "", (*b)->description());

            ExtraText t(dw);
            (*b)->accept(t);

            dw.end_arg_group();
        }

        dw.end_description();
    }

    if (h->begin_environment_lines() != h->end_environment_lines())
    {
        dw.start_environment();

        for (ArgsHandler::EnvironmentLineConstIterator a(h->begin_environment_lines()),
                a_end(h->end_environment_lines()) ; a != a_end ; ++a)
        {
            dw.environment_line(a->first, a->second);
        }

        dw.end_environment();
    }

    if (h->begin_notes() != h->end_notes())
    {
        dw.start_notes();
        std::for_each(h->begin_notes(), h->end_notes(), tr1::bind(&DocWriter::note, &dw, _1));
        dw.end_notes();
    }

    if (h->begin_examples() != h->end_examples())
    {
        dw.start_examples();

        for (ArgsHandler::ExamplesConstIterator a(h->begin_examples()), a_end(h->end_examples()) ; a != a_end ; ++a)
            dw.example(a->first, a->second);

        dw.end_examples();
    }
}

DocWriter::~DocWriter()
{
}

HtmlWriter::HtmlWriter(std::ostream & os) :
    _os(os)
{
}

HtmlWriter::~HtmlWriter()
{
}

void
HtmlWriter::heading(const std::string & name, const std::string & section_, const std::string & synopsis)
{
    _os << "<h1>" << name << "(" << section_ << ")" << "</h1>" << endl;
    _os << "<h2>Name</h2>" << endl;
    _os << "<p>" << name << " - " << synopsis << "</p>" << endl;
    _os << "<h2>Synopsis</h2>" << endl;
}

void
HtmlWriter::usage_line(const std::string & name, const std::string & line)
{
    _os << "<p>" << name << " " << line << "</p>" << endl;
}

void
HtmlWriter::start_description(const std::string & description)
{
    _os << "<h2>Description</h2>" << endl;
    _os << "<p>" << description << "</p>" << endl;
    _os << "<h2>Options</h2>" << endl;
}

void
HtmlWriter::start_arg_group(const std::string & name, const std::string & description)
{
    _os << "<h3>" << name << "</h3>" << endl;
    _os << "<p>" << description << "</p>" << endl;
    _os << "<dl>";
}

void
HtmlWriter::arg_group_item(const char & short_name, const std::string & long_name,
        const std::string & negated_long_name, const std::string & description)
{
    _os << "<dt>";
    if (short_name)
        _os << "-" << short_name << ", ";
    _os << "--" << long_name;
    if (! negated_long_name.empty())
        _os << " (" << "--" << negated_long_name << ")";
    _os << "</dt>" << endl;
    _os << "<dd>" << description << endl;
}

void
HtmlWriter::start_extra_arg()
{
    _os << "<dl>" << endl;
}

void
HtmlWriter::extra_arg_enum(const std::string & first, const std::string & second, const std::string & default_arg)
{
    std::string default_string;
    if (first == default_arg)
        default_string = " (default)";

    _os << "<dt>" << first << "</dt>" << endl;
    _os << "<dd>" << second << default_string << "</dd>" << endl;
}

void
HtmlWriter::extra_arg_string_set(const std::string & first, const std::string & second)
{
    _os << "<dt>" << first << "</dt>" << endl;
    _os << "<dd>" << second << "</dd>" << endl;
}

void
HtmlWriter::end_extra_arg()
{
    _os << "</dl>" << endl;
}

void
HtmlWriter::end_arg_group()
{
    _os << "</dd>" << endl;
}

void
HtmlWriter::end_description()
{
    _os << "</dl>" << endl;
}

void
HtmlWriter::start_environment()
{
    _os << "<h2>Environment</h2>" << endl;
    _os << "<dl>" << endl;
}

void
HtmlWriter::environment_line(const std::string & first, const std::string & second)
{
    _os << "<dt>" << first << "</dt>" << endl;
    _os << "<dd>" << second << "</dd>" << endl;
}

void
HtmlWriter::end_environment()
{
    _os << "</dl>" << endl;
}

void
HtmlWriter::start_notes()
{
    _os << "<h2>Notes</h2>" << endl;
    _os << "<ul>" << endl;
}

void
HtmlWriter::end_notes()
{
    _os << "</ul>" << endl;
}

void
HtmlWriter::note(const std::string & s)
{
    _os << "<li>" << s << "</li>" << endl;
}

void
HtmlWriter::start_examples()
{
    _os << "<h2>Examples</h2>" << endl;
}

void
HtmlWriter::example(const std::string & first, const std::string & second)
{
    _os << "<pre>" << first << "</pre>" << endl;
    _os << "<p>" << second << "</p>" << endl;
}

void
HtmlWriter::end_examples()
{
}

void
HtmlWriter::section(const std::string & title)
{
    _os << "<h2>" << title << "</h2>" << endl;
}

void
HtmlWriter::subsection(const std::string & title)
{
    _os << "<h3>" << title << "</h3>" << endl;
}

void
HtmlWriter::paragraph(const std::string & text)
{
    _os << "<p>" << text << "</p>" << endl;
}


ManWriter::ManWriter(std::ostream & os) :
    _os(os)
{
}

ManWriter::~ManWriter()
{
}

void
ManWriter::heading(const std::string & name, const std::string & section_, const std::string & synopsis)
{
    _os << ".TH \"" << name << "\" " << section_ << endl;
    _os << ".SH NAME" << endl;
    _os << name << " \\- " << synopsis << endl;
    _os << ".SH SYNOPSIS" << endl;
}

void
ManWriter::usage_line(const std::string & name, const std::string & line)
{
    _os << ".B " << name << " " << line << endl << endl;
}

void
ManWriter::start_description(const std::string & description)
{
    _os << ".SH DESCRIPTION" << endl;
    _os << description << endl;
    _os << ".SH OPTIONS" << endl;
}

void
ManWriter::start_arg_group(const std::string & name, const std::string & description)
{
    _os << ".SS \"" << name << "\"" << endl;
    _os << description << endl;
}

void
ManWriter::arg_group_item(const char & short_name, const std::string & long_name,
        const std::string & negated_long_name, const std::string & description)
{
    _os << ".TP" << endl;
    _os << ".B \"";
    if (short_name)
        _os << "\\-" << short_name << " , ";
    _os << "\\-\\-" << long_name;
    if (! negated_long_name.empty())
        _os << " (\\-\\-" << negated_long_name << ")\"";
    _os << endl;
    _os << description << endl;
}

void
ManWriter::start_extra_arg()
{
}

void
ManWriter::extra_arg_enum(const std::string & first, const std::string & second, const std::string & default_arg)
{
    std::string default_string;
    if (first == default_arg)
        default_string = " (default)";

    _os << ".RS" << endl;
    _os << ".TP" << endl;
    _os << ".B \"" << first << "\"" << endl;
    _os << second << default_string << endl;
    _os << ".RE" << endl;
}

void
ManWriter::extra_arg_string_set(const std::string & first, const std::string & second)
{
    _os << ".RS" << endl;
    _os << ".TP" << endl;
    _os << ".B \"" << first << "\"" << endl;
    _os << second << endl;
    _os << ".RE" << endl;
}

void
ManWriter::end_extra_arg()
{
}

void
ManWriter::end_arg_group()
{
}

void
ManWriter::end_description()
{
}

void
ManWriter::start_environment()
{
    _os << ".SH ENVIRONMENT" << endl;
}

void
ManWriter::environment_line(const std::string & first, const std::string & second)
{
    _os << ".TP" << endl;
    _os << ".B \"" << first << "\"" << endl;
    _os << second << endl;
}

void
ManWriter::end_environment()
{
}

void
ManWriter::start_examples()
{
    _os << ".SH EXAMPLES" << endl;
}

namespace
{
    std::string ungroff(const std::string & s)
    {
        if ((! s.empty() && ('.' == s.at(0))))
            return " " + s;
        else
            return s;
    }
}

void
ManWriter::example(const std::string & first, const std::string & second)
{
    _os << ".TP" << endl;
    _os << first << endl;
    _os << ungroff(second) << endl << endl;
}

void
ManWriter::end_examples()
{
}

void
ManWriter::start_notes()
{
    _os << ".SH NOTES" << endl;
}

void
ManWriter::note(const std::string & s)
{
    _os << s << endl << endl;
}

void
ManWriter::end_notes()
{
}

void
ManWriter::section(const std::string & title)
{
    _os << ".SH " << title << endl;
}

void
ManWriter::subsection(const std::string & title)
{
    _os << ".SS \"" << title << "\"" << endl;
}

void
ManWriter::paragraph(const std::string & text)
{
    _os <<  text << endl;
}

