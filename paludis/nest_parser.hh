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

#ifndef PALUDIS_GUARD_PALUDIS_NEST_PARSER_HH
#define PALUDIS_GUARD_PALUDIS_NEST_PARSER_HH 1

#include <paludis/nest_atom.hh>
#include <paludis/counted_ptr.hh>
#include <paludis/instantiation_policy.hh>
#include <paludis/exception.hh>
#include <paludis/dep_lexer.hh>
#include <string>

/** \file
 * Declarations for the NestParser class.
 *
 * \ingroup DepResolver
 * \ingroup Exception
 */

namespace paludis
{
    /**
     * A NestStringParseError is thrown if an error is encountered when parsing
     * a dependency string.
     *
     * \ingroup Exception
     * \ingroup DepResolver
     */
    class NestStringParseError : public DepStringError
    {
        public:
            /**
             * Constructor.
             */
            NestStringParseError(const std::string & dep_string,
                    const std::string & message) throw ();
    };

    /**
     * A NestStringNestingError is thrown if a dependency string does not have
     * properly balanced parentheses.
     */
    class NestStringNestingError : public NestStringParseError
    {
        public:
            /**
             * Constructor.
             */
            NestStringNestingError(const std::string & dep_string) throw ();
    };

    /**
     * The NestParser converts string representations of a reduced nested
     * string specification into a NestAtom instance.
     */
    class NestParser :
        private InstantiationPolicy<NestParser, instantiation_method::NonInstantiableTag>
    {
        public:
            /**
             * Parse a given dependency string, and return an appropriate
             * NestAtom tree.
             */
            static CompositeNestAtom::ConstPointer parse(const std::string & s);
    };
}

#endif
