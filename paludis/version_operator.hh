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

    /**
     * Represents an operator attached to a VersionSpec.
     *
     * \ingroup grpversions
     */
    enum VersionOperatorValue
    {
        vo_less_equal,       ///< <= dependency
        vo_less,             ///< < dependency
        vo_equal,            ///< = dependency
        vo_tilde,            ///< ~ dependency
        vo_greater,          ///< > dependency
        vo_greater_equal,    ///< >= dependency
        vo_equal_star,       ///< =* dependency
        vo_tilde_greater,    ///< ~> dependency (gems)
        last_vo              ///< number of items
    };

    /**
     * An operator attached to a VersionSpec, validated.
     *
     * \ingroup grpversions
     */
    class VersionOperator : public ComparisonPolicy<VersionOperator,
                                comparison_mode::EqualityComparisonTag,
                                comparison_method::CompareByMemberTag<VersionOperatorValue> >
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
                ComparisonPolicyType(&VersionOperator::_v),
                _v(v)
            {
            }

            /**
             * Copy constructor.
             */
            VersionOperator(const VersionOperator & other) :
                ComparisonPolicyType(other),
                _v(other._v)
            {
            }

            /**
             * Constructor, from a string.
             */
            explicit VersionOperator(const std::string & v) :
                ComparisonPolicyType(&VersionOperator::_v),
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
             * Return a pointer to member operator for VersionSpec that
             * corresponds to a particular operator.
             */
            bool (VersionSpec::* as_version_spec_operator() const)(const VersionSpec &) const;
    };

    /**
     * Thrown if a bad version operator is encountered.
     *
     * \ingroup grpversions
     * \ingroup grpexceptions
     */
    class BadVersionOperatorError :
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
    std::ostream & operator<< (std::ostream & s, const VersionOperator &);
}

#endif
