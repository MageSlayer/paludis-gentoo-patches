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

#ifndef PALUDIS_GUARD_PALUDIS_VERSION_OPERATOR_HH
#define PALUDIS_GUARD_PALUDIS_VERSION_OPERATOR_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/validated.hh>

#include <string>
#include <iosfwd>

/** \file
 * Declarations for the VersionOperator class.
 *
 * \ingroup grpversions
 */

namespace paludis
{
    class VersionSpec;

#include <paludis/version_operator-se.hh>

    /**
     * An operator attached to a VersionSpec, validated.
     *
     * \ingroup grpversions
     */
    class PALUDIS_VISIBLE VersionOperator :
        public equality_operators::HasEqualityOperators
    {
        friend std::ostream & operator<< (std::ostream &, const VersionOperator &);

        private:
            static VersionOperatorValue _decode(const std::string & v);

            VersionOperatorValue _v;

        public:
            /**
             * Constructor.
             */
            VersionOperator(const VersionOperatorValue v) :
                _v(v)
            {
            }

            /**
             * Copy constructor.
             */
            VersionOperator(const VersionOperator & other) :
                _v(other._v)
            {
            }

            /**
             * Constructor, from a string.
             */
            explicit VersionOperator(const std::string & v) :
                _v(_decode(v))
            {
            }

            /**
             * Assignment.
             */
            const VersionOperator & operator= (const VersionOperator & other)
            {
                _v = other._v;
                return *this;
            }

            /**
             * Return value.
             */
            VersionOperatorValue value() const
            {
                return _v;
            }

            /**
             * A VersionSpec comparator function.
             */
            typedef bool (* VersionSpecComparator) (const VersionSpec &, const VersionSpec &);

            /**
             * Fetch a VersionSpecComparator.
             */
            VersionSpecComparator as_version_spec_comparator() const;

            ///\name Comparison operators
            ///\{

            bool operator== (const VersionOperator & other) const
            {
                return _v == other._v;
            }

            ///\}
    };

    /**
     * Thrown if a bad version operator is encountered.
     *
     * \ingroup grpversions
     * \ingroup grpexceptions
     */
    class PALUDIS_VISIBLE BadVersionOperatorError :
        public Exception
    {
        public:
            /**
             * Constructor.
             */
            BadVersionOperatorError(const std::string & msg) throw ();
    };

    /**
     * A VersionOperator can be written to an ostream.
     *
     * \ingroup grpversions
     */
    std::ostream & operator<< (std::ostream & s, const VersionOperator &) PALUDIS_VISIBLE;
}

#endif
