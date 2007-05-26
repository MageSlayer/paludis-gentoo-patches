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
    namespace visitor_internals
    {
        template <unsigned u_>
        class NoType;

        template <typename H_>
        class ConstAcceptInterface;

        template <typename H_>
        class MutableAcceptInterface;

        template <typename H_, typename T_>
        class ConstAcceptInterfaceVisitsThis;

        template <typename H_, typename T_>
        class MutableAcceptInterfaceVisitsThis;

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
        class MutableProxyVisitor;

        template <typename H_, typename LargerH_>
        class MutableProxyIterator;

        template <typename H_>
        class ConstVisitor;

        template <typename H_>
        class MutableVisitor;

        template <
            typename Heirarchy_,
            typename BasicNode_,
            typename ContainedItem1_ = visitor_internals::NoType<1>,
            typename ContainedItem2_ = visitor_internals::NoType<2>,
            typename ContainedItem3_ = visitor_internals::NoType<3>,
            typename ContainedItem4_ = visitor_internals::NoType<4>,
            typename ContainedItem5_ = visitor_internals::NoType<5>,
            typename ContainedItem6_ = visitor_internals::NoType<6>,
            typename ContainedItem7_ = visitor_internals::NoType<7>,
            typename ContainedItem8_ = visitor_internals::NoType<8>,
            typename ContainedItem9_ = visitor_internals::NoType<9> >
        class VisitorTypes;

        template <typename I_>
        const typename I_::Heirarchy::BasicNode *
        get_const_item(const I_ & i);
    }

    using visitor_internals::TreeSequence;
    using visitor_internals::ConstTreeSequence;
    using visitor_internals::TreeLeaf;
    using visitor_internals::VisitorTypes;
    using visitor_internals::MutableVisitor;
    using visitor_internals::ConstVisitor;
    using visitor_internals::Visits;
    using visitor_internals::ConstAcceptInterface;
    using visitor_internals::MutableAcceptInterface;
    using visitor_internals::ConstAcceptInterfaceVisitsThis;
    using visitor_internals::MutableAcceptInterfaceVisitsThis;
    using visitor_internals::get_const_item;

    template <typename Visitor_>
    class AcceptVisitor;

    template <typename Visitor_>
    AcceptVisitor<Visitor_> accept_visitor(Visitor_ &);
}

#endif
