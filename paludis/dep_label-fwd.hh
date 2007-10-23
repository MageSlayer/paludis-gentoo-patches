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
    typedef ConcreteURILabel<enum URIMirrorsThenListedLabelTag { }> URIMirrorsThenListedLabel;
    typedef ConcreteURILabel<enum URIMirrorsOnlyLabelTag { }> URIMirrorsOnlyLabel;
    typedef ConcreteURILabel<enum URIListedOnlyLabelTag { }> URIListedOnlyLabel;
    typedef ConcreteURILabel<enum URIListedThenMirrorsLabelTag { }> URIListedThenMirrorsLabel;
    typedef ConcreteURILabel<enum URILocalMirrorsOnlyLabelTag { }> URILocalMirrorsOnlyLabel;
    typedef ConcreteURILabel<enum URIManualOnlyLabelTag { }> URIManualOnlyLabel;

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

    template <typename T_, typename Category_> struct ConcreteDependencyLabel;

    /**
     * A DependencyHostLabel specifies host requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencyHostLabelTag { },
            DependencySystemLabel> DependencyHostLabel;

    /**
     * A DependencyTargetLabel specifies target requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencyTargetLabelTag { },
            DependencySystemLabel> DependencyTargetLabel;

    /**
     * A DependencyBuildLabel specifies build-time requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencyBuildLabelTag { },
            DependencyTypeLabel> DependencyBuildLabel;

    /**
     * A DependencyRunLabel specifies runtime requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencyRunLabelTag { },
            DependencyTypeLabel> DependencyRunLabel;

    /**
     * A DependencyInstallLabel specifies install-time requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencyInstallLabelTag { },
            DependencyTypeLabel> DependencyInstallLabel;

    /**
     * A DependencyCompileLabel specifies compiled-against requirements for building a package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencyCompileLabelTag { },
            DependencyTypeLabel> DependencyCompileLabel;

    /**
     * A DependencySuggestLabel specifies that a dependency is suggested.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencySuggestedLabelTag { },
            DependencySuggestLabel> DependencySuggestedLabel;

    /**
     * A DependencyRecommendedLabel specifies that a dependency is recommended.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencyRecommendedLabelTag { },
            DependencySuggestLabel> DependencyRecommendedLabel;

    /**
     * A DependencyRequiredLabel specifies that a dependency is required.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencyRequiredLabelTag { },
            DependencySuggestLabel> DependencyRequiredLabel;

    /**
     * A DependencyAnyLabel specifies that a dependency can be satisfied by
     * any ABI.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencyAnyLabelTag { },
            DependencyABIsLabel> DependencyAnyLabel;

    /**
     * A DependencyMineLabel specifies that a dependency is satisfied by
     * ABIs equal to those being used to create the depending package.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencyMineLabelTag { },
            DependencyABIsLabel> DependencyMineLabel;

    /**
     * A DependencyPrimaryLabel specifies that a dependency can be satisfied by
     * the primary ABI.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencyPrimaryLabelTag { },
            DependencyABIsLabel> DependencyPrimaryLabel;

    /**
     * A DependencyABILabel specifies that a dependency can be satisfied by
     * a named ABI.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    typedef ConcreteDependencyLabel<enum DependencyABILabelTag { },
            DependencyABIsLabel> DependencyABILabel;

    /**
     * A DependencyLabel can be written to a stream.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    std::ostream & operator<< (std::ostream &, const DependencyLabel &) PALUDIS_VISIBLE;
}

#endif
