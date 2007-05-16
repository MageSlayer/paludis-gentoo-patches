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

#ifndef PALUDIS_GUARD_PALUDIS_COMPARISON_POLICY_HH
#define PALUDIS_GUARD_PALUDIS_COMPARISON_POLICY_HH 1

/** \file
 * Declarations for the ComparisonPolicy class.
 *
 * \ingroup grpcompare
 */

#include <paludis/util/comparison_policy-fwd.hh>

namespace paludis
{
    /**
     * Comparison modes for paludis::ComparisonPolicy.
     *
     * \ingroup grpcompare
     */
    namespace comparison_mode
    {
        /**
         * No comparisons can be made.
         *
         * \ingroup grpcompare
         */
        struct NoComparisonTag;

        /**
         * Comparisons can be made via operator== and operator!=.
         *
         * \ingroup grpcompare
         */
        struct EqualityComparisonTag;

        /**
         * The full range of comparison operators is available.
         *
         * \ingroup grpcompare
         */
        struct FullComparisonTag;
    }

    /**
     * Comparison methods for paludis::ComparisonPolicy.
     *
     * \ingroup grpcompare
     */
    namespace comparison_method
    {
        /**
         * Comparisons are done via a member of type MemberType_.
         *
         * \ingroup grpcompare
         */
        template <typename MemberType_>
        struct CompareByMemberTag;

        /**
         * Comparisons are done by a member function that returns an integer
         * less than zero (less than), equal to zero (equal to) or greater than
         * zero (greater than).
         *
         * \ingroup grpcompare
         */
        struct CompareByMemberComparisonFunctionTag;

        /**
         * Comparisons are done via a member function that returns an item of
         * type MemberType_.
         *
         * \ingroup grpcompare
         */
        template <typename MemberType_>
        struct CompareByMemberFetchFunctionTag;
    }

    template <typename OurType_, typename ComparisonModeTag_, typename ComparisonMethodTag_>
    struct ComparisonPolicy;

    /**
     * ComparisonPolicy: specialisation for NoComparisonTag.
     *
     * \ingroup grpcompare
     * \nosubgrouping
     */
    template <typename OurType_, typename ComparisonMethodTag_>
    class ComparisonPolicy<OurType_, comparison_mode::NoComparisonTag, ComparisonMethodTag_>
    {
        public:
            ///\name Comparison policy tags
            ///\{

            /// Our comparison mode.
            typedef comparison_mode::NoComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef ComparisonMethodTag_ ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            ///\}
    };

    /**
     * ComparisonPolicy: specialisation for EqualityComparisonTag +
     * CompareByMemberTag.
     *
     * \ingroup grpcompare
     * \nosubgrouping
     */
    template <typename OurType_, typename MemberType_>
    class ComparisonPolicy<OurType_, comparison_mode::EqualityComparisonTag,
          comparison_method::CompareByMemberTag<MemberType_> >
    {
        private:
            const MemberType_ OurType_::* const _v;

        public:
            ///\name Comparison policy tags
            ///\{

            /// Our comparison mode.
            typedef comparison_mode::EqualityComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef comparison_method::CompareByMemberTag<MemberType_> ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            ///\}

            ///\name Basic operations
            ///\{

            ComparisonPolicy(const MemberType_ OurType_::* const v) :
                _v(v)
            {
            }

            ComparisonPolicy(const ComparisonPolicy & other) :
                _v(other._v)
            {
            }

            ///\}

            ///\name Comparison operators
            ///\{

#undef PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR
#define PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(op) \
            bool operator op (const OurType_ & other) const \
            { \
                return static_cast<const OurType_ *>(this)->*_v op other.*( \
                        (static_cast<const ComparisonPolicyType *>(&other))->_v); \
            }

            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(==)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(!=)

            ///\}

    };

    /**
     * ComparisonPolicy: specialisation for EqualityComparisonTag +
     * CompareByMemberComparisonFunctionTag.
     *
     * \ingroup grpcompare
     * \nosubgrouping
     */
    template <typename OurType_>
    class ComparisonPolicy<OurType_, comparison_mode::EqualityComparisonTag,
        comparison_method::CompareByMemberComparisonFunctionTag>
    {
        private:
            bool (OurType_::* const _v)(const OurType_ &) const;

        public:
            ///\name Comparison policy tags
            ///\{

            /// Our comparison mode.
            typedef comparison_mode::EqualityComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef comparison_method::CompareByMemberComparisonFunctionTag ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            ///\}

            ///\name Basic operations
            ///\{

            ComparisonPolicy(bool (OurType_::* const v)(const OurType_ &) const) :
                _v(v)
            {
            }

            ComparisonPolicy(const ComparisonPolicy & other) :
                _v(other._v)
            {
            }

            ///\}

            ///\name Comparison operators
            ///\{

            bool operator== (const OurType_ & other) const
            {
                return (static_cast<const OurType_ *>(this)->*_v)(other);
            }

            bool operator!= (const OurType_ & other) const
            {
                return ! (static_cast<const OurType_ *>(this)->*_v)(other);
            }

            ///\}
    };

    /**
     * ComparisonPolicy: specialisation for FullComparisonTag +
     * CompareByMemberTag.
     *
     * \ingroup grpcompare
     * \nosubgrouping
     */
    template <typename OurType_, typename MemberType_>
    class ComparisonPolicy<OurType_, comparison_mode::FullComparisonTag,
        comparison_method::CompareByMemberTag<MemberType_> >
    {
        private:
            const MemberType_ OurType_::* const _v;

        public:
            ///\name Comparison policy tags
            ///\{

            /// Our comparison mode.
            typedef comparison_mode::FullComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef comparison_method::CompareByMemberTag<MemberType_> ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            ///\}

            ///\name Basic operations
            ///\{

            ComparisonPolicy(const MemberType_ OurType_::* const v) :
                _v(v)
            {
            }

            ComparisonPolicy(const ComparisonPolicy & other) :
                _v(other._v)
            {
            }

            ///\}

            ///\name Comparison operators
            ///\{

#undef PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR
#define PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(op) \
            bool operator op (const OurType_ & other) const \
            { \
                return static_cast<const OurType_ *>(this)->*_v op other.*( \
                        (static_cast<const ComparisonPolicyType *>(&other))->_v); \
            }

            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(==)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(!=)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(<=)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(>=)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(<)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(>)

            ///\}
    };

    /**
     * ComparisonPolicy: specialisation for FullComparisonTag +
     * CompareByMemberComparisonFunctionTag.
     *
     * \ingroup grpcompare
     * \nosubgrouping
     */
    template <typename OurType_>
    class ComparisonPolicy<OurType_, comparison_mode::FullComparisonTag,
        comparison_method::CompareByMemberComparisonFunctionTag>
    {
        private:
            int (OurType_::* const _v)(const OurType_ &) const;

        public:
            ///\name Comparison policy tags
            //\{

            /// Our comparison mode.
            typedef comparison_mode::FullComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef comparison_method::CompareByMemberComparisonFunctionTag ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            ///\}

            ///\name Basic operations
            ///\{

            ComparisonPolicy(int (OurType_::* v)(const OurType_ &) const) :
                _v(v)
            {
            }

            ComparisonPolicy(const ComparisonPolicy & other) :
                _v(other._v)
            {
            }

            ///\}

            ///\name Comparison operators
            ///\{

#undef PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR
#define PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(op) \
            bool operator op (const OurType_ & other) const \
            { \
                return (static_cast<const OurType_ *>(this)->*_v)(other) op 0; \
            }

            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(==)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(!=)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(<=)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(>=)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(<)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(>)

            ///\}
    };

    /**
     * ComparisonPolicy: specialisation for EqualityComparisonTag +
     * CompareByMemberFetchFunctionTag.
     *
     * \ingroup grpcompare
     * \nosubgrouping
     */
    template <typename OurType_, typename MemberType_>
    class ComparisonPolicy<OurType_, comparison_mode::EqualityComparisonTag,
        comparison_method::CompareByMemberFetchFunctionTag<MemberType_> >
    {
        private:
            MemberType_ (OurType_::* const _v)() const;

        public:
            ///\name Comparison policy tags
            ///\{

            /// Our comparison mode.
            typedef comparison_mode::EqualityComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef comparison_method::CompareByMemberFetchFunctionTag<MemberType_> ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            ///\}

            ///\name Basic operations
            ///\{

            ComparisonPolicy(MemberType_ (OurType_::* const v)() const) :
                _v(v)
            {
            }

            ComparisonPolicy(const ComparisonPolicy & other) :
                _v(other._v)
            {
            }

            ///\}

            ///\name Comparison operators
            ///\{

#undef PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR
#define PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(op) \
            bool operator op (const OurType_ & other) const \
            { \
                return (static_cast<const OurType_ *>(this)->*_v)() op (other.*other._v)(); \
            }

            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(==)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(!=)

            ///\}
    };

    /**
     * ComparisonPolicy: specialisation for FullComparisonTag +
     * CompareByMemberFetchFunctionTag.
     *
     * \ingroup grpcompare
     * \nosubgrouping
     */
    template <typename OurType_, typename MemberType_>
    class ComparisonPolicy<OurType_, comparison_mode::FullComparisonTag,
          comparison_method::CompareByMemberFetchFunctionTag<MemberType_> >
    {
        private:
            MemberType_ (OurType_::* const _v)() const;

        public:
            ///\name Comparison policy tags
            ///\{

            /// Our comparison mode.
            typedef comparison_mode::FullComparisonTag ComparisonPolicyModeTag;

            /// Our comparison method.
            typedef comparison_method::CompareByMemberFetchFunctionTag<MemberType_> ComparisonPolicyMethodTag;

            /// Our comparison policy.
            typedef ComparisonPolicy<OurType_, ComparisonPolicyModeTag, ComparisonPolicyMethodTag> ComparisonPolicyType;

            ///\}

            ///\name Basic operations
            ///\{

            ComparisonPolicy(MemberType_ (OurType_::* const v)() const) :
                _v(v)
            {
            }

            ComparisonPolicy(const ComparisonPolicy & other) :
                _v(other._v)
            {
            }

            ///\}

            ///\name Comparison operators
            ///\{

#undef PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR
#define PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(op) \
            bool operator op (const OurType_ & other) const \
            { \
                return ((static_cast<const OurType_ *>(this)->*_v)()) op \
                    ((other.*(static_cast<const OurType_ *>(&other)->_v))()); \
            }

            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(==)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(!=)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(<=)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(>=)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(<)
            PALUDIS_COMPARISON_POLICY_MAKE_OPERATOR(>)

            ///\}
    };
}

#endif

