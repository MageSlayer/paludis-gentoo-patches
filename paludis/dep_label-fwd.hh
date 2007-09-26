/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
    typedef ConcreteDependencyLabel<enum DependencyHostLabelTag { },
            DependencySystemLabel> DependencyHostLabel;
    typedef ConcreteDependencyLabel<enum DependencyTargetLabelTag { },
            DependencySystemLabel> DependencyTargetLabel;
    typedef ConcreteDependencyLabel<enum DependencyBuildLabelTag { },
            DependencyTypeLabel> DependencyBuildLabel;
    typedef ConcreteDependencyLabel<enum DependencyRunLabelTag { },
            DependencyTypeLabel> DependencyRunLabel;
    typedef ConcreteDependencyLabel<enum DependencyInstallLabelTag { },
            DependencyTypeLabel> DependencyInstallLabel;
    typedef ConcreteDependencyLabel<enum DependencyCompileLabelTag { },
            DependencyTypeLabel> DependencyCompileLabel;
    typedef ConcreteDependencyLabel<enum DependencySuggestedLabelTag { },
            DependencySuggestLabel> DependencySuggestedLabel;
    typedef ConcreteDependencyLabel<enum DependencyRecommendedLabelTag { },
            DependencySuggestLabel> DependencyRecommendedLabel;
    typedef ConcreteDependencyLabel<enum DependencyRequiredLabelTag { },
            DependencySuggestLabel> DependencyRequiredLabel;
    typedef ConcreteDependencyLabel<enum DependencyAnyLabelTag { },
            DependencyABIsLabel> DependencyAnyLabel;
    typedef ConcreteDependencyLabel<enum DependencyMineLabelTag { },
            DependencyABIsLabel> DependencyMineLabel;
    typedef ConcreteDependencyLabel<enum DependencyPrimaryLabelTag { },
            DependencyABIsLabel> DependencyPrimaryLabel;
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
