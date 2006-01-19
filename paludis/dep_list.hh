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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_HH 1

#include <paludis/instantiation_policy.hh>
#include <paludis/private_implementation_pattern.hh>
#include <paludis/dep_atom.hh>
#include <paludis/environment.hh>
#include <paludis/dep_atom_visitor.hh>
#include <paludis/dep_list_entry.hh>
#include <list>

namespace paludis
{
    /**
     * Used in DepList::set_rdepend_post
     */
    enum DepListRdependOption 
    { 
        /**
         * RDEPENDs are always merged before the package; abort if this fails.
         */
        dlro_never,

        /**
         * RDEPENDs can be merged after the package, just before PDEPEND, if this is
         * necessary for correct resolution
         */
        dlro_as_needed,

        /**
         * RDEPENDs are always merged with PDEPENDs.
         */
        dlro_always
    };

    /**
     * Holds a list of dependencies in merge order.
     */
    class DepList :
        private InstantiationPolicy<DepList, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<DepList>,
        protected DepAtomVisitorTypes::ConstVisitor
    {
        private:
            void _add_in_role(DepAtom::ConstPointer, const std::string & role);

        protected:
            void visit(const PackageDepAtom * const);
            void visit(const UseDepAtom * const);
            void visit(const AnyDepAtom * const);
            void visit(const BlockDepAtom * const);
            void visit(const AllDepAtom * const);

        public:
            /**
             * Iterator for our children.
             */
            typedef std::list<DepListEntry>::const_iterator Iterator;

            /**
             * Constructor.
             */
            DepList(const Environment * const);

            /**
             * Destructor.
             */
            virtual ~DepList();

            /**
             * Add the packages required to resolve an additional dependency
             * atom.
             */
            void add(DepAtom::ConstPointer);

            /**
             * Start of our dependency list.
             */
            Iterator begin() const;

            /**
             * One past the end of our dependency list.
             */
            Iterator end() const;

            /**
             * Behaviour: determines when RDEPEND entries can be treated as PDEPEND.
             */
            void set_rdepend_post(const DepListRdependOption value);

            /**
             * Behaviour: if set, a package that depends directly upon itself
             * will be accepted.
             */
            void set_drop_self_circular(const bool value);

            /**
             * Behaviour: if set, any circular dependencies are treated as if
             * they do not exist.
             */
            void set_drop_circular(const bool value);

            /**
             * Behaviour: ignore installed packages.
             */
            void set_ignore_installed(const bool value);

            /**
             * Behaviour: check nth level dependencies for packages that are
             * already installed.
             */
            void set_recursive_deps(const bool value);

            /**
             * Behaviour: set the maximum stack depth.
             */
            void set_max_stack_depth(const int value);
    };
}

#endif
