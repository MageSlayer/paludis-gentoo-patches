/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SPEC_TREE_HH
#define PALUDIS_GUARD_PALUDIS_SPEC_TREE_HH 1

#include <paludis/spec_tree-fwd.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/sequence.hh>
#include <type_traits>

namespace paludis
{
    namespace spec_tree_internals
    {
        template <typename Tree_>
        class PALUDIS_VISIBLE BasicNode :
            public virtual DeclareAbstractAcceptMethods<BasicNode<Tree_>, typename Tree_::VisitableTypeList>
        {
        };

        template <typename Tree_, typename Item_>
        class PALUDIS_VISIBLE LeafNode :
            public BasicNode<Tree_>,
            public ImplementAcceptMethods<BasicNode<Tree_>, LeafNode<Tree_, Item_> >
        {
            private:
                const std::shared_ptr<const Item_> _spec;

            public:
                explicit LeafNode(const std::shared_ptr<const Item_> & i);

                template <typename OtherTree_>
                operator LeafNode<OtherTree_, Item_> () const;

                const std::shared_ptr<const Item_> spec() const;
        };

        template <typename Tree_, typename Item_>
        struct InnerNode;

        template <typename Tree_>
        class PALUDIS_VISIBLE BasicInnerNode :
            public BasicNode<Tree_>
        {
            private:
                typedef Sequence<std::shared_ptr<const BasicNode<Tree_> > > ChildList;
                std::shared_ptr<ChildList> _child_list;

            public:
                BasicInnerNode();

                typedef BasicInnerNodeConstIteratorTag<Tree_> ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag,
                        const std::shared_ptr<const BasicNode<Tree_> > > ConstIterator;

                ConstIterator begin() const;

                ConstIterator end() const;

                void append_node(const std::shared_ptr<const BasicNode<Tree_> > & t);

                template <typename T_>
                const std::shared_ptr<typename Tree_::template NodeType<T_>::Type>
                append(const std::shared_ptr<const T_> & t);

                template <typename T_>
                const std::shared_ptr<typename Tree_::template NodeType<T_>::Type>
                append(const std::shared_ptr<T_> & t);
        };

        template <typename Tree_, typename Item_>
        class PALUDIS_VISIBLE InnerNode :
            public BasicInnerNode<Tree_>,
            public ImplementAcceptMethods<BasicNode<Tree_>, InnerNode<Tree_, Item_> >
        {
            private:
                const std::shared_ptr<const Item_> _spec;

            public:
                explicit InnerNode(const std::shared_ptr<const Item_> & i);

                template <typename OtherTree_>
                operator InnerNode<OtherTree_, Item_> () const;

                const std::shared_ptr<const Item_> spec() const;
        };

        template <typename Tree_>
        struct MakeVisitableTypeListEntry<Tree_, TypeListTail>
        {
            typedef TypeListTail Type;
        };

        template <typename Tree_, typename Item_, typename Tail_>
        struct MakeVisitableTypeListEntry<Tree_, TypeListEntry<SpecTreeInnerNodeType<Item_>, Tail_> >
        {
            typedef TypeListEntry<InnerNode<Tree_, Item_>,
                    typename MakeVisitableTypeListEntry<Tree_, Tail_>::Type> Type;
        };

        template <typename Tree_, typename Item_, typename Tail_>
        struct MakeVisitableTypeListEntry<Tree_, TypeListEntry<SpecTreeLeafNodeType<Item_>, Tail_> >
        {
            typedef TypeListEntry<LeafNode<Tree_, Item_>,
                    typename MakeVisitableTypeListEntry<Tree_, Tail_>::Type> Type;
        };

        template <typename Tree_, typename NodeList_>
        struct MakeVisitableTypeList
        {
            typedef typename MakeVisitableTypeListEntry<Tree_, NodeList_>::Type Type;
        };
    }

    template <typename NodeList_, typename RootNode_>
    class PALUDIS_VISIBLE SpecTree
    {
        public:
            typedef typename spec_tree_internals::MakeVisitableTypeList<SpecTree, NodeList_>::Type VisitableTypeList;

            typedef spec_tree_internals::BasicNode<SpecTree> BasicNode;
            typedef spec_tree_internals::BasicInnerNode<SpecTree> BasicInnerNode;

            template <typename Node_>
            struct LeafNodeType
            {
                typedef spec_tree_internals::LeafNode<SpecTree, Node_> Type;
            };

            template <typename Node_>
            struct InnerNodeType
            {
                typedef spec_tree_internals::InnerNode<SpecTree, Node_> Type;
            };

            template <typename Node_>
            struct NodeType
            {
                typedef typename std::conditional<
                    TypeListContains<VisitableTypeList, typename LeafNodeType<Node_>::Type>::value,
                    typename SpecTree::template LeafNodeType<Node_>::Type,
                    typename std::conditional<
                        TypeListContains<VisitableTypeList, typename InnerNodeType<Node_>::Type>::value,
                        typename InnerNodeType<Node_>::Type,
                        spec_tree_internals::TreeCannotContainNodeType<SpecTree, Node_>
                            >::type
                    >::type Type;
            };

            explicit SpecTree(const std::shared_ptr<RootNode_> & spec);

            explicit SpecTree(const std::shared_ptr<const RootNode_> & spec);

            const std::shared_ptr<typename InnerNodeType<RootNode_>::Type> top();

            const std::shared_ptr<const typename InnerNodeType<RootNode_>::Type> top() const;

        private:
            const std::shared_ptr<typename InnerNodeType<RootNode_>::Type> _top;
    };

    extern template class WrappedForwardIterator<spec_tree_internals::BasicInnerNode<GenericSpecTree>::ConstIteratorTag,
           const std::shared_ptr<const spec_tree_internals::BasicNode<GenericSpecTree> > >;
    extern template class WrappedForwardIterator<spec_tree_internals::BasicInnerNode<DependencySpecTree>::ConstIteratorTag,
           const std::shared_ptr<const spec_tree_internals::BasicNode<DependencySpecTree> > >;
    extern template class WrappedForwardIterator<spec_tree_internals::BasicInnerNode<SetSpecTree>::ConstIteratorTag,
             const std::shared_ptr<const spec_tree_internals::BasicNode<SetSpecTree> > >;
    extern template class WrappedForwardIterator<spec_tree_internals::BasicInnerNode<PlainTextSpecTree>::ConstIteratorTag,
             const std::shared_ptr<const spec_tree_internals::BasicNode<PlainTextSpecTree> > >;
    extern template class WrappedForwardIterator<spec_tree_internals::BasicInnerNode<RequiredUseSpecTree>::ConstIteratorTag,
             const std::shared_ptr<const spec_tree_internals::BasicNode<RequiredUseSpecTree> > >;
    extern template class WrappedForwardIterator<spec_tree_internals::BasicInnerNode<ProvideSpecTree>::ConstIteratorTag,
             const std::shared_ptr<const spec_tree_internals::BasicNode<ProvideSpecTree> > >;
    extern template class WrappedForwardIterator<spec_tree_internals::BasicInnerNode<SimpleURISpecTree>::ConstIteratorTag,
             const std::shared_ptr<const spec_tree_internals::BasicNode<SimpleURISpecTree> > >;
    extern template class WrappedForwardIterator<spec_tree_internals::BasicInnerNode<FetchableURISpecTree>::ConstIteratorTag,
             const std::shared_ptr<const spec_tree_internals::BasicNode<FetchableURISpecTree> > >;
    extern template class WrappedForwardIterator<spec_tree_internals::BasicInnerNode<LicenseSpecTree>::ConstIteratorTag,
             const std::shared_ptr<const spec_tree_internals::BasicNode<LicenseSpecTree> > >;

}

#endif
