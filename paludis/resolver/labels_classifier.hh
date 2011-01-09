/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_LABELS_CLASSIFIER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_LABELS_CLASSIFIER_HH 1

#include <paludis/resolver/labels_classifier-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/dep_label-fwd.hh>
#include <paludis/serialise-fwd.hh>

namespace paludis
{
    namespace resolver
    {
        class LabelsClassifier
        {
            private:
                Pimp<LabelsClassifier> _imp;

            public:
                LabelsClassifier(const Environment * const, const std::shared_ptr<const PackageID> &);
                ~LabelsClassifier();

                bool any_enabled;

                bool includes_buildish;
                bool includes_compile_against;
                bool includes_fetch;
                bool includes_non_post_runish;
                bool includes_non_test_buildish;
                bool includes_post;
                bool includes_postish;

                bool is_recommendation;
                bool is_requirement;
                bool is_suggestion;

                void visit(const DependenciesBuildLabel &);
                void visit(const DependenciesCompileAgainstLabel &);
                void visit(const DependenciesFetchLabel &);
                void visit(const DependenciesInstallLabel &);
                void visit(const DependenciesPostLabel &);
                void visit(const DependenciesRecommendationLabel &);
                void visit(const DependenciesRunLabel &);
                void visit(const DependenciesSuggestionLabel &);
                void visit(const DependenciesTestLabel &);

                void serialise(Serialiser &) const;

                static const std::shared_ptr<LabelsClassifier> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
