/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_TREE_HH
#define PALUDIS_GUARD_PALUDIS_DEP_TREE_HH 1

#include <paludis/dep_tree-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/util/visitor.hh>

namespace paludis
{
    /**
     * A generic DepSpec heirarchy.
     *
     * \ingroup grpdepspecs
     */
    struct GenericSpecTree :
        VisitorTypes<
            GenericSpecTree,
            DepSpec,
            TreeLeaf<GenericSpecTree, PlainTextDepSpec>,
            TreeLeaf<GenericSpecTree, SimpleURIDepSpec>,
            TreeLeaf<GenericSpecTree, FetchableURIDepSpec>,
            TreeLeaf<GenericSpecTree, LicenseDepSpec>,
            TreeLeaf<GenericSpecTree, PackageDepSpec>,
            TreeLeaf<GenericSpecTree, BlockDepSpec>,
            TreeLeaf<GenericSpecTree, URILabelsDepSpec>,
            TreeLeaf<GenericSpecTree, DependencyLabelsDepSpec>,
            ConstTreeSequence<GenericSpecTree, AllDepSpec>,
            ConstTreeSequence<GenericSpecTree, AnyDepSpec>,
            ConstTreeSequence<GenericSpecTree, UseDepSpec>
        >
    {
        typedef Formatter<
            UseDepSpec,
            PlainTextDepSpec,
            SimpleURIDepSpec,
            FetchableURIDepSpec,
            LicenseDepSpec,
            PackageDepSpec,
            BlockDepSpec,
            URILabelsDepSpec,
            DependencyLabelsDepSpec
                > Formatter;
    };

    /**
     * A DepSpec heirarchy containing things meaningful for licenses.
     *
     * \ingroup grpdepspecs
     */
    struct LicenseSpecTree :
        VisitorTypes<
            LicenseSpecTree,
            DepSpec,
            TreeLeaf<LicenseSpecTree, LicenseDepSpec>,
            ConstTreeSequence<LicenseSpecTree, AllDepSpec>,
            ConstTreeSequence<LicenseSpecTree, AnyDepSpec>,
            ConstTreeSequence<LicenseSpecTree, UseDepSpec>
        >
    {
        typedef Formatter<
            UseDepSpec,
            LicenseDepSpec
                > Formatter;
    };

    /**
     * A DepSpec heirarchy containing things meaningful for fetchable URIs.
     *
     * \ingroup grpdepspecs
     */
    struct FetchableURISpecTree :
        VisitorTypes<
            FetchableURISpecTree,
            DepSpec,
            TreeLeaf<FetchableURISpecTree, FetchableURIDepSpec>,
            TreeLeaf<FetchableURISpecTree, LabelsDepSpec<URILabelVisitorTypes> >,
            ConstTreeSequence<FetchableURISpecTree, AllDepSpec>,
            ConstTreeSequence<FetchableURISpecTree, UseDepSpec>
        >
    {
        typedef Formatter<
            UseDepSpec,
            FetchableURIDepSpec,
            URILabelsDepSpec
                > Formatter;
    };

    /**
     * A DepSpec heirarchy containing things meaningful for simple URIs.
     *
     * \ingroup grpdepspecs
     */
    struct SimpleURISpecTree :
        VisitorTypes<
            SimpleURISpecTree,
            DepSpec,
            TreeLeaf<SimpleURISpecTree, SimpleURIDepSpec>,
            ConstTreeSequence<SimpleURISpecTree, AllDepSpec>,
            ConstTreeSequence<SimpleURISpecTree, UseDepSpec>
        >
    {
        typedef Formatter<
            UseDepSpec,
            SimpleURIDepSpec
                > Formatter;
    };

    /**
     * A DepSpec heirarchy containing things that can be flattened.
     *
     * \ingroup grpdepspecs
     */
    struct FlattenableSpecTree :
        VisitorTypes<
            FlattenableSpecTree,
            DepSpec,
            TreeLeaf<FlattenableSpecTree, PlainTextDepSpec>,
            TreeLeaf<FlattenableSpecTree, SimpleURIDepSpec>,
            TreeLeaf<FlattenableSpecTree, FetchableURIDepSpec>,
            TreeLeaf<FlattenableSpecTree, PackageDepSpec>,
            TreeLeaf<FlattenableSpecTree, BlockDepSpec>,
            TreeLeaf<FlattenableSpecTree, LicenseDepSpec>,
            ConstTreeSequence<FlattenableSpecTree, AllDepSpec>,
            ConstTreeSequence<FlattenableSpecTree, UseDepSpec>
        >
    {
        typedef Formatter<
            UseDepSpec,
            PlainTextDepSpec,
            SimpleURIDepSpec,
            FetchableURIDepSpec,
            LicenseDepSpec,
            PackageDepSpec,
            BlockDepSpec
                > Formatter;
    };

    /**
     * A DepSpec heirarchy containing things meaningful for provides.
     *
     * \ingroup grpdepspecs
     */
    struct ProvideSpecTree :
        VisitorTypes<
            ProvideSpecTree,
            DepSpec,
            TreeLeaf<ProvideSpecTree, PackageDepSpec>,
            ConstTreeSequence<ProvideSpecTree, AllDepSpec>,
            ConstTreeSequence<ProvideSpecTree, UseDepSpec>
        >
    {
        typedef Formatter<
            UseDepSpec,
            PackageDepSpec
                > Formatter;
    };

    /**
     * A DepSpec heirarchy containing things meaningful for restricts.
     *
     * \ingroup grpdepspecs
     */
    struct RestrictSpecTree :
        VisitorTypes<
            RestrictSpecTree,
            DepSpec,
            TreeLeaf<RestrictSpecTree, PlainTextDepSpec>,
            ConstTreeSequence<RestrictSpecTree, AllDepSpec>,
            ConstTreeSequence<RestrictSpecTree, UseDepSpec>
        >
    {
        typedef Formatter<
            UseDepSpec,
            PlainTextDepSpec
                > Formatter;
    };

    /**
     * A DepSpec heirarchy containing things meaningful for dependencies.
     *
     * \ingroup grpdepspecs
     */
    struct DependencySpecTree :
        VisitorTypes<
            DependencySpecTree,
            DepSpec,
            TreeLeaf<DependencySpecTree, PackageDepSpec>,
            TreeLeaf<DependencySpecTree, BlockDepSpec>,
            TreeLeaf<DependencySpecTree, DependencyLabelsDepSpec>,
            ConstTreeSequence<DependencySpecTree, AllDepSpec>,
            ConstTreeSequence<DependencySpecTree, AnyDepSpec>,
            ConstTreeSequence<DependencySpecTree, UseDepSpec>
        >
    {
        typedef Formatter<
            UseDepSpec,
            PackageDepSpec,
            BlockDepSpec,
            DependencyLabelsDepSpec
                > Formatter;
    };

    /**
     * A DepSpec heirarchy containing things meaningful for sets.
     *
     * \ingroup grpdepspecs
     */
    struct SetSpecTree :
        VisitorTypes<
            SetSpecTree,
            DepSpec,
            TreeLeaf<SetSpecTree, PackageDepSpec>,
            ConstTreeSequence<SetSpecTree, AllDepSpec>
        >
    {
        typedef Formatter<
            PackageDepSpec
                > Formatter;
    };
}

#endif
