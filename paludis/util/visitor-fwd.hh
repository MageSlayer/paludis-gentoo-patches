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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_VISITOR_FWD_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_VISITOR_FWD_HH 1

namespace paludis
{
    template <typename NodePtrType_>
    class Visits;

    namespace visitor_internals
    {
        template <unsigned n_>
        struct NoType;

        template <typename>
        struct MakePointerToConst;

        template <typename T_>
        struct MakePointerToConst<T_ *>;

        template <typename NodePtrType_>
        class Visits;

        template <unsigned n_>
        class Visits<const visitor_internals::NoType<n_> * >;

        template <unsigned n_>
        class Visits<visitor_internals::NoType<n_> * >;
    }

    template <typename VisitorType_>
    class VisitableInterface;

    template <typename OurType_, typename VisitorType_>
    class Visitable;

    template <
        typename N1_,
        typename N2_ = visitor_internals::NoType<2> *,
        typename N3_ = visitor_internals::NoType<3> *,
        typename N4_ = visitor_internals::NoType<4> *,
        typename N5_ = visitor_internals::NoType<5> *,
        typename N6_ = visitor_internals::NoType<6> *,
        typename N7_ = visitor_internals::NoType<7> *,
        typename N8_ = visitor_internals::NoType<8> *,
        typename N9_ = visitor_internals::NoType<9> *>
    class VisitorTypes;

    template <typename VisitorPointer_>
    class AcceptVisitor;

    template <typename VisitorPointer_>
    AcceptVisitor<VisitorPointer_> accept_visitor(VisitorPointer_ * const p);
}


#endif
