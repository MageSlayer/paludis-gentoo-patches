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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SIMPLE_PARSER_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SIMPLE_PARSER_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/simple_parser-fwd.hh>
#include <functional>
#include <string>

namespace paludis
{
    namespace simple_parser
    {
        typedef std::function<std::string::size_type (const std::string &, const std::string::size_type)> SimpleParserMatchFunction;

        class PALUDIS_VISIBLE SimpleParserExpression
        {
            private:
                const SimpleParserMatchFunction _match;

            public:
                SimpleParserExpression(const SimpleParserMatchFunction &);

                std::string::size_type match(const std::string &, const std::string::size_type) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        SimpleParserExpression operator& (const SimpleParserExpression &, const SimpleParserExpression &)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        SimpleParserExpression operator>> (const SimpleParserExpression &, std::string &)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        SimpleParserExpression operator* (const SimpleParserExpression &)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        SimpleParserExpression operator+ (const SimpleParserExpression &)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        SimpleParserExpression operator- (const SimpleParserExpression &)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        SimpleParserExpression exact(const std::string &)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        SimpleParserExpression exact_ignoring_case(const std::string &)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        SimpleParserExpression any_of(const std::string &)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        SimpleParserExpression any_except(const std::string &)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));
    }

    class PALUDIS_VISIBLE SimpleParser
    {
        private:
            Pimp<SimpleParser> _imp;

        public:
            SimpleParser(const std::string &);
            ~SimpleParser();

            bool consume(const simple_parser::SimpleParserExpression &) PALUDIS_ATTRIBUTE((warn_unused_result));
            bool lookahead(const simple_parser::SimpleParserExpression &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool eof() const PALUDIS_ATTRIBUTE((warn_unused_result));

            std::string::size_type offset() const PALUDIS_ATTRIBUTE((warn_unused_result));
            unsigned current_line_number() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Pimp<SimpleParser>;
}

#endif
