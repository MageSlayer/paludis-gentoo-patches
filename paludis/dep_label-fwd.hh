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

    std::ostream & operator<< (std::ostream &, const URILabel &) PALUDIS_VISIBLE;

    struct DependencyLabelVisitorTypes;
    struct DependencyLabel;

    template <typename T_> struct ConcreteDependencyLabel;
    typedef ConcreteDependencyLabel<enum DependencyHostLabelTag { }> DependencyHostLabel;
    typedef ConcreteDependencyLabel<enum DependencyTargetLabelTag { }> DependencyTargetLabel;
    typedef ConcreteDependencyLabel<enum DependencyBuildLabelTag { }> DependencyBuildLabel;
    typedef ConcreteDependencyLabel<enum DependencyRunLabelTag { }> DependencyRunLabel;
    typedef ConcreteDependencyLabel<enum DependencyInstallLabelTag { }> DependencyInstallLabel;
    typedef ConcreteDependencyLabel<enum DependencyCompileLabelTag { }> DependencyCompileLabel;
    typedef ConcreteDependencyLabel<enum DependencySuggestedLabelTag { }> DependencySuggestedLabel;
    typedef ConcreteDependencyLabel<enum DependencyRecommendedLabelTag { }> DependencyRecommendedLabel;
    typedef ConcreteDependencyLabel<enum DependencyRequiredLabelTag { }> DependencyRequiredLabel;
    typedef ConcreteDependencyLabel<enum DependencyAnyLabelTag { }> DependencyAnyLabel;
    typedef ConcreteDependencyLabel<enum DependencyMineLabelTag { }> DependencyMineLabel;
    typedef ConcreteDependencyLabel<enum DependencyPrimaryLabelTag { }> DependencyPrimaryLabel;
    typedef ConcreteDependencyLabel<enum DependencyABILabelTag { }> DependencyABILabel;

    std::ostream & operator<< (std::ostream &, const DependencyLabel &) PALUDIS_VISIBLE;
}

#endif
