/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_RANGE_REWRITER_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_RANGE_REWRITER_HH 1

#include <paludis/dep_atom.hh>

namespace paludis
{
    class RangeRewriter :
        public DepAtomVisitorTypes::ConstVisitor
    {
        private:
            PackageDepAtom::Pointer _atom;
            bool _invalid;

        public:
            RangeRewriter();
            virtual ~RangeRewriter();

            PackageDepAtom::ConstPointer atom() const
            {
                if (_invalid)
                    return PackageDepAtom::ConstPointer(0);

                return _atom;
            }

            void visit(const AllDepAtom *);
            void visit(const AnyDepAtom *);
            void visit(const UseDepAtom *);
            void visit(const PlainTextDepAtom *);
            void visit(const PackageDepAtom *);
            void visit(const BlockDepAtom *);
    };
}

#endif
