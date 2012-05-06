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

#include <paludis/spec_tree.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::spec_tree_internals;

template <typename Tree_, typename Item_>
LeafNode<Tree_, Item_>::LeafNode(const std::shared_ptr<const Item_> & i) :
    _spec(i)
{
}

template <typename Tree_, typename Item_>
template <typename OtherTree_>
LeafNode<Tree_, Item_>::operator LeafNode<OtherTree_, Item_> () const
{
    return LeafNode<OtherTree_, Item_>(_spec);
}

template <typename Tree_, typename Item_>
const std::shared_ptr<const Item_>
LeafNode<Tree_, Item_>::spec() const
{
    return _spec;
}

template <typename Tree_>
BasicInnerNode<Tree_>::BasicInnerNode() :
    _child_list(std::make_shared<ChildList>())
{
}

template <typename Tree_>
typename BasicInnerNode<Tree_>::ConstIterator
BasicInnerNode<Tree_>::begin() const
{
    return ConstIterator(_child_list->begin());
}

template <typename Tree_>
typename BasicInnerNode<Tree_>::ConstIterator
BasicInnerNode<Tree_>::end() const
{
    return ConstIterator(_child_list->end());
}

template <typename Tree_>
void
BasicInnerNode<Tree_>::append_node(const std::shared_ptr<const BasicNode<Tree_> > & t)
{
    _child_list->push_back(t);
}

template <typename Tree_>
template <typename T_>
const std::shared_ptr<typename Tree_::template NodeType<T_>::Type>
BasicInnerNode<Tree_>::append(const std::shared_ptr<const T_> & t)
{
    const std::shared_ptr<typename Tree_::template NodeType<T_>::Type> tt(
            std::make_shared<typename Tree_::template NodeType<T_>::Type>(t));
    append_node(tt);
    return tt;
}

template <typename Tree_>
template <typename T_>
const std::shared_ptr<typename Tree_::template NodeType<T_>::Type>
BasicInnerNode<Tree_>::append(const std::shared_ptr<T_> & t)
{
    return append(std::shared_ptr<const T_>(t));
}

template <typename Tree_, typename Item_>
InnerNode<Tree_, Item_>::InnerNode(const std::shared_ptr<const Item_> & i) :
    _spec(i)
{
}

template <typename Tree_, typename Item_>
const std::shared_ptr<const Item_>
InnerNode<Tree_, Item_>::spec() const
{
    return _spec;
}

template <typename NodeList_, typename RootNode_>
SpecTree<NodeList_, RootNode_>::SpecTree(const std::shared_ptr<RootNode_> & spec) :
    _top(std::make_shared<typename InnerNodeType<RootNode_>::Type>(spec))
{
}

template <typename NodeList_, typename RootNode_>
SpecTree<NodeList_, RootNode_>::SpecTree(const std::shared_ptr<const RootNode_> & spec) :
    _top(std::make_shared<typename InnerNodeType<RootNode_>::Type>(spec))
{
}

template <typename NodeList_, typename RootNode_>
const std::shared_ptr<typename SpecTree<NodeList_, RootNode_>::template InnerNodeType<RootNode_>::Type>
SpecTree<NodeList_, RootNode_>::top()
{
    return _top;
}

template <typename NodeList_, typename RootNode_>
const std::shared_ptr<const typename SpecTree<NodeList_, RootNode_>::template InnerNodeType<RootNode_>::Type>
SpecTree<NodeList_, RootNode_>::top() const
{
    return _top;
}

namespace
{
    template <typename Tree_, typename OtherTree_, typename Item_>
    struct InnerNodeCopier
    {
        InnerNode<OtherTree_, Item_> result;

        InnerNodeCopier(const std::shared_ptr<const Item_> & i) :
            result(i)
        {
        }

        template <typename T_>
        void visit(const LeafNode<Tree_, T_> & n)
        {
            result.append(n.spec());
        }

        template <typename T_>
        void visit(const InnerNode<Tree_, T_> & n)
        {
            result.append_node(std::make_shared<InnerNode<OtherTree_, T_>>(n));
        }
    };
}

template <typename Tree_, typename Item_>
template <typename OtherTree_>
InnerNode<Tree_, Item_>::operator InnerNode<OtherTree_, Item_> () const
{
    InnerNodeCopier<Tree_, OtherTree_, Item_> copier(this->spec());
    std::for_each(indirect_iterator(this->begin()), indirect_iterator(this->end()), accept_visitor(copier));
    return copier.result;
}

namespace paludis
{
    template <typename T_>
    struct WrappedForwardIteratorTraits<BasicInnerNodeConstIteratorTag<T_> >
    {
        typedef typename Sequence<std::shared_ptr<const BasicNode<T_> > >::ConstIterator UnderlyingIterator;
    };
}

namespace paludis
{
    template class SpecTree<MakeTypeList<
        SpecTreeLeafNodeType<PlainTextDepSpec>,
            SpecTreeLeafNodeType<PlainTextLabelDepSpec>,
            SpecTreeLeafNodeType<SimpleURIDepSpec>,
            SpecTreeLeafNodeType<FetchableURIDepSpec>,
            SpecTreeLeafNodeType<LicenseDepSpec>,
            SpecTreeLeafNodeType<PackageDepSpec>,
            SpecTreeLeafNodeType<BlockDepSpec>,
            SpecTreeLeafNodeType<URILabelsDepSpec>,
            SpecTreeLeafNodeType<DependenciesLabelsDepSpec>,
            SpecTreeLeafNodeType<NamedSetDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<AnyDepSpec>,
            SpecTreeInnerNodeType<ExactlyOneDepSpec>,
            SpecTreeInnerNodeType<AtMostOneDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec>;

    template class SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<LicenseDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<AnyDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec>;

    template class SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<FetchableURIDepSpec>,
            SpecTreeLeafNodeType<URILabelsDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec>;

    template class SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<SimpleURIDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec>;

    template class SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<PackageDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec>;

    template class SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<PlainTextDepSpec>,
            SpecTreeLeafNodeType<PlainTextLabelDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec>;

    template class SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<PlainTextDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<AnyDepSpec>,
            SpecTreeInnerNodeType<ExactlyOneDepSpec>,
            SpecTreeInnerNodeType<AtMostOneDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec>;

    template class SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<PackageDepSpec>,
            SpecTreeLeafNodeType<BlockDepSpec>,
            SpecTreeLeafNodeType<DependenciesLabelsDepSpec>,
            SpecTreeLeafNodeType<NamedSetDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<AnyDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec>;

    template class SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<PackageDepSpec>,
            SpecTreeLeafNodeType<NamedSetDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>
        >::Type, AllDepSpec>;

    /* GenericSpecTree */

    template const std::shared_ptr<GenericSpecTree::NodeType<PackageDepSpec>::Type>
        BasicInnerNode<GenericSpecTree>::append<PackageDepSpec>(const std::shared_ptr<const PackageDepSpec> &);
    template const std::shared_ptr<GenericSpecTree::NodeType<PackageDepSpec>::Type>
        BasicInnerNode<GenericSpecTree>::append<PackageDepSpec>(const std::shared_ptr<PackageDepSpec> &);

    template const std::shared_ptr<GenericSpecTree::NodeType<NamedSetDepSpec>::Type>
        BasicInnerNode<GenericSpecTree>::append<NamedSetDepSpec>(const std::shared_ptr<const NamedSetDepSpec> &);
    template const std::shared_ptr<GenericSpecTree::NodeType<NamedSetDepSpec>::Type>
        BasicInnerNode<GenericSpecTree>::append<NamedSetDepSpec>(const std::shared_ptr<NamedSetDepSpec> &);

    namespace spec_tree_internals
    {
        template class BasicInnerNode<GenericSpecTree>;
    }

    template class WrappedForwardIterator<BasicInnerNode<GenericSpecTree>::ConstIteratorTag,
             const std::shared_ptr<const BasicNode<GenericSpecTree> > >;

    namespace spec_tree_internals
    {
        template class PALUDIS_VISIBLE LeafNode<GenericSpecTree, PackageDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<GenericSpecTree, BlockDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<GenericSpecTree, DependenciesLabelsDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<GenericSpecTree, NamedSetDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<GenericSpecTree, PlainTextDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<GenericSpecTree, PlainTextLabelDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<GenericSpecTree, SimpleURIDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<GenericSpecTree, FetchableURIDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<GenericSpecTree, LicenseDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<GenericSpecTree, URILabelsDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<GenericSpecTree, AllDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<GenericSpecTree, AnyDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<GenericSpecTree, ExactlyOneDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<GenericSpecTree, AtMostOneDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<GenericSpecTree, ConditionalDepSpec>;
    }
}

/* DependencySpecTree */

template const std::shared_ptr<DependencySpecTree::NodeType<PackageDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<PackageDepSpec>(const std::shared_ptr<const PackageDepSpec> &);
template const std::shared_ptr<DependencySpecTree::NodeType<PackageDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<PackageDepSpec>(const std::shared_ptr<PackageDepSpec> &);

template const std::shared_ptr<DependencySpecTree::NodeType<NamedSetDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<NamedSetDepSpec>(const std::shared_ptr<const NamedSetDepSpec> &);
template const std::shared_ptr<DependencySpecTree::NodeType<NamedSetDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<NamedSetDepSpec>(const std::shared_ptr<NamedSetDepSpec> &);

template const std::shared_ptr<DependencySpecTree::NodeType<BlockDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<BlockDepSpec>(const std::shared_ptr<const BlockDepSpec> &);
template const std::shared_ptr<DependencySpecTree::NodeType<BlockDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<BlockDepSpec>(const std::shared_ptr<BlockDepSpec> &);

template const std::shared_ptr<DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<DependenciesLabelsDepSpec>(const std::shared_ptr<const DependenciesLabelsDepSpec> &);
template const std::shared_ptr<DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<DependenciesLabelsDepSpec>(const std::shared_ptr<DependenciesLabelsDepSpec> &);

template const std::shared_ptr<DependencySpecTree::NodeType<AnyDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<AnyDepSpec>(const std::shared_ptr<const AnyDepSpec> &);
template const std::shared_ptr<DependencySpecTree::NodeType<AnyDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<AnyDepSpec>(const std::shared_ptr<AnyDepSpec> &);

template const std::shared_ptr<DependencySpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<AllDepSpec>(const std::shared_ptr<const AllDepSpec> &);
template const std::shared_ptr<DependencySpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<AllDepSpec>(const std::shared_ptr<AllDepSpec> &);

template const std::shared_ptr<DependencySpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<const ConditionalDepSpec> &);
template const std::shared_ptr<DependencySpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<DependencySpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<ConditionalDepSpec> &);

namespace paludis
{
    template class WrappedForwardIterator<BasicInnerNode<DependencySpecTree>::ConstIteratorTag,
             const std::shared_ptr<const BasicNode<DependencySpecTree> > >;

    namespace spec_tree_internals
    {
        template class BasicInnerNode<DependencySpecTree>;

        template class PALUDIS_VISIBLE LeafNode<DependencySpecTree, PackageDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<DependencySpecTree, BlockDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<DependencySpecTree, DependenciesLabelsDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<DependencySpecTree, NamedSetDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<DependencySpecTree, AllDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<DependencySpecTree, AnyDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<DependencySpecTree, ConditionalDepSpec>;
    }
}
/* SetSpecTree */

template const std::shared_ptr<SetSpecTree::NodeType<PackageDepSpec>::Type>
    BasicInnerNode<SetSpecTree>::append<PackageDepSpec>(const std::shared_ptr<const PackageDepSpec> &);
template const std::shared_ptr<SetSpecTree::NodeType<PackageDepSpec>::Type>
    BasicInnerNode<SetSpecTree>::append<PackageDepSpec>(const std::shared_ptr<PackageDepSpec> &);

template const std::shared_ptr<SetSpecTree::NodeType<NamedSetDepSpec>::Type>
    BasicInnerNode<SetSpecTree>::append<NamedSetDepSpec>(const std::shared_ptr<const NamedSetDepSpec> &);
template const std::shared_ptr<SetSpecTree::NodeType<NamedSetDepSpec>::Type>
    BasicInnerNode<SetSpecTree>::append<NamedSetDepSpec>(const std::shared_ptr<NamedSetDepSpec> &);

template const std::shared_ptr<SetSpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<SetSpecTree>::append<AllDepSpec>(const std::shared_ptr<const AllDepSpec> &);
template const std::shared_ptr<SetSpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<SetSpecTree>::append<AllDepSpec>(const std::shared_ptr<AllDepSpec> &);

namespace paludis
{
    template class WrappedForwardIterator<BasicInnerNode<SetSpecTree>::ConstIteratorTag,
             const std::shared_ptr<const BasicNode<SetSpecTree> > >;

    namespace spec_tree_internals
    {
        template class BasicInnerNode<SetSpecTree>;
        template class PALUDIS_VISIBLE LeafNode<SetSpecTree, PackageDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<SetSpecTree, NamedSetDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<SetSpecTree, AllDepSpec>;
    }
}

/* PlainTextSpecTree */

template const std::shared_ptr<PlainTextSpecTree::NodeType<PlainTextDepSpec>::Type>
    BasicInnerNode<PlainTextSpecTree>::append<PlainTextDepSpec>(const std::shared_ptr<const PlainTextDepSpec> &);
template const std::shared_ptr<PlainTextSpecTree::NodeType<PlainTextDepSpec>::Type>
    BasicInnerNode<PlainTextSpecTree>::append<PlainTextDepSpec>(const std::shared_ptr<PlainTextDepSpec> &);

template const std::shared_ptr<PlainTextSpecTree::NodeType<PlainTextLabelDepSpec>::Type>
    BasicInnerNode<PlainTextSpecTree>::append<PlainTextLabelDepSpec>(const std::shared_ptr<const PlainTextLabelDepSpec> &);
template const std::shared_ptr<PlainTextSpecTree::NodeType<PlainTextLabelDepSpec>::Type>
    BasicInnerNode<PlainTextSpecTree>::append<PlainTextLabelDepSpec>(const std::shared_ptr<PlainTextLabelDepSpec> &);

template const std::shared_ptr<PlainTextSpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<PlainTextSpecTree>::append<AllDepSpec>(const std::shared_ptr<const AllDepSpec> &);
template const std::shared_ptr<PlainTextSpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<PlainTextSpecTree>::append<AllDepSpec>(const std::shared_ptr<AllDepSpec> &);

template const std::shared_ptr<PlainTextSpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<PlainTextSpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<const ConditionalDepSpec> &);
template const std::shared_ptr<PlainTextSpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<PlainTextSpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<ConditionalDepSpec> &);

namespace paludis
{
    template class WrappedForwardIterator<BasicInnerNode<PlainTextSpecTree>::ConstIteratorTag,
             const std::shared_ptr<const BasicNode<PlainTextSpecTree> > >;

    namespace spec_tree_internals
    {
        template class BasicInnerNode<PlainTextSpecTree>;
        template class PALUDIS_VISIBLE LeafNode<PlainTextSpecTree, PlainTextDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<PlainTextSpecTree, PlainTextLabelDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<PlainTextSpecTree, AllDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<PlainTextSpecTree, ConditionalDepSpec>;
    }
}
/* RequiredUseSpecTree */

template const std::shared_ptr<RequiredUseSpecTree::NodeType<PlainTextDepSpec>::Type>
    BasicInnerNode<RequiredUseSpecTree>::append<PlainTextDepSpec>(const std::shared_ptr<const PlainTextDepSpec> &);
template const std::shared_ptr<RequiredUseSpecTree::NodeType<PlainTextDepSpec>::Type>
    BasicInnerNode<RequiredUseSpecTree>::append<PlainTextDepSpec>(const std::shared_ptr<PlainTextDepSpec> &);

template const std::shared_ptr<RequiredUseSpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<RequiredUseSpecTree>::append<AllDepSpec>(const std::shared_ptr<const AllDepSpec> &);
template const std::shared_ptr<RequiredUseSpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<RequiredUseSpecTree>::append<AllDepSpec>(const std::shared_ptr<AllDepSpec> &);

template const std::shared_ptr<RequiredUseSpecTree::NodeType<AnyDepSpec>::Type>
    BasicInnerNode<RequiredUseSpecTree>::append<AnyDepSpec>(const std::shared_ptr<const AnyDepSpec> &);
template const std::shared_ptr<RequiredUseSpecTree::NodeType<AnyDepSpec>::Type>
    BasicInnerNode<RequiredUseSpecTree>::append<AnyDepSpec>(const std::shared_ptr<AnyDepSpec> &);

template const std::shared_ptr<RequiredUseSpecTree::NodeType<ExactlyOneDepSpec>::Type>
    BasicInnerNode<RequiredUseSpecTree>::append<ExactlyOneDepSpec>(const std::shared_ptr<const ExactlyOneDepSpec> &);
template const std::shared_ptr<RequiredUseSpecTree::NodeType<ExactlyOneDepSpec>::Type>
    BasicInnerNode<RequiredUseSpecTree>::append<ExactlyOneDepSpec>(const std::shared_ptr<ExactlyOneDepSpec> &);

template const std::shared_ptr<RequiredUseSpecTree::NodeType<AtMostOneDepSpec>::Type>
    BasicInnerNode<RequiredUseSpecTree>::append<AtMostOneDepSpec>(const std::shared_ptr<const AtMostOneDepSpec> &);
template const std::shared_ptr<RequiredUseSpecTree::NodeType<AtMostOneDepSpec>::Type>
    BasicInnerNode<RequiredUseSpecTree>::append<AtMostOneDepSpec>(const std::shared_ptr<AtMostOneDepSpec> &);

template const std::shared_ptr<RequiredUseSpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<RequiredUseSpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<const ConditionalDepSpec> &);
template const std::shared_ptr<RequiredUseSpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<RequiredUseSpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<ConditionalDepSpec> &);

namespace paludis
{
    template class WrappedForwardIterator<BasicInnerNode<RequiredUseSpecTree>::ConstIteratorTag,
             const std::shared_ptr<const BasicNode<RequiredUseSpecTree> > >;

    namespace spec_tree_internals
    {
        template class BasicInnerNode<RequiredUseSpecTree>;
        template class PALUDIS_VISIBLE LeafNode<RequiredUseSpecTree, PlainTextDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<RequiredUseSpecTree, AllDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<RequiredUseSpecTree, AnyDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<RequiredUseSpecTree, ExactlyOneDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<RequiredUseSpecTree, AtMostOneDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<RequiredUseSpecTree, ConditionalDepSpec>;
    }
}
/* SimpleURISpecTree */

template const std::shared_ptr<SimpleURISpecTree::NodeType<SimpleURIDepSpec>::Type>
    BasicInnerNode<SimpleURISpecTree>::append<SimpleURIDepSpec>(const std::shared_ptr<const SimpleURIDepSpec> &);
template const std::shared_ptr<SimpleURISpecTree::NodeType<SimpleURIDepSpec>::Type>
    BasicInnerNode<SimpleURISpecTree>::append<SimpleURIDepSpec>(const std::shared_ptr<SimpleURIDepSpec> &);

template const std::shared_ptr<SimpleURISpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<SimpleURISpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<const ConditionalDepSpec> &);
template const std::shared_ptr<SimpleURISpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<SimpleURISpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<ConditionalDepSpec> &);

template const std::shared_ptr<SimpleURISpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<SimpleURISpecTree>::append<AllDepSpec>(const std::shared_ptr<const AllDepSpec> &);
template const std::shared_ptr<SimpleURISpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<SimpleURISpecTree>::append<AllDepSpec>(const std::shared_ptr<AllDepSpec> &);

namespace paludis
{
    template class WrappedForwardIterator<BasicInnerNode<SimpleURISpecTree>::ConstIteratorTag,
             const std::shared_ptr<const BasicNode<SimpleURISpecTree> > >;

    namespace spec_tree_internals
    {
        template class BasicInnerNode<SimpleURISpecTree>;
        template class PALUDIS_VISIBLE LeafNode<SimpleURISpecTree, SimpleURIDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<SimpleURISpecTree, AllDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<SimpleURISpecTree, ConditionalDepSpec>;
    }
}
/* FetchableURISpecTree */

template const std::shared_ptr<FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type>
    BasicInnerNode<FetchableURISpecTree>::append<FetchableURIDepSpec>(const std::shared_ptr<const FetchableURIDepSpec> &);
template const std::shared_ptr<FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type>
    BasicInnerNode<FetchableURISpecTree>::append<FetchableURIDepSpec>(const std::shared_ptr<FetchableURIDepSpec> &);

template const std::shared_ptr<FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type>
    BasicInnerNode<FetchableURISpecTree>::append<URILabelsDepSpec>(const std::shared_ptr<const URILabelsDepSpec> &);
template const std::shared_ptr<FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type>
    BasicInnerNode<FetchableURISpecTree>::append<URILabelsDepSpec>(const std::shared_ptr<URILabelsDepSpec> &);

template const std::shared_ptr<FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<FetchableURISpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<const ConditionalDepSpec> &);
template const std::shared_ptr<FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<FetchableURISpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<ConditionalDepSpec> &);

template const std::shared_ptr<FetchableURISpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<FetchableURISpecTree>::append<AllDepSpec>(const std::shared_ptr<const AllDepSpec> &);
template const std::shared_ptr<FetchableURISpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<FetchableURISpecTree>::append<AllDepSpec>(const std::shared_ptr<AllDepSpec> &);

namespace paludis
{
    template class WrappedForwardIterator<BasicInnerNode<FetchableURISpecTree>::ConstIteratorTag,
             const std::shared_ptr<const BasicNode<FetchableURISpecTree> > >;

    namespace spec_tree_internals
    {
        template class BasicInnerNode<FetchableURISpecTree>;

        template class PALUDIS_VISIBLE LeafNode<FetchableURISpecTree, FetchableURIDepSpec>;
        template class PALUDIS_VISIBLE LeafNode<FetchableURISpecTree, URILabelsDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<FetchableURISpecTree, AllDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<FetchableURISpecTree, ConditionalDepSpec>;
    }
}

/* LicenseSpecTree */

template const std::shared_ptr<LicenseSpecTree::NodeType<LicenseDepSpec>::Type>
    BasicInnerNode<LicenseSpecTree>::append<LicenseDepSpec>(const std::shared_ptr<const LicenseDepSpec> &);
template const std::shared_ptr<LicenseSpecTree::NodeType<LicenseDepSpec>::Type>
    BasicInnerNode<LicenseSpecTree>::append<LicenseDepSpec>(const std::shared_ptr<LicenseDepSpec> &);

template const std::shared_ptr<LicenseSpecTree::NodeType<AnyDepSpec>::Type>
    BasicInnerNode<LicenseSpecTree>::append<AnyDepSpec>(const std::shared_ptr<const AnyDepSpec> &);
template const std::shared_ptr<LicenseSpecTree::NodeType<AnyDepSpec>::Type>
    BasicInnerNode<LicenseSpecTree>::append<AnyDepSpec>(const std::shared_ptr<AnyDepSpec> &);

template const std::shared_ptr<LicenseSpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<LicenseSpecTree>::append<AllDepSpec>(const std::shared_ptr<const AllDepSpec> &);
template const std::shared_ptr<LicenseSpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<LicenseSpecTree>::append<AllDepSpec>(const std::shared_ptr<AllDepSpec> &);

template const std::shared_ptr<LicenseSpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<LicenseSpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<const ConditionalDepSpec> &);
template const std::shared_ptr<LicenseSpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<LicenseSpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<ConditionalDepSpec> &);

namespace paludis
{
    template class WrappedForwardIterator<BasicInnerNode<LicenseSpecTree>::ConstIteratorTag,
             const std::shared_ptr<const BasicNode<LicenseSpecTree> > >;

    namespace spec_tree_internals
    {
        template class BasicInnerNode<LicenseSpecTree>;

        template class PALUDIS_VISIBLE LeafNode<LicenseSpecTree, LicenseDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<LicenseSpecTree, AllDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<LicenseSpecTree, AnyDepSpec>;
        template class PALUDIS_VISIBLE InnerNode<LicenseSpecTree, ConditionalDepSpec>;
    }
}

/* SetSpecTree -> DependencySpecTree */

template InnerNode<SetSpecTree, AllDepSpec>::operator InnerNode<DependencySpecTree, AllDepSpec> () const;
template LeafNode<SetSpecTree, PackageDepSpec>::operator LeafNode<DependencySpecTree, PackageDepSpec> () const;
template LeafNode<SetSpecTree, NamedSetDepSpec>::operator LeafNode<DependencySpecTree, NamedSetDepSpec> () const;

/* SetSpecTree -> GenericSpecTree */

template InnerNode<SetSpecTree, AllDepSpec>::operator InnerNode<GenericSpecTree, AllDepSpec> () const;
template LeafNode<SetSpecTree, PackageDepSpec>::operator LeafNode<GenericSpecTree, PackageDepSpec> () const;
template LeafNode<SetSpecTree, NamedSetDepSpec>::operator LeafNode<GenericSpecTree, NamedSetDepSpec> () const;

/* DependencySpecTree -> GenericSpecTree */

template InnerNode<DependencySpecTree, AllDepSpec>::operator InnerNode<GenericSpecTree, AllDepSpec> () const;
template InnerNode<DependencySpecTree, AnyDepSpec>::operator InnerNode<GenericSpecTree, AnyDepSpec> () const;
template InnerNode<DependencySpecTree, ConditionalDepSpec>::operator InnerNode<GenericSpecTree, ConditionalDepSpec> () const;
template LeafNode<DependencySpecTree, PackageDepSpec>::operator LeafNode<GenericSpecTree, PackageDepSpec> () const;
template LeafNode<DependencySpecTree, NamedSetDepSpec>::operator LeafNode<GenericSpecTree, NamedSetDepSpec> () const;
template LeafNode<DependencySpecTree, DependenciesLabelsDepSpec>::operator LeafNode<GenericSpecTree, DependenciesLabelsDepSpec> () const;
template LeafNode<DependencySpecTree, BlockDepSpec>::operator LeafNode<GenericSpecTree, BlockDepSpec> () const;

/* LicenseSpecTree -> GenericSpecTree */

template InnerNode<LicenseSpecTree, AllDepSpec>::operator InnerNode<GenericSpecTree, AllDepSpec> () const;
template InnerNode<LicenseSpecTree, AnyDepSpec>::operator InnerNode<GenericSpecTree, AnyDepSpec> () const;
template InnerNode<LicenseSpecTree, ConditionalDepSpec>::operator InnerNode<GenericSpecTree, ConditionalDepSpec> () const;
template LeafNode<LicenseSpecTree, LicenseDepSpec>::operator LeafNode<GenericSpecTree, LicenseDepSpec> () const;

/* SimpleURISpecTree -> GenericSpecTree */

template InnerNode<SimpleURISpecTree, AllDepSpec>::operator InnerNode<GenericSpecTree, AllDepSpec> () const;
template InnerNode<SimpleURISpecTree, ConditionalDepSpec>::operator InnerNode<GenericSpecTree, ConditionalDepSpec> () const;
template LeafNode<SimpleURISpecTree, SimpleURIDepSpec>::operator LeafNode<GenericSpecTree, SimpleURIDepSpec> () const;

/* FetchableURISpecTree -> GenericSpecTree */

template InnerNode<FetchableURISpecTree, AllDepSpec>::operator InnerNode<GenericSpecTree, AllDepSpec> () const;
template InnerNode<FetchableURISpecTree, ConditionalDepSpec>::operator InnerNode<GenericSpecTree, ConditionalDepSpec> () const;
template LeafNode<FetchableURISpecTree, FetchableURIDepSpec>::operator LeafNode<GenericSpecTree, FetchableURIDepSpec> () const;
template LeafNode<FetchableURISpecTree, URILabelsDepSpec>::operator LeafNode<GenericSpecTree, URILabelsDepSpec> () const;

/* PlainTextSpecTree -> GenericSpecTree */

template InnerNode<PlainTextSpecTree, AllDepSpec>::operator InnerNode<GenericSpecTree, AllDepSpec> () const;
template InnerNode<PlainTextSpecTree, ConditionalDepSpec>::operator InnerNode<GenericSpecTree, ConditionalDepSpec> () const;
template LeafNode<PlainTextSpecTree, PlainTextDepSpec>::operator LeafNode<GenericSpecTree, PlainTextDepSpec> () const;
template LeafNode<PlainTextSpecTree, PlainTextLabelDepSpec>::operator LeafNode<GenericSpecTree, PlainTextLabelDepSpec> () const;

/* RequiredUseSpecTree -> GenericSpecTree */

template InnerNode<RequiredUseSpecTree, AllDepSpec>::operator InnerNode<GenericSpecTree, AllDepSpec> () const;
template InnerNode<RequiredUseSpecTree, AnyDepSpec>::operator InnerNode<GenericSpecTree, AnyDepSpec> () const;
template InnerNode<RequiredUseSpecTree, ExactlyOneDepSpec>::operator InnerNode<GenericSpecTree, ExactlyOneDepSpec> () const;
template InnerNode<RequiredUseSpecTree, AtMostOneDepSpec>::operator InnerNode<GenericSpecTree, AtMostOneDepSpec> () const;
template InnerNode<RequiredUseSpecTree, ConditionalDepSpec>::operator InnerNode<GenericSpecTree, ConditionalDepSpec> () const;
template LeafNode<RequiredUseSpecTree, PlainTextDepSpec>::operator LeafNode<GenericSpecTree, PlainTextDepSpec> () const;

