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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_ATOM_HH
#define PALUDIS_GUARD_PALUDIS_DEP_ATOM_HH 1

#include <paludis/attributes.hh>
#include <paludis/composite_pattern.hh>
#include <paludis/counted_ptr.hh>
#include <paludis/dep_atom_visitor.hh>
#include <paludis/instantiation_policy.hh>
#include <paludis/visitor.hh>

/** \file
 * Declarations for the DepAtom class.
 *
 * \ingroup DepResolver
 */

namespace paludis
{
    class CompositeDepAtom;
    class UseDepAtom;

    /**
     * Base class for a dependency atom.
     *
     * \ingroup DepResolver
     */
    class DepAtom :
        public virtual VisitableInterface<DepAtomVisitorTypes>,
        public virtual Composite<DepAtom, CompositeDepAtom>,
        private InstantiationPolicy<DepAtom, instantiation_method::NonCopyableTag>,
        public InternalCounted<DepAtom>
    {
        protected:
            DepAtom();

        public:
            virtual ~DepAtom();

            /**
             * Return us as a UseDepAtom, or 0 if we are not a
             * UseDepAtom.
             */
            virtual const UseDepAtom * as_use_dep_atom() const PALUDIS_ATTRIBUTE((pure));
    };
}

#endif
