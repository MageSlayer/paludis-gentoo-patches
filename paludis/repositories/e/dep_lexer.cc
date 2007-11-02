/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#include <paludis/dep_spec.hh>
#include <paludis/repositories/e/dep_lexer.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/tokeniser.hh>
#include <vector>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

template class WrappedForwardIterator<DepLexer::ConstIteratorTag,
         const std::pair<DepLexerLexeme, std::string> >;

namespace paludis
{
    /**
     * Implementation data for DepLexer.
     *
     * \ingroup grpdeplexer
     */
    template<>
    struct Implementation<DepLexer>
    {
        std::list<std::pair<DepLexerLexeme, std::string> > tokens;
    };
}

DepStringLexError::DepStringLexError(const std::string & dep_string,
        const std::string & our_message) throw () :
    DepStringError(dep_string, "in lex phase: " + our_message)
{
}

DepStringError::DepStringError(const std::string & d, const std::string & m) throw () :
    Exception("Bad dependency string '" + d + "': " + m)
{
}

DepLexer::DepLexer(const std::string & s) :
    PrivateImplementationPattern<DepLexer>(new Implementation<DepLexer>)
{
    Context context("When lexing dependency string '" + s + "':");

    std::vector<std::string> tokens;
    Tokeniser<delim_kind::AnyOfTag, delim_mode::BoundaryTag>::tokenise(s, " \n\t", std::back_inserter(tokens));

    for (std::vector<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
            t != t_end ; ++t)
    {
        if (t->empty())
            continue;

        if (*t == "||")
            _imp->tokens.push_back(std::make_pair(dpl_double_bar, *t));
        else if ('|' == (*t)[0])
            throw DepStringLexError(s, "'|' should be followed by '|'");
        else if (*t == "->")
            _imp->tokens.push_back(std::make_pair(dpl_arrow, *t));
        else if (*t == "(")
            _imp->tokens.push_back(std::make_pair(dpl_open_paren, *t));
        else if ('(' == (*t)[0])
            throw DepStringLexError(s, "'(' should be followed by whitespace");
        else if (*t == ")")
            _imp->tokens.push_back(std::make_pair(dpl_close_paren, *t));
        else if (')' == (*t)[0])
            throw DepStringLexError(s, "')' should be followed by whitespace");
        else if (std::string::npos == t->find_first_not_of(" \t\n"))
            _imp->tokens.push_back(std::make_pair(dpl_whitespace, *t));
        else if (':' == (*t)[t->length() - 1])
            _imp->tokens.push_back(std::make_pair(dpl_label, *t));
        else if ('?' == (*t)[t->length() - 1])
            _imp->tokens.push_back(std::make_pair(dpl_use_flag, *t));
        else
            _imp->tokens.push_back(std::make_pair(dpl_text, *t));
    }
}

DepLexer::~DepLexer()
{
}

DepLexer::ConstIterator
DepLexer::begin() const
{
    return ConstIterator(_imp->tokens.begin());
}

DepLexer::ConstIterator
DepLexer::end() const
{
    return ConstIterator(_imp->tokens.end());
}

