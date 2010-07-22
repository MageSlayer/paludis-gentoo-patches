/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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
    _child_list(new ChildList)
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
    _root(std::make_shared<typename InnerNodeType<RootNode_>::Type>(spec))
{
}

template <typename NodeList_, typename RootNode_>
SpecTree<NodeList_, RootNode_>::SpecTree(const std::shared_ptr<const RootNode_> & spec) :
    _root(std::make_shared<typename InnerNodeType<RootNode_>::Type>(spec))
{
}

template <typename NodeList_, typename RootNode_>
const std::shared_ptr<typename SpecTree<NodeList_, RootNode_>::template InnerNodeType<RootNode_>::Type>
SpecTree<NodeList_, RootNode_>::root()
{
    return _root;
}

template <typename NodeList_, typename RootNode_>
const std::shared_ptr<const typename SpecTree<NodeList_, RootNode_>::template InnerNodeType<RootNode_>::Type>
SpecTree<NodeList_, RootNode_>::root() const
{
    return _root;
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

template class BasicInnerNode<GenericSpecTree>;
template class WrappedForwardIterator<BasicInnerNode<GenericSpecTree>::ConstIteratorTag,
         const std::shared_ptr<const BasicNode<GenericSpecTree> > >;

template class LeafNode<GenericSpecTree, PackageDepSpec>;
template class LeafNode<GenericSpecTree, BlockDepSpec>;
template class LeafNode<GenericSpecTree, DependenciesLabelsDepSpec>;
template class LeafNode<GenericSpecTree, NamedSetDepSpec>;
template class LeafNode<GenericSpecTree, PlainTextDepSpec>;
template class LeafNode<GenericSpecTree, PlainTextLabelDepSpec>;
template class LeafNode<GenericSpecTree, SimpleURIDepSpec>;
template class LeafNode<GenericSpecTree, FetchableURIDepSpec>;
template class LeafNode<GenericSpecTree, LicenseDepSpec>;
template class LeafNode<GenericSpecTree, URILabelsDepSpec>;
template class InnerNode<GenericSpecTree, AllDepSpec>;
template class InnerNode<GenericSpecTree, AnyDepSpec>;
template class InnerNode<GenericSpecTree, ConditionalDepSpec>;

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


template class BasicInnerNode<DependencySpecTree>;
template class WrappedForwardIterator<BasicInnerNode<DependencySpecTree>::ConstIteratorTag,
         const std::shared_ptr<const BasicNode<DependencySpecTree> > >;

template class LeafNode<DependencySpecTree, PackageDepSpec>;
template class LeafNode<DependencySpecTree, BlockDepSpec>;
template class LeafNode<DependencySpecTree, DependenciesLabelsDepSpec>;
template class LeafNode<DependencySpecTree, NamedSetDepSpec>;
template class InnerNode<DependencySpecTree, AllDepSpec>;
template class InnerNode<DependencySpecTree, AnyDepSpec>;
template class InnerNode<DependencySpecTree, ConditionalDepSpec>;

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

template class BasicInnerNode<SetSpecTree>;
template class WrappedForwardIterator<BasicInnerNode<SetSpecTree>::ConstIteratorTag,
         const std::shared_ptr<const BasicNode<SetSpecTree> > >;
template class LeafNode<SetSpecTree, PackageDepSpec>;
template class LeafNode<SetSpecTree, NamedSetDepSpec>;
template class InnerNode<SetSpecTree, AllDepSpec>;

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

template class BasicInnerNode<PlainTextSpecTree>;
template class WrappedForwardIterator<BasicInnerNode<PlainTextSpecTree>::ConstIteratorTag,
         const std::shared_ptr<const BasicNode<PlainTextSpecTree> > >;
template class LeafNode<PlainTextSpecTree, PlainTextDepSpec>;
template class LeafNode<PlainTextSpecTree, PlainTextLabelDepSpec>;
template class InnerNode<PlainTextSpecTree, AllDepSpec>;
template class InnerNode<PlainTextSpecTree, ConditionalDepSpec>;

/* ProvideSpecTree */

template const std::shared_ptr<ProvideSpecTree::NodeType<PackageDepSpec>::Type>
    BasicInnerNode<ProvideSpecTree>::append<PackageDepSpec>(const std::shared_ptr<const PackageDepSpec> &);
template const std::shared_ptr<ProvideSpecTree::NodeType<PackageDepSpec>::Type>
    BasicInnerNode<ProvideSpecTree>::append<PackageDepSpec>(const std::shared_ptr<PackageDepSpec> &);

template const std::shared_ptr<ProvideSpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<ProvideSpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<const ConditionalDepSpec> &);
template const std::shared_ptr<ProvideSpecTree::NodeType<ConditionalDepSpec>::Type>
    BasicInnerNode<ProvideSpecTree>::append<ConditionalDepSpec>(const std::shared_ptr<ConditionalDepSpec> &);

template const std::shared_ptr<ProvideSpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<ProvideSpecTree>::append<AllDepSpec>(const std::shared_ptr<const AllDepSpec> &);
template const std::shared_ptr<ProvideSpecTree::NodeType<AllDepSpec>::Type>
    BasicInnerNode<ProvideSpecTree>::append<AllDepSpec>(const std::shared_ptr<AllDepSpec> &);

template class BasicInnerNode<ProvideSpecTree>;
template class WrappedForwardIterator<BasicInnerNode<ProvideSpecTree>::ConstIteratorTag,
         const std::shared_ptr<const BasicNode<ProvideSpecTree> > >;
template class LeafNode<ProvideSpecTree, PackageDepSpec>;
template class InnerNode<ProvideSpecTree, ConditionalDepSpec>;
template class InnerNode<ProvideSpecTree, AllDepSpec>;

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

template class BasicInnerNode<SimpleURISpecTree>;
template class WrappedForwardIterator<BasicInnerNode<SimpleURISpecTree>::ConstIteratorTag,
         const std::shared_ptr<const BasicNode<SimpleURISpecTree> > >;
template class LeafNode<SimpleURISpecTree, SimpleURIDepSpec>;
template class InnerNode<SimpleURISpecTree, AllDepSpec>;
template class InnerNode<SimpleURISpecTree, ConditionalDepSpec>;

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

template class BasicInnerNode<FetchableURISpecTree>;
template class WrappedForwardIterator<BasicInnerNode<FetchableURISpecTree>::ConstIteratorTag,
         const std::shared_ptr<const BasicNode<FetchableURISpecTree> > >;

template class LeafNode<FetchableURISpecTree, FetchableURIDepSpec>;
template class LeafNode<FetchableURISpecTree, URILabelsDepSpec>;
template class InnerNode<FetchableURISpecTree, AllDepSpec>;
template class InnerNode<FetchableURISpecTree, ConditionalDepSpec>;

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

template class BasicInnerNode<LicenseSpecTree>;
template class WrappedForwardIterator<BasicInnerNode<LicenseSpecTree>::ConstIteratorTag,
         const std::shared_ptr<const BasicNode<LicenseSpecTree> > >;

template class LeafNode<LicenseSpecTree, LicenseDepSpec>;
template class InnerNode<LicenseSpecTree, AllDepSpec>;
template class InnerNode<LicenseSpecTree, AnyDepSpec>;
template class InnerNode<LicenseSpecTree, ConditionalDepSpec>;

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

/* ProvideSpecTree -> GenericSpecTree */

template InnerNode<ProvideSpecTree, AllDepSpec>::operator InnerNode<GenericSpecTree, AllDepSpec> () const;
template InnerNode<ProvideSpecTree, ConditionalDepSpec>::operator InnerNode<GenericSpecTree, ConditionalDepSpec> () const;
template LeafNode<ProvideSpecTree, PackageDepSpec>::operator LeafNode<GenericSpecTree, PackageDepSpec> () const;

