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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_PARSER_HH
#define PALUDIS_GUARD_PALUDIS_DEP_PARSER_HH 1

#include <paludis/dep_atom.hh>
#include <paludis/counted_ptr.hh>
#include <paludis/instantiation_policy.hh>
#include <paludis/exception.hh>
#include <paludis/dep_lexer.hh>
#include <bitset>
#include <string>

/** \file
 * Declarations for the DepParser class.
 *
 * \ingroup DepResolver
 * \ingroup Exception
 */

namespace paludis
{
    /**
     * A DepStringParseError is thrown if an error is encountered when parsing
     * a dependency string.
     *
     * \ingroup Exception
     * \ingroup DepResolver
     */
    class DepStringParseError : public DepStringError
    {
        public:
            /**
             * Constructor.
             */
            DepStringParseError(const std::string & dep_string,
                    const std::string & message) throw ();
    };

    /**
     * A DepStringNestingError is thrown if a dependency string does not have
     * properly balanced parentheses.
     */
    class DepStringNestingError : public DepStringParseError
    {
        public:
            /**
             * Constructor.
             */
            DepStringNestingError(const std::string & dep_string) throw ();
    };

    /**
     * Options for a DepParser.
     */
    enum DepParserOptionValues
    {
        dpo_qualified_package_names, ///< Tokens must be qualified package names
        dpo_allow_any_blocks,        ///< Allow || () blocks?
        last_dpo
    };

    /**
     * A set of options for DepParser.
     */
    typedef std::bitset<last_dpo> DepParserOptions;

    /**
     * The DepParser converts string representations of a dependency
     * specification into a DepAtom instance.
     */
    class DepParser :
        private InstantiationPolicy<DepParser, instantiation_method::NonInstantiableTag>
    {
        protected:
            /**
             * Default options for parse.
             */
            static DepParserOptions default_options();

        public:
            /**
             * Parse a given dependency string, and return an appropriate
             * DepAtom tree.
             */
            static CompositeDepAtom::ConstPointer parse(const std::string & s,
                    const DepParserOptions & opts = default_options());
    };
}

#endif
