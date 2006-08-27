/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_PORTAGE_DEP_PARSER_HH
#define PALUDIS_GUARD_PALUDIS_PORTAGE_DEP_PARSER_HH 1

#include <paludis/dep_atom.hh>
#include <paludis/portage_dep_lexer.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <string>

/** \file
 * Declarations for the PortageDepParser class.
 *
 * \ingroup grpdepparser
 */

namespace paludis
{
    /**
     * A DepStringParseError is thrown if an error is encountered when parsing
     * a dependency string.
     *
     * \ingroup grpexceptions
     * \ingroup grpdepparser
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
     *
     * \ingroup grpexceptions
     * \ingroup grpdepparser
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
     * Interface provided by PortageDepParserPolicy classes.
     *
     * \see PortageDepParserPolicy
     *
     * \ingroup grpdepparser
     */
    struct PortageDepParserPolicyInterface
    {
        /**
         * Create a new text atom from the provided string.
         */
        virtual CountedPtr<DepAtom> new_text_atom(const std::string &) const = 0;

        /**
         * Are || ( ) deps permitted?
         */
        virtual bool permit_any_deps() const = 0;

        /**
         * Destructor.
         */
        virtual ~PortageDepParserPolicyInterface()
        {
        }
    };

    /**
     * Policy class describing how PortageDepParser::parse should behave.
     *
     * \see PortageDepParser
     *
     * \ingroup grpdepparser
     */
    template <typename TextAtom_, bool permit_any_>
    class PortageDepParserPolicy :
        public PortageDepParserPolicyInterface,
        public InstantiationPolicy<PortageDepParserPolicy<TextAtom_, permit_any_>,
            instantiation_method::SingletonAsNeededTag>
    {
        friend class InstantiationPolicy<PortageDepParserPolicy<TextAtom_, permit_any_>,
            instantiation_method::SingletonAsNeededTag>;

        private:
            PortageDepParserPolicy()
            {
            }

        public:
            virtual CountedPtr<DepAtom> new_text_atom(const std::string & s) const
            {
                return CountedPtr<DepAtom>(new TextAtom_(s));
            }

            virtual bool permit_any_deps() const
            {
                return permit_any_;
            }
    };

    /**
     * Policy class describing how PortageDepParser::parse should behave
     * (specialisation for PackageDepAtom).
     *
     * \see PortageDepParser
     *
     * \ingroup grpdepparser
     */
    template <bool permit_any_>
    class PortageDepParserPolicy<PackageDepAtom, permit_any_> :
        public PortageDepParserPolicyInterface,
        public InstantiationPolicy<PortageDepParserPolicy<PackageDepAtom, permit_any_>,
            instantiation_method::SingletonAsNeededTag>
    {
        friend class InstantiationPolicy<PortageDepParserPolicy<PackageDepAtom, permit_any_>,
            instantiation_method::SingletonAsNeededTag>;

        private:
            PortageDepParserPolicy()
            {
            }

        public:
            virtual CountedPtr<DepAtom> new_text_atom(const std::string & s) const
            {
                if ((! s.empty()) && ('!' == s.at(0)))
                    return CountedPtr<DepAtom>(new BlockDepAtom(
                                CountedPtr<PackageDepAtom>(new PackageDepAtom(s.substr(1)))));
                else
                    return CountedPtr<DepAtom>(new PackageDepAtom(s));
            }

            virtual bool permit_any_deps() const
            {
                return permit_any_;
            }
    };

    /**
     * The PortageDepParser converts string representations of a dependency
     * specification into a DepAtom instance. The PortageDepLexer class is
     * used as the first stage.
     *
     * \ingroup grpdepparser
     */
    class PortageDepParser :
        private InstantiationPolicy<PortageDepParser, instantiation_method::NonInstantiableTag>
    {
        private:
            typedef PortageDepParserPolicy<PackageDepAtom, true> DefaultPolicy;

        public:
            /**
             * Parse a given dependency string, and return an appropriate
             * DepAtom tree.
             */
            static CompositeDepAtom::Pointer parse(const std::string & s,
                    const PortageDepParserPolicyInterface * const policy = DefaultPolicy::get_instance());

            /**
             * Convenience wrapper for parse for depend strings, for VersionMetadata.
             */
            static DepAtom::ConstPointer parse_depend(const std::string & s);
    };
}

#endif
