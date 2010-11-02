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

#include <paludis/resolver/labels_classifier.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/stringify.hh>
#include <paludis/dep_label.hh>
#include <paludis/serialise-impl.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::resolver;

LabelsClassifier::LabelsClassifier() :
    any_enabled(false),
    includes_buildish(false),
    includes_compile_against(false),
    includes_fetch(false),
    includes_non_post_runish(false),
    includes_non_test_buildish(false),
    includes_post(false),
    includes_postish(false),
    is_recommendation(false),
    is_requirement(false),
    is_suggestion(false)
{
}

void
LabelsClassifier::visit(const DependenciesBuildLabel & l)
{
    if (l.enabled())
    {
        is_requirement = true;
        includes_buildish = true;
        includes_non_test_buildish = true;
        any_enabled = true;
    }
}

void
LabelsClassifier::visit(const DependenciesTestLabel & l)
{
    if (l.enabled())
    {
        is_requirement = true;
        includes_buildish = true;
        any_enabled = true;
    }
}

void
LabelsClassifier::visit(const DependenciesFetchLabel & l)
{
    if (l.enabled())
    {
        is_requirement = true;
        includes_buildish = true;
        includes_non_test_buildish = true;
        includes_fetch = true;
        any_enabled = true;
    }
}

void
LabelsClassifier::visit(const DependenciesRunLabel & l)
{
    if (l.enabled())
    {
        is_requirement = true;
        includes_non_post_runish = true;
        any_enabled = true;
    }
}

void
LabelsClassifier::visit(const DependenciesPostLabel & l)
{
    if (l.enabled())
    {
        is_requirement = true;
        includes_postish = true;
        any_enabled = true;
    }
}

void
LabelsClassifier::visit(const DependenciesInstallLabel & l)
{
    if (l.enabled())
    {
        is_requirement = true;
        includes_buildish = true;
        includes_non_test_buildish = true;
        any_enabled = true;
    }
}

void
LabelsClassifier::visit(const DependenciesCompileAgainstLabel & l)
{
    if (l.enabled())
    {
        is_requirement = true;
        includes_non_post_runish = true;
        includes_buildish = true;
        includes_non_test_buildish = true;
        any_enabled = true;
    }
}

void
LabelsClassifier::visit(const DependenciesRecommendationLabel & l)
{
    if (l.enabled())
    {
        is_recommendation = true;
        includes_postish = true;
        any_enabled = true;
    }
}

void
LabelsClassifier::visit(const DependenciesSuggestionLabel & l)
{
    if (l.enabled())
    {
        is_suggestion = true;
        includes_postish = true;
        any_enabled = true;
    }
}

void
LabelsClassifier::serialise(Serialiser & s) const
{
    s.object("LabelsClassifier")
        .member(SerialiserFlags<>(), "any_enabled", any_enabled)
        .member(SerialiserFlags<>(), "includes_buildish", includes_buildish)
        .member(SerialiserFlags<>(), "includes_compile_against", includes_compile_against)
        .member(SerialiserFlags<>(), "includes_fetch", includes_fetch)
        .member(SerialiserFlags<>(), "includes_non_post_runish", includes_non_post_runish)
        .member(SerialiserFlags<>(), "includes_non_test_buildish", includes_non_test_buildish)
        .member(SerialiserFlags<>(), "includes_post", includes_post)
        .member(SerialiserFlags<>(), "includes_postish", includes_postish)
        .member(SerialiserFlags<>(), "is_recommendation", is_recommendation)
        .member(SerialiserFlags<>(), "is_requirement", is_requirement)
        .member(SerialiserFlags<>(), "is_suggestion", is_suggestion)
        ;
}

const std::shared_ptr<LabelsClassifier>
LabelsClassifier::deserialise(
        Deserialisation & d)
{
    Deserialisator v(d, "LabelsClassifier");
    auto result(std::make_shared<LabelsClassifier>());
    result->any_enabled = v.member<bool>("any_enabled");
    result->includes_buildish = v.member<bool>("includes_buildish");
    result->includes_compile_against = v.member<bool>("includes_compile_against");
    result->includes_fetch = v.member<bool>("includes_fetch");
    result->includes_non_post_runish = v.member<bool>("includes_non_post_runish");
    result->includes_non_test_buildish = v.member<bool>("includes_non_test_buildish");
    result->includes_post = v.member<bool>("includes_post");
    result->includes_postish = v.member<bool>("includes_postish");
    result->is_recommendation = v.member<bool>("is_recommendation");
    result->is_requirement = v.member<bool>("is_requirement");
    result->is_suggestion = v.member<bool>("is_suggestion");

    return result;
}

bool
paludis::resolver::is_suggestion(const SanitisedDependency & dep)
{
    if (dep.active_dependency_labels()->empty())
        return false;

    LabelsClassifier v;
    std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
            indirect_iterator(dep.active_dependency_labels()->end()),
            accept_visitor(v));
    return v.is_suggestion && (! v.is_recommendation) && (! v.is_requirement);
}

bool
paludis::resolver::is_recommendation(const SanitisedDependency & dep)
{
    if (dep.active_dependency_labels()->empty())
        return false;

    LabelsClassifier v;
    std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
            indirect_iterator(dep.active_dependency_labels()->end()),
            accept_visitor(v));
    return v.is_recommendation && (! v.is_requirement);
}

bool
paludis::resolver::is_just_build_dep(const SanitisedDependency & dep)
{
    if (dep.active_dependency_labels()->empty())
        throw InternalError(PALUDIS_HERE, "not implemented");

    LabelsClassifier v;
    std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
            indirect_iterator(dep.active_dependency_labels()->end()),
            accept_visitor(v));
    return v.includes_buildish && ! v.includes_non_post_runish && ! v.includes_postish;
}

bool
paludis::resolver::is_compiled_against_dep(const SanitisedDependency & dep)
{
    if (dep.active_dependency_labels()->empty())
        throw InternalError(PALUDIS_HERE, "not implemented");

    LabelsClassifier v;
    std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
            indirect_iterator(dep.active_dependency_labels()->end()),
            accept_visitor(v));
    return v.includes_compile_against;
}

bool
paludis::resolver::is_enabled_dep(const SanitisedDependency & dep)
{
    if (dep.active_dependency_labels()->empty())
        throw InternalError(PALUDIS_HERE, "not implemented");

    LabelsClassifier v;
    std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
            indirect_iterator(dep.active_dependency_labels()->end()),
            accept_visitor(v));
    return v.any_enabled;
}

bool
paludis::resolver::is_run_or_post_dep(const SanitisedDependency & dep)
{
    if (dep.active_dependency_labels()->empty())
        throw InternalError(PALUDIS_HERE, "not implemented");

    LabelsClassifier v;
    std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
            indirect_iterator(dep.active_dependency_labels()->end()),
            accept_visitor(v));
    return v.includes_non_post_runish || v.includes_postish;
}

