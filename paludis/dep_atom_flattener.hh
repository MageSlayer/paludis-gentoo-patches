/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#include <paludis/attributes.hh>
#include <paludis/dep_atom.hh>
#include <paludis/dep_atom_visitor.hh>
#include <paludis/environment.hh>
#include <paludis/instantiation_policy.hh>
#include <paludis/package_database.hh>
#include <list>

namespace paludis
{
    class DepAtomFlattener :
        private InstantiationPolicy<DepAtomFlattener, instantiation_method::NonCopyableTag>,
        protected DepAtomVisitorTypes::ConstVisitor
    {
        private:
            const Environment * const _env;

            const PackageDatabaseEntry * const _pkg;

            DepAtom::ConstPointer _a;

            mutable std::list<const PackageDepAtom *> _atoms;

            mutable bool _done;

        protected:
            void visit(const AllDepAtom *);
            void visit(const AnyDepAtom *) PALUDIS_ATTRIBUTE((noreturn));
            void visit(const UseDepAtom *);
            void visit(const BlockDepAtom *) PALUDIS_ATTRIBUTE((noreturn));
            void visit(const PackageDepAtom *);

        public:
            DepAtomFlattener(const Environment * const,
                    const PackageDatabaseEntry * const,
                    const DepAtom::ConstPointer);

            ~DepAtomFlattener();

            typedef std::list<const PackageDepAtom *>::const_iterator Iterator;

            Iterator begin();

            Iterator end() const
            {
                return _atoms.end();
            }
    };
}

#endif
