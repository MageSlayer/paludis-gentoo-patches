/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#include <paludis/util/no_type.hh>

/** \file
 * Forward declarations for paludis/util/visitor.hh .
 *
 * \ingroup g_visitors
 */

namespace paludis
{
    namespace visitor_internals
    {
        template <typename H_>
        class ConstAcceptInterface;

        template <typename H_>
        class AcceptInterface;

        template <typename H_, typename T_>
        class ConstAcceptInterfaceVisitsThis;

        template <typename H_, typename T_>
        class AcceptInterfaceVisitsThis;

        template <typename H_, typename I_>
        class TreeLeaf;

        template <typename H_, typename I_>
        class TreeSequence;

        template <typename H_, typename I_>
        class ConstTreeSequence;

        template <typename T_>
        class Visits;

        template <unsigned u_>
        class Visits<NoType<u_> >;

        template <unsigned u_>
        class Visits<const NoType<u_> >;

        template <typename H_, typename T_>
        class Visits<TreeLeaf<H_, T_> >;

        template <typename H_, typename T_>
        class Visits<const TreeLeaf<H_, T_> >;

        template <typename H_, typename T_>
        class Visits<TreeSequence<H_, T_> >;

        template <typename H_, typename T_>
        class Visits<const TreeSequence<H_, T_> >;

        template <typename H_, typename T_>
        class Visits<const ConstTreeSequence<H_, T_> >;

        template <typename H_>
        class TreeSequenceIteratorTypes;

        template <typename H_, typename LargerH_, typename T_>
        class ProxyVisits;

        template <typename H_, typename LargerH_, unsigned u_>
        class ProxyVisits<H_, LargerH_, NoType<u_> >;

        template <typename H_, typename LargerH_, unsigned u_>
        class ProxyVisits<H_, LargerH_, const NoType<u_> >;

        template <typename H_, typename LargerH_, typename T_>
        class ProxyVisits<H_, LargerH_, TreeLeaf<H_, T_> >;

        template <typename H_, typename LargerH_, typename T_>
        class ProxyVisits<H_, LargerH_, const TreeLeaf<H_, T_> >;

        template <typename H_, typename LargerH_, typename T_>
        class ProxyVisits<H_, LargerH_, TreeSequence<H_, T_> >;

        template <typename H_, typename LargerH_, typename T_>
        class ProxyVisits<H_, LargerH_, const TreeSequence<H_, T_> >;

        template <typename H_, typename LargerH_, typename T_>
        class ProxyVisits<H_, LargerH_, const ConstTreeSequence<H_, T_> >;

        template <typename H_, typename LargerH_>
        class ConstProxyVisitor;

        template <typename H_, typename LargerH_>
        class ConstProxyIterator;

        template <typename H_, typename LargerH_>
        class ProxyVisitor;

        template <typename H_, typename LargerH_>
        class ProxyIterator;

        template <typename H_>
        class ConstVisitor;

        template <typename H_>
        class Visitor;

        template <
            typename Heirarchy_,
            typename BasicNode_,
            typename ContainedItem1_ = NoType<1>,
            typename ContainedItem2_ = NoType<2>,
            typename ContainedItem3_ = NoType<3>,
            typename ContainedItem4_ = NoType<4>,
            typename ContainedItem5_ = NoType<5>,
            typename ContainedItem6_ = NoType<6>,
            typename ContainedItem7_ = NoType<7>,
            typename ContainedItem8_ = NoType<8>,
            typename ContainedItem9_ = NoType<9>,
            typename ContainedItem10_ = NoType<10>,
            typename ContainedItem11_ = NoType<11>,
            typename ContainedItem12_ = NoType<12>,
            typename ContainedItem13_ = NoType<13>,
            typename ContainedItem14_ = NoType<14>,
            typename ContainedItem15_ = NoType<15>,
            typename ContainedItem16_ = NoType<16>,
            typename ContainedItem17_ = NoType<17>,
            typename ContainedItem18_ = NoType<18>,
            typename ContainedItem19_ = NoType<19>,
            typename ContainedItem20_ = NoType<20>,
            typename ContainedItem21_ = NoType<21>,
            typename ContainedItem22_ = NoType<22>,
            typename ContainedItem23_ = NoType<23>,
            typename ContainedItem24_ = NoType<24>,
            typename ContainedItem25_ = NoType<25>,
            typename ContainedItem26_ = NoType<26>,
            typename ContainedItem27_ = NoType<27>,
            typename ContainedItem28_ = NoType<28>,
            typename ContainedItem29_ = NoType<29>,
            typename ContainedItem30_ = NoType<30> >
        class VisitorTypes;

        template <typename I_>
        const typename I_::Heirarchy::BasicNode *
        get_const_item(const I_ & i);
    }

    using visitor_internals::TreeSequence;
    using visitor_internals::ConstTreeSequence;
    using visitor_internals::TreeLeaf;
    using visitor_internals::VisitorTypes;
    using visitor_internals::Visitor;
    using visitor_internals::ConstVisitor;
    using visitor_internals::Visits;
    using visitor_internals::ConstAcceptInterface;
    using visitor_internals::AcceptInterface;
    using visitor_internals::ConstAcceptInterfaceVisitsThis;
    using visitor_internals::AcceptInterfaceVisitsThis;
    using visitor_internals::get_const_item;

    template <typename Visitor_>
    class AcceptVisitor;

    template <typename Visitor_>
    AcceptVisitor<Visitor_> accept_visitor(Visitor_ &);
}

#endif
