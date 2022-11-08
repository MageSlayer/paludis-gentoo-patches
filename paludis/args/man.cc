/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2011 Ingmar Vanhassel
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
#include <paludis/util/upper_lower.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <functional>
#include <ostream>
#include <sstream>
#include <algorithm>

using namespace paludis;
using namespace paludis::args;
using std::endl;

namespace
{
    struct ExtraText
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
            if (e.allowed_args().empty())
                return;

            _dw.start_extra_arg();

            for (const auto & arg : e.allowed_args())
                _dw.extra_arg_enum(arg, e.default_arg());

            _dw.end_extra_arg();
        }

        void visit(const StringSetArg & e)
        {
            if (e.allowed_args().empty())
                return;

            _dw.start_extra_arg();

            for (const auto & arg : e.allowed_args())
                _dw.extra_arg_string_set(arg.first, arg.second);

            _dw.end_extra_arg();
        }

        void visit(const StringSequenceArg &)
        {
        }
    };
}


void
paludis::args::generate_doc(DocWriter & dw, const ArgsHandler * const h)
{
    using namespace std::placeholders;

    dw.heading(h->app_name(), h->man_section(), h->app_synopsis());

    if (! h->usage_lines().empty())
    {
        dw.start_usage_lines();

        for (const auto & usage_line : h->usage_lines())
            dw.usage_line(h->app_name(), usage_line);
    }

    dw.start_description(h->app_description());
    for (const auto & description_line : h->description_lines())
        dw.extra_description(description_line);
    dw.end_description();

    for (const auto & section : h->args_sections())
    {
        dw.start_options(section.name());
        for (const auto & group : section)
        {
            dw.start_arg_group(group.name(), group.description());

            for (const auto & option : group)
            {
                if (visitor_cast<const paludis::args::AliasArg>(*option) &&
                        visitor_cast<const paludis::args::AliasArg>(*option)->hidden())
                    continue;

                dw.arg_group_item(option->short_name(), option->long_name(),
                        option->can_be_negated() ? "no-" + option->long_name() : "", option->description());

                ExtraText t(dw);
                option->accept(t);

            }
            dw.end_arg_group();
        }
    }
    dw.end_options();

    if (! h->environment_lines().empty())
    {
        dw.start_environment();

        for (const auto & environment_line : h->environment_lines())
            dw.environment_line(environment_line.first, environment_line.second);

        dw.end_environment();
    }

    if (! h->notes().empty())
    {
        dw.start_notes();
        std::for_each(h->begin_notes(), h->end_notes(), std::bind(&DocWriter::note, &dw, _1));
        dw.end_notes();
    }

    if (! h->examples().empty())
    {
        dw.start_examples();

        for (const auto & example : h->examples())
            dw.example(example.first, example.second);

        dw.end_examples();
    }

    if (! h->see_alsos().empty())
    {
        dw.start_see_alsos();

        bool first{true};
        for (const auto & see_also : h->see_alsos())
        {
            dw.see_also(see_also.first, see_also.second, first);
            first = false;
        }

        dw.end_see_alsos();
    }
}

DocWriter::~DocWriter() = default;

namespace
{
    void escape_asciidoc(std::ostream & stream, const std::string & s)
    {
        char previous('\0');
        for (char t : s)
        {
            switch (previous)
            {
                case '\0':
                case ' ':
                case '\n':
                case '\t':
                case '\'':
                    if ('*' == t)
                        stream << '\\';
                    break;
                // Escape '*/*' -> '\*/*'
            }
            stream << t;
            previous = t;
        }
    }
}

AsciidocWriter::AsciidocWriter(std::ostream & os) :
    _os(os)
{
}

void
AsciidocWriter::heading(const std::string & name, const std::string & section_, const std::string & synopsis)
{
    std::string name_no_spaces(name);
    std::replace(name_no_spaces.begin(), name_no_spaces.end(), ' ', '-');
    _os << name_no_spaces << "(" << section_ << ")" << endl;
    _os << std::string(name_no_spaces.size() + 3, '=') << endl << endl << endl;
    _os << "NAME" << endl;
    _os << "----" << endl << endl;
    _os << name_no_spaces << " - " << synopsis << endl << endl;
}

void
AsciidocWriter::start_usage_lines()
{
    _os << "SYNOPSIS" << endl;
    _os << "--------" << endl << endl;
}

void
AsciidocWriter::usage_line(const std::string & name, const std::string & line)
{
    _os << "*" << name;
    if (! line.empty())
    {
        _os << " ";
        escape_asciidoc(_os, line);
    }
    _os << "*" << endl << endl;
}

void
AsciidocWriter::start_description(const std::string & description)
{
    _os << "DESCRIPTION" << endl;
    _os << "-----------" << endl;
    escape_asciidoc(_os, description);
    _os << endl;
}

void
AsciidocWriter::extra_description(const std::string & description)
{
    _os << endl;
    escape_asciidoc(_os, description);
    _os << endl << endl;
}

void
AsciidocWriter::end_description()
{
    _os << endl;
}

void
AsciidocWriter::start_options(const std::string & s)
{
    _os << toupper(s) << endl;
    _os << std::string(s.size(), char('-')) << endl;
}

void
AsciidocWriter::start_arg_group(const std::string & name, const std::string & description)
{
    _os << name << endl;
    _os << std::string(name.size(), char('~')) << endl;
    escape_asciidoc(_os, description);
    _os << endl << endl;
}

void
AsciidocWriter::arg_group_item(const char & short_name, const std::string & long_name,
        const std::string & negated_long_name, const std::string & description)
{
    _os << "*";
    if (short_name)
        _os << "-" << short_name << " , ";
    _os << "--" << long_name;
    if (! negated_long_name.empty())
    {
        _os << " (";
        if (short_name)
            _os << "+" << short_name << " , ";
        _os << "--" << negated_long_name << ")";
    }
    _os << "*::" << endl;
    _os << "        ";
    escape_asciidoc(_os, description);
    _os << endl << endl;
}

void
AsciidocWriter::start_extra_arg()
{
}

void
AsciidocWriter::extra_arg_enum(const AllowedEnumArg & e, const std::string & default_arg)
{
    std::string default_string;

    if (e.long_name() == default_arg)
        default_string = " (default)";
    _os << "    *" << e.long_name();
    if (e.short_name())
        _os << " (" << std::string(1, e.short_name()) << ")";
    _os << "*;;" << endl;
    _os << "        ";
    escape_asciidoc(_os, e.description());
    _os << default_string << endl << endl;
}

void
AsciidocWriter::extra_arg_string_set(const std::string & first, const std::string & second)
{
    _os << "*" << first << "*:::" << endl;
    _os << "        ";
    escape_asciidoc(_os, second);
    _os << endl;
}

void
AsciidocWriter::end_extra_arg()
{
    _os << endl;
}

void
AsciidocWriter::end_arg_group()
{
    _os << endl;
}

void
AsciidocWriter::end_options()
{
    _os << endl;
}

void
AsciidocWriter::start_environment()
{
    _os << "ENVIRONMENT" << endl;
    _os << "-----------" << endl;
}

void
AsciidocWriter::environment_line(const std::string & first, const std::string & second)
{
    _os << first << "::" << endl;
    escape_asciidoc(_os, second);
    _os << endl;
}

void
AsciidocWriter::end_environment()
{
    _os << endl;
}

void
AsciidocWriter::start_examples()
{
    _os << "EXAMPLES" << endl;
    _os << "--------" << endl;
}

void
AsciidocWriter::example(const std::string & first, const std::string & second)
{
    _os << first << "::" << endl;
    escape_asciidoc(_os, second);
    _os << endl;
}

void
AsciidocWriter::end_examples()
{
    _os << endl;
}

void
AsciidocWriter::start_notes()
{
    _os << "NOTES" << endl;
    _os << "-----" << endl;
}

void
AsciidocWriter::note(const std::string & s)
{
    _os << "* ";
    escape_asciidoc(_os, s);
    _os << endl;
}

void
AsciidocWriter::end_notes()
{
    _os << endl;
}

void
AsciidocWriter::section(const std::string & title)
{
    _os << title << endl;
    _os << std::string(title.size(), '-') << endl;
}

void
AsciidocWriter::subsection(const std::string & title)
{
    _os << title << endl;
    _os << std::string(title.size(), '~') << endl;
}

void
AsciidocWriter::paragraph(const std::string & text)
{
    escape_asciidoc(_os, text);
    _os << endl;
}

void
AsciidocWriter::start_see_alsos()
{
    _os << "SEE ALSO" << endl;
    _os << "--------" << endl;
}

void
AsciidocWriter::see_also(const std::string & page, const int s, const bool)
{
    _os << "*" << page << "*(" << s << ")" << endl;
}

void
AsciidocWriter::end_see_alsos()
{
    _os << endl;
}

