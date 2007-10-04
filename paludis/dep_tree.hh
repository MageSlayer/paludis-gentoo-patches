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

/** \file
 * Declarations for dependency spec trees.
 *
 * \ingroup g_dep_spec
 *
 * \section Examples
 *
 * - \ref example_dep_tree.cc "example_dep_tree.cc" (for specification trees)
 * - \ref example_dep_spec.cc "example_dep_spec.cc" (for specifications)
 * - \ref example_dep_label.cc "example_dep_label.cc" (for labels)
 */

namespace paludis
{
    /**
     * A generic dep tree heirarchy.
     *
     * Heirarchies conforming to this type will likely not ever be created.
     * However, the heirarchy is useful for creating generic visitors that can
     * handle any of the subheirarchies.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
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
        /**
         * A formatter that can handle any formattable type found in a
         * GenericSpecTree.
         *
         * \since 0.26
         * \ingroup g_dep_spec
         * \nosubgrouping
         */
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
     * A heirarchy for licence specifications.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
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
        /**
         * A formatter that can handle any formattable type found in a
         * LicenseSpecTree.
         *
         * \since 0.26
         * \ingroup g_dep_spec
         * \nosubgrouping
         */
        typedef Formatter<
            UseDepSpec,
            LicenseDepSpec
                > Formatter;
    };

    /**
     * A heirarchy for fetchable URI heirarchies.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
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
        /**
         * A formatter that can handle any formattable type found in a
         * FetchableURIDepSpec.
         *
         * \since 0.26
         * \ingroup g_dep_spec
         * \nosubgrouping
         */
        typedef Formatter<
            UseDepSpec,
            FetchableURIDepSpec,
            URILabelsDepSpec
                > Formatter;
    };

    /**
     * A heirarchy for simple URI heirarchies.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
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
        /**
         * A formatter that can handle any formattable type found in a
         * SimpleURISpecTree.
         *
         * \since 0.26
         * \ingroup g_dep_spec
         * \nosubgrouping
         */
        typedef Formatter<
            UseDepSpec,
            SimpleURIDepSpec
                > Formatter;
    };

    /**
     * A heirarchy of things that can be flattened.
     *
     * This heirarchy is not constructed; it is only used for declaring
     * certain visitors (e.g. DepSpecFlattener).
     *
     * \see DepSpecFlattener
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
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
        /**
         * A formatter that can handle any formattable type found in a
         * FlattenableSpecTree.
         *
         * \ingroup g_dep_spec
         * \since 0.26
         * \nosubgrouping
         */
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
     * A heirarchy of provided packages.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
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
        /**
         * A formatter that can handle any formattable type found in a
         * ProvideSpecTree.
         *
         * \ingroup g_dep_spec
         * \since 0.26
         * \nosubgrouping
         */
        typedef Formatter<
            UseDepSpec,
            PackageDepSpec
                > Formatter;
    };

    /**
     * A heirarchy of restrict keywords.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
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
        /**
         * A formatter that can handle any formattable type found in a
         * RestrictSpecTree.
         *
         * \ingroup g_dep_spec
         * \since 0.26
         * \nosubgrouping
         */
        typedef Formatter<
            UseDepSpec,
            PlainTextDepSpec
                > Formatter;
    };

    /**
     * A heirarchy for dependencies.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
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
        /**
         * A formatter that can handle any formattable type found in a
         * DependencySpecTree.
         *
         * \ingroup g_dep_spec
         * \since 0.26
         * \nosubgrouping
         */
        typedef Formatter<
            UseDepSpec,
            PackageDepSpec,
            BlockDepSpec,
            DependencyLabelsDepSpec
                > Formatter;
    };

    /**
     * A heirarchy for things that can be found in package sets.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
     */
    struct SetSpecTree :
        VisitorTypes<
            SetSpecTree,
            DepSpec,
            TreeLeaf<SetSpecTree, PackageDepSpec>,
            ConstTreeSequence<SetSpecTree, AllDepSpec>
        >
    {
        /**
         * A formatter that can handle any formattable type found in a
         * SetSpecTree.
         *
         * \ingroup g_dep_spec
         * \since 0.26
         * \nosubgrouping
         */
        typedef Formatter<
            PackageDepSpec
                > Formatter;
    };
}

#endif
