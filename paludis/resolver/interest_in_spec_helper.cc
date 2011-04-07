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

#include <paludis/resolver/interest_in_spec_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/match_qpns.hh>
#include <paludis/resolver/labels_classifier.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/environment.hh>
#include <paludis/package_dep_spec_collection.hh>
#include <paludis/metadata_key.hh>
#include <paludis/match_package.hh>
#include <paludis/dep_spec_annotations.hh>
#include <paludis/package_dep_spec_requirement.hh>
#include <list>
#include <set>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<InterestInSpecHelper>
    {
        const Environment * const env;
        std::list<PackageDepSpec> take_specs;
        std::set<std::string> take_groups;
        std::list<PackageDepSpec> take_from_specs;
        std::list<PackageDepSpec> ignore_specs;
        std::set<std::string> ignore_groups;
        std::list<PackageDepSpec> ignore_from_specs;
        PackageDepSpecCollection no_blockers_from_specs;
        PackageDepSpecCollection no_dependencies_from_specs;
        bool follow_installed_build_dependencies;
        bool follow_installed_dependencies;
        Tribool take_suggestions;
        Tribool take_recommendations;

        Imp(const Environment * const e) :
            env(e),
            no_blockers_from_specs(make_null_shared_ptr()),
            no_dependencies_from_specs(make_null_shared_ptr()),
            follow_installed_build_dependencies(false),
            follow_installed_dependencies(true),
            take_suggestions(indeterminate),
            take_recommendations(true)
        {
        }
    };
}

InterestInSpecHelper::InterestInSpecHelper(const Environment * const e) :
    _imp(e)
{
}

InterestInSpecHelper::~InterestInSpecHelper() = default;

void
InterestInSpecHelper::add_take_spec(const PackageDepSpec & spec)
{
    _imp->take_specs.push_back(spec);
}

void
InterestInSpecHelper::add_take_group(const std::string & group)
{
    _imp->take_groups.insert(group);
}

void
InterestInSpecHelper::add_take_from_spec(const PackageDepSpec & spec)
{
    _imp->take_from_specs.push_back(spec);
}

void
InterestInSpecHelper::add_ignore_spec(const PackageDepSpec & spec)
{
    _imp->ignore_specs.push_back(spec);
}

void
InterestInSpecHelper::add_ignore_group(const std::string & group)
{
    _imp->ignore_groups.insert(group);
}

void
InterestInSpecHelper::add_ignore_from_spec(const PackageDepSpec & spec)
{
    _imp->ignore_from_specs.push_back(spec);
}

void
InterestInSpecHelper::add_no_blockers_from_spec(const PackageDepSpec & spec)
{
    _imp->no_blockers_from_specs.insert(spec);
}

void
InterestInSpecHelper::add_no_dependencies_from_spec(const PackageDepSpec & spec)
{
    _imp->no_dependencies_from_specs.insert(spec);
}

void
InterestInSpecHelper::set_follow_installed_dependencies(const bool b)
{
    _imp->follow_installed_dependencies = b;
}

void
InterestInSpecHelper::set_follow_installed_build_dependencies(const bool b)
{
    _imp->follow_installed_build_dependencies = b;
}

void
InterestInSpecHelper::set_take_suggestions(const Tribool v)
{
    _imp->take_suggestions = v;
}

void
InterestInSpecHelper::set_take_recommendations(const Tribool v)
{
    _imp->take_recommendations = v;
}

namespace
{
    bool ignore_dep_from(
            const Environment * const env,
            const PackageDepSpecCollection & no_blockers_from_specs,
            const PackageDepSpecCollection & no_dependencies_from_specs,
            const std::shared_ptr<const PackageID> & id,
            const bool is_block)
    {
        const auto & list(is_block ? no_blockers_from_specs : no_dependencies_from_specs);

        return list.match_any(env, id, { });
    }

    struct CareAboutDepFnVisitor
    {
        const Environment * const env;
        const PackageDepSpecCollection & no_blockers_from_specs;
        const PackageDepSpecCollection & no_dependencies_from_specs;
        const bool follow_installed_build_dependencies;
        const bool follow_installed_dependencies;
        const SanitisedDependency dep;

        bool visit(const ExistingNoChangeDecision & decision) const
        {
            if (ignore_dep_from(env, no_blockers_from_specs, no_dependencies_from_specs, decision.existing_id(), bool(dep.spec().if_block())))
                return false;

            if (! is_enabled_dep(env, decision.existing_id(), dep))
                return false;

            if (! follow_installed_build_dependencies)
                if (is_just_build_dep(env, decision.existing_id(), dep))
                    return false;
            if (! follow_installed_dependencies)
                if (! is_compiled_against_dep(env, decision.existing_id(), dep))
                    return false;

            if (is_suggestion(env, decision.existing_id(), dep) || is_recommendation(env, decision.existing_id(), dep))
            {
                /* we only take a suggestion or recommendation for an existing
                 * package if it's already met. for now, we ignore suggested
                 * and recommended blocks no matter what. */
                if (dep.spec().if_block())
                    return false;

                const std::shared_ptr<const PackageIDSequence> installed_ids(
                        (*env)[selection::SomeArbitraryVersion(
                            generator::Matches(*dep.spec().if_package(), dep.from_id(), { }) |
                            filter::InstalledAtRoot(env->system_root_key()->value()))]);
                if (installed_ids->empty())
                    return false;
            }

            return true;
        }

        bool visit(const NothingNoChangeDecision &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "NothingNoChangeDecision shouldn't have deps");
        }

        bool visit(const UnableToMakeDecision &) const
        {
            /* might've gone from a sensible decision to unable later on */
            return false;
        }

        bool visit(const RemoveDecision &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "RemoveDecision shouldn't have deps");
        }

        bool visit(const BreakDecision &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "BreakDecision shouldn't have deps");
        }

        bool visit(const ChangesToMakeDecision & decision) const
        {
            if (ignore_dep_from(env, no_blockers_from_specs, no_dependencies_from_specs, decision.origin_id(), bool(dep.spec().if_block())))
                return false;

            if (is_enabled_dep(env, decision.origin_id(), dep))
                return true;

            return false;
        }
    };
}

SpecInterest
InterestInSpecHelper::operator() (
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id,
        const SanitisedDependency & dep) const
{
    Context context("When determining interest in '" + stringify(dep.spec()) + "':");

    CareAboutDepFnVisitor v{_imp->env, _imp->no_blockers_from_specs, _imp->no_dependencies_from_specs,
        _imp->follow_installed_build_dependencies, _imp->follow_installed_dependencies, dep};

    if (resolution->decision()->accept_returning<bool>(v))
    {
        bool suggestion(is_suggestion(_imp->env, id, dep)), recommendation(is_recommendation(_imp->env, id, dep));

        if (! (suggestion || recommendation))
            return si_take;

        if (dep.spec().if_package())
        {
            for (auto l(_imp->take_specs.begin()), l_end(_imp->take_specs.end()) ;
                    l != l_end ; ++l)
                if (match_qpns(*_imp->env, *l, dep.spec().if_package()->package_name_requirement()->name()))
                    return si_take;
        }

        for (auto l(_imp->take_from_specs.begin()), l_end(_imp->take_from_specs.end()) ;
                l != l_end ; ++l)
        {
            if (match_package(*_imp->env, *l, id, make_null_shared_ptr(), { }))
                return si_take;
        }

        std::string spec_group;
        {
            const DepSpec & spec(dep.spec().if_block() ? static_cast<const DepSpec &>(*dep.spec().if_block()) : *dep.spec().if_package());
            if (spec.maybe_annotations())
            {
                auto a(spec.maybe_annotations()->find(dsar_suggestions_group_name));
                if (a != spec.maybe_annotations()->end())
                    spec_group = a->value();
            }
        }

        if ((! spec_group.empty()) && _imp->take_groups.end() != _imp->take_groups.find(spec_group))
            return si_take;

        if (dep.spec().if_package())
        {
            for (auto l(_imp->ignore_specs.begin()), l_end(_imp->ignore_specs.end()) ;
                    l != l_end ; ++l)
                if (match_qpns(*_imp->env, *l, dep.spec().if_package()->package_name_requirement()->name()))
                    return si_ignore;
        }

        for (auto l(_imp->ignore_from_specs.begin()), l_end(_imp->ignore_from_specs.end()) ;
                l != l_end ; ++l)
        {
            if (match_package(*_imp->env, *l, id, make_null_shared_ptr(), { }))
                return si_ignore;
        }

        if ((! spec_group.empty()) && _imp->ignore_groups.end() != _imp->ignore_groups.find(spec_group))
            return si_ignore;

        if (dep.spec().if_package() && (suggestion || recommendation))
        {
            auto e(_imp->env->interest_in_suggestion(id, *dep.spec().if_package()));
            if (e.is_true())
                return si_take;
            else if (e.is_false())
                return si_ignore;
        }

        if (suggestion)
        {
            if (_imp->take_suggestions.is_true())
                return si_take;
            else if (_imp->take_suggestions.is_false())
                return si_ignore;
        }

        if (recommendation)
        {
            if (_imp->take_recommendations.is_true())
                return si_take;
            else if (_imp->take_recommendations.is_false())
                return si_ignore;
        }

        /* we also take suggestions and recommendations that have already been installed */
        if (dep.spec().if_package())
        {
            const std::shared_ptr<const PackageIDSequence> installed_ids(
                    (*_imp->env)[selection::SomeArbitraryVersion(
                        generator::Matches(*dep.spec().if_package(), dep.from_id(), { }) |
                        filter::InstalledAtRoot(_imp->env->system_root_key()->value()))]);
            if (! installed_ids->empty())
                return si_take;
        }

        return si_untaken;
    }
    else
        return si_ignore;
}

template class Pimp<InterestInSpecHelper>;

