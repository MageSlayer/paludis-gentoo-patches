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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_SPEC_FWD_HH
#define PALUDIS_GUARD_PALUDIS_DEP_SPEC_FWD_HH 1

#include <iosfwd>
#include <string>
#include <paludis/dep_label.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/visitor.hh>

namespace paludis
{
    class DepSpec;
    class PackageDepSpec;
    class PlainTextDepSpec;
    class URIDepSpec;
    class AllDepSpec;
    class AnyDepSpec;
    class UseDepSpec;
    class BlockDepSpec;
    class StringDepSpec;
    template <typename T_> class LabelsDepSpec;

#include <paludis/dep_spec-se.hh>

    /**
     * A PlainTextDepSpec can be written to an ostream.
     *
     * \ingroup grpdepspecs
     */
    std::ostream & operator<< (std::ostream &, const PlainTextDepSpec &) PALUDIS_VISIBLE;

    class PackageDepSpecError;

    /**
     * A PackageDepSpec can be written to an ostream.
     *
     * \ingroup grpdepspecs
     */
    std::ostream & operator<< (std::ostream &, const PackageDepSpec &) PALUDIS_VISIBLE;

    /**
     * A LabelsDepSpec<URILabelVisitorTypes> can be written to an ostream.
     *
     * \ingroup grpdepspecs
     */
    std::ostream & operator<< (std::ostream &, const LabelsDepSpec<URILabelVisitorTypes> &) PALUDIS_VISIBLE;

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
            TreeLeaf<GenericSpecTree, URIDepSpec>,
            TreeLeaf<GenericSpecTree, PackageDepSpec>,
            TreeLeaf<GenericSpecTree, BlockDepSpec>,
            TreeLeaf<GenericSpecTree, LabelsDepSpec<URILabelVisitorTypes> >,
            ConstTreeSequence<GenericSpecTree, AllDepSpec>,
            ConstTreeSequence<GenericSpecTree, AnyDepSpec>,
            ConstTreeSequence<GenericSpecTree, UseDepSpec>
        >
    {
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
            TreeLeaf<LicenseSpecTree, PlainTextDepSpec>,
            ConstTreeSequence<LicenseSpecTree, AllDepSpec>,
            ConstTreeSequence<LicenseSpecTree, AnyDepSpec>,
            ConstTreeSequence<LicenseSpecTree, UseDepSpec>
        >
    {
    };

    /**
     * A DepSpec heirarchy containing things meaningful for URIs.
     *
     * \ingroup grpdepspecs
     */
    struct URISpecTree :
        VisitorTypes<
            URISpecTree,
            DepSpec,
            TreeLeaf<URISpecTree, URIDepSpec>,
            TreeLeaf<URISpecTree, LabelsDepSpec<URILabelVisitorTypes> >,
            ConstTreeSequence<URISpecTree, AllDepSpec>,
            ConstTreeSequence<URISpecTree, UseDepSpec>
        >
    {
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
            TreeLeaf<FlattenableSpecTree, URIDepSpec>,
            TreeLeaf<FlattenableSpecTree, PackageDepSpec>,
            TreeLeaf<FlattenableSpecTree, BlockDepSpec>,
            ConstTreeSequence<FlattenableSpecTree, AllDepSpec>,
            ConstTreeSequence<FlattenableSpecTree, UseDepSpec>
        >
    {
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
            ConstTreeSequence<DependencySpecTree, AllDepSpec>,
            ConstTreeSequence<DependencySpecTree, AnyDepSpec>,
            ConstTreeSequence<DependencySpecTree, UseDepSpec>
        >
    {
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
    };
}

#endif
