/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/resolver/resolver_test.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/decisions.hh>
#include <paludis/resolver/decider.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/change_by_resolvent.hh>
#include <paludis/resolver/labels_classifier.hh>

#include <paludis/util/map.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/join.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/sequence-impl.hh>

#include <paludis/repository_factory.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/elike_slot_requirement.hh>
#include <paludis/partially_made_package_dep_spec.hh>

#include <paludis/repositories/fake/fake_installed_repository.hh>

#include <algorithm>
#include "config.h"

using namespace paludis;
using namespace paludis::resolver;
using namespace paludis::resolver::resolver_test;

std::string
paludis::resolver::resolver_test::from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
        const std::string & k)
{
    Map<std::string, std::string>::ConstIterator mm(m->find(k));
    if (m->end() == mm)
        return "";
    else
        return mm->second;
}

ResolverTestData::ResolverTestData(const std::string & t, const std::string & e, const std::string & l) :
    allow_choice_changes_helper(&env),
    allowed_to_remove_helper(&env),
    allowed_to_restart_helper(&env),
    always_via_binary_helper(&env),
    can_use_helper(&env),
    confirm_helper(&env),
    find_replacing_helper(&env),
    find_repository_for_helper(&env),
    get_constraints_for_dependent_helper(&env),
    get_constraints_for_purge_helper(&env),
    get_constraints_for_via_binary_helper(&env),
    get_destination_types_for_blocker_helper(&env),
    get_destination_types_for_error_helper(&env),
    get_initial_constraints_for_helper(&env),
    get_use_existing_nothing_helper(&env),
    interest_in_spec_helper(&env),
    make_destination_filtered_generator_helper(&env),
    make_origin_filtered_generator_helper(&env),
    make_unmaskable_filter_helper(&env),
    order_early_helper(&env),
    prefer_or_avoid_helper(&env),
    promote_binaries_helper(&env),
    remove_hidden_helper(&env),
    remove_if_dependent_helper(&env),
    get_resolvents_for_helper(&env, std::cref(remove_hidden_helper))
{
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / ("resolver_TEST_" + t + "_dir") / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / ("resolver_TEST_" + t + "_dir") / "repo/profiles/profile"));
    keys->insert("layout", l);
    keys->insert("eapi_when_unknown", e);
    keys->insert("eapi_when_unspecified", e);
    keys->insert("profile_eapi", e);
    keys->insert("distdir", stringify(FSPath::cwd() / ("resolver_TEST_" + t + "_dir") / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / ("resolver_TEST_" + t + "_dir") / "build"));
    repo = RepositoryFactory::get_instance()->create(&env,
            std::bind(from_keys, keys, std::placeholders::_1));
    env.add_repository(1, repo);

    keys = std::make_shared<Map<std::string, std::string>>();
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / ("resolver_TEST_" + t + "_dir") / "installed"));
    keys->insert("builddir", stringify(FSPath::cwd() / ("resolver_TEST_" + t + "_dir") / "build"));
    inst_repo = RepositoryFactory::get_instance()->create(&env,
            std::bind(from_keys, keys, std::placeholders::_1));
    env.add_repository(1, inst_repo);

    fake_inst_repo = std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake-inst"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    ));
    env.add_repository(1, fake_inst_repo);

    interest_in_spec_helper.set_follow_installed_dependencies(true);
    interest_in_spec_helper.set_follow_installed_build_dependencies(true);

    make_unmaskable_filter_helper.set_override_masks(false);
}

ResolverWithBinaryTestData::ResolverWithBinaryTestData(const std::string & t, const std::string & e, const std::string & l, bool u) :
    ResolverTestData(t, e, l)
{
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / ("resolver_TEST_" + t + "_dir") / "binrepo"));
    keys->insert("profiles", stringify(FSPath::cwd() / ("resolver_TEST_" + t + "_dir") / "repo/profiles/profile"));
    keys->insert("layout", l);
    keys->insert("eapi_when_unknown", e);
    keys->insert("eapi_when_unspecified", e);
    keys->insert("profile_eapi", e);
    keys->insert("binary_destination", "true");
    keys->insert("binary_keywords_filter", "test ~test");
    keys->insert("distdir", stringify(FSPath::cwd() / ("resolver_TEST_" + t + "_dir") / "distdir"));
    keys->insert("binary_distdir", stringify(FSPath::cwd() / ("resolver_TEST_" + t + "_dir") / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / ("resolver_TEST_" + t + "_dir") / "build"));
    bin_repo = RepositoryFactory::get_instance()->create(&env,
            std::bind(from_keys, keys, std::placeholders::_1));
    env.add_repository(u ? 0 : 2, bin_repo);
}

ResolverFunctions
ResolverTestData::get_resolver_functions()
{
    return make_named_values<ResolverFunctions>(
            n::allow_choice_changes_fn() = std::cref(allow_choice_changes_helper),
            n::allowed_to_remove_fn() = std::cref(allowed_to_remove_helper),
            n::allowed_to_restart_fn() = std::cref(allowed_to_restart_helper),
            n::always_via_binary_fn() = std::cref(always_via_binary_helper),
            n::can_use_fn() = std::cref(can_use_helper),
            n::confirm_fn() = std::cref(confirm_helper),
            n::find_replacing_fn() = std::cref(find_replacing_helper),
            n::find_repository_for_fn() = std::cref(find_repository_for_helper),
            n::get_constraints_for_dependent_fn() = std::cref(get_constraints_for_dependent_helper),
            n::get_constraints_for_purge_fn() = std::cref(get_constraints_for_purge_helper),
            n::get_constraints_for_via_binary_fn() = std::cref(get_constraints_for_via_binary_helper),
            n::get_destination_types_for_blocker_fn() = std::cref(get_destination_types_for_blocker_helper),
            n::get_destination_types_for_error_fn() = std::cref(get_destination_types_for_error_helper),
            n::get_initial_constraints_for_fn() = std::cref(get_initial_constraints_for_helper),
            n::get_resolvents_for_fn() = std::cref(get_resolvents_for_helper),
            n::get_use_existing_nothing_fn() = std::cref(get_use_existing_nothing_helper),
            n::interest_in_spec_fn() = std::cref(interest_in_spec_helper),
            n::make_destination_filtered_generator_fn() = std::cref(make_destination_filtered_generator_helper),
            n::make_origin_filtered_generator_fn() = std::cref(make_origin_filtered_generator_helper),
            n::make_unmaskable_filter_fn() = std::cref(make_unmaskable_filter_helper),
            n::order_early_fn() = std::cref(order_early_helper),
            n::prefer_or_avoid_fn() = std::cref(prefer_or_avoid_helper),
            n::promote_binaries_fn() = std::cref(promote_binaries_helper),
            n::remove_hidden_fn() = std::cref(remove_hidden_helper),
            n::remove_if_dependent_fn() = std::cref(remove_if_dependent_helper)
            );
}

const std::shared_ptr<const Resolved>
ResolverTestData::get_resolved(const PackageOrBlockDepSpec & target)
{
    while (true)
    {
        try
        {
            Resolver resolver(&env, get_resolver_functions());
            resolver.add_target(target, "");
            resolver.resolve();
            return resolver.resolved();
        }
        catch (const SuggestRestart & e)
        {
            get_initial_constraints_for_helper.add_suggested_restart(e);
        }
    }
}

const std::shared_ptr<const Resolved>
ResolverTestData::get_resolved(const std::string & target)
{
    PackageDepSpec target_spec(parse_user_package_dep_spec(target, &env, { }));
    return get_resolved(target_spec);
}

namespace
{
    template <typename T_>
    std::shared_ptr<T_> get_decision(const std::shared_ptr<T_> & d)
    {
        return d;
    }

    template <typename T_, typename N_>
    std::shared_ptr<T_> get_decision(const std::pair<std::shared_ptr<T_>, N_> & d)
    {
        return d.first;
    }
}

template <typename Decisions_>
void
ResolverTestCase::check_resolved_one(
        const std::shared_ptr<Decisions_> & decisions,
        const std::shared_ptr<const DecisionChecks> & decision_checks)
{
    DecisionChecks::List::const_iterator decision_check(decision_checks->checks.begin());
    DecisionChecks::List::const_iterator decision_check_end(decision_checks->checks.end());
    typename Decisions_::ConstIterator decision(decisions->begin());
    typename Decisions_::ConstIterator decision_end(decisions->end());

    while (true)
    {
        if (decision_check == decision_check_end)
            break;

        std::shared_ptr<const Decision> d;
        if (decision != decision_end)
            d = get_decision(*decision++);

        EXPECT_TRUE(decision_check->first(d)) << decision_check->second(d);
        ++decision_check;
    }

    ASSERT_TRUE(decision_check == decision_check_end);
    ASSERT_TRUE(decision == decision_end);
}

void
ResolverTestCase::check_resolved(
        const std::shared_ptr<const Resolved> & resolved,
        const NamedValue<n::taken_change_or_remove_decisions, const std::shared_ptr<const DecisionChecks> > & taken_change_or_remove_decisions,
        const NamedValue<n::taken_unable_to_make_decisions, const std::shared_ptr<const DecisionChecks> > & taken_unable_to_make_decisions,
        const NamedValue<n::taken_unconfirmed_decisions, const std::shared_ptr<const DecisionChecks> > & taken_unconfirmed_decisions,
        const NamedValue<n::taken_unorderable_decisions, const std::shared_ptr<const DecisionChecks> > & taken_unorderable_decisions,
        const NamedValue<n::untaken_change_or_remove_decisions, const std::shared_ptr<const DecisionChecks> > & untaken_change_or_remove_decisions,
        const NamedValue<n::untaken_unable_to_make_decisions, const std::shared_ptr<const DecisionChecks> > & untaken_unable_to_make_decisions
        )
{
    check_resolved_one(resolved->taken_change_or_remove_decisions(), taken_change_or_remove_decisions());
    check_resolved_one(resolved->taken_unable_to_make_decisions(), taken_unable_to_make_decisions());
    check_resolved_one(resolved->taken_unconfirmed_decisions(), taken_unconfirmed_decisions());
    check_resolved_one(resolved->taken_unorderable_decisions(), taken_unorderable_decisions());
    check_resolved_one(resolved->untaken_change_or_remove_decisions(), untaken_change_or_remove_decisions());
    check_resolved_one(resolved->untaken_unable_to_make_decisions(), untaken_unable_to_make_decisions());
}

ResolverTestCase::DecisionChecks &
ResolverTestCase::DecisionChecks::finished()
{
    checks.push_back(std::make_pair(
                &check_finished,
                &check_finished_msg
                ));
    return *this;
}

ResolverTestCase::DecisionChecks &
ResolverTestCase::DecisionChecks::breaking(const QualifiedPackageName & q)
{
    checks.push_back(std::make_pair(
                std::bind(&check_breaking, q, std::placeholders::_1),
                std::bind(&check_breaking_msg, q, std::placeholders::_1)
                ));
    return *this;
}

ResolverTestCase::DecisionChecks &
ResolverTestCase::DecisionChecks::change_slot(const QualifiedPackageName & q, const SlotName & s)
{
    checks.push_back(std::make_pair(
                std::bind(&check_change_slot, q, s, std::placeholders::_1),
                std::bind(&check_change_slot_msg, q, s, std::placeholders::_1)
                ));
    return *this;
}

ResolverTestCase::DecisionChecks &
ResolverTestCase::DecisionChecks::change(const QualifiedPackageName & q)
{
    checks.push_back(std::make_pair(
                std::bind(&check_change, q, std::placeholders::_1),
                std::bind(&check_change_msg, q, std::placeholders::_1)
                ));
    return *this;
}

ResolverTestCase::DecisionChecks &
ResolverTestCase::DecisionChecks::remove(const QualifiedPackageName & q)
{
    checks.push_back(std::make_pair(
                std::bind(&check_remove, q, std::placeholders::_1),
                std::bind(&check_remove_msg, q, std::placeholders::_1)
                ));
    return *this;
}

ResolverTestCase::DecisionChecks &
ResolverTestCase::DecisionChecks::unable(const QualifiedPackageName & q)
{
    checks.push_back(std::make_pair(
                std::bind(&check_unable, q, std::placeholders::_1),
                std::bind(&check_unable_msg, q, std::placeholders::_1)
                ));
    return *this;
}

bool
ResolverTestCase::DecisionChecks::check_finished(const std::shared_ptr<const Decision> & r)
{
    return ! r;
}

std::string
ResolverTestCase::DecisionChecks::check_finished_msg(const std::shared_ptr<const Decision> & r)
{
    return check_generic_msg("finished", r);
}

bool
ResolverTestCase::DecisionChecks::check_change(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & d)
{
    if (! d)
        return false;

    return visitor_cast<const ChangesToMakeDecision>(*d) && d->resolvent().package() == q;
}

bool
ResolverTestCase::DecisionChecks::check_change_slot(const QualifiedPackageName & q, const SlotName & s, const std::shared_ptr<const Decision> & d)
{
    if (! d)
        return false;

    return visitor_cast<const ChangesToMakeDecision>(*d) && d->resolvent().package() == q &&
        d->resolvent().slot().name_or_null() && *d->resolvent().slot().name_or_null() == s;
}

bool
ResolverTestCase::DecisionChecks::check_breaking(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & d)
{
    if (! d)
        return false;

    return visitor_cast<const BreakDecision>(*d) && d->resolvent().package() == q;
}

bool
ResolverTestCase::DecisionChecks::check_remove(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & d)
{
    if (! d)
        return false;

    return visitor_cast<const RemoveDecision>(*d) && d->resolvent().package() == q;
}

bool
ResolverTestCase::DecisionChecks::check_unable(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & d)
{
    if (! d)
        return false;

    return visitor_cast<const UnableToMakeDecision>(*d) && d->resolvent().package() == q;
}

std::string
ResolverTestCase::DecisionChecks::check_change_msg(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & r)
{
    return check_generic_msg(stringify(q), r);
}

std::string
ResolverTestCase::DecisionChecks::check_change_slot_msg(const QualifiedPackageName & q, const SlotName & s, const std::shared_ptr<const Decision> & r)
{
    return check_generic_msg(stringify(q) + ":" + stringify(s), r);
}

std::string
ResolverTestCase::DecisionChecks::check_breaking_msg(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & r)
{
    return check_generic_msg("break " + stringify(q), r);
}

std::string
ResolverTestCase::DecisionChecks::check_remove_msg(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & r)
{
    return check_generic_msg("remove " + stringify(q), r);
}

std::string
ResolverTestCase::DecisionChecks::check_unable_msg(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & r)
{
    return check_generic_msg("unable " + stringify(q), r);
}

std::string
ResolverTestCase::DecisionChecks::check_generic_msg(const std::string & q, const std::shared_ptr<const Decision> & r)
{
    if (! r)
        return "Expected " + stringify(q) + " but got finished";
    else
        return "Expected " + stringify(q) + " but got " + r->make_accept_returning(
                [&] (const NothingNoChangeDecision &)  { return std::string{ "NothingNoChangeDecision" }; },
                [&] (const UnableToMakeDecision & d)   { return "UnableToMakeDecision(" + stringify(d.resolvent()) + ")"; },
                [&] (const ChangesToMakeDecision & d)  { return "ChangesToMakeDecision(" + stringify(*d.origin_id()) + ")"; },
                [&] (const ExistingNoChangeDecision &) { return "ExistingNoChangeDecision"; },
                [&] (const BreakDecision &)            { return "BreakDecision"; },
                [&] (const RemoveDecision & d) {
                    return "RemoveDecision(" + join(indirect_iterator(d.ids()->begin()), indirect_iterator(d.ids()->end()), ", ") + ")";
                }
                );
}

const std::shared_ptr<FakePackageID>
ResolverTestData::install(const std::string & c, const std::string & p, const std::string & v)
{
    return fake_inst_repo->add_version(c, p, v);
}

