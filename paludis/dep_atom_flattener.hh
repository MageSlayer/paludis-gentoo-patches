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

#include <list>
#include <paludis/dep_atom.hh>
#include <paludis/package_database.hh>
#include <paludis/qa/environment.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>

namespace paludis
{
    /**
     * Extract the enabled components of a dep heirarchy for a particular
     * package.
     */
    class DepAtomFlattener :
        private InstantiationPolicy<DepAtomFlattener, instantiation_method::NonCopyableTag>,
        protected DepAtomVisitorTypes::ConstVisitor
    {
        private:
            const Environment * const _env;

            const PackageDatabaseEntry * const _pkg;

            DepAtom::ConstPointer _a;

            mutable std::list<const StringDepAtom *> _atoms;

            mutable bool _done;

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
            /**
             * Constructor.
             */
            DepAtomFlattener(const Environment * const,
                    const PackageDatabaseEntry * const,
                    const DepAtom::ConstPointer);

            /**
             * Destructor.
             */
            ~DepAtomFlattener();

            /**
             * Iterate over our dep atoms.
             */
            typedef std::list<const StringDepAtom *>::const_iterator Iterator;

            /**
             * Iterator to the start of our dep atoms.
             */
            Iterator begin();

            /**
             * Iterator to past the end of our dep atoms.
             */
            Iterator end() const
            {
                return _atoms.end();
            }
    };
}

#endif
