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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_ATOM_FLATTENER_HH
#define PALUDIS_GUARD_PALUDIS_DEP_ATOM_FLATTENER_HH 1

#include <paludis/dep_atom.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>

#include <libwrapiter/libwrapiter.hh>

/** \file
 * Declarations for DepAtomFlattener.
 *
 * \ingroup grpdepatomflattener
 */

namespace paludis
{
    /**
     * Extract the enabled components of a dep heirarchy for a particular
     * package.
     *
     * This is useful for picking out SRC_URI, PROVIDE etc components. It is
     * <b>not</b> suitable for heirarchies that can contain || ( ) blocks.
     *
     * \ingroup grpdepatomflattener
     */
    class DepAtomFlattener :
        private InstantiationPolicy<DepAtomFlattener, instantiation_method::NonCopyableTag>,
        protected DepAtomVisitorTypes::ConstVisitor,
        private PrivateImplementationPattern<DepAtomFlattener>
    {
        protected:
            ///\name Visit methods
            ///{
            void visit(const AllDepAtom *);
            void visit(const AnyDepAtom *) PALUDIS_ATTRIBUTE((noreturn));
            void visit(const UseDepAtom *);
            void visit(const PlainTextDepAtom *);
            void visit(const PackageDepAtom *);
            void visit(const BlockDepAtom *);
            ///}

        public:
            ///\name Basic operations
            ///\{

            DepAtomFlattener(const Environment * const,
                    const PackageDatabaseEntry * const,
                    const DepAtom::ConstPointer);

            ~DepAtomFlattener();

            ///\}

            ///\name Iterate over our dep atoms
            ///{

            typedef libwrapiter::ForwardIterator<DepAtomFlattener, const StringDepAtom *> Iterator;

            Iterator begin();

            Iterator end() const;

            ///\}
    };
}

#endif
