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

#ifndef PALUDIS_GUARD_PALUDIS_COMPOSITE_VISITOR_PATTERN_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_COMPOSITE_VISITOR_PATTERN_IMPL_HH 1

#include <paludis/visitor_pattern-impl.hh>

namespace paludis
{
    template <typename OurType_, typename CNodeType_>
    void
    VisitsComposite<OurType_, CNodeType_>::visit(const CNodeType_ * const n)
    {
        enter(n);

        typename CNodeType_::Iterator i(n->begin()), e(n->end());
        for ( ; i != e ; ++i)
            (*i)->accept(static_cast<OurType_ *>(this));

        leave(n);
    }
}

#endif
