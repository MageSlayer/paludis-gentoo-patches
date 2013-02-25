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

#ifndef PALUDIS_GUARD_PALUDIS_SPEC_TREE_FWD_HH
#define PALUDIS_GUARD_PALUDIS_SPEC_TREE_FWD_HH 1

#include <paludis/util/type_list.hh>
#include <paludis/dep_spec-fwd.hh>

namespace paludis
{
    template <typename T_>
    struct SpecTreeInnerNodeType;

    template <typename T_>
    struct SpecTreeLeafNodeType;

    namespace spec_tree_internals
    {
        template <typename Tree_>
        class BasicNode;

        template <typename Tree_, typename Item_>
        class LeafNode;

        template <typename Tree_, typename Item_>
        class InnerNode;

        template <typename Tree_>
        class BasicInnerNode;

        template <typename Tree_>
        struct BasicInnerNodeConstIteratorTag;

        template <typename Tree_, typename NodeList_>
        struct MakeVisitableTypeListEntry;

        template <typename Tree_>
        struct MakeVisitableTypeListEntry<Tree_, TypeListTail>;

        template <typename Tree_, typename Item_, typename Tail_>
        struct MakeVisitableTypeListEntry<Tree_, TypeListEntry<SpecTreeInnerNodeType<Item_>, Tail_> >;

        template <typename Tree_, typename Item_, typename Tail_>
        struct MakeVisitableTypeListEntry<Tree_, TypeListEntry<SpecTreeLeafNodeType<Item_>, Tail_> >;

        template <typename Tree_, typename NodeList_>
        struct MakeVisitableTypeList;

        template <typename Tree_, typename Node_>
        struct TreeCannotContainNodeType;
    }

    template <typename NodeList_, typename RootNode_>
    class SpecTree;

    typedef SpecTree<MakeTypeList<
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
        >::Type, AllDepSpec> GenericSpecTree;

    typedef SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<LicenseDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<AnyDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec> LicenseSpecTree;

    typedef SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<FetchableURIDepSpec>,
            SpecTreeLeafNodeType<URILabelsDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec> FetchableURISpecTree;

    typedef SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<SimpleURIDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec> SimpleURISpecTree;

    typedef SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<PlainTextDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<AnyDepSpec>,
            SpecTreeInnerNodeType<ExactlyOneDepSpec>,
            SpecTreeInnerNodeType<AtMostOneDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec> RequiredUseSpecTree;

    typedef SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<PlainTextDepSpec>,
            SpecTreeLeafNodeType<PlainTextLabelDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec> PlainTextSpecTree;

    typedef SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<PackageDepSpec>,
            SpecTreeLeafNodeType<BlockDepSpec>,
            SpecTreeLeafNodeType<DependenciesLabelsDepSpec>,
            SpecTreeLeafNodeType<NamedSetDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>,
            SpecTreeInnerNodeType<AnyDepSpec>,
            SpecTreeInnerNodeType<ConditionalDepSpec>
        >::Type, AllDepSpec> DependencySpecTree;

    typedef SpecTree<MakeTypeList<
            SpecTreeLeafNodeType<PackageDepSpec>,
            SpecTreeLeafNodeType<NamedSetDepSpec>,
            SpecTreeInnerNodeType<AllDepSpec>
        >::Type, AllDepSpec> SetSpecTree;
}

#endif
