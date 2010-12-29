/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/always_enabled_dependency_label.hh>

using namespace paludis;

template <typename Label_>
AlwaysEnabledDependencyLabel<Label_>::AlwaysEnabledDependencyLabel(const std::string & t) :
    _text(t)
{
}

template <typename Label_>
AlwaysEnabledDependencyLabel<Label_>::~AlwaysEnabledDependencyLabel()
{
}

template <typename Label_>
const std::string
AlwaysEnabledDependencyLabel<Label_>::text() const
{
    return _text;
}

template <typename Label_>
bool
AlwaysEnabledDependencyLabel<Label_>::enabled() const
{
    return true;
}

template class AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag>;
template class AlwaysEnabledDependencyLabel<DependenciesRunLabelTag>;
template class AlwaysEnabledDependencyLabel<DependenciesPostLabelTag>;
template class AlwaysEnabledDependencyLabel<DependenciesCompileAgainstLabelTag>;
template class AlwaysEnabledDependencyLabel<DependenciesFetchLabelTag>;
template class AlwaysEnabledDependencyLabel<DependenciesInstallLabelTag>;
template class AlwaysEnabledDependencyLabel<DependenciesSuggestionLabelTag>;
template class AlwaysEnabledDependencyLabel<DependenciesRecommendationLabelTag>;
template class AlwaysEnabledDependencyLabel<DependenciesTestLabelTag>;

