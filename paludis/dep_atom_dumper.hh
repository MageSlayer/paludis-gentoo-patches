/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_ATOM_DUMPER_HH
#define PALUDIS_GUARD_PALUDIS_DEP_ATOM_DUMPER_HH 1

#include <ostream>
#include <paludis/util/instantiation_policy.hh>

/** \file
 * Declarations for the DepAtomDumper class.
 *
 * \ingroup grpdepatomdumper
 */

namespace paludis
{
    /**
     * Dump dependency atoms to a stream in pseudo-XML form, for testing.
     *
     * \ingroup grpdepatomdumper
     */
    class DepAtomDumper :
        public DepAtomVisitorTypes::ConstVisitor,
        private InstantiationPolicy<DepAtomDumper, instantiation_method::NonCopyableTag>
    {
        private:
            std::ostream * const _o;

        public:
            /**
             * Constructor.
             */
            DepAtomDumper(std::ostream * const o);

            /// \name Visit functions
            ///{
            void visit(const AllDepAtom * const);

            void visit(const AnyDepAtom * const);

            void visit(const UseDepAtom * const);

            void visit(const PackageDepAtom * const);

            void visit(const PlainTextDepAtom * const);

            void visit(const BlockDepAtom * const);
            ///}
    };
}

#endif
