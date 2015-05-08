/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2010 Ciaran McCreesh
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

#include <paludis/util/operators.hh>
#include <paludis/util/exception.hh>
#include <paludis/version_operator-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <string>

/** \file
 * Declarations for VersionOperator classes.
 *
 * \ingroup g_names
 *
 * \section Examples
 *
 * - \ref example_version_operator.cc "example_version_operator.cc"
 * - \ref example_version_spec.cc "example_version_spec.cc"
 */

namespace paludis
{
    /**
     * An operator attached to a VersionSpec, validated.
     *
     * \ingroup g_names
     */
    class PALUDIS_VISIBLE VersionOperator :
        public equality_operators::HasEqualityOperators
    {
        friend std::ostream & operator<< (std::ostream &, const VersionOperator &);

        private:
            static VersionOperatorValue _decode(const std::string & v);

            VersionOperatorValue _v;

        public:
            ///\name Basic operations
            ///\{

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

            ///\}

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
     * \ingroup g_names
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE BadVersionOperatorError :
        public Exception
    {
        public:
            /**
             * Constructor.
             */
            BadVersionOperatorError(const std::string & msg) noexcept;
    };
}

#endif
