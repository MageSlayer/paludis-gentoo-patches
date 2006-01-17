/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_PALUDIS_MASK_REASONS_HH
#define PALUDIS_GUARD_PALUDIS_MASK_REASONS_HH 1

#include <bitset>

namespace paludis
{
    /**
     * Each value represents one reason for a package being
     * masked.
     */
    enum MaskReason
    {
        mr_keyword,           ///< no keyword match
        mr_user_mask,         ///< user package.mask
        mr_profile_mask,      ///< profile package.mask
        mr_repository_mask,   ///< repository package.mask
        mr_eapi,              ///< unknown eapi
        last_mr               ///< number of entries
    };

    /**
     * A collection of reasons for why a package is masked.
     */
    typedef std::bitset<last_mr> MaskReasons;
}

#endif
