/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_MASK_REASONS_HH
#define PALUDIS_GUARD_PALUDIS_MASK_REASONS_HH 1

#include <bitset>
#include <iosfwd>

/** \file
 * Declarations for MaskReasons and related classes.
 *
 * \ingroup grpmaskreasons
 */

namespace paludis
{
    /**
     * Each value represents one reason for a package being
     * masked.
     *
     * \see MaskReasons
     *
     * \ingroup grpmaskreasons
     */
    enum MaskReason
    {
        mr_keyword,               ///< no keyword match
        mr_user_mask,             ///< user package.mask
        mr_profile_mask,          ///< profile package.mask
        mr_repository_mask,       ///< repository package.mask
        mr_eapi,                  ///< unknown eapi
        mr_license,               ///< unaccepted license
        mr_licence = mr_license,  ///< convenience alias for those who can spell,
        mr_by_association,        ///< masked by association with another package (for virtuals)
        last_mr                   ///< number of entries
    };

    /**
     * A collection of reasons for why a package is masked.
     *
     * \ingroup grpmaskreasons
     */
    typedef std::bitset<last_mr> MaskReasons;

    /**
     * Stringify a MaskReason.
     *
     * \ingroup grpmaskreasons
     */
    std::ostream &
    operator<< (std::ostream &, const MaskReason &);
}

#endif
