/* vim: set sw=4 sts=4 et foldmethod=syntax : */

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

#ifndef PALUDIS_GUARD_PALUDIS_BLOCK_DEP_ATOM_HH
#define PALUDIS_GUARD_PALUDIS_BLOCK_DEP_ATOM_HH 1

#include <paludis/dep_atom.hh>
#include <paludis/package_dep_atom.hh>
#include <paludis/dep_atom_visitor.hh>

/** \file
 * Declarations for the BlockDepAtom class.
 *
 * \ingroup DepResolver
 */

namespace paludis
{
    /**
     * A BlockDepAtom represents a block on a package name (for example,
     * 'app-editors/vim'), possibly with associated version and SLOT
     * restrictions.
     *
     * \ingroup DepResolver
     */
    class BlockDepAtom :
        public DepAtom,
        public Visitable<BlockDepAtom, DepAtomVisitorTypes>
    {
        private:
            PackageDepAtom::ConstPointer _atom;

        public:
            /**
             * Constructor, with blocking atom.
             */
            BlockDepAtom(PackageDepAtom::ConstPointer atom);

            /**
             * Fetch the atom we're blocking.
             */
            PackageDepAtom::ConstPointer blocked_atom() const
            {
                return _atom;
            }

            typedef CountedPtr<BlockDepAtom, count_policy::InternalCountTag> Pointer;
            typedef CountedPtr<const BlockDepAtom, count_policy::InternalCountTag> ConstPointer;
    };
}


#endif
