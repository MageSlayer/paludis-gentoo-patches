/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/util/visitor-fwd.hh>
#include <paludis/util/kc-fwd.hh>
#include <paludis/util/keys.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <tr1/memory>

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

    class MaskVisitorTypes;

    /**
     * Information about a RepositoryMask.
     *
     * The mask_file key holds the file whence the mask originates.
     *
     * The comment key is a sequence of lines explaining the mask.
     *
     * \ingroup g_package_id
     * \since 0.26
     * \nosubgrouping
     */
    typedef kc::KeyedClass<
        kc::Field<k::mask_file, FSEntry>,
        kc::Field<k::comment, std::tr1::shared_ptr<const Sequence<std::string> > >
            > RepositoryMaskInfo;
}

#endif
