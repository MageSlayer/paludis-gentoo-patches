#if 0
ifdef(`__gnu__',`',`errprint(`This is not GNU m4...
')m4exit(1)') include(`misc/generated-file.txt')
dnl vim: set ft=cpp et sw=4 sts=4 :
#endif

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

#ifndef PALUDIS_GUARD_PALUDIS_COMPARISON_POLICY_HH
#define PALUDIS_GUARD_PALUDIS_COMPARISON_POLICY_HH 1

namespace paludis
{
    /**
     * Comparison modes for paludis::ComparisonPolicy.
     */
    namespace comparison_mode
    {
        /**
         * No comparisons can be made.
         */
        struct NoComparisonTag
        {
        };

        /**
         * Comparisons can be made via operator== and operator!=.
         */
        struct EqualityComparisonTag
        {
        };

        /**
         * The full range of comparison operators is available.
         */
        struct FullComparisonTag
        {
        };
    }

    /**
     * Comparison methods for paludis::ComparisonPolicy.
     */
    namespace comparison_method
    {
        /**
         * Comparisons are done via a member of type MemberType_.
         */
        template <typename MemberType_>
        struct CompareByMemberTag
        {
        };

        /**
         * Comparisons are done by a member function that returns an integer
         * less than zero (less than), equal to zero (equal to) or greater than
         * zero (greater than).
         */
        struct CompareByMemberComparisonFunctionTag
        {
        };


        /**
         * Comparisons are done via a member function that returns an item of
         * type MemberType_.
         */
        template <typename MemberType_>
        struct CompareByMemberFetchFunctionTag
        {
        };
    }

#ifdef DOXYGEN
    /**
     * ComparisonPolicy specifies the availabillity of comparison methods and
     * the strategy used to do comparisons.
     */
    template <typename OurType_, typename ComparisonModeTag_, typename ComparisonMethodTag_>
    struct ComparisonPolicy
    {
    };
#else
    template <typename OurType_, typename ComparisonModeTag_, typename ComparisonMethodTag_>
    struct ComparisonPolicy;
#endif

    /**
     * ComparisonPolicy: specialisation for NoComparisonTag.
     */
    template <typename OurType_, typename ComparisonMethodTag_>
    class ComparisonPolicy<OurType_, comparison_mode::NoComparisonTag, ComparisonMethodTag_>
    {
        public:
            /// Our comparison mode.
            typedef comparison_mode::NoComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef ComparisonMethodTag_ ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;
    };

    /**
     * ComparisonPolicy: specialisation for EqualityComparisonTag +
     * CompareByMemberTag.
     */
    template <typename OurType_, typename MemberType_>
    class ComparisonPolicy<OurType_, comparison_mode::EqualityComparisonTag,
          comparison_method::CompareByMemberTag<MemberType_> >
    {
        private:
            const MemberType_ OurType_::* const _v;

        public:
            /// Our comparison mode.
            typedef comparison_mode::EqualityComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef comparison_method::CompareByMemberTag<MemberType_> ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            /// Constructor.
            ComparisonPolicy(const MemberType_ OurType_::* const v) :
                _v(v)
            {
            }

            /// Copy constructor.
            ComparisonPolicy(const ComparisonPolicy & other) :
                _v(other._v)
            {
            }

define(`make_operator', `
            /// $1 operator.
            bool operator$1 (const OurType_ & other) const
            {
                return static_cast<const OurType_ *>(this)->*_v $1 other.*(
                        (static_cast<const ComparisonPolicyType *>(&other))->_v);
            }
')
make_operator(`==')
make_operator(`!=')

    };

    /**
     * ComparisonPolicy: specialisation for EqualityComparisonTag +
     * CompareByMemberComparisonFunctionTag.
     */
    template <typename OurType_>
    class ComparisonPolicy<OurType_, comparison_mode::EqualityComparisonTag,
        comparison_method::CompareByMemberComparisonFunctionTag>
    {
        private:
            bool (OurType_::* const _v)(const OurType_ &) const;

        public:
            /// Our comparison mode.
            typedef comparison_mode::EqualityComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef comparison_method::CompareByMemberComparisonFunctionTag ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            /// Constructor.
            ComparisonPolicy(bool (OurType_::* const v)(const OurType_ &) const) :
                _v(v)
            {
            }

            /// Copy constructor.
            ComparisonPolicy(const ComparisonPolicy & other) :
                _v(other._v)
            {
            }

            /// Equal operator.
            bool operator== (const OurType_ & other) const
            {
                return (static_cast<const OurType_ *>(this)->*_v)(other);
            }

            /// Not equal operator.
            bool operator!= (const OurType_ & other) const
            {
                return ! (static_cast<const OurType_ *>(this)->*_v)(other);
            }
    };

    /**
     * ComparisonPolicy: specialisation for FullComparisonTag +
     * CompareByMemberTag.
     */
    template <typename OurType_, typename MemberType_>
    class ComparisonPolicy<OurType_, comparison_mode::FullComparisonTag,
        comparison_method::CompareByMemberTag<MemberType_> >
    {
        private:
            const MemberType_ OurType_::* const _v;

        public:
            /// Our comparison mode.
            typedef comparison_mode::FullComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef comparison_method::CompareByMemberTag<MemberType_> ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            /// Constructor.
            ComparisonPolicy(const MemberType_ OurType_::* const v) :
                _v(v)
            {
            }

            /// Copy constructor.
            ComparisonPolicy(const ComparisonPolicy & other) :
                _v(other._v)
            {
            }

define(`make_operator', `
            /// $1 operator.
            bool operator$1 (const OurType_ & other) const
            {
                return static_cast<const OurType_ *>(this)->*_v $1 other.*(
                        (static_cast<const ComparisonPolicyType *>(&other))->_v);
            }
')
make_operator(`==')
make_operator(`!=')
make_operator(`<=')
make_operator(`>=')
make_operator(`<')
make_operator(`>')

    };

    /**
     * ComparisonPolicy: specialisation for FullComparisonTag +
     * CompareByMemberComparisonFunctionTag.
     */
    template <typename OurType_>
    class ComparisonPolicy<OurType_, comparison_mode::FullComparisonTag,
        comparison_method::CompareByMemberComparisonFunctionTag>
    {
        private:
            int (OurType_::* const _v)(const OurType_ &) const;

        public:
            /// Our comparison mode.
            typedef comparison_mode::FullComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef comparison_method::CompareByMemberComparisonFunctionTag ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            /// Constructor.
            ComparisonPolicy(int (OurType_::* v)(const OurType_ &) const) :
                _v(v)
            {
            }

            /// Copy constructor.
            ComparisonPolicy(const ComparisonPolicy & other) :
                _v(other._v)
            {
            }

define(`make_operator', `
            /// $1 operator.
            bool operator$1 (const OurType_ & other) const
            {
                return (static_cast<const OurType_ *>(this)->*_v)(other) $1 0;
            }
')
make_operator(`==')
make_operator(`!=')
make_operator(`<=')
make_operator(`>=')
make_operator(`<')
make_operator(`>')

    };

    /**
     * ComparisonPolicy: specialisation for EqualityComparisonTag +
     * CompareByMemberFetchFunctionTag.
     */
    template <typename OurType_, typename MemberType_>
    class ComparisonPolicy<OurType_, comparison_mode::EqualityComparisonTag,
        comparison_method::CompareByMemberFetchFunctionTag<MemberType_> >
    {
        private:
            MemberType_ (OurType_::* const _v)() const;

        public:
            /// Our comparison mode.
            typedef comparison_mode::EqualityComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef comparison_method::CompareByMemberFetchFunctionTag<MemberType_> ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            /// Constructor.
            ComparisonPolicy(MemberType_ (OurType_::* const v)() const) :
                _v(v)
            {
            }

            /// Copy constructor.
            ComparisonPolicy(const ComparisonPolicy & other) :
                _v(other._v)
            {
            }

define(`make_operator', `
            /// $1 operator.
            bool operator$1 (const OurType_ & other) const
            {
                return (static_cast<const OurType_ *>(this)->*_v)() $1 (other.*other._v)();
            }
')
make_operator(`==')
make_operator(`!=')
    };

    /**
     * ComparisonPolicy: specialisation for FullComparisonTag +
     * CompareByMemberFetchFunctionTag.
     */
    template <typename OurType_, typename MemberType_>
    class ComparisonPolicy<OurType_, comparison_mode::FullComparisonTag,
          comparison_method::CompareByMemberFetchFunctionTag<MemberType_> >
    {
        private:
            MemberType_ (OurType_::* const _v)() const;

        public:
            /// Our comparison mode.
            typedef comparison_mode::FullComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef comparison_method::CompareByMemberFetchFunctionTag<MemberType_> ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            /// Constructor.
            ComparisonPolicy(MemberType_ (OurType_::* const v)() const) :
                _v(v)
            {
            }

            /// Copy constructor.
            ComparisonPolicy(const ComparisonPolicy & other) :
                _v(other._v)
            {
            }

define(`make_operator', `
            /// $1 operator.
            bool operator$1 (const OurType_ & other) const
            {
                return ((static_cast<const OurType_ *>(this)->*_v)()) $1
                    ((other.*(static_cast<const OurType_ *>(&other)->_v))());
            }
')
make_operator(`==')
make_operator(`!=')
make_operator(`>=')
make_operator(`<=')
make_operator(`>')
make_operator(`<')

    };
}

#endif
