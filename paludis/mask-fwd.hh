/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_MASK_FWD_HH
#define PALUDIS_GUARD_PALUDIS_MASK_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <iosfwd>

/** \file
 * Forward declarations for paludis/mask.hh .
 *
 * \ingroup g_mask
 */

namespace paludis
{
    class Mask;
    class UserMask;
    class UnacceptedMask;
    class RepositoryMask;
    class UnsupportedMask;
    class AssociationMask;

    struct OverriddenMask;

#include <paludis/mask-se.hh>

    /**
     * Fetch the token associated with a Mask, or "" if there is
     * no such token.
     *
     * \since 0.60
     * \ingroup g_mask
     */
    const std::string get_mask_token(const Mask &) PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));
}

#endif
