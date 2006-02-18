/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "exception.hh"
#include "stringify.hh"
#include "nest_atom.hh"
#include "nest_parser.hh"
#include "dep_lexer.hh"
#include <stack>

using namespace paludis;

NestStringParseError::NestStringParseError(const std::string & d,
        const std::string & m) throw () :
    DepStringError(d, "in nest parse phase: " + m)
{
}

NestStringNestingError::NestStringNestingError(const std::string & dep_string) throw () :
    NestStringParseError(dep_string, "improperly balanced parentheses")
{
}

enum NestParserState
{
    nps_initial,
    nps_had_paren,
    nps_had_use_flag,
    nps_had_use_flag_space
};

CompositeNestAtom::ConstPointer
NestParser::parse(const std::string & s)
{
    Context context("When parsing nest string '" + s + "':");

    std::stack<CompositeNestAtom::Pointer> stack;
    stack.push(CompositeNestAtom::Pointer(new AllNestAtom));

    NestParserState state(nps_initial);
    DepLexer lexer(s);
    DepLexer::Iterator i(lexer.begin()), i_end(lexer.end());

    for ( ; i != i_end ; ++i)
    {
        Context context("When handling lexer token '" + i->second +
                "' (" + stringify(i->first) + "):");
        do
        {
            switch (state)
            {
                case nps_initial:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_whitespace:
                                 continue;

                            case dpl_text:
                                 {
                                     if (i->second.empty())
                                         throw NestStringParseError(i->second, "Empty item name");
                                     stack.top()->add_child(NestAtom::Pointer(
                                                 new TextNestAtom(i->second)));
                                 }
                                 continue;

                            case dpl_open_paren:
                                 {
                                     CompositeNestAtom::Pointer a(new AllNestAtom);
                                     stack.top()->add_child(a);
                                     stack.push(a);
                                     state = nps_had_paren;
                                 }
                                 continue;

                            case dpl_close_paren:
                                 if (stack.empty())
                                     throw NestStringNestingError(s);
                                 stack.pop();
                                 if (stack.empty())
                                     throw NestStringNestingError(s);
                                 state = nps_had_paren;
                                 continue;

                            case dpl_double_bar:
                                 throw NestStringParseError(s, "|| not allowed");

                            case dpl_use_flag:
                                 {
                                     std::string f(i->second);
                                     bool inv(f.length() && ('!' == f.at(0)));
                                     if (inv)
                                         f.erase(0, 1);

                                     if (f.empty())
                                         throw NestStringParseError(s,
                                                 "Bad use flag name '" + i->second + "'");
                                     if ('?' != f.at(f.length() - 1))
                                         throw NestStringParseError(s,
                                                 "Use flag name '" + i->second + "' missing '?'");

                                     f.erase(f.length() - 1);
                                     CompositeNestAtom::Pointer a(
                                             new UseNestAtom(UseFlagName(f), inv));
                                     stack.top()->add_child(a);
                                     stack.push(a);
                                     state = nps_had_use_flag;
                                 }
                                 continue;

                        }
                        throw InternalError(PALUDIS_HERE,
                                "nps_initial: i->first is " + stringify(i->first));

                    } while (0);
                    continue;

                case nps_had_paren:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_whitespace:
                                state = nps_initial;
                                continue;

                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_open_paren:
                            case dpl_close_paren:
                                throw NestStringParseError(s, "Expected space after '(' or ')'");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "nps_had_paren: i->first is " + stringify(i->first));
                    } while (0);
                    continue;

                case nps_had_use_flag:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_whitespace:
                                state = nps_had_use_flag_space;
                                continue;

                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_open_paren:
                            case dpl_close_paren:
                                throw NestStringParseError(s, "Expected space after use flag");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "nps_had_use_flag: i->first is " + stringify(i->first));
                    } while (0);
                    continue;

                case nps_had_use_flag_space:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_open_paren:
                                state = nps_had_paren;
                                continue;

                            case dpl_whitespace:
                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_close_paren:
                                throw NestStringParseError(s, "Expected '(' after use flag");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "nps_had_use_flag_space: i->first is " + stringify(i->first));
                    } while (0);
                    continue;
            }
            throw InternalError(PALUDIS_HERE,
                    "state is " + stringify(state));

        } while (0);
    }

    if (stack.empty())
        throw NestStringNestingError(s);
    CompositeNestAtom::Pointer result(stack.top());
    stack.pop();
    if (! stack.empty())
        throw NestStringNestingError(s);
    return result;
}
