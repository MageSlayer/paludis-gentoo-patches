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

#ifndef PALUDIS_GUARD_PALUDIS_COMPOSITE_DEP_ATOM_HH
#define PALUDIS_GUARD_PALUDIS_COMPOSITE_DEP_ATOM_HH 1

#include <paludis/dep_atom.hh>
#include <paludis/counted_ptr.hh>
#include <deque>

/** \file
 * Declarations for the CompositeDepAtom class.
 *
 * \ingroup DepResolver
 */

namespace paludis
{
    /**
     * Class for dependency atoms that have a number of child dependency
     * atoms.
     *
     * \ingroup DepResolver
     */
    class CompositeDepAtom : public DepAtom,
                             public virtual Composite<DepAtom, CompositeDepAtom>
    {
        private:
            std::deque<DepAtom::ConstPointer> _children;

        protected:
            /**
             * Constructor.
             */
            CompositeDepAtom();

        public:
            /**
             * Append a child to our collection.
             */
            virtual void add_child(DepAtom::ConstPointer);

            /**
             * Iterator for iterating over our children.
             */
            typedef std::deque<DepAtom::ConstPointer>::const_iterator Iterator;

            /**
             * Iterator to the start of our children.
             */
            Iterator begin() const
            {
                return _children.begin();
            }

            /**
             * Iterator to past the end of our children.
             */
            Iterator end() const
            {
                return _children.end();
            }

            typedef CountedPtr<CompositeDepAtom, count_policy::InternalCountTag> Pointer;
            typedef CountedPtr<const CompositeDepAtom, count_policy::InternalCountTag> ConstPointer;
    };
}

#endif
