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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_COMPARISON_POLICY_FWD_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_COMPARISON_POLICY_FWD_HH 1

namespace paludis
{
    namespace comparison_mode
    {
        struct NoComparisonTag;
        struct EqualityComparisonTag;
        struct FullComparisonTag;
    }

    namespace comparison_method
    {
        template <typename>
        struct CompareByMemberTag;

        struct CompareByMemberComparisonFunctionTag;

        template <typename>
        struct CompareByMemberFetchFunctionTag;
    }

    template <typename OurType_, typename ComparisonModeTag_, typename ComparisonMethodTag_>
    struct ComparisonPolicy;

    template <typename OurType_, typename ComparisonMethodTag_>
    class ComparisonPolicy<OurType_, comparison_mode::NoComparisonTag, ComparisonMethodTag_>;

    template <typename OurType_, typename MemberType_>
    class ComparisonPolicy<OurType_, comparison_mode::EqualityComparisonTag,
          comparison_method::CompareByMemberTag<MemberType_> >;

    template <typename OurType_>
    class ComparisonPolicy<OurType_, comparison_mode::EqualityComparisonTag,
        comparison_method::CompareByMemberComparisonFunctionTag>;

    template <typename OurType_, typename MemberType_>
    class ComparisonPolicy<OurType_, comparison_mode::FullComparisonTag,
        comparison_method::CompareByMemberTag<MemberType_> >;

    template <typename OurType_>
    class ComparisonPolicy<OurType_, comparison_mode::FullComparisonTag,
        comparison_method::CompareByMemberComparisonFunctionTag>;

    template <typename OurType_, typename MemberType_>
    class ComparisonPolicy<OurType_, comparison_mode::EqualityComparisonTag,
        comparison_method::CompareByMemberFetchFunctionTag<MemberType_> >;

    template <typename OurType_, typename MemberType_>
    class ComparisonPolicy<OurType_, comparison_mode::FullComparisonTag,
          comparison_method::CompareByMemberFetchFunctionTag<MemberType_> >;
}

#endif
