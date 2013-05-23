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

#include <paludis/resolver/labels_classifier.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/pool-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/dep_label.hh>
#include <paludis/serialise-impl.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<LabelsClassifierBuilder>
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> package_id;

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

        Imp(const Environment * const e, const std::shared_ptr<const PackageID> & i) :
            env(e),
            package_id(i),
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
    };
}

LabelsClassifierBuilder::LabelsClassifierBuilder(const Environment * const env, const std::shared_ptr<const PackageID> & id) :
    _imp(env, id)
{
}

LabelsClassifierBuilder::~LabelsClassifierBuilder() = default;

void
LabelsClassifierBuilder::visit(const DependenciesBuildLabel & l)
{
    if (l.enabled(_imp->env, _imp->package_id))
    {
        _imp->is_requirement = true;
        _imp->includes_buildish = true;
        _imp->includes_non_test_buildish = true;
        _imp->any_enabled = true;
    }
}

void
LabelsClassifierBuilder::visit(const DependenciesTestLabel & l)
{
    if (l.enabled(_imp->env, _imp->package_id))
    {
        _imp->is_requirement = true;
        _imp->includes_buildish = true;
        _imp->any_enabled = true;
    }
}

void
LabelsClassifierBuilder::visit(const DependenciesFetchLabel & l)
{
    if (l.enabled(_imp->env, _imp->package_id))
    {
        _imp->is_requirement = true;
        _imp->includes_buildish = true;
        _imp->includes_non_test_buildish = true;
        _imp->includes_fetch = true;
        _imp->any_enabled = true;
    }
}

void
LabelsClassifierBuilder::visit(const DependenciesRunLabel & l)
{
    if (l.enabled(_imp->env, _imp->package_id))
    {
        _imp->is_requirement = true;
        _imp->includes_non_post_runish = true;
        _imp->any_enabled = true;
    }
}

void
LabelsClassifierBuilder::visit(const DependenciesPostLabel & l)
{
    if (l.enabled(_imp->env, _imp->package_id))
    {
        _imp->is_requirement = true;
        _imp->includes_postish = true;
        _imp->any_enabled = true;
    }
}

void
LabelsClassifierBuilder::visit(const DependenciesInstallLabel & l)
{
    if (l.enabled(_imp->env, _imp->package_id))
    {
        _imp->is_requirement = true;
        _imp->includes_buildish = true;
        _imp->includes_non_test_buildish = true;
        _imp->any_enabled = true;
    }
}

void
LabelsClassifierBuilder::visit(const DependenciesCompileAgainstLabel & l)
{
    if (l.enabled(_imp->env, _imp->package_id))
    {
        _imp->is_requirement = true;
        _imp->includes_non_post_runish = true;
        _imp->includes_buildish = true;
        _imp->includes_non_test_buildish = true;
        _imp->any_enabled = true;
    }
}

void
LabelsClassifierBuilder::visit(const DependenciesRecommendationLabel & l)
{
    if (l.enabled(_imp->env, _imp->package_id))
    {
        _imp->is_recommendation = true;
        _imp->includes_postish = true;
        _imp->any_enabled = true;
    }
}

void
LabelsClassifierBuilder::visit(const DependenciesSuggestionLabel & l)
{
    if (l.enabled(_imp->env, _imp->package_id))
    {
        _imp->is_suggestion = true;
        _imp->includes_postish = true;
        _imp->any_enabled = true;
    }
}

const std::shared_ptr<const LabelsClassifier>
LabelsClassifierBuilder::create() const
{
    return Pool<LabelsClassifier>::get_instance()->create(
            _imp->any_enabled,
            _imp->includes_buildish,
            _imp->includes_compile_against,
            _imp->includes_fetch,
            _imp->includes_non_post_runish,
            _imp->includes_non_test_buildish,
            _imp->includes_post,
            _imp->includes_postish,
            _imp->is_recommendation,
            _imp->is_requirement,
            _imp->is_suggestion
            );
}

LabelsClassifier::LabelsClassifier(
        bool any_enabled_v,
        bool includes_buildish_v,
        bool includes_compile_against_v,
        bool includes_fetch_v,
        bool includes_non_post_runish_v,
        bool includes_non_test_buildish_v,
        bool includes_post_v,
        bool includes_postish_v,
        bool is_recommendation_v,
        bool is_requirement_v,
        bool is_suggestion_v
        ) :
    any_enabled(any_enabled_v),
    includes_buildish(includes_buildish_v),
    includes_compile_against(includes_compile_against_v),
    includes_fetch(includes_fetch_v),
    includes_non_post_runish(includes_non_post_runish_v),
    includes_non_test_buildish(includes_non_test_buildish_v),
    includes_post(includes_post_v),
    includes_postish(includes_postish_v),
    is_recommendation(is_recommendation_v),
    is_requirement(is_requirement_v),
    is_suggestion(is_suggestion_v)
{
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

const std::shared_ptr<const LabelsClassifier>
LabelsClassifier::deserialise(
        Deserialisation & d)
{
    Deserialisator v(d, "LabelsClassifier");
    return Pool<LabelsClassifier>::get_instance()->create(
            v.member<bool>("any_enabled"),
            v.member<bool>("includes_buildish"),
            v.member<bool>("includes_compile_against"),
            v.member<bool>("includes_fetch"),
            v.member<bool>("includes_non_post_runish"),
            v.member<bool>("includes_non_test_buildish"),
            v.member<bool>("includes_post"),
            v.member<bool>("includes_postish"),
            v.member<bool>("is_recommendation"),
            v.member<bool>("is_requirement"),
            v.member<bool>("is_suggestion")
            );
}

bool
paludis::resolver::is_suggestion(const Environment * const env, const std::shared_ptr<const PackageID> & id, const SanitisedDependency & dep)
{
    if (dep.active_dependency_labels()->empty())
        return false;

    LabelsClassifierBuilder v(env, id);
    std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
            indirect_iterator(dep.active_dependency_labels()->end()),
            accept_visitor(v));
    auto c(v.create());
    return c->is_suggestion && (! c->is_recommendation) && (! c->is_requirement);
}

bool
paludis::resolver::is_recommendation(const Environment * const env, const std::shared_ptr<const PackageID> & id, const SanitisedDependency & dep)
{
    if (dep.active_dependency_labels()->empty())
        return false;

    LabelsClassifierBuilder v(env, id);
    std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
            indirect_iterator(dep.active_dependency_labels()->end()),
            accept_visitor(v));
    auto c(v.create());
    return c->is_recommendation && (! c->is_requirement);
}

bool
paludis::resolver::is_just_build_dep(const Environment * const env, const std::shared_ptr<const PackageID> & id, const SanitisedDependency & dep)
{
    if (dep.active_dependency_labels()->empty())
        throw InternalError(PALUDIS_HERE, "not implemented");

    LabelsClassifierBuilder v(env, id);
    std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
            indirect_iterator(dep.active_dependency_labels()->end()),
            accept_visitor(v));
    auto c(v.create());
    return c->includes_buildish && ! c->includes_non_post_runish && ! c->includes_postish;
}

bool
paludis::resolver::is_compiled_against_dep(const Environment * const env, const std::shared_ptr<const PackageID> & id, const SanitisedDependency & dep)
{
    if (dep.active_dependency_labels()->empty())
        throw InternalError(PALUDIS_HERE, "not implemented");

    LabelsClassifierBuilder v(env, id);
    std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
            indirect_iterator(dep.active_dependency_labels()->end()),
            accept_visitor(v));
    auto c(v.create());
    return c->includes_compile_against;
}

bool
paludis::resolver::is_enabled_dep(const Environment * const env, const std::shared_ptr<const PackageID> & id, const SanitisedDependency & dep)
{
    if (dep.active_dependency_labels()->empty())
        throw InternalError(PALUDIS_HERE, "not implemented");

    LabelsClassifierBuilder v(env, id);
    std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
            indirect_iterator(dep.active_dependency_labels()->end()),
            accept_visitor(v));
    auto c(v.create());
    return c->any_enabled;
}

bool
paludis::resolver::is_run_or_post_dep(const Environment * const env, const std::shared_ptr<const PackageID> & id, const SanitisedDependency & dep)
{
    if (dep.active_dependency_labels()->empty())
        throw InternalError(PALUDIS_HERE, "not implemented");

    LabelsClassifierBuilder v(env, id);
    std::for_each(indirect_iterator(dep.active_dependency_labels()->begin()),
            indirect_iterator(dep.active_dependency_labels()->end()),
            accept_visitor(v));
    auto c(v.create());
    return c->includes_non_post_runish || c->includes_postish;
}

