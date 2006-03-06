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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_PARSER_LEXER_HH
#define PALUDIS_GUARD_PALUDIS_DEP_PARSER_LEXER_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/exception.hh>
#include <list>
#include <string>

/** \file
 * Declarations for the DepLexer class.
 *
 * \ingroup DepResolver
 * \ingroup Exception
 */

namespace paludis
{
    /**
     * A DepStringError descendent is thrown if an invalid depend string is
     * encountered.
     *
     * \ingroup Exception
     * \ingroup DepResolver
     */
    class DepStringError : public Exception
    {
        protected:
            /**
             * Constructor.
             */
            DepStringError(const std::string & dep_string,
                    const std::string & message) throw ();
    };

    /**
     * A DepStringLexError is thrown if a lex-level error is encountered when
     * parsing a dependency string.
     *
     * \ingroup Exception
     * \ingroup DepResolver
     */
    class DepStringLexError : public DepStringError
    {
        public:
            /**
             * Constructor.
             */
            DepStringLexError(const std::string & dep_string,
                    const std::string & message) throw ();
    };

    /**
     * Lexemes used by DepLexer.
     */
    enum DepLexerLexeme
    {
        dpl_whitespace,        ///< whitespace
        dpl_text,              ///< a package or item name
        dpl_use_flag,          ///< a use flag
        dpl_double_bar,        ///< a double bar ('any' marker)
        dpl_open_paren,        ///< open paren
        dpl_close_paren        ///< close paren
    };

    /**
     * Converts a dependency string into a sequence of tokens.
     */
    class DepLexer :
        private InstantiationPolicy<DepLexer, instantiation_method::NonCopyableTag>
    {
        private:
            std::list<std::pair<DepLexerLexeme, std::string> > _tokens;

        public:
            /**
             * Iterator for our tokens.
             */
            typedef std::list<std::pair<DepLexerLexeme, std::string> >::const_iterator Iterator;

            /**
             * Iterator to the start of our tokens.
             */
            Iterator begin() const
            {
                return _tokens.begin();
            }

            /**
             * Iterator to the end of our tokens.
             */
            Iterator end() const
            {
                return _tokens.end();
            }

            /**
             * Constructor.
             */
            DepLexer(const std::string &);
    };
}

#endif
