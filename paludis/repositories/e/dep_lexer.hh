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

#ifndef PALUDIS_GUARD_PALUDIS_E_DEP_LEXER_HH
#define PALUDIS_GUARD_PALUDIS_E_DEP_LEXER_HH 1

#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <string>

/** \file
 * Declarations for the DepLexer class.
 *
 * \ingroup grpdeplexer
 */

namespace paludis
{
    namespace erepository
    {
        /**
         * A DepStringError descendent is thrown if an invalid depend string is
         * encountered.
         *
         * \ingroup grpdeplexer
         * \ingroup grpexceptions
         */
        class PALUDIS_VISIBLE DepStringError : public Exception
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
         * \ingroup grpdeplexer
         * \ingroup grpexceptions
         */
        class PALUDIS_VISIBLE DepStringLexError : public DepStringError
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
         *
         * \see DepLexer
         *
         * \ingroup grpdeplexer
         */
        enum DepLexerLexeme
        {
            dpl_whitespace,        ///< whitespace
            dpl_text,              ///< a package or item name
            dpl_use_flag,          ///< a use flag
            dpl_double_bar,        ///< a double bar ('any' marker)
            dpl_open_paren,        ///< open paren
            dpl_close_paren,       ///< close paren
            dpl_arrow,             ///< arrow
            dpl_label              ///< a label
        };

        /**
         * Converts a dependency string into a sequence of tokens, which are
         * then handled by DepParser.
         *
         * \see DepParser
         * \ingroup grpdeplexer
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE DepLexer :
            private InstantiationPolicy<DepLexer, instantiation_method::NonCopyableTag>,
            private PrivateImplementationPattern<DepLexer>
        {
            public:
                ///\name Iterate over our items
                ///\{

                typedef WrappedForwardIterator<enum ConstIteratorTag { },
                        const std::pair<DepLexerLexeme, std::string> > ConstIterator;

                ConstIterator begin() const;

                ConstIterator end() const;

                ///\}

                ///\name Basic operations
                ///\{

                DepLexer(const std::string &);
                ~DepLexer();

                ///\}
        };
    }
}

#endif
