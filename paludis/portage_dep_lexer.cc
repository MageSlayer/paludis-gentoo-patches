/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/portage_dep_lexer.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/tokeniser.hh>
#include <vector>
#include <list>

/** \file
 * Implementation of dep_lexer.hh things.
 *
 * \ingroup grpdeplexer
 */

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for PortageDepLexer.
     *
     * \ingroup grpdeplexer
     */
    template<>
    struct Implementation<PortageDepLexer>
    {
        std::list<std::pair<PortageDepLexerLexeme, std::string> > tokens;
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

PortageDepLexer::PortageDepLexer(const std::string & s) :
    PrivateImplementationPattern<PortageDepLexer>(new Implementation<PortageDepLexer>)
{
    Context context("When lexing dependency string '" + s + "':");

    Tokeniser<delim_kind::AnyOfTag, delim_mode::BoundaryTag> tokeniser(" \n\t");
    std::vector<std::string> tokens;
    tokeniser.tokenise(s, std::back_inserter(tokens));

    for (std::vector<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
            t != t_end ; ++t)
    {
        if (t->empty())
            continue;

        if (*t == "||")
            _imp->tokens.push_back(std::make_pair(dpl_double_bar, *t));
        else if ('|' == (*t)[0])
            throw DepStringLexError(s, "'|' should be followed by '|'");
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
        else if ('?' == (*t)[t->length() - 1])
            _imp->tokens.push_back(std::make_pair(dpl_use_flag, *t));
        else
            _imp->tokens.push_back(std::make_pair(dpl_text, *t));
    }
}

PortageDepLexer::~PortageDepLexer()
{
}

PortageDepLexer::Iterator
PortageDepLexer::begin() const
{
    return Iterator(_imp->tokens.begin());
}

PortageDepLexer::Iterator
PortageDepLexer::end() const
{
    return Iterator(_imp->tokens.end());
}

