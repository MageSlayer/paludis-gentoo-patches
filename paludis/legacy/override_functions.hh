/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_OVERRIDE_FUNCTIONS_HH
#define PALUDIS_GUARD_PALUDIS_OVERRIDE_FUNCTIONS_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/mask-fwd.hh>

/** \file
 * Declarations for various override functions for DepList.
 *
 * \ingroup g_dep_list
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Override a mask if it is a ~keyword mask, and keyword is accepted.
     *
     * \ingroup g_dep_list
     * \since 0.26
     */
    bool override_tilde_keywords(const Environment * const e, const std::shared_ptr<const PackageID> & i, const Mask & m)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    /**
     * Override a mask if it is due to a missing keyword.
     *
     * \ingroup g_dep_list
     * \since 0.26
     */
    bool override_unkeyworded(const Environment * const e, const std::shared_ptr<const PackageID> & i, const Mask & m)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    /**
     * Override a mask if it is a repository mask.
     *
     * \ingroup g_dep_list
     * \since 0.26
     */
    bool override_repository_masks(const Mask & m)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    /**
     * Override a mask if it is a license mask.
     *
     * \ingroup g_dep_list
     * \since 0.26
     */
    bool override_license(const Mask & m)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;
}

#endif
