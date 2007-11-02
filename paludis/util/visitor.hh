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

#ifndef PALUDIS_GUARD_PALUDIS_VISITOR_HH
#define PALUDIS_GUARD_PALUDIS_VISITOR_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/visitor-fwd.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/tr1_type_traits.hh>
#include <paludis/util/operators.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>

/** \file
 * Declares the Visitor and related classes.
 *
 * \ingroup g_visitors
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * \namespace visitor_internals
     * \ingroup g_visitors
     *
     * For internal use by visitor classes.
     */
    namespace visitor_internals
    {
        /**
         * Derived classes can accept a const visitor of heirarchy H_.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_>
        class PALUDIS_VISIBLE ConstAcceptInterface
        {
            protected:
                ///\name Visitor operations
                ///\{

                virtual void real_const_accept(ConstVisitor<H_> &) const = 0;

                ///\}

            private:
                template <bool b_, typename T_>
                struct PALUDIS_VISIBLE ConstAccept
                {
                    static void forward(const ConstAcceptInterface * const h, T_ & t);
                };

                template <typename T_>
                struct PALUDIS_VISIBLE ConstAccept<false, T_>
                {
                    static void forward(const ConstAcceptInterface * const h, T_ & t);
                };

            public:
                ///\name Basic operations
                ///\{

                virtual ~ConstAcceptInterface();

                ///\}

                ///\name Visitor operations
                ///\{

                typedef H_ Heirarchy;

                /**
                 * Accept a const visitor of either our visitable type, or a
                 * visitor that can visit a superset of that heirarchy.
                 */
                template <typename V_>
                void const_accept(V_ & v) const
                {
                    ConstAccept<tr1::is_same<typename H_::Heirarchy, typename V_::Heirarchy>::value, V_>::forward(this, v);
                }

                /**
                 * Accept a const visitor of either our visitable type, or a
                 * visitor that can visit a superset of that heirarchy.
                 */
                template <typename V_>
                void accept(V_ & v) const
                {
                    const_accept(v);
                }

                ///\}
        };

        /**
         * Implementation of ConstAcceptInterface for class T_.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename T_>
        class PALUDIS_VISIBLE ConstAcceptInterfaceVisitsThis :
            public virtual ConstAcceptInterface<H_>
        {
            protected:
                ///\name Visitor operations
                ///\{

                virtual void real_const_accept(ConstVisitor<H_> & v) const;

                ///\}
        };

        /**
         * Derived classes can accept a const or non-const visitor of heirarchy H_.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_>
        class PALUDIS_VISIBLE AcceptInterface :
            public ConstAcceptInterface<H_>
        {
            private:
                template <bool b_, typename T_>
                struct PALUDIS_VISIBLE ConstAccept
                {
                    static void forward(AcceptInterface * const h, T_ & t);
                };

                template <typename T_>
                struct PALUDIS_VISIBLE ConstAccept<true, T_>
                {
                    static void forward(AcceptInterface * const h, T_ & t);
                };

                template <bool b_, typename T_>
                struct PALUDIS_VISIBLE Accept
                {
                    static void forward(AcceptInterface * const h, T_ & t);
                };

                template <typename T_>
                struct PALUDIS_VISIBLE Accept<false, T_>
                {
                    static void forward(AcceptInterface * const h, T_ & t);
                };

            protected:
                ///\name Visitor operations
                ///\{

                virtual void real_mutable_accept(Visitor<H_> &) = 0;

                ///\}

            public:
                ///\name Basic operations
                ///\{

                virtual ~AcceptInterface();

                ///\}

                ///\name Visitor operations
                ///\{

                template <typename V_>
                void mutable_accept(V_ & v)
                {
                    Accept<tr1::is_same<typename H_::Heirarchy, typename V_::Heirarchy>::value, V_>::forward(this, v);
                }

                template <typename V_>
                void accept(V_ & v) const
                {
                    const_accept(v);
                }

                template <typename V_>
                void accept(V_ & v)
                {
                    ConstAccept<V_::visitor_is_const, V_>::forward(this, v);
                }

                ///\}
        };

        /**
         * Implementation of ConstAcceptInterface for class T_.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename T_>
        class PALUDIS_VISIBLE AcceptInterfaceVisitsThis :
            public virtual AcceptInterface<H_>
        {
            protected:
                ///\name Visitor operations
                ///\{

                virtual void real_const_accept(ConstVisitor<H_> & v) const;

                virtual void real_mutable_accept(Visitor<H_> & v);

                ///\}
        };

        /**
         * Used to contain a node with no children in a detached visitable
         * heirarchy.
         *
         * \ingroup g_visitors
         */
        template <typename H_, typename T_>
        class PALUDIS_VISIBLE TreeLeaf :
            public AcceptInterface<H_>
        {
            private:
                TreeLeaf(const TreeLeaf &);
                const TreeLeaf & operator= (const TreeLeaf &);

                const tr1::shared_ptr<T_> _item;

            protected:
                ///\name Visitor operations
                ///\{

                virtual void real_mutable_accept(Visitor<H_> & v);

                virtual void real_const_accept(ConstVisitor<H_> & v) const;

                ///\}

            public:
                ///\name Basic operations
                ///\{

                virtual ~TreeLeaf();

                TreeLeaf(const tr1::shared_ptr<T_> & i);

                ///\}

                ///\name Fetch our contained item
                ///\{

                tr1::shared_ptr<T_> item();

                tr1::shared_ptr<const T_> item() const;

                ///\}
        };

        /**
         * Used to contain a node with a sequence of children in a detached visitable
         * heirarchy.
         *
         * \ingroup g_visitors
         */
        template <typename H_, typename T_>
        class PALUDIS_VISIBLE TreeSequence :
            public AcceptInterface<H_>
        {
            private:
                TreeSequence(const TreeSequence &);
                const TreeSequence & operator= (const TreeSequence &);

                const tr1::shared_ptr<T_> _item;
                const tr1::shared_ptr<Sequence<tr1::shared_ptr<AcceptInterface<H_> > > > _items;

            protected:
                ///\name Visitor operations
                ///\{

                virtual void real_mutable_accept(Visitor<H_> & v);

                virtual void real_const_accept(ConstVisitor<H_> & v) const;

                ///\}

            public:
                ///\name Basic operations
                ///\{

                virtual ~TreeSequence();

                TreeSequence(tr1::shared_ptr<T_> i);

                ///\}

                ///\name Fetch our contained item
                ///\{

                tr1::shared_ptr<const T_> item() const;

                tr1::shared_ptr<T_> item();

                ///\}

                ///\name Work on our children
                ///\{

                void add(tr1::shared_ptr<AcceptInterface<H_> > i);

                typename H_::ConstSequenceIterator
                const_begin() const;

                typename H_::ConstSequenceIterator
                const_end() const;

                typename H_::SequenceIterator
                mutable_begin();

                typename H_::SequenceIterator
                mutable_end();

                ///\}
        };

        /**
         * Used to contain a node with a sequence of children in a detached visitable
         * heirarchy, where children can only be created in const form.
         *
         * \ingroup g_visitors
         */
        template <typename H_, typename T_>
        class PALUDIS_VISIBLE ConstTreeSequence :
            public ConstAcceptInterface<H_>
        {
            private:
                ConstTreeSequence(const ConstTreeSequence &);
                const ConstTreeSequence & operator= (const ConstTreeSequence &);

                const tr1::shared_ptr<T_> _item;
                const tr1::shared_ptr<Sequence<tr1::shared_ptr<const ConstAcceptInterface<H_> > > > _items;

            protected:
                ///\name Visitor operations
                ///\{

                virtual void real_const_accept(ConstVisitor<H_> & v) const;

                ///\}

            public:
                ///\name Basic operations
                ///\{

                virtual ~ConstTreeSequence();

                ConstTreeSequence(tr1::shared_ptr<T_> i);

                ///\}

                ///\name Fetch our contained item
                ///\{

                tr1::shared_ptr<T_> item();

                tr1::shared_ptr<const T_> item() const;

                ///\}

                ///\name Work on our children
                ///\{

                void add(tr1::shared_ptr<const ConstAcceptInterface<H_> > i);

                typename H_::ConstSequenceIterator
                const_begin() const;

                typename H_::ConstSequenceIterator
                const_end() const;

                ///\}
        };

        /**
         * Derived classes can visit an item of type T_.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename T_>
        class PALUDIS_VISIBLE Visits
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~Visits();

                ///\}

                ///\name Visitor operations
                ///\{

                virtual void visit(T_ &) = 0;

                ///\}
        };

        /**
         * Derived classes can visit an item of type T_ (specialisation for
         * NoType).
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <unsigned u_>
        class PALUDIS_VISIBLE Visits<NoType<u_> >
        {
        };

        /**
         * Derived classes can visit an item of type T_ (specialisation for
         * NoType).
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <unsigned u_>
        class PALUDIS_VISIBLE Visits<const NoType<u_> >
        {
        };

        /**
         * Derived classes can visit an item of type T_ (specialisation for
         * TreeLeaf).
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename T_>
        class PALUDIS_VISIBLE Visits<TreeLeaf<H_, T_> >
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~Visits();

                ///\}

                ///\name Visitor operations
                ///\{

                virtual void visit_leaf(T_ &) = 0;

                void visit(TreeLeaf<H_, T_> & l);

                ///\}
        };

        /**
         * Derived classes can visit an item of type T_ (specialisation for
         * TreeLeaf).
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename T_>
        class PALUDIS_VISIBLE Visits<const TreeLeaf<H_, T_> >
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~Visits();

                ///\}

                ///\name Visitor operations
                ///\{

                virtual void visit_leaf(const T_ &) = 0;

                void visit(const TreeLeaf<H_, T_> & l);

                ///\}
        };

        /**
         * Derived classes can visit an item of type T_ (specialisation for
         * TreeSequence).
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename T_>
        class PALUDIS_VISIBLE Visits<TreeSequence<H_, T_> >
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~Visits();

                ///\}

                ///\name Visitor operations
                ///\{

                virtual void visit_sequence(T_ &,
                        typename TreeSequenceIteratorTypes<H_>::Iterator,
                        typename TreeSequenceIteratorTypes<H_>::Iterator) = 0;

                void visit(TreeSequence<H_, T_> & s);

                ///\}
        };

        /**
         * Derived classes can visit an item of type T_ (specialisation for
         * TreeSequence).
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename T_>
        class PALUDIS_VISIBLE Visits<const TreeSequence<H_, T_> >
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~Visits();

                ///\}

                ///\name Visitor operations
                ///\{

                virtual void visit_sequence(const T_ &,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator) = 0;

                void visit(const TreeSequence<H_, T_> & s);

                ///\}
        };

        /**
         * Derived classes can visit an item of type T_ (specialisation for
         * ConstTreeSequence).
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename T_>
        class PALUDIS_VISIBLE Visits<ConstTreeSequence<H_, T_> >
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~Visits();

                ///\}

                ///\name Visitor operations
                ///\{

                virtual void visit_sequence(const T_ &,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator) = 0;

                void visit(ConstTreeSequence<H_, T_> & s);

                ///\}
        };

        /**
         * Derived classes can visit an item of type T_ (specialisation for
         * ConstTreeSequence).
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename T_>
        class PALUDIS_VISIBLE Visits<const ConstTreeSequence<H_, T_> >
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~Visits();

                ///\}

                ///\name Visitor operations
                ///\{

                virtual void visit_sequence(const T_ &,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator) = 0;

                void visit(const ConstTreeSequence<H_, T_> & s);

                ///\}
        };

        /**
         * Container class providing convenience typedefs for iterators over a
         * TreeSequence.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_>
        struct TreeSequenceIteratorTypes
        {
            ///\name Visitor operations
            ///\{

            typedef WrappedForwardIterator<enum ConstIteratorTag { }, const ConstAcceptInterface<H_> > ConstIterator;
            typedef WrappedForwardIterator<enum IteratorTag { }, AcceptInterface<H_> > Iterator;

            ///\}
        };

        /**
         * Proxy visitor adapter.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename LargerH_, typename T_>
        class PALUDIS_VISIBLE ProxyVisits :
            public virtual Visits<T_>
        {
        };

        /**
         * Proxy visitor adapter.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename LargerH_, unsigned u_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, NoType<u_> >
        {
        };

        /**
         * Proxy visitor adapter.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename LargerH_, unsigned u_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, const NoType<u_> >
        {
        };

        /**
         * Proxy visitor adapter.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename LargerH_, typename T_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, const TreeLeaf<H_, T_> > :
            public virtual Visits<const TreeLeaf<H_, T_> >
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~ProxyVisits();

                ///\}

                ///\name Visitor operations
                ///\{

                virtual void visit_leaf(const T_ & v);

                ///\}
        };

        /**
         * Proxy visitor adapter.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename LargerH_, typename T_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, TreeLeaf<H_, T_> > :
            public virtual Visits<TreeLeaf<H_, T_> >
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~ProxyVisits();

                ///\}

                ///\name Visitor operations
                ///\{

                virtual void visit_leaf(T_ & v);

                ///\}
        };

        /**
         * Proxy visitor adapter.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename LargerH_, typename T_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, const ConstTreeSequence<H_, T_> > :
            public virtual Visits<const ConstTreeSequence<H_, T_> >
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~ProxyVisits();

                ///\}

                ///\name Visitor operations
                ///\{

                virtual void visit_sequence(const T_ & t,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator c,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator e);

                ///\}
        };

        /**
         * Proxy visitor adapter.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename LargerH_, typename T_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, const TreeSequence<H_, T_> > :
            public virtual Visits<const TreeSequence<H_, T_> >
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~ProxyVisits();

                ///\}

                ///\name Visitor operations
                ///\{

                virtual void visit_sequence(const T_ & t,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator c,
                        typename TreeSequenceIteratorTypes<H_>::ConstIterator e);

                ///\}
        };

        /**
         * Proxy visitor adapter.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename LargerH_, typename T_>
        class PALUDIS_VISIBLE ProxyVisits<H_, LargerH_, TreeSequence<H_, T_> > :
            public virtual Visits<TreeSequence<H_, T_> >
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~ProxyVisits();

                ///\}

                ///\name Visitor operations
                ///\{

                virtual void visit_sequence(T_ & t,
                        typename TreeSequenceIteratorTypes<H_>::Iterator c,
                        typename TreeSequenceIteratorTypes<H_>::Iterator e);

                ///\}
        };

        /**
         * Proxy visitor.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
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
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem9>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem10>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem11>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem12>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem13>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem14>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem15>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem16>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem17>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem18>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem19>,
            public ProxyVisits<H_, LargerH_, const typename H_::ContainedItem20>
        {
            private:
                ConstVisitor<LargerH_> * const _larger_h;

            public:
                ///\name Basic operations
                ///\{

                ConstProxyVisitor(ConstVisitor<LargerH_> * const l);

                ///\}

                ///\name Visitor operations
                ///\{

                ConstVisitor<LargerH_> * larger_visitor() const;

                ///\}
        };

        /**
         * Proxy visitor adapter.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename LargerH_>
        class PALUDIS_VISIBLE ProxyVisitor :
            public Visitor<H_>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem1>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem2>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem3>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem4>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem5>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem6>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem7>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem8>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem9>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem10>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem11>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem12>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem13>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem14>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem15>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem16>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem17>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem18>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem19>,
            public ProxyVisits<H_, LargerH_, typename H_::ContainedItem20>
        {
            private:
                Visitor<LargerH_> * const _larger_h;

            public:
                ///\name Basic operations
                ///\{

                ProxyVisitor(Visitor<LargerH_> * const l);

                ///\}

                ///\name Visitor operations
                ///\{

                Visitor<LargerH_> * larger_visitor() const;

                ///\}
        };

        /**
         * Proxy visitor adapter iterator.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
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
                ///\name Basic operations
                ///\{

                ConstProxyIterator(typename TreeSequenceIteratorTypes<H_>::ConstIterator i);

                bool operator== (const ConstProxyIterator & other) const;

                const ConstAcceptInterface<LargerH_> * operator-> () const;

                const ConstAcceptInterface<LargerH_> & operator* () const;

                ConstProxyIterator & operator++ ();

                ///\}
        };

        /**
         * Proxy visitor adapter iterator.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename H_, typename LargerH_>
        class PALUDIS_VISIBLE ProxyIterator :
            public paludis::equality_operators::HasEqualityOperators
        {
            private:
                struct PALUDIS_VISIBLE Adapter :
                    AcceptInterface<LargerH_>
                {
                    AcceptInterface<H_> & _i;

                    Adapter(AcceptInterface<H_> & i);

                    void real_const_accept(ConstVisitor<LargerH_> & v) const;

                    void real_mutable_accept(Visitor<LargerH_> & v);
                };

                typename TreeSequenceIteratorTypes<H_>::Iterator _i;
                mutable tr1::shared_ptr<Adapter> _c;

            public:
                ///\name Basic operations
                ///\{

                ProxyIterator(typename TreeSequenceIteratorTypes<H_>::Iterator i);

                bool operator== (const ProxyIterator & other) const;

                AcceptInterface<LargerH_> * operator-> () const;

                AcceptInterface<LargerH_> & operator* () const;

                ProxyIterator & operator++ ();

                ///\}
        };

        /**
         * Define policy for a set of visitor types.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
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
            typename ContainedItem9_,
            typename ContainedItem10_,
            typename ContainedItem11_,
            typename ContainedItem12_,
            typename ContainedItem13_,
            typename ContainedItem14_,
            typename ContainedItem15_,
            typename ContainedItem16_,
            typename ContainedItem17_,
            typename ContainedItem18_,
            typename ContainedItem19_,
            typename ContainedItem20_>
        class VisitorTypes
        {
            public:
                ///\name Visitor type definitions
                ///\{

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
                typedef ContainedItem10_ ContainedItem10;
                typedef ContainedItem11_ ContainedItem11;
                typedef ContainedItem12_ ContainedItem12;
                typedef ContainedItem13_ ContainedItem13;
                typedef ContainedItem14_ ContainedItem14;
                typedef ContainedItem15_ ContainedItem15;
                typedef ContainedItem16_ ContainedItem16;
                typedef ContainedItem17_ ContainedItem17;
                typedef ContainedItem18_ ContainedItem18;
                typedef ContainedItem19_ ContainedItem19;
                typedef ContainedItem20_ ContainedItem20;

                typedef AcceptInterface<Heirarchy_> Item;
                typedef const ConstAcceptInterface<Heirarchy_> ConstItem;

                typedef typename TreeSequenceIteratorTypes<Heirarchy_>::Iterator SequenceIterator;
                typedef typename TreeSequenceIteratorTypes<Heirarchy_>::ConstIterator ConstSequenceIterator;

                ///\}
        };

        /**
         * A ConstVisitor visits a visitable heirarchy.
         *
         * \nosubgrouping
         * \ingroup g_visitors
         */
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
            public virtual Visits<const typename H_::ContainedItem9>,
            public virtual Visits<const typename H_::ContainedItem10>,
            public virtual Visits<const typename H_::ContainedItem11>,
            public virtual Visits<const typename H_::ContainedItem12>,
            public virtual Visits<const typename H_::ContainedItem13>,
            public virtual Visits<const typename H_::ContainedItem14>,
            public virtual Visits<const typename H_::ContainedItem15>,
            public virtual Visits<const typename H_::ContainedItem16>,
            public virtual Visits<const typename H_::ContainedItem17>,
            public virtual Visits<const typename H_::ContainedItem18>,
            public virtual Visits<const typename H_::ContainedItem19>,
            public virtual Visits<const typename H_::ContainedItem20>
        {
            public:
                ///\name Visitor type definitions
                ///\{

                enum { visitor_is_const = 1 };

                typedef typename H_::Heirarchy Heirarchy;

                /**
                 * Derived classes visit a ConstTreeSequence by visiting all of
                 * its children.
                 *
                 * \ingroup g_visitors
                 * \nosubgrouping
                 */
                template <typename A_, typename B_>
                struct PALUDIS_VISIBLE VisitConstSequence :
                    virtual Visits<const ConstTreeSequence<Heirarchy, B_> >
                {
                    ///\name Visitor operations
                    ///\{

                    virtual void visit_sequence(const B_ &,
                            typename TreeSequenceIteratorTypes<Heirarchy>::ConstIterator c,
                            typename TreeSequenceIteratorTypes<Heirarchy>::ConstIterator e);

                    ///\}
                };

                /**
                 * Derived classes visit a TreeSequence by visiting all of
                 * its children.
                 *
                 * \ingroup g_visitors
                 * \nosubgrouping
                 */
                template <typename A_, typename B_>
                struct PALUDIS_VISIBLE VisitSequence :
                    virtual Visits<const TreeSequence<Heirarchy, B_> >
                {
                    ///\name Visitor operations
                    ///\{

                    virtual void visit_sequence(const B_ &,
                            typename TreeSequenceIteratorTypes<Heirarchy>::ConstIterator c,
                            typename TreeSequenceIteratorTypes<Heirarchy>::ConstIterator e);

                    ///\}
                };

                /**
                 * Query whether we contain a particular type.
                 *
                 * \ingroup g_visitors
                 * \nosubgrouping
                 */
                template <typename A_>
                struct Contains
                {
                    /**
                     * Do we contain the queried type?
                     */
                    enum {
                        value =
                            (tr1::is_same<const A_, const typename H_::ContainedItem1>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem2>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem3>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem4>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem5>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem6>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem7>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem8>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem9>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem10>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem11>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem12>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem13>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem14>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem15>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem16>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem17>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem18>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem19>::value ? 1 : 0) |
                            (tr1::is_same<const A_, const typename H_::ContainedItem20>::value ? 1 : 0)
                    };
                };

                ///\}
        };

        /**
         * A ConstVisitor visits a visitable heirarchy.
         *
         * \nosubgrouping
         * \ingroup g_visitors
         */
        template <typename H_>
        class Visitor :
            public virtual Visits<typename H_::ContainedItem1>,
            public virtual Visits<typename H_::ContainedItem2>,
            public virtual Visits<typename H_::ContainedItem3>,
            public virtual Visits<typename H_::ContainedItem4>,
            public virtual Visits<typename H_::ContainedItem5>,
            public virtual Visits<typename H_::ContainedItem6>,
            public virtual Visits<typename H_::ContainedItem7>,
            public virtual Visits<typename H_::ContainedItem8>,
            public virtual Visits<typename H_::ContainedItem9>,
            public virtual Visits<typename H_::ContainedItem10>,
            public virtual Visits<typename H_::ContainedItem11>,
            public virtual Visits<typename H_::ContainedItem12>,
            public virtual Visits<typename H_::ContainedItem13>,
            public virtual Visits<typename H_::ContainedItem14>,
            public virtual Visits<typename H_::ContainedItem15>,
            public virtual Visits<typename H_::ContainedItem16>,
            public virtual Visits<typename H_::ContainedItem17>,
            public virtual Visits<typename H_::ContainedItem18>,
            public virtual Visits<typename H_::ContainedItem19>,
            public virtual Visits<typename H_::ContainedItem20>
        {
            public:
                ///\name Visitor type definitions
                ///\{

                enum { visitor_is_const = 0 };

                typedef typename H_::Heirarchy Heirarchy;

                /**
                 * Derived classes visit a TreeSequence by visiting all of
                 * its children.
                 *
                 * \ingroup g_visitors
                 * \nosubgrouping
                 */
                template <typename A_, typename B_>
                struct PALUDIS_VISIBLE VisitSequence :
                    virtual Visits<TreeSequence<Heirarchy, B_> >
                {
                    ///\name Visitor operations
                    ///\{

                    virtual void visit_sequence(B_ &,
                            typename TreeSequenceIteratorTypes<Heirarchy>::Iterator c,
                            typename TreeSequenceIteratorTypes<Heirarchy>::Iterator e);

                    ///\}
                };

                /**
                 * Query whether we contain a particular type.
                 *
                 * \ingroup g_visitors
                 * \nosubgrouping
                 */
                template <typename A_>
                struct Contains
                {
                    /**
                     * Do we contain the queried type?
                     */
                    enum {
                        value =
                            (tr1::is_same<A_, typename H_::ContainedItem1>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem2>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem3>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem4>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem5>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem6>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem7>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem8>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem9>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem10>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem11>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem12>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem13>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem14>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem15>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem16>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem17>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem18>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem19>::value ? 1 : 0) |
                            (tr1::is_same<A_, typename H_::ContainedItem20>::value ? 1 : 0)
                    };
                };

                ///\}
        };

        /**
         * For use by get_const_item.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename I_, typename H_, typename T_>
        struct GetConstItemVisits;

        /**
         * For use by get_const_item.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename I_, typename H_, unsigned u_>
        struct PALUDIS_VISIBLE GetConstItemVisits<I_, H_, const NoType<u_> >
        {
        };

        /**
         * For use by get_const_item.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename I_, typename T_, typename H_>
        struct PALUDIS_VISIBLE GetConstItemVisits<I_, H_, const TreeLeaf<H_, T_> > :
            virtual visitor_internals::Visits<const TreeLeaf<H_, T_> >
        {
            ///\name Visitor operations
            ///\{

            void visit_leaf(const T_ & t);

            ///\}
        };

        /**
         * For use by get_const_item.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename I_, typename H_, typename T_>
        struct PALUDIS_VISIBLE GetConstItemVisits<I_, H_, const ConstTreeSequence<H_, T_> > :
            virtual visitor_internals::Visits<const ConstTreeSequence<H_, T_> >
        {
            ///\name Visitor operations
            ///\{

            void visit_sequence(const T_ & t,
                    typename H_::ConstSequenceIterator,
                    typename H_::ConstSequenceIterator);

            ///\}
        };

        /**
         * For use by get_const_item.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename I_, typename H_, typename T_>
        struct PALUDIS_VISIBLE GetConstItemVisits<I_, H_, const TreeSequence<H_, T_> > :
            virtual visitor_internals::Visits<const TreeSequence<H_, T_> >
        {
            ///\name Visitor operations
            ///\{

            void visit_sequence(const T_ & t,
                    typename H_::ConstSequenceIterator,
                    typename H_::ConstSequenceIterator);

            ///\}
        };

        /**
         * For use by get_const_item.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
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
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem9>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem10>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem11>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem12>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem13>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem14>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem15>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem16>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem17>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem18>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem19>,
            GetConstItemVisits<GetConstItemVisitor<I_>, typename I_::Heirarchy, const typename I_::Heirarchy::ContainedItem20>
        {
            ///\name Visitor operations
            ///\{

            const typename I_::Heirarchy::BasicNode * item;

            GetConstItemVisitor();

            ///\}
        };

        /**
         * Given a heirarchy node, fetch the associated item.
         *
         * \ingroup g_visitors
         * \nosubgrouping
         */
        template <typename I_>
        const typename I_::Heirarchy::BasicNode *
        get_const_item(const I_ & i);
    }

    /**
     * Used by accept_visitor.
     *
     * \nosubgrouping
     * \ingroup g_visitors
     */
    template <typename Visitor_>
    class PALUDIS_VISIBLE AcceptVisitor
    {
        private:
            Visitor_ & _v;

        public:
            typedef void result_type;

            ///\name Visitor operations
            ///\{

            AcceptVisitor(Visitor_ & v) :
                _v(v)
            {
            }

            template <typename T_>
            void operator() (T_ & t) const
            {
                t.accept(_v);
            }

            ///\}
    };

    /**
     * Convenience function for using a visitor with a standard algorithm.
     *
     * \ingroup g_visitors
     */
    template <typename Visitor_>
    AcceptVisitor<Visitor_> PALUDIS_VISIBLE accept_visitor(Visitor_ & v)
    {
        return AcceptVisitor<Visitor_>(v);
    }
}

#endif
