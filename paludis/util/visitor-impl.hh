/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_VISITOR_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_VISITOR_IMPL_HH 1

#include <algorithm>
#include <paludis/util/visitor.hh>

template <
    typename N1_,
    typename N2_,
    typename N3_,
    typename N4_,
    typename N5_,
    typename N6_,
    typename N7_,
    typename N8_,
    typename N9_>
template <typename OurType_, typename C1_>
void
paludis::VisitorTypes<N1_, N2_, N3_, N4_, N5_, N6_, N7_, N8_, N9_>::ConstVisitor::VisitChildren<OurType_, C1_>::visit(
        typename paludis::visitor_internals::MakePointerToConst<C1_ *>::Type const c)
{
    std::for_each(c->begin(), c->end(), paludis::accept_visitor(static_cast<OurType_ *>(this)));
}

#endif
