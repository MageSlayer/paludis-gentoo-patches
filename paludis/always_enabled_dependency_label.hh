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

#ifndef PALUDIS_GUARD_PALUDIS_ALWAYS_ENABLED_DEPENDENCY_LABEL_HH
#define PALUDIS_GUARD_PALUDIS_ALWAYS_ENABLED_DEPENDENCY_LABEL_HH 1

#include <paludis/always_enabled_dependency_label-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/dep_label.hh>

namespace paludis
{
    template <typename Label_>
    class PALUDIS_VISIBLE AlwaysEnabledDependencyLabel :
        public SpecificDependenciesLabel<Label_>
    {
        private:
            const std::string _text;

        public:
            AlwaysEnabledDependencyLabel(const std::string &);
            ~AlwaysEnabledDependencyLabel();

            virtual const std::string text() const;
            virtual bool enabled(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &) const;
    };

    extern template class AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag>;
    extern template class AlwaysEnabledDependencyLabel<DependenciesRunLabelTag>;
    extern template class AlwaysEnabledDependencyLabel<DependenciesPostLabelTag>;
    extern template class AlwaysEnabledDependencyLabel<DependenciesCompileAgainstLabelTag>;
    extern template class AlwaysEnabledDependencyLabel<DependenciesFetchLabelTag>;
    extern template class AlwaysEnabledDependencyLabel<DependenciesInstallLabelTag>;
    extern template class AlwaysEnabledDependencyLabel<DependenciesSuggestionLabelTag>;
    extern template class AlwaysEnabledDependencyLabel<DependenciesRecommendationLabelTag>;
    extern template class AlwaysEnabledDependencyLabel<DependenciesTestLabelTag>;
}

#endif
