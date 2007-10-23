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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_VISITOR_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_VISITOR_IMPL_HH 1

#include <paludis/util/visitor.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/iterator.hh>

/** \file
 * Implementation for paludis/util/visitor.hh .
 *
 * \ingroup g_visitors
 */

namespace paludis
{
    namespace visitor_internals
    {
        template <typename H_>
        template <bool b_, typename T_>
        void
        ConstAcceptInterface<H_>::ConstAccept<b_, T_>::forward(const ConstAcceptInterface * const h, T_ & t)
        {
            h->real_const_accept(t);
        }

        template <typename H_>
        template <typename T_>
        void
        ConstAcceptInterface<H_>::ConstAccept<false, T_>::forward(const ConstAcceptInterface * const h, T_ & t)
        {
            ConstProxyVisitor<typename H_::Heirarchy, typename T_::Heirarchy> p(&t);
            h->accept(p);
        }

        template <typename H_>
        ConstAcceptInterface<H_>::~ConstAcceptInterface()
        {
        }

        template <typename H_, typename T_>
        void
        ConstAcceptInterfaceVisitsThis<H_, T_>::real_const_accept(ConstVisitor<H_> & v) const
        {
            static_cast<Visits<const T_> *>(&v)->visit(*static_cast<const T_ *>(this));
        }

        template <typename H_>
        template <bool b_, typename T_>
        void
        AcceptInterface<H_>::ConstAccept<b_, T_>::forward(AcceptInterface * const h, T_ & t)
        {
            h->mutable_accept(t);
        }

        template <typename H_>
        template <typename T_>
        void
        AcceptInterface<H_>::ConstAccept<true, T_>::forward(AcceptInterface * const h, T_ & t)
        {
            h->const_accept(t);
        }

        template <typename H_>
        template <bool b_, typename T_>
        void
        AcceptInterface<H_>::Accept<b_, T_>::forward(AcceptInterface * const h, T_ & t)
        {
            h->real_mutable_accept(t);
        }

        template <typename H_>
        template <typename T_>
        void
        AcceptInterface<H_>::Accept<false, T_>::forward(AcceptInterface * const h, T_ & t)
        {
            ProxyVisitor<typename H_::Heirarchy, typename T_::Heirarchy> p(&t);
            h->accept(p);
        }

        template <typename H_>
        AcceptInterface<H_>::~AcceptInterface()
        {
        }

        template <typename H_, typename T_>
        void
        AcceptInterfaceVisitsThis<H_, T_>::real_const_accept(ConstVisitor<H_> & v) const
        {
            static_cast<Visits<const T_> *>(&v)->visit(*static_cast<const T_ *>(this));
        }

        template <typename H_, typename T_>
        void
        AcceptInterfaceVisitsThis<H_, T_>::real_mutable_accept(Visitor<H_> & v)
        {
            static_cast<Visits<T_> *>(&v)->visit(*static_cast<T_ *>(this));
        }

        template <typename H_, typename T_>
        void
        TreeLeaf<H_, T_>::real_mutable_accept(Visitor<H_> & v)
        {
            static_cast<Visits<TreeLeaf<H_, T_> > *>(&v)->visit(*this);
        }

        template <typename H_, typename T_>
        void
        TreeLeaf<H_, T_>::real_const_accept(ConstVisitor<H_> & v) const
        {
            static_cast<Visits<const TreeLeaf<H_, T_> > *>(&v)->visit(*this);
        }

        template <typename H_, typename T_>
        TreeLeaf<H_, T_>::~TreeLeaf()
        {
        }

        template <typename H_, typename T_>
        TreeLeaf<H_, T_>::TreeLeaf(const tr1::shared_ptr<T_> & i) :
            _item(i)
        {
        }

        template <typename H_, typename T_>
        tr1::shared_ptr<T_>
        TreeLeaf<H_, T_>::item()
        {
            return _item;
        }

        template <typename H_, typename T_>
        tr1::shared_ptr<const T_>
        TreeLeaf<H_, T_>::item() const
        {
            return _item;
        }

        template <typename H_, typename T_>
        void
        TreeSequence<H_, T_>::real_mutable_accept(Visitor<H_> & v)
        {
            static_cast<Visits<TreeSequence<H_, T_> > *>(&v)->visit(*this);
        }

        template <typename H_, typename T_>
        void
        TreeSequence<H_, T_>::real_const_accept(ConstVisitor<H_> & v) const
        {
            static_cast<Visits<const TreeSequence<H_, T_> > *>(&v)->visit(*this);
        }

        template <typename H_, typename T_>
        TreeSequence<H_, T_>::~TreeSequence()
        {
        }

        template <typename H_, typename T_>
        TreeSequence<H_, T_>::TreeSequence(tr1::shared_ptr<T_> i) :
            _item(i),
            _items(new Sequence<tr1::shared_ptr<AcceptInterface<H_> > >)
        {
        }

        template <typename H_, typename T_>
        tr1::shared_ptr<const T_>
        TreeSequence<H_, T_>::item() const
        {
            return _item;
        }

        template <typename H_, typename T_>
        tr1::shared_ptr<T_>
        TreeSequence<H_, T_>::item()
        {
            return _item;
        }

        template <typename H_, typename T_>
        void
        TreeSequence<H_, T_>::add(tr1::shared_ptr<AcceptInterface<H_> > i)
        {
            _items->push_back(i);
        }

        template <typename H_, typename T_>
        typename H_::ConstSequenceIterator
        TreeSequence<H_, T_>::const_begin() const
        {
            return typename H_::ConstSequenceIterator(
                    paludis::indirect_iterator(_items->begin()));
        }

        template <typename H_, typename T_>
        typename H_::ConstSequenceIterator
        TreeSequence<H_, T_>::const_end() const
        {
            return typename H_::ConstSequenceIterator(
                    paludis::indirect_iterator(_items->end()));
        }

        template <typename H_, typename T_>
        typename H_::SequenceIterator
        TreeSequence<H_, T_>::mutable_begin()
        {
            return typename H_::SequenceIterator(
                    paludis::indirect_iterator(_items->begin()));
        }

        template <typename H_, typename T_>
        typename H_::SequenceIterator
        TreeSequence<H_, T_>::mutable_end()
        {
            return typename H_::SequenceIterator(
                    paludis::indirect_iterator(_items->end()));
        }

        template <typename H_, typename T_>
        void
        ConstTreeSequence<H_, T_>::real_const_accept(ConstVisitor<H_> & v) const
        {
            static_cast<Visits<const ConstTreeSequence<H_, T_> > *>(&v)->visit(*this);
        }

        template <typename H_, typename T_>
        ConstTreeSequence<H_, T_>::~ConstTreeSequence()
        {
        }

        template <typename H_, typename T_>
        ConstTreeSequence<H_, T_>::ConstTreeSequence(tr1::shared_ptr<T_> i) :
            _item(i),
            _items(new Sequence<tr1::shared_ptr<const ConstAcceptInterface<H_> > >)
        {
        }

        template <typename H_, typename T_>
        tr1::shared_ptr<T_>
        ConstTreeSequence<H_, T_>::item()
        {
            return _item;
        }

        template <typename H_, typename T_>
        tr1::shared_ptr<const T_>
        ConstTreeSequence<H_, T_>::item() const
        {
            return _item;
        }

        template <typename H_, typename T_>
        void
        ConstTreeSequence<H_, T_>::add(tr1::shared_ptr<const ConstAcceptInterface<H_> > i)
        {
            _items->push_back(i);
        }

        template <typename H_, typename T_>
        typename H_::ConstSequenceIterator
        ConstTreeSequence<H_, T_>::const_begin() const
        {
            return typename H_::ConstSequenceIterator(
                    paludis::indirect_iterator(_items->begin()));
        }

        template <typename H_, typename T_>
        typename H_::ConstSequenceIterator
        ConstTreeSequence<H_, T_>::const_end() const
        {
            return typename H_::ConstSequenceIterator(
                    paludis::indirect_iterator(_items->end()));
        }

        template <typename T_>
        Visits<T_>::~Visits()
        {
        }

        template <typename H_, typename T_>
        Visits<TreeLeaf<H_, T_> >::~Visits()
        {
        }

        template <typename H_, typename T_>
        void
        Visits<TreeLeaf<H_, T_> >::visit(TreeLeaf<H_, T_> & l)
        {
            visit_leaf(*l.item());
        }

        template <typename H_, typename T_>
        Visits<const TreeLeaf<H_, T_> >::~Visits()
        {
        }

        template <typename H_, typename T_>
        void
        Visits<const TreeLeaf<H_, T_> >::visit(const TreeLeaf<H_, T_> & l)
        {
            visit_leaf(*l.item());
        }

        template <typename H_, typename T_>
        Visits<TreeSequence<H_, T_> >::~Visits()
        {
        }

        template <typename H_, typename T_>
        void
        Visits<TreeSequence<H_, T_> >::visit(TreeSequence<H_, T_> & s)
        {
            visit_sequence(*s.item(), s.mutable_begin(), s.mutable_end());
        }

        template <typename H_, typename T_>
        Visits<const TreeSequence<H_, T_> >::~Visits()
        {
        }

        template <typename H_, typename T_>
        void
        Visits<const TreeSequence<H_, T_> >::visit(const TreeSequence<H_, T_> & s)
        {
            visit_sequence(*s.item(), s.const_begin(), s.const_end());
        }

        template <typename H_, typename T_>
        Visits<ConstTreeSequence<H_, T_> >::~Visits()
        {
        }

        template <typename H_, typename T_>
        void
        Visits<ConstTreeSequence<H_, T_> >::visit(ConstTreeSequence<H_, T_> & s)
        {
            visit_sequence(*s.item(), s.const_begin(), s.const_end());
        }

        template <typename H_, typename T_>
        Visits<const ConstTreeSequence<H_, T_> >::~Visits()
        {
        }

        template <typename H_, typename T_>
        void
        Visits<const ConstTreeSequence<H_, T_> >::visit(const ConstTreeSequence<H_, T_> & s)
        {
            visit_sequence(*s.item(), s.const_begin(), s.const_end());
        }

        template <typename H_, typename LargerH_, typename T_>
        ProxyVisits<H_, LargerH_, const TreeLeaf<H_, T_> >::~ProxyVisits()
        {
        }

        template <typename H_, typename LargerH_, typename T_>
        void
        ProxyVisits<H_, LargerH_, const TreeLeaf<H_, T_> >::visit_leaf(const T_ & v)
        {
            static_cast<Visits<const TreeLeaf<LargerH_, T_> > *>(
                    static_cast<ConstProxyVisitor<H_, LargerH_> *>(this)->larger_visitor())->visit_leaf(v);
        }

        template <typename H_, typename LargerH_, typename T_>
        ProxyVisits<H_, LargerH_, TreeLeaf<H_, T_> >::~ProxyVisits()
        {
        }

        template <typename H_, typename LargerH_, typename T_>
        void
        ProxyVisits<H_, LargerH_, TreeLeaf<H_, T_> >::visit_leaf(T_ & v)
        {
            static_cast<Visits<TreeLeaf<LargerH_, T_> > *>(
                    static_cast<ProxyVisitor<H_, LargerH_> *>(this)->larger_visitor())->visit_leaf(v);
        }

        template <typename H_, typename LargerH_, typename T_>
        ProxyVisits<H_, LargerH_, const ConstTreeSequence<H_, T_> >::~ProxyVisits()
        {
        }

        template <typename H_, typename LargerH_, typename T_>
        void
        ProxyVisits<H_, LargerH_, const ConstTreeSequence<H_, T_> >::visit_sequence(const T_ & t,
                typename TreeSequenceIteratorTypes<H_>::ConstIterator c,
                typename TreeSequenceIteratorTypes<H_>::ConstIterator e)
        {
            static_cast<Visits<const ConstTreeSequence<LargerH_, T_> > *>(
                    static_cast<ConstProxyVisitor<H_, LargerH_> *>(this)->larger_visitor())->visit_sequence(t,
                    typename TreeSequenceIteratorTypes<LargerH_>::ConstIterator(ConstProxyIterator<H_, LargerH_>(c)),
                    typename TreeSequenceIteratorTypes<LargerH_>::ConstIterator(ConstProxyIterator<H_, LargerH_>(e)));
        }

        template <typename H_, typename LargerH_, typename T_>
        ProxyVisits<H_, LargerH_, const TreeSequence<H_, T_> >::~ProxyVisits()
        {
        }

        template <typename H_, typename LargerH_, typename T_>
        void
        ProxyVisits<H_, LargerH_, const TreeSequence<H_, T_> >::visit_sequence(const T_ & t,
                typename TreeSequenceIteratorTypes<H_>::ConstIterator c,
                typename TreeSequenceIteratorTypes<H_>::ConstIterator e)
        {
            static_cast<Visits<const TreeSequence<LargerH_, T_> > *>(
                    static_cast<ConstProxyVisitor<H_, LargerH_> *>(this)->larger_visitor())->visit_sequence(t,
                    typename TreeSequenceIteratorTypes<LargerH_>::ConstIterator(ConstProxyIterator<H_, LargerH_>(c)),
                    typename TreeSequenceIteratorTypes<LargerH_>::ConstIterator(ConstProxyIterator<H_, LargerH_>(e)));
        }

        template <typename H_, typename LargerH_, typename T_>
        ProxyVisits<H_, LargerH_, TreeSequence<H_, T_> >::~ProxyVisits()
        {
        }

        template <typename H_, typename LargerH_, typename T_>
        void
        ProxyVisits<H_, LargerH_, TreeSequence<H_, T_> >::visit_sequence(T_ & t,
                typename TreeSequenceIteratorTypes<H_>::Iterator c,
                typename TreeSequenceIteratorTypes<H_>::Iterator e)
        {
            static_cast<Visits<TreeSequence<LargerH_, T_> > *>(
                    static_cast<ProxyVisitor<H_, LargerH_> *>(this)->larger_visitor())->visit_sequence(t,
                    typename TreeSequenceIteratorTypes<LargerH_>::Iterator(ProxyIterator<H_, LargerH_>(c)),
                    typename TreeSequenceIteratorTypes<LargerH_>::Iterator(ProxyIterator<H_, LargerH_>(e)));
        }

        template <typename H_, typename LargerH_>
        ConstProxyVisitor<H_, LargerH_>::ConstProxyVisitor(ConstVisitor<LargerH_> * const l) :
            _larger_h(l)
        {
        }

        template <typename H_, typename LargerH_>
        ConstVisitor<LargerH_> *
        ConstProxyVisitor<H_, LargerH_>::larger_visitor() const
        {
            return _larger_h;
        }

        template <typename H_, typename LargerH_>
        ProxyVisitor<H_, LargerH_>::ProxyVisitor(Visitor<LargerH_> * const l) :
            _larger_h(l)
        {
        }

        template <typename H_, typename LargerH_>
        Visitor<LargerH_> *
        ProxyVisitor<H_, LargerH_>::larger_visitor() const
        {
            return _larger_h;
        }

        template <typename H_, typename LargerH_>
        ConstProxyIterator<H_, LargerH_>::Adapter::Adapter(const ConstAcceptInterface<H_> & i) :
            _i(i)
        {
        }

        template <typename H_, typename LargerH_>
        void
        ConstProxyIterator<H_, LargerH_>::Adapter::real_const_accept(ConstVisitor<LargerH_> & v) const
        {
            _i.accept(v);
        }

        template <typename H_, typename LargerH_>
        ConstProxyIterator<H_, LargerH_>::ConstProxyIterator(typename TreeSequenceIteratorTypes<H_>::ConstIterator i) :
            _i(i)
        {
        }

        template <typename H_, typename LargerH_>
        bool
        ConstProxyIterator<H_, LargerH_>::operator== (const ConstProxyIterator & other) const
        {
            return _i == other._i;
        }

        template <typename H_, typename LargerH_>
        const ConstAcceptInterface<LargerH_> *
        ConstProxyIterator<H_, LargerH_>::operator-> () const
        {
            if (! _c)
                _c.reset(new Adapter(*_i));
            return _c.get();
        }

        template <typename H_, typename LargerH_>
        const ConstAcceptInterface<LargerH_> &
        ConstProxyIterator<H_, LargerH_>::operator* () const
        {
            return *operator-> ();
        }

        template <typename H_, typename LargerH_>
        ConstProxyIterator<H_, LargerH_> &
        ConstProxyIterator<H_, LargerH_>::operator++ ()
        {
            _c.reset();
            _i++;
            return *this;
        }

        template <typename H_, typename LargerH_>
        ProxyIterator<H_, LargerH_>::Adapter::Adapter(AcceptInterface<H_> & i) :
            _i(i)
        {
        }

        template <typename H_, typename LargerH_>
        void
        ProxyIterator<H_, LargerH_>::Adapter::real_const_accept(ConstVisitor<LargerH_> & v) const
        {
            _i.accept(v);
        }

        template <typename H_, typename LargerH_>
        void
        ProxyIterator<H_, LargerH_>::Adapter::real_mutable_accept(Visitor<LargerH_> & v)
        {
            _i.accept(v);
        }

        template <typename H_, typename LargerH_>
        ProxyIterator<H_, LargerH_>::ProxyIterator(typename TreeSequenceIteratorTypes<H_>::Iterator i) :
            _i(i)
        {
        }

        template <typename H_, typename LargerH_>
        bool
        ProxyIterator<H_, LargerH_>::operator== (const ProxyIterator & other) const
        {
            return _i == other._i;
        }

        template <typename H_, typename LargerH_>
        AcceptInterface<LargerH_> *
        ProxyIterator<H_, LargerH_>::operator-> () const
        {
            if (! _c)
                _c.reset(new Adapter(*_i));
            return _c.get();
        }

        template <typename H_, typename LargerH_>
        AcceptInterface<LargerH_> &
        ProxyIterator<H_, LargerH_>::operator* () const
        {
            return *operator-> ();
        }

        template <typename H_, typename LargerH_>
        ProxyIterator<H_, LargerH_> &
        ProxyIterator<H_, LargerH_>::operator++ ()
        {
            _c.reset();
            _i++;
            return *this;
        }

        template <typename H_>
        template <typename A_, typename B_>
        void
        ConstVisitor<H_>::VisitConstSequence<A_, B_>::visit_sequence(const B_ &,
                typename TreeSequenceIteratorTypes<Heirarchy>::ConstIterator c,
                typename TreeSequenceIteratorTypes<Heirarchy>::ConstIterator e)
        {
            std::for_each(c, e, accept_visitor(*static_cast<A_ *>(this)));
        }

        template <typename H_>
        template <typename A_, typename B_>
        void
        ConstVisitor<H_>::VisitSequence<A_, B_>::visit_sequence(const B_ &,
                typename TreeSequenceIteratorTypes<Heirarchy>::ConstIterator c,
                typename TreeSequenceIteratorTypes<Heirarchy>::ConstIterator e)
        {
            std::for_each(c, e, accept_visitor(*static_cast<A_ *>(this)));
        }

        template <typename H_>
        template <typename A_, typename B_>
        void
        Visitor<H_>::VisitSequence<A_, B_>::visit_sequence(B_ &,
                typename TreeSequenceIteratorTypes<Heirarchy>::Iterator c,
                typename TreeSequenceIteratorTypes<Heirarchy>::Iterator e)
        {
            std::for_each(c, e, accept_visitor(*static_cast<A_ *>(this)));
        }

        template <typename I_, typename T_, typename H_>
        void
        GetConstItemVisits<I_, H_, const TreeLeaf<H_, T_> >::visit_leaf(const T_ & t)
        {
            static_cast<I_ *>(this)->item = &t;
        }

        template <typename I_, typename H_, typename T_>
        void
        GetConstItemVisits<I_, H_, const ConstTreeSequence<H_, T_> >::visit_sequence(const T_ & t,
                typename H_::ConstSequenceIterator,
                typename H_::ConstSequenceIterator)
        {
                static_cast<I_ *>(this)->item = &t;
        }

        template <typename I_, typename H_, typename T_>
        void
        GetConstItemVisits<I_, H_, const TreeSequence<H_, T_> >::visit_sequence(const T_ & t,
                typename H_::ConstSequenceIterator,
                typename H_::ConstSequenceIterator)
        {
            static_cast<I_ *>(this)->item = &t;
        }

        template <typename I_>
        GetConstItemVisitor<I_>::GetConstItemVisitor() :
            item(0)
        {
        }

        template <typename I_>
        const typename I_::Heirarchy::BasicNode *
        get_const_item(const I_ & i)
        {
            GetConstItemVisitor<I_> v;
            i.accept(v);
            return v.item;
        }
    }
}

#endif
