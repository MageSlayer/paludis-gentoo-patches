/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/simple_parser.hh>
#include <paludis/util/pimp-impl.hh>
#include <strings.h>

using namespace paludis;
using namespace paludis::simple_parser;


SimpleParserExpression::SimpleParserExpression(const SimpleParserMatchFunction & f) :
    _match(f)
{
}

std::string::size_type
SimpleParserExpression::match(const std::string & s, const std::string::size_type offset) const
{
    return _match(s, offset);
}

namespace
{
    std::string::size_type
    m_and(const SimpleParserExpression & e1, const SimpleParserExpression & e2, const std::string & text,
            const std::string::size_type offset)
    {
        if (offset >= text.length())
            return std::string::npos;

        std::string::size_type len_1(e1.match(text, offset));
        if (std::string::npos == len_1)
            return std::string::npos;

        std::string::size_type len_2(e2.match(text, offset + len_1));
        if (std::string::npos == len_2)
            return std::string::npos;

        return len_1 + len_2;
    }

    std::string::size_type
    m_capture(const SimpleParserExpression & e1, std::string & var, const std::string & text,
            const std::string::size_type offset)
    {
        std::string::size_type len_1(e1.match(text, offset));
        if (std::string::npos != len_1)
            var = text.substr(offset, len_1);
        return len_1;
    }

    std::string::size_type
    m_star(const SimpleParserExpression & e1, const std::string & text,
            const std::string::size_type offset)
    {
        std::string::size_type new_offset(offset);

        while (true)
        {
            std::string::size_type len_1(e1.match(text, new_offset));
            if (std::string::npos == len_1 || 0 == len_1)
                return new_offset - offset;
            new_offset += len_1;
        }
    }

    std::string::size_type
    m_plus(const SimpleParserExpression & e1, const std::string & text,
            const std::string::size_type offset)
    {
        bool any(false);
        std::string::size_type new_offset(offset);

        while (true)
        {
            std::string::size_type len_1(e1.match(text, new_offset));
            if (std::string::npos == len_1)
                return any ? new_offset - offset : std::string::npos;
            else if (0 == len_1)
                return new_offset - offset;

            new_offset += len_1;
            any = true;
        }
    }

    std::string::size_type
    m_minus(const SimpleParserExpression & e1, const std::string & text,
            const std::string::size_type offset)
    {
        std::string::size_type len_1(e1.match(text, offset));
        return std::string::npos == len_1 ? 0 : len_1;
    }

    int compare_ignoring_case(const std::string & text, const std::string::size_type offset, const std::string::size_type length,
            const std::string & pattern)
    {
        return strncasecmp(text.c_str() + offset, pattern.c_str(), length);
    }

    std::string::size_type
    m_exact(const std::string & pattern, const std::string & text,
            const std::string::size_type offset, const bool ignore_case)
    {
        if (offset >= text.length())
            return std::string::npos;

        if (! ignore_case)
            return (0 == text.compare(offset, pattern.length(), pattern)) ? pattern.length() : std::string::npos;
        else
            return (0 == compare_ignoring_case(text, offset, pattern.length(), pattern)) ? pattern.length() : std::string::npos;
    }

    std::string::size_type
    m_any_of(const std::string & allowed, const std::string & text,
            const std::string::size_type offset)
    {
        if (offset >= text.length())
            return std::string::npos;

        if (std::string::npos != allowed.find(text.at(offset)))
            return 1;
        else
            return std::string::npos;
    }

    std::string::size_type
    m_any_except(const std::string & not_allowed, const std::string & text,
            const std::string::size_type offset)
    {
        if (offset >= text.length())
            return std::string::npos;

        if (std::string::npos != not_allowed.find(text.at(offset)))
            return std::string::npos;
        else
            return 1;
    }
}

SimpleParserExpression
paludis::simple_parser::operator& (const SimpleParserExpression & e1, const SimpleParserExpression & e2)
{
    using namespace std::placeholders;
    return SimpleParserExpression(std::bind(m_and, e1, e2, _1, _2));
}

SimpleParserExpression
paludis::simple_parser::operator>> (const SimpleParserExpression & e1, std::string & var)
{
    using namespace std::placeholders;
    return SimpleParserExpression(std::bind(m_capture, e1, std::ref(var), _1, _2));
}

SimpleParserExpression
paludis::simple_parser::operator* (const SimpleParserExpression & e1)
{
    using namespace std::placeholders;
    return SimpleParserExpression(std::bind(m_star, e1, _1, _2));
}

SimpleParserExpression
paludis::simple_parser::operator+ (const SimpleParserExpression & e1)
{
    using namespace std::placeholders;
    return SimpleParserExpression(std::bind(m_plus, e1, _1, _2));
}

SimpleParserExpression
paludis::simple_parser::operator- (const SimpleParserExpression & e1)
{
    using namespace std::placeholders;
    return SimpleParserExpression(std::bind(m_minus, e1, _1, _2));
}

SimpleParserExpression
paludis::simple_parser::exact(const std::string & s)
{
    using namespace std::placeholders;
    return SimpleParserExpression(std::bind(m_exact, s, _1, _2, false));
}

SimpleParserExpression
paludis::simple_parser::exact_ignoring_case(const std::string & s)
{
    using namespace std::placeholders;
    return SimpleParserExpression(std::bind(m_exact, s, _1, _2, true));
}

SimpleParserExpression
paludis::simple_parser::any_of(const std::string & s)
{
    using namespace std::placeholders;
    return SimpleParserExpression(std::bind(m_any_of, s, _1, _2));
}

SimpleParserExpression
paludis::simple_parser::any_except(const std::string & s)
{
    using namespace std::placeholders;
    return SimpleParserExpression(std::bind(m_any_except, s, _1, _2));
}

namespace paludis
{
    template <>
    struct Imp<SimpleParser>
    {
        std::string text;
        std::string::size_type offset;

        Imp(const std::string & t) :
            text(t),
            offset(0)
        {
        }
    };
}

SimpleParser::SimpleParser(const std::string & s) :
    _imp(s)
{
}

SimpleParser::~SimpleParser() = default;

bool
SimpleParser::consume(const SimpleParserExpression & e1)
{
    std::string::size_type len_1(e1.match(_imp->text, _imp->offset));
    if (std::string::npos == len_1)
        return false;
    else
    {
        _imp->offset += len_1;
        return true;
    }
}

bool
SimpleParser::lookahead(const SimpleParserExpression & e1) const
{
    std::string::size_type len_1(e1.match(_imp->text, _imp->offset));
    return std::string::npos != len_1;
}

bool
SimpleParser::eof() const
{
    return _imp->offset == _imp->text.length();
}

std::string::size_type
SimpleParser::offset() const
{
    return _imp->offset;
}

const std::string
SimpleParser::text() const
{
    return _imp->text;
}

unsigned
SimpleParser::current_line_number() const
{
    unsigned line_number(1);
    for (std::string::size_type p(0) ; p < _imp->offset ; ++p)
        if (_imp->text[p] == '\n')
            ++line_number;

    return line_number;
}

namespace paludis
{
    template class Pimp<SimpleParser>;
}
