/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/elike_dep_parser.hh>
#include <paludis/util/simple_parser.hh>
#include <paludis/util/kc.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/name.hh>

using namespace paludis;

namespace
{
    void error(const SimpleParser &, const ELikeDepParserCallbacks &, const std::string &) PALUDIS_ATTRIBUTE((noreturn));

    void error(const SimpleParser & parser, const ELikeDepParserCallbacks & callbacks, const std::string & msg)
    {
        callbacks[k::on_error()](parser.text(), parser.offset(), msg);
        throw InternalError(PALUDIS_HERE, "Got error '" + msg + "' parsing '" + parser.text() +
                "', but the error function returned");
    }

    void
    parse(SimpleParser & parser, const ELikeDepParserCallbacks & callbacks, const bool end_with_close_paren,
            const bool child_of_any)
    {
        while (true)
        {
            Context context("When parsing from offset '" + stringify(parser.offset()) + "':");
            std::string word;

            if (parser.eof())
            {
                if (end_with_close_paren)
                    error(parser, callbacks, "Reached end of text but wanted ')'");
                else
                    return;
            }
            else if (parser.consume(simple_parser::exact("(")))
            {
                if (! parser.consume(+simple_parser::any_of(" \t\r\n")))
                    error(parser, callbacks, "Expected space after '('");

                callbacks[k::on_all()]();
                parse(parser, callbacks, true, false);
            }
            else if (parser.consume(+simple_parser::any_of(" \t\r\n")))
            {
                /* discard whitespace */
            }
            else if (parser.consume(simple_parser::exact(")")))
            {
                if (end_with_close_paren)
                {
                    if (! parser.eof())
                        if (! parser.consume(+simple_parser::any_of(" \t\r\n")))
                            error(parser, callbacks, "Expected space or end of text after ')'");
                    callbacks[k::on_pop()]();
                    return;
                }
                else
                    error(parser, callbacks, "Got ')' but expected end of text");
            }
            else if (parser.consume(simple_parser::exact("->")))
            {
                error(parser, callbacks, "Can't have '->' here");
            }
            else if (parser.consume(simple_parser::exact("||")))
            {
                if (! parser.consume(+simple_parser::any_of(" \t\r\n")))
                    error(parser, callbacks, "Expected space after '||'");

                if (! parser.consume(simple_parser::exact("(")))
                    error(parser, callbacks, "Expected '(' after '||' then space");

                if (! parser.consume(+simple_parser::any_of(" \t\r\n")))
                    error(parser, callbacks, "Expected space after '|| ('");

                callbacks[k::on_any()]();
                parse(parser, callbacks, true, true);
            }
            else if (parser.consume(+simple_parser::any_except(" \t\r\n") >> word))
            {
                if ('?' == word.at(word.length() - 1))
                {
                    if (! parser.consume(+simple_parser::any_of(" \t\r\n")))
                        error(parser, callbacks, "Expected space after 'use?'");

                    if (! parser.consume(simple_parser::exact("(")))
                        error(parser, callbacks, "Expected '(' after 'use?' then space");

                    if (! parser.consume(+simple_parser::any_of(" \t\r\n")))
                        error(parser, callbacks, "Expected space after 'use? ('");

                    if (child_of_any)
                        callbacks[k::on_use_under_any()]();

                    callbacks[k::on_use()](word);
                    parse(parser, callbacks, true, false);
                }
                else if (':' == word.at(word.length() - 1))
                {
                    callbacks[k::on_label()](word);
                }
                else if (parser.consume(+simple_parser::any_of(" \t\r\n") & simple_parser::exact("->")))
                {
                    if (! parser.consume(+simple_parser::any_of(" \t\r\n")))
                        error(parser, callbacks, "Expected space after '->'");

                    std::string second;
                    if (! parser.consume(+simple_parser::any_except(" \t\r\n") >> second))
                        error(parser, callbacks, "Expected word after '->' then space");

                    if ("->" == second || "||" == second || "(" == second || ")" == second)
                        error(parser, callbacks, "Expected word after '->' then space");

                    callbacks[k::on_arrow()](word, second);
                }
                else
                    callbacks[k::on_string()](word);
            }
            else
                error(parser, callbacks, "Unexpected trailing text");
        }
    }
}

void
paludis::parse_elike_dependencies(const std::string & s, const ELikeDepParserCallbacks & callbacks)
{
    Context context("When parsing '" + s + "':");

    SimpleParser parser(s);
    parse(parser, callbacks, false, false);
    callbacks[k::on_should_be_empty()]();
}

