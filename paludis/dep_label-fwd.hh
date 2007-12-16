/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LABEL_FWD_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LABEL_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/tr1_memory.hh>
#include <iosfwd>

/** \file
 * Forward declarations for paludis/dep_label.hh .
 *
 * \ingroup g_dep_spec
 */

namespace paludis
{
    struct URILabelVisitorTypes;
    struct URILabel;

    template <typename T_> struct ConcreteURILabel;

    struct URIMirrorsThenListedLabelTag;
    typedef ConcreteURILabel<URIMirrorsThenListedLabelTag> URIMirrorsThenListedLabel;

    struct URIMirrorsOnlyLabelTag;
    typedef ConcreteURILabel<URIMirrorsOnlyLabelTag> URIMirrorsOnlyLabel;

    struct URIListedOnlyLabelTag;
    typedef ConcreteURILabel<URIListedOnlyLabelTag> URIListedOnlyLabel;

    struct URIListedThenMirrorsLabelTag;
    typedef ConcreteURILabel<URIListedThenMirrorsLabelTag> URIListedThenMirrorsLabel;

    struct URILocalMirrorsOnlyLabelTag;
    typedef ConcreteURILabel<URILocalMirrorsOnlyLabelTag> URILocalMirrorsOnlyLabel;

    struct URIManualOnlyLabelTag;
    typedef ConcreteURILabel<URIManualOnlyLabelTag> URIManualOnlyLabel;

    /**
     * A URILabel can be written to a stream.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    std::ostream & operator<< (std::ostream &, const URILabel &) PALUDIS_VISIBLE;

    struct DependencyLabelVisitorTypes;
    struct DependencyLabel;
    struct DependencySystemLabel;
    struct DependencyTypeLabel;
    struct DependencySuggestLabel;
    struct DependencyABIsLabel;

    /**
     * A collection of DependencyLabel instances.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef Sequence<tr1::shared_ptr<const DependencyLabel> > DependencyLabelSequence;

    /**
     * A collection of DependencySystemLabel instances.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef Sequence<tr1::shared_ptr<const DependencySystemLabel> > DependencySystemLabelSequence;

    /**
     * A collection of DependencyTypeLabel instances.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef Sequence<tr1::shared_ptr<const DependencyTypeLabel> > DependencyTypeLabelSequence;

    /**
     * A collection of DependencySuggestLabel instances.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef Sequence<tr1::shared_ptr<const DependencySuggestLabel> > DependencySuggestLabelSequence;

    /**
     * A collection of DependencyABIsLabel instances.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef Sequence<tr1::shared_ptr<const DependencyABIsLabel> > DependencyABIsLabelSequence;

    template <typename T_, typename Category_> struct ConcreteDependencyLabel;

    struct DependencyHostLabelTag;

    /**
     * A DependencyHostLabel specifies host requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyHostLabelTag, DependencySystemLabel> DependencyHostLabel;

    struct DependencyTargetLabelTag;

    /**
     * A DependencyTargetLabel specifies target requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyTargetLabelTag, DependencySystemLabel> DependencyTargetLabel;

    struct DependencyBuildLabelTag;

    /**
     * A DependencyBuildLabel specifies build-time requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyBuildLabelTag, DependencyTypeLabel> DependencyBuildLabel;

    struct DependencyRunLabelTag;

    /**
     * A DependencyRunLabel specifies runtime requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyRunLabelTag, DependencyTypeLabel> DependencyRunLabel;

    struct DependencyPostLabelTag;

    /**
     * A DependencyPostLabel specifies build-time requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyPostLabelTag, DependencyTypeLabel> DependencyPostLabel;

    struct DependencyInstallLabelTag;

    /**
     * A DependencyInstallLabel specifies install-time requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyInstallLabelTag, DependencyTypeLabel> DependencyInstallLabel;

    struct DependencyCompileLabelTag;

    /**
     * A DependencyCompileLabel specifies compiled-against requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyCompileLabelTag, DependencyTypeLabel> DependencyCompileLabel;

    struct DependencySuggestedLabelTag;

    /**
     * A DependencySuggestLabel specifies that a dependency is suggested.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencySuggestedLabelTag, DependencySuggestLabel> DependencySuggestedLabel;

    struct DependencyRecommendedLabelTag;

    /**
     * A DependencyRecommendedLabel specifies that a dependency is recommended.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyRecommendedLabelTag, DependencySuggestLabel> DependencyRecommendedLabel;

    struct DependencyRequiredLabelTag;

    /**
     * A DependencyRequiredLabel specifies that a dependency is required.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyRequiredLabelTag, DependencySuggestLabel> DependencyRequiredLabel;

    struct DependencyAnyLabelTag;

    /**
     * A DependencyAnyLabel specifies that a dependency can be satisfied by
     * any ABI.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyAnyLabelTag, DependencyABIsLabel> DependencyAnyLabel;

    struct DependencyMineLabelTag;

    /**
     * A DependencyMineLabel specifies that a dependency is satisfied by
     * ABIs equal to those being used to create the depending package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyMineLabelTag, DependencyABIsLabel> DependencyMineLabel;

    struct DependencyPrimaryLabelTag;

    /**
     * A DependencyPrimaryLabel specifies that a dependency can be satisfied by
     * the primary ABI.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyPrimaryLabelTag, DependencyABIsLabel> DependencyPrimaryLabel;

    struct DependencyABILabelTag;

    /**
     * A DependencyABILabel specifies that a dependency can be satisfied by
     * a named ABI.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<DependencyABILabelTag, DependencyABIsLabel> DependencyABILabel;

    /**
     * A DependencyLabel can be written to a stream.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    std::ostream & operator<< (std::ostream &, const DependencyLabel &) PALUDIS_VISIBLE;

    struct ActiveDependencyLabels;
}

#endif
