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

#ifndef PALUDIS_GUARD_PALUDIS_VISITOR_HH
#define PALUDIS_GUARD_PALUDIS_VISITOR_HH 1

/** \file
 * Declares the Visitor and related classes.
 *
 * \ingroup grpvisitor
 */

#include <paludis/util/attributes.hh>
#include <paludis/util/visitor-fwd.hh>
#include <paludis/util/collection-fwd.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/tr1_type_traits.hh>
#include <paludis/util/operators.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

namespace paludis
{
    namespace visitor_internals
    {
        template <typename H_>
        class PALUDIS_VISIBLE ConstAcceptInterface
        {
            protected:
                virtual void real_const_accept(ConstVisitor<H_> &) const = 0;

            private:
                template <bool b_, typename T_>
                struct ConstAccept
                {
                    static void forward(const ConstAcceptInterface * const h, T_ & t);
                };

                template <typename T_>
                struct ConstAccept<false, T_>
                {
                    static void forward(const ConstAcceptInterface * const h, T_ & t);
                };

            public:
                typedef H_ Heirarchy;

                virtual ~ConstAcceptInterface();

                template <typename V_>
                void const_accept(V_ & v) const
                {
                    ConstAccept<tr1::is_same<typename H_::Heirarchy, typename V_::Heirarchy>::value, V_>::forward(this, v);
                }

                template <typename V_>
                void accept(V_ & v) const
                {
                    const_accept(v);
                }
        };

        template <typename H_, typename T_>
        class PALUDIS_VISIBLE ConstAcceptInterfaceVisitsThis :
            public virtual ConstAcceptInterface<H_>
        {
            protected:
                virtual void real_const_accept(ConstVisitor<H_> & v) const;
        };

        template <typename H_>
        class PALUDIS_VISIBLE MutableAcceptInterface :
            public ConstAcceptInterface<H_>
        {
            private:
                template <bool b_, typename T_>
                struct Accept
                {
                    static void forward(MutableAcceptInterface * const h, T_ & t);
                };

                template <typename T_>
                struct Accept<true, T_>
                {
                    static void forward(MutableAcceptInterface * const h, T_ & t);
                };

                template <bool b_, typename T_>
                struct MutableAccept
                {
                    static void forward(MutableAcceptInterface * const h, T_ & t);
                };

                template <typename T_>
                struct MutableAccept<false, T_>
                {
                    static void forward(MutableAcceptInterface * const h, T_ & t);
                };

            protected:
                virtual void real_mutable_accept(MutableVisitor<H_> &) = 0;

            public:
                virtual ~MutableAcceptInterface();

                template <typename V_>
                void mutable_accept(V_ & v)
                {
                    MutableAccept<tr1::is_same<typename H_::Heirarchy, typename V_::Heirarchy>::value, V_>::forward(this, v);
                }

                template <typename V_>
                void accept(V_ & v) const
                {
                    const_accept(v);
                }

                template <typename V_>
                void accept(V_ & v)
                {
                    Accept<V_::visitor_is_const, V_>::forward(this, v);
                }
        };

        template <typename H_, typename T_>
        class PALUDIS_VISIBLE MutableAcceptInterfaceVisitsThis :
            public virtual MutableAcceptInterface<H_>
        {
            protected:
                virtual void real_const_accept(ConstVisitor<H_> & v) const;

                virtual void real_mutable_accept(MutableVisitor<H_> & v);
        };

        template <typename H_, typename T_>
        class PALUDIS_VISIBLE TreeLeaf :
            public MutableAcceptInterface<H_>
        {
            private:
                TreeLeaf(const TreeLeaf &);
                const TreeLeaf & operator= (const TreeLeaf &);

                const tr1::shared_ptr<T_> _item;

            protected:
                virtual void real_mutable_accept(MutableVisitor<H_> & v);

                virtual void real_const_accept(ConstVisitor<H_> & v) const;

            public:
                virtual ~TreeLeaf();

                TreeLeaf(const tr1::shared_ptr<T_> & i);

                tr1::shared_ptr<T_> item();

                tr1::shared_ptr<const T_> item() const;
        };

        template <typename H_, typename T_>
        class PALUDIS_VISIBLE TreeSequence :
            public MutableAcceptInterface<H_>
        {
            private:
                TreeSequence(const TreeSequence &);
                const TreeSequence & operator= (const TreeSequence &);

                const tr1::shared_ptr<T_> _item;
                const tr1::shared_ptr<SequentialCollection<tr1::shared_ptr<MutableAcceptInterface<H_> > > > _items;

            protected:
                virtual void real_mutable_accept(MutableVisitor<H_> & v);

                virtual void real_const_accept(ConstVisitor<H_> & v) const;

            public:
                virtual ~TreeSequence();

                TreeSequence(tr1::shared_ptr<T_> i);

                tr1::shared_ptr<const T_> item() const;

                tr1::shared_ptr<T_> item();

                void add(tr1::shared_ptr<MutableAcceptInterface<H_> > i);

                typename H_::ConstSequenceIterator
                const_begin() const;

                typename H_::ConstSequenceIterator
                const_end() const;

                typename H_::MutableSequenceIterator
                mutable_begin();

                typename H_::MutableSequenceIterator
                mutable_end();
        };

        template <typename H_, typename T_>
        class PALUDIS_VISIBLE ConstTreeSequence :
            public ConstAcceptInterface<H_>
        {
            private:
                ConstTreeSequence(const ConstTreeSequence &);
                const ConstTreeSequence & operator= (const ConstTreeSequence &);

                const tr1::shared_ptr<T_> _item;
                const tr1::shared_ptr<SequentialCollection<tr1::shared_ptr<const ConstAcceptInterface<H_> > > > _items;

            protected:
                virtual void real_const_accept(ConstVisitor<H_> & v) const;

            public:
                virtual ~ConstTreeSequence();

                ConstTreeSequence(tr1::shared_ptr<T_> i);

                tr1::shared_ptr<T_> item();

                tr1::shared_ptr<const T_> item() const;

                void add(tr1::shared_ptr<const ConstAcceptInterface<H_> > i);

                typename H_::ConstSequenceIterator
                const_begin() const;

                typename H_::ConstSequenceIterator
                const_end() const;
        };

        template <typename T_>
        class PALUDIS_VISIBLE Visits
        {
            public:
                virtual ~Visits();

                virtual void visit(T_ &) = 0;
        };

        template <unsigned u_>
        class PALUDIS_VISIBLE Visits<NoType<u_> >
        {
        };

        template <unsigned u_>
        class PALUDIS_VISIBLE Visits<const NoType<u_> >
        {
        };

        template <typename H_, typename T_>
        class PALUDIS_VISIBLE Visits<TreeLeaf<H_, T_> >
        {
            public:
                virtual ~Visits();

                virtual void visit_leaf(T_ &) = 0;

                void visit(TreeLeaf<H_, T_> & l);
        };

        template <typename H_, typename T_>
        class PALUDIS_VISIBLE Visits<const TreeLeaf<H_, T_> >
        {
            public:
                virtual ~Visits();

                virtual void visit_leaf(const T_ &) = 0;

                void visit(const TreeLeaf<H_, T_> & l);
        };

        template <typename H_, typename T_>
        class PALUDIS_VISIBLE Visits<TreeSequence<H_, T_> >
        {
            public:
                virtual ~Visits();

                virtual void visit_sequence(T_ &,
                        typename TreeSequenceIteratorTypes<H_>::MutableIterator,
                        typename TreeSequenceIteratorTypes<H_>::MutableIterator) = 0;

                void visit(TreeSequence<H_, T_> & s);
        };

        template <typename H_, typename T_>
        class PALUDIS_VISIBLE Visits<const TreeSequence<H_, T_> >
        {
            public:
                virtual ~Visits();

                virtual void visit_sequence(const T_ &,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator) = 0;

                void visit(const TreeSequence<H_, T_> & s);
        };

        template <typename H_, typename T_>
        class PALUDIS_VISIBLE Visits<ConstTreeSequence<H_, T_> >
        {
            public:
                virtual ~Visits();

                virtual void visit_sequence(const T_ &,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator) = 0;

                void visit(ConstTreeSequence<H_, T_> & s);
        };

        template <typename H_, typename T_>
        class PALUDIS_VISIBLE Visits<const ConstTreeSequence<H_, T_> >
        {
            public:
                virtual ~Visits();

                virtual void visit_sequence(const T_ &,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator) = 0;

                void visit(const ConstTreeSequence<H_, T_> & s);
        };

        template <typename H_>
        struct TreeSequenceIteratorTypes
        {
            typedef libwrapiter::ForwardIterator<TreeSequenceIteratorTypes, const ConstAcceptInterface<H_> > ConstIterator;
            typedef libwrapiter::ForwardIterator<TreeSequenceIteratorTypes, MutableAcceptInterface<H_> > MutableIterator;
        };

        template <typename H_, typename LargerH_, typename T_>
        class PALUDIS_VISIBLE ProxyVisits :
            public virtual Visits<T_>
        {
        };

        template <typename H_, typename LargerH_, unsigned u_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, NoType<u_> >
        {
        };

        template <typename H_, typename LargerH_, unsigned u_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, const NoType<u_> >
        {
        };

        template <typename H_, typename LargerH_, typename T_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, const TreeLeaf<H_, T_> > :
            public virtual Visits<const TreeLeaf<H_, T_> >
        {
            public:
                virtual ~ProxyVisits();

                virtual void visit_leaf(const T_ & v);
        };

        template <typename H_, typename LargerH_, typename T_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, TreeLeaf<H_, T_> > :
            public virtual Visits<TreeLeaf<H_, T_> >
        {
            public:
                virtual ~ProxyVisits();

                virtual void visit_leaf(T_ & v);
        };

        template <typename H_, typename LargerH_, typename T_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, const ConstTreeSequence<H_, T_> > :
            public virtual Visits<const ConstTreeSequence<H_, T_> >
        {
            public:
                virtual ~ProxyVisits();

                virtual void visit_sequence(const T_ & t,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator c,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator e);
        };

        template <typename H_, typename LargerH_, typename T_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, const TreeSequence<H_, T_> > :
            public virtual Visits<const TreeSequence<H_, T_> >
        {
            public:
                virtual ~ProxyVisits();

                virtual void visit_sequence(const T_ & t,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator c,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator e);
        };

        template <typename H_, typename LargerH_, typename T_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, TreeSequence<H_, T_> > :
            public virtual Visits<TreeSequence<H_, T_> >
        {
            public:
                virtual ~ProxyVisits();

                virtual void visit_sequence(T_ & t,
                        typename TreeSequenceIteratorTypes<H_>::MutableIterator c,
                        typename TreeSequenceIteratorTypes<H_>::MutableIterator e);
        };

        template <typename H_, typename LargerH_>
        class PALUDIS_VISIBLE ConstProxyVisitor :
            public ConstVisitor<H_>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem1>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem2>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem3>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem4>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem5>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem6>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem7>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem8>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem9>
        {
            private:
                ConstVisitor<LargerH_> * const _larger_h;

            public:
                ConstProxyVisitor(ConstVisitor<LargerH_> * const l);

                ConstVisitor<LargerH_> * larger_visitor() const;
        };

        template <typename H_, typename LargerH_>
        class PALUDIS_VISIBLE MutableProxyVisitor :
            public MutableVisitor<H_>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem1>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem2>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem3>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem4>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem5>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem6>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem7>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem8>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem9>
        {
            private:
                MutableVisitor<LargerH_> * const _larger_h;

            public:
                MutableProxyVisitor(MutableVisitor<LargerH_> * const l);

                MutableVisitor<LargerH_> * larger_visitor() const;
        };

        template <typename H_, typename LargerH_>
        class PALUDIS_VISIBLE ConstProxyIterator :
            public paludis::equality_operators::HasEqualityOperators
        {
            private:
                struct PALUDIS_VISIBLE Adapter :
                    ConstAcceptInterface<LargerH_>
                {
                    const ConstAcceptInterface<H_> & _i;

                    Adapter(const ConstAcceptInterface<H_> & i);

                    void real_const_accept(ConstVisitor<LargerH_> & v) const;
                };

                typename TreeSequenceIteratorTypes<H_>::ConstIterator _i;
                mutable tr1::shared_ptr<Adapter> _c;

            public:
                ConstProxyIterator(typename TreeSequenceIteratorTypes<H_>::ConstIterator i);

                bool operator== (const ConstProxyIterator & other) const;

                const ConstAcceptInterface<LargerH_> * operator-> () const;

                const ConstAcceptInterface<LargerH_> & operator* () const;

                ConstProxyIterator & operator++ ();
        };

        template <typename H_, typename LargerH_>
        class PALUDIS_VISIBLE MutableProxyIterator :
            public paludis::equality_operators::HasEqualityOperators
        {
            private:
                struct PALUDIS_VISIBLE Adapter :
                    MutableAcceptInterface<LargerH_>
                {
                    MutableAcceptInterface<H_> & _i;

                    Adapter(MutableAcceptInterface<H_> & i);

                    void real_const_accept(ConstVisitor<LargerH_> & v) const;

                    void real_mutable_accept(MutableVisitor<LargerH_> & v);
                };

                typename TreeSequenceIteratorTypes<H_>::MutableIterator _i;
                mutable tr1::shared_ptr<Adapter> _c;

            public:
                MutableProxyIterator(typename TreeSequenceIteratorTypes<H_>::MutableIterator i);

                bool operator== (const MutableProxyIterator & other) const;

                MutableAcceptInterface<LargerH_> * operator-> () const;

                MutableAcceptInterface<LargerH_> & operator* () const;

                MutableProxyIterator & operator++ ();
        };

        template <
            typename Heirarchy_,
            typename BasicNode_,
            typename ContainedItem1_,
            typename ContainedItem2_,
            typename ContainedItem3_,
            typename ContainedItem4_,
            typename ContainedItem5_,
            typename ContainedItem6_,
            typename ContainedItem7_,
            typename ContainedItem8_,
            typename ContainedItem9_>
        class VisitorTypes
        {
            public:
                typedef Heirarchy_ Heirarchy;
                typedef BasicNode_ BasicNode;

                typedef ContainedItem1_ ContainedItem1;
                typedef ContainedItem2_ ContainedItem2;
                typedef ContainedItem3_ ContainedItem3;
                typedef ContainedItem4_ ContainedItem4;
                typedef ContainedItem5_ ContainedItem5;
                typedef ContainedItem6_ ContainedItem6;
                typedef ContainedItem7_ ContainedItem7;
                typedef ContainedItem8_ ContainedItem8;
                typedef ContainedItem9_ ContainedItem9;

                typedef MutableAcceptInterface<Heirarchy_> Item;
                typedef const ConstAcceptInterface<Heirarchy_> ConstItem;

                typedef typename TreeSequenceIteratorTypes<Heirarchy_>::MutableIterator MutableSequenceIterator;
                typedef typename TreeSequenceIteratorTypes<Heirarchy_>::ConstIterator ConstSequenceIterator;
        };

        template <typename H_>
        class ConstVisitor :
            public virtual Visits<const typename H_::ContainedItem1>,
            public virtual Visits<const typename H_::ContainedItem2>,
            public virtual Visits<const typename H_::ContainedItem3>,
            public virtual Visits<const typename H_::ContainedItem4>,
            public virtual Visits<const typename H_::ContainedItem5>,
            public virtual Visits<const typename H_::ContainedItem6>,
            public virtual Visits<const typename H_::ContainedItem7>,
            public virtual Visits<const typename H_::ContainedItem8>,
            public virtual Visits<const typename H_::ContainedItem9>
        {
            public:
                enum { visitor_is_const = 1 };

                typedef typename H_::Heirarchy Heirarchy;

                template <typename A_, typename B_>
                struct PALUDIS_VISIBLE VisitConstSequence :
                    virtual Visits<const ConstTreeSequence<Heirarchy, B_> >
                {
                    virtual void visit_sequence(const B_ &,
                            typename TreeSequenceIteratorTypes<Heirarchy>::ConstIterator c,
                            typename TreeSequenceIteratorTypes<Heirarchy>::ConstIterator e);
                };

                template <typename A_, typename B_>
                struct PALUDIS_VISIBLE VisitSequence :
                    virtual Visits<const TreeSequence<Heirarchy, B_> >
                {
                    virtual void visit_sequence(const B_ &,
                            typename TreeSequenceIteratorTypes<Heirarchy>::ConstIterator c,
                            typename TreeSequenceIteratorTypes<Heirarchy>::ConstIterator e);
                };
        };

        template <typename H_>
        class MutableVisitor :
            public virtual Visits<typename H_::ContainedItem1>,
            public virtual Visits<typename H_::ContainedItem2>,
            public virtual Visits<typename H_::ContainedItem3>,
            public virtual Visits<typename H_::ContainedItem4>,
            public virtual Visits<typename H_::ContainedItem5>,
            public virtual Visits<typename H_::ContainedItem6>,
            public virtual Visits<typename H_::ContainedItem7>,
            public virtual Visits<typename H_::ContainedItem8>,
            public virtual Visits<typename H_::ContainedItem9>
        {
            public:
                enum { visitor_is_const = 0 };

                typedef typename H_::Heirarchy Heirarchy;

                template <typename A_, typename B_>
                struct PALUDIS_VISIBLE VisitSequence :
                    virtual Visits<TreeSequence<Heirarchy, B_> >
                {
                    virtual void visit_sequence(B_ &,
                            typename TreeSequenceIteratorTypes<Heirarchy>::MutableIterator c,
                            typename TreeSequenceIteratorTypes<Heirarchy>::MutableIterator e);
                };
        };

        template <typename I_, typename H_, typename T_>
        struct GetConstItemVisits;

        template <typename I_, typename H_, unsigned u_>
        struct PALUDIS_VISIBLE GetConstItemVisits<I_, H_, const visitor_internals::NoType<u_> >
        {
        };

        template <typename I_, typename T_, typename H_>
        struct PALUDIS_VISIBLE GetConstItemVisits<I_, H_, const TreeLeaf<H_, T_> > :
            virtual visitor_internals::Visits<const TreeLeaf<H_, T_> >
        {
            void visit_leaf(const T_ & t);
        };

        template <typename I_, typename H_, typename T_>
        struct PALUDIS_VISIBLE GetConstItemVisits<I_, H_, const ConstTreeSequence<H_, T_> > :
            virtual visitor_internals::Visits<const ConstTreeSequence<H_, T_> >
        {
            void visit_sequence(const T_ & t,
                    typename H_::ConstSequenceIterator,
                    typename H_::ConstSequenceIterator);
        };

        template <typename I_, typename H_, typename T_>
        struct PALUDIS_VISIBLE GetConstItemVisits<I_, H_, const TreeSequence<H_, T_> > :
            virtual visitor_internals::Visits<const TreeSequence<H_, T_> >
        {
            void visit_sequence(const T_ & t,
                    typename H_::ConstSequenceIterator,
                    typename H_::ConstSequenceIterator);
        };

        template <typename I_>
        struct PALUDIS_VISIBLE GetConstItemVisitor :
            ConstVisitor<typename I_::Heirarchy>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem1>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem2>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem3>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem4>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem5>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem6>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem7>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem8>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem9>
        {
            const typename I_::Heirarchy::BasicNode * item;

            GetConstItemVisitor();
        };

        template <typename I_>
        const typename I_::Heirarchy::BasicNode *
        get_const_item(const I_ & i);
    }

    template <typename Visitor_>
    class PALUDIS_VISIBLE AcceptVisitor
    {
        private:
            Visitor_ & _v;

        public:
            AcceptVisitor(Visitor_ & v) :
                _v(v)
            {
            }

            template <typename T_>
            void operator() (T_ & t) const
            {
                t.accept(_v);
            }
    };

    template <typename Visitor_>
    AcceptVisitor<Visitor_> PALUDIS_VISIBLE accept_visitor(Visitor_ & v)
    {
        return AcceptVisitor<Visitor_>(v);
    }
}

#endif
