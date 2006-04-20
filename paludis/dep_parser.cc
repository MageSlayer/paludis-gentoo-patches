/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/dep_atom.hh>
#include <paludis/dep_lexer.hh>
#include <paludis/dep_parser.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <stack>

/** \file
 * Implementation for dep_parser.hh things.
 *
 * \ingroup grpdepparser
 */

using namespace paludis;

DepStringParseError::DepStringParseError(const std::string & d,
        const std::string & m) throw () :
    DepStringError(d, "in parse phase: " + m)
{
}

DepStringNestingError::DepStringNestingError(const std::string & dep_string) throw () :
    DepStringParseError(dep_string, "improperly balanced parentheses")
{
}

namespace
{
    /**
     * Our current state.
     *
     * \ingroup grpdepparser
     */
    enum DepParserState
    {
        dps_initial,
        dps_had_double_bar,
        dps_had_double_bar_space,
        dps_had_paren,
        dps_had_use_flag,
        dps_had_use_flag_space
    };
}

CompositeDepAtom::Pointer
DepParser::parse(const std::string & s, const DepParserPolicyInterface * const policy)
{
    Context context("When parsing dependency string '" + s + "':");

    std::stack<CompositeDepAtom::Pointer> stack;
    stack.push(CompositeDepAtom::Pointer(new AllDepAtom));

    DepParserState state(dps_initial);
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
                case dps_initial:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_whitespace:
                                 continue;

                            case dpl_text:
                                 {
                                     if (i->second.empty())
                                         throw DepStringParseError(i->second, "Empty text entry");
                                     stack.top()->add_child(policy->new_text_atom(i->second));
                                 }
                                 continue;

                            case dpl_open_paren:
                                 {
                                     CompositeDepAtom::Pointer a(new AllDepAtom);
                                     stack.top()->add_child(a);
                                     stack.push(a);
                                     state = dps_had_paren;
                                 }
                                 continue;

                            case dpl_close_paren:
                                 if (stack.empty())
                                     throw DepStringNestingError(s);
                                 stack.pop();
                                 if (stack.empty())
                                     throw DepStringNestingError(s);
                                 state = dps_had_paren;
                                 continue;

                            case dpl_double_bar:
                                 if (policy->permit_any_deps())
                                 {
                                     CompositeDepAtom::Pointer a(new AnyDepAtom);
                                     stack.top()->add_child(a);
                                     stack.push(a);
                                     state = dps_had_double_bar;
                                 }
                                 else
                                     throw DepStringParseError(s, "|| is not allowed here");

                                 continue;

                            case dpl_use_flag:
                                 {
                                     std::string f(i->second);
                                     bool inv(f.length() && ('!' == f.at(0)));
                                     if (inv)
                                         f.erase(0, 1);

                                     if (f.empty())
                                         throw DepStringParseError(s,
                                                 "Bad use flag name '" + i->second + "'");
                                     if ('?' != f.at(f.length() - 1))
                                         throw DepStringParseError(s,
                                                 "Use flag name '" + i->second + "' missing '?'");

                                     f.erase(f.length() - 1);
                                     CompositeDepAtom::Pointer a(
                                             new UseDepAtom(UseFlagName(f), inv));
                                     stack.top()->add_child(a);
                                     stack.push(a);
                                     state = dps_had_use_flag;
                                 }
                                 continue;

                        }
                        throw InternalError(PALUDIS_HERE,
                                "dps_initial: i->first is " + stringify(i->first));

                    } while (0);
                    continue;

                case dps_had_double_bar:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_whitespace:
                                state = dps_had_double_bar_space;
                                continue;

                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_open_paren:
                            case dpl_close_paren:
                                throw DepStringParseError(s, "Expected space after '||'");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "dps_had_double_bar: i->first is " + stringify(i->first));

                    } while (0);
                    continue;

                case dps_had_double_bar_space:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_open_paren:
                                state = dps_initial;
                                continue;

                            case dpl_whitespace:
                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_close_paren:
                                throw DepStringParseError(s, "Expected '(' after '|| '");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "dps_had_double_bar_space: i->first is " + stringify(i->first));
                    } while (0);
                    continue;

                case dps_had_paren:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_whitespace:
                                state = dps_initial;
                                continue;

                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_open_paren:
                            case dpl_close_paren:
                                throw DepStringParseError(s, "Expected space after '(' or ')'");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "dps_had_paren: i->first is " + stringify(i->first));
                    } while (0);
                    continue;

                case dps_had_use_flag:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_whitespace:
                                state = dps_had_use_flag_space;
                                continue;

                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_open_paren:
                            case dpl_close_paren:
                                throw DepStringParseError(s, "Expected space after use flag");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "dps_had_use_flag: i->first is " + stringify(i->first));
                    } while (0);
                    continue;

                case dps_had_use_flag_space:
                    do
                    {
                        switch (i->first)
                        {
                            case dpl_open_paren:
                                state = dps_had_paren;
                                continue;

                            case dpl_whitespace:
                            case dpl_text:
                            case dpl_use_flag:
                            case dpl_double_bar:
                            case dpl_close_paren:
                                throw DepStringParseError(s, "Expected '(' after use flag");
                        }
                        throw InternalError(PALUDIS_HERE,
                                "dps_had_use_flag_space: i->first is " + stringify(i->first));
                    } while (0);
                    continue;
            }
            throw InternalError(PALUDIS_HERE,
                    "state is " + stringify(state));

        } while (0);
    }

    if (stack.empty())
        throw DepStringNestingError(s);
    CompositeDepAtom::Pointer result(stack.top());
    stack.pop();
    if (! stack.empty())
        throw DepStringNestingError(s);
    return result;
}

