/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repository_factory.hh>
#include <paludis/package_database.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/elike_slot_requirement.hh>

#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/sequence-impl.hh>

#include <algorithm>
#include "config.h"

using namespace paludis;
using namespace paludis::resolver;
using namespace paludis::resolver::resolver_test;
using namespace test;

std::string
paludis::resolver::resolver_test::from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
        const std::string & k)
{
    Map<std::string, std::string>::ConstIterator mm(m->find(k));
    if (m->end() == mm)
        return "";
    else
        return mm->second;
}

const std::tr1::shared_ptr<Constraints>
paludis::resolver::resolver_test::initial_constraints_for_fn(
        const InitialConstraints & initial_constraints,
        const Resolvent & resolvent)
{
    InitialConstraints::const_iterator i(initial_constraints.find(resolvent));
    if (i == initial_constraints.end())
        return make_shared_ptr(new Constraints);
    else
        return i->second;
}

std::tr1::shared_ptr<Resolvents>
paludis::resolver::resolver_test::get_resolvents_for_fn(const PackageDepSpec & spec,
        const std::tr1::shared_ptr<const SlotName> & slot,
        const std::tr1::shared_ptr<const Reason> &)
{
    std::tr1::shared_ptr<Resolvents> result(new Resolvents);
    result->push_back(Resolvent(spec, slot ? *slot : SlotName("0"), dt_install_to_slash));
    return result;
}

FilteredGenerator
paludis::resolver::resolver_test::make_destination_filtered_generator_fn(const Generator & g,
        const std::tr1::shared_ptr<const Resolution> & resolution)
{
    switch (resolution->resolvent().destination_type())
    {
        case dt_install_to_slash:
            return g | filter::InstalledAtRoot(FSEntry("/"));

        case dt_create_binary:
            throw InternalError(PALUDIS_HERE, "no dt_create_binary yet");

        case last_dt:
            break;
    }

    throw InternalError(PALUDIS_HERE, "unhandled dt");
}

FilteredGenerator
paludis::resolver::resolver_test::make_origin_filtered_generator_fn(const Generator & g,
        const std::tr1::shared_ptr<const Resolution> &)
{
    return g;
}

DestinationTypes
paludis::resolver::resolver_test::get_destination_types_for_fn(const PackageDepSpec &,
        const std::tr1::shared_ptr<const PackageID> &,
        const std::tr1::shared_ptr<const Reason> &)
{
    return DestinationTypes() + dt_install_to_slash;
}

namespace
{
#ifdef ENABLE_VIRTUALS_REPOSITORY
    std::string virtuals_repo_keys(const std::string & k)
    {
        if (k == "format")
            return "virtuals";
        else if (k == "root")
            return "/";
        else
            return "";
    }

    std::string installed_virtuals_repo_keys(const std::string & k)
    {
        if (k == "format")
            return "installed_virtuals";
        else if (k == "root")
            return "/";
        else
            return "";
    }
#endif
}

SpecInterest
paludis::resolver::resolver_test::interest_in_spec_fn(
        const std::tr1::shared_ptr<const Resolution> &, const SanitisedDependency & dep)
{
    if (is_suggestion(dep))
        return si_untaken;
    else
        return si_take;
}

UseExisting
paludis::resolver::resolver_test::get_use_existing_fn(
        const std::tr1::shared_ptr<const Resolution> &,
        const PackageDepSpec &,
        const std::tr1::shared_ptr<const Reason> &)
{
    return ue_never;
}

const std::tr1::shared_ptr<const Repository>
paludis::resolver::resolver_test::find_repository_for_fn(
        const Environment * const env,
        const std::tr1::shared_ptr<const Resolution> &,
        const ChangesToMakeDecision &)
{
    return env->package_database()->fetch_repository(RepositoryName("installed"));
}

bool
paludis::resolver::resolver_test::allowed_to_remove_fn(
        const std::tr1::shared_ptr<const QualifiedPackageNameSet> & s,
        const std::tr1::shared_ptr<const Resolution> &,
        const std::tr1::shared_ptr<const PackageID> & i)
{
    return s->end() != s->find(i->name());
}

bool
paludis::resolver::resolver_test::remove_if_dependent_fn(
        const std::tr1::shared_ptr<const QualifiedPackageNameSet> & s,
        const std::tr1::shared_ptr<const PackageID> & i)
{
    return s->end() != s->find(i->name());
}

Tribool
paludis::resolver::resolver_test::prefer_or_avoid_fn(
        const std::tr1::shared_ptr<const Map<QualifiedPackageName, bool> > & s,
        const QualifiedPackageName & q)
{
    const Map<QualifiedPackageName, bool>::ConstIterator r(s->find(q));
    if (s->end() != r)
        return Tribool(r->second);
    else
        return indeterminate;
}

Tribool
paludis::resolver::resolver_test::order_early_fn(
        const std::tr1::shared_ptr<const Resolution> &)
{
    return indeterminate;
}

bool
paludis::resolver::resolver_test::confirm_fn(
        const std::tr1::shared_ptr<const Resolution> &,
        const std::tr1::shared_ptr<const RequiredConfirmation> &)
{
    return true;
}

const std::tr1::shared_ptr<ConstraintSequence>
paludis::resolver::resolver_test::get_constraints_for_dependent_fn(
        const std::tr1::shared_ptr<const Resolution> &,
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const ChangeByResolventSequence> & ids)
{
    const std::tr1::shared_ptr<ConstraintSequence> result(new ConstraintSequence);

    PartiallyMadePackageDepSpec partial_spec((PartiallyMadePackageDepSpecOptions()));
    partial_spec.package(id->name());
    if (id->slot_key())
        partial_spec.slot_requirement(make_shared_ptr(new ELikeSlotExactRequirement(
                        id->slot_key()->value(), false)));
    PackageDepSpec spec(partial_spec);

    for (ChangeByResolventSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
            i != i_end ; ++i)
    {
        const std::tr1::shared_ptr<DependentReason> reason(new DependentReason(*i));

        result->push_back(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                            n::destination_type() = dt_install_to_slash,
                            n::nothing_is_fine_too() = true,
                            n::reason() = reason,
                            n::spec() = BlockDepSpec("!" + stringify(spec), spec, false),
                            n::untaken() = false,
                            n::use_existing() = ue_if_possible
                            ))));
    }

    return result;
}

const std::tr1::shared_ptr<ConstraintSequence>
paludis::resolver::resolver_test::get_constraints_for_purge_fn(
        const std::tr1::shared_ptr<const Resolution> &,
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const ChangeByResolventSequence> & ids)
{
    const std::tr1::shared_ptr<ConstraintSequence> result(new ConstraintSequence);

    PartiallyMadePackageDepSpec partial_spec((PartiallyMadePackageDepSpecOptions()));
    partial_spec.package(id->name());
    if (id->slot_key())
        partial_spec.slot_requirement(make_shared_ptr(new ELikeSlotExactRequirement(
                        id->slot_key()->value(), false)));
    PackageDepSpec spec(partial_spec);

    const std::tr1::shared_ptr<WasUsedByReason> reason(new WasUsedByReason(ids));

    result->push_back(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                        n::destination_type() = dt_install_to_slash,
                        n::nothing_is_fine_too() = true,
                        n::reason() = reason,
                        n::spec() = BlockDepSpec("!" + stringify(spec), spec, false),
                        n::untaken() = false,
                        n::use_existing() = ue_if_possible
                        ))));

    return result;
}

const std::tr1::shared_ptr<ConstraintSequence>
paludis::resolver::resolver_test::get_constraints_for_via_binary_fn(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const Resolution> & because_resolution)
{
    PartiallyMadePackageDepSpec partial_spec((PartiallyMadePackageDepSpecOptions()));
    partial_spec.package(resolution->resolvent().package());
    PackageDepSpec spec(partial_spec);

    std::tr1::shared_ptr<ConstraintSequence> result(new ConstraintSequence);
    result->push_back(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                        n::destination_type() = resolution->resolvent().destination_type(),
                        n::nothing_is_fine_too() = false,
                        n::reason() = make_shared_ptr(new ViaBinaryReason(because_resolution->resolvent())),
                        n::spec() = spec,
                        n::untaken() = false,
                        n::use_existing() = ue_if_possible
                        ))));

    return result;
}

bool
paludis::resolver::resolver_test::can_use_fn(
        const std::tr1::shared_ptr<const PackageID> &)
{
    return true;
}

bool
paludis::resolver::resolver_test::always_via_binary_fn(
        const std::tr1::shared_ptr<const Resolution> &)
{
    return false;
}

ResolverTestCase::ResolverTestCase(const std::string & t, const std::string & s, const std::string & e,
        const std::string & l) :
    TestCase(s),
    allowed_to_remove_names(new QualifiedPackageNameSet),
    remove_if_dependent_names(new QualifiedPackageNameSet),
    prefer_or_avoid_names(new Map<QualifiedPackageName, bool>)
{
    std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
    keys->insert("format", "exheres");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSEntry::cwd() / ("resolver_TEST_" + t + "_dir") / "repo"));
    keys->insert("profiles", stringify(FSEntry::cwd() / ("resolver_TEST_" + t + "_dir") / "repo/profiles/profile"));
    keys->insert("layout", l);
    keys->insert("eapi_when_unknown", e);
    keys->insert("eapi_when_unspecified", e);
    keys->insert("profile_eapi", e);
    keys->insert("distdir", stringify(FSEntry::cwd() / ("resolver_TEST_" + t + "_dir") / "distdir"));
    keys->insert("builddir", stringify(FSEntry::cwd() / ("resolver_TEST_" + t + "_dir") / "build"));
    repo = RepositoryFactory::get_instance()->create(&env,
            std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1));
    env.package_database()->add_repository(1, repo);

    keys.reset(new Map<std::string, std::string>);
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("provides_cache", "/var/empty");
    keys->insert("location", stringify(FSEntry::cwd() / ("resolver_TEST_" + t + "_dir") / "installed"));
    keys->insert("builddir", stringify(FSEntry::cwd() / ("resolver_TEST_" + t + "_dir") / "build"));
    inst_repo = RepositoryFactory::get_instance()->create(&env,
            std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1));
    env.package_database()->add_repository(1, inst_repo);

    fake_inst_repo.reset(new FakeInstalledRepository(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake-inst"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));
    env.package_database()->add_repository(1, fake_inst_repo);

#ifdef ENABLE_VIRTUALS_REPOSITORY
    env.package_database()->add_repository(0, RepositoryFactory::get_instance()->create(&env, virtuals_repo_keys));
    env.package_database()->add_repository(0, RepositoryFactory::get_instance()->create(&env, installed_virtuals_repo_keys));
#endif
}

ResolverFunctions
ResolverTestCase::get_resolver_functions(InitialConstraints & initial_constraints)
{
    return make_named_values<ResolverFunctions>(
            n::allowed_to_remove_fn() = std::tr1::bind(&allowed_to_remove_fn,
                    allowed_to_remove_names, std::tr1::placeholders::_1, std::tr1::placeholders::_2),
            n::always_via_binary_fn() = &always_via_binary_fn,
            n::can_use_fn() = &can_use_fn,
            n::confirm_fn() = &confirm_fn,
            n::find_repository_for_fn() = std::tr1::bind(&find_repository_for_fn,
                    &env, std::tr1::placeholders::_1, std::tr1::placeholders::_2),
            n::get_constraints_for_dependent_fn() = &get_constraints_for_dependent_fn,
            n::get_constraints_for_purge_fn() = &get_constraints_for_purge_fn,
            n::get_constraints_for_via_binary_fn() = &get_constraints_for_via_binary_fn,
            n::get_destination_types_for_fn() = &get_destination_types_for_fn,
            n::get_initial_constraints_for_fn() =
                std::tr1::bind(&initial_constraints_for_fn, std::tr1::ref(initial_constraints),
                    std::tr1::placeholders::_1),
            n::get_resolvents_for_fn() = &get_resolvents_for_fn,
            n::get_use_existing_fn() = &get_use_existing_fn,
            n::interest_in_spec_fn() = &interest_in_spec_fn,
            n::make_destination_filtered_generator_fn() = &make_destination_filtered_generator_fn,
            n::make_origin_filtered_generator_fn() = &make_origin_filtered_generator_fn,
            n::order_early_fn() = &order_early_fn,
            n::prefer_or_avoid_fn() = std::tr1::bind(&prefer_or_avoid_fn,
                    prefer_or_avoid_names, std::tr1::placeholders::_1),
            n::remove_if_dependent_fn() = std::tr1::bind(&remove_if_dependent_fn,
                    remove_if_dependent_names, std::tr1::placeholders::_1)
            );
}

const std::tr1::shared_ptr<const Resolved>
ResolverTestCase::get_resolved(const PackageOrBlockDepSpec & target)
{
    InitialConstraints initial_constraints;

    while (true)
    {
        try
        {
            Resolver resolver(&env, get_resolver_functions(initial_constraints));
            resolver.add_target(target, "");
            resolver.resolve();
            return resolver.resolved();
        }
        catch (const SuggestRestart & e)
        {
            initial_constraints.insert(std::make_pair(e.resolvent(), make_shared_ptr(new Constraints))).first->second->add(e.suggested_preset());
        }
    }
}

const std::tr1::shared_ptr<const Resolved>
ResolverTestCase::get_resolved(const std::string & target)
{
    PackageDepSpec target_spec(parse_user_package_dep_spec(target, &env, UserPackageDepSpecOptions()));
    return get_resolved(target_spec);
}

namespace
{
    template <typename T_>
    std::tr1::shared_ptr<T_> get_decision(const std::tr1::shared_ptr<T_> & d)
    {
        return d;
    }

    template <typename T_, typename N_>
    std::tr1::shared_ptr<T_> get_decision(const std::pair<std::tr1::shared_ptr<T_>, N_> & d)
    {
        return d.first;
    }
}

template <typename Decisions_>
void
ResolverTestCase::check_resolved_one(
        const std::tr1::shared_ptr<Decisions_> & decisions,
        const std::tr1::shared_ptr<const DecisionChecks> & decision_checks)
{
    DecisionChecks::List::const_iterator decision_check(decision_checks->checks.begin()), decision_check_end(decision_checks->checks.end());
    typename Decisions_::ConstIterator decision(decisions->begin()), decision_end(decisions->end());

    while (true)
    {
        if (decision_check == decision_check_end)
            break;

        std::tr1::shared_ptr<const Decision> d;
        if (decision != decision_end)
            d = get_decision(*decision++);

        TEST_CHECK_MESSAGE(decision_check->first(d), decision_check->second(d));
        ++decision_check;
    }

    TEST_CHECK(decision_check == decision_check_end);
    TEST_CHECK(decision == decision_end);
}

void
ResolverTestCase::check_resolved(
        const std::tr1::shared_ptr<const Resolved> & resolved,
        const NamedValue<n::taken_change_or_remove_decisions, const std::tr1::shared_ptr<const DecisionChecks> > & taken_change_or_remove_decisions,
        const NamedValue<n::taken_unable_to_make_decisions, const std::tr1::shared_ptr<const DecisionChecks> > & taken_unable_to_make_decisions,
        const NamedValue<n::taken_unconfirmed_decisions, const std::tr1::shared_ptr<const DecisionChecks> > & taken_unconfirmed_decisions,
        const NamedValue<n::taken_unorderable_decisions, const std::tr1::shared_ptr<const DecisionChecks> > & taken_unorderable_decisions,
        const NamedValue<n::untaken_change_or_remove_decisions, const std::tr1::shared_ptr<const DecisionChecks> > & untaken_change_or_remove_decisions,
        const NamedValue<n::untaken_unable_to_make_decisions, const std::tr1::shared_ptr<const DecisionChecks> > & untaken_unable_to_make_decisions
        )
{
    {
        TestMessageSuffix s("taken change or remove");
        check_resolved_one(resolved->taken_change_or_remove_decisions(), taken_change_or_remove_decisions());
    }

    {
        TestMessageSuffix s("taken unable to make");
        check_resolved_one(resolved->taken_unable_to_make_decisions(), taken_unable_to_make_decisions());
    }

    {
        TestMessageSuffix s("taken unconfirmed");
        check_resolved_one(resolved->taken_unconfirmed_decisions(), taken_unconfirmed_decisions());
    }

    {
        TestMessageSuffix s("taken unorderable");
        check_resolved_one(resolved->taken_unorderable_decisions(), taken_unorderable_decisions());
    }

    {
        TestMessageSuffix s("untaken change or remove");
        check_resolved_one(resolved->untaken_change_or_remove_decisions(), untaken_change_or_remove_decisions());
    }

    {
        TestMessageSuffix s("untaken unable to make");
        check_resolved_one(resolved->untaken_unable_to_make_decisions(), untaken_unable_to_make_decisions());
    }
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
                std::tr1::bind(&check_breaking, q, std::tr1::placeholders::_1),
                std::tr1::bind(&check_breaking_msg, q, std::tr1::placeholders::_1)
                ));
    return *this;
}

ResolverTestCase::DecisionChecks &
ResolverTestCase::DecisionChecks::change(const QualifiedPackageName & q)
{
    checks.push_back(std::make_pair(
                std::tr1::bind(&check_change, q, std::tr1::placeholders::_1),
                std::tr1::bind(&check_change_msg, q, std::tr1::placeholders::_1)
                ));
    return *this;
}

ResolverTestCase::DecisionChecks &
ResolverTestCase::DecisionChecks::remove(const QualifiedPackageName & q)
{
    checks.push_back(std::make_pair(
                std::tr1::bind(&check_remove, q, std::tr1::placeholders::_1),
                std::tr1::bind(&check_remove_msg, q, std::tr1::placeholders::_1)
                ));
    return *this;
}

ResolverTestCase::DecisionChecks &
ResolverTestCase::DecisionChecks::unable(const QualifiedPackageName & q)
{
    checks.push_back(std::make_pair(
                std::tr1::bind(&check_unable, q, std::tr1::placeholders::_1),
                std::tr1::bind(&check_unable_msg, q, std::tr1::placeholders::_1)
                ));
    return *this;
}

bool
ResolverTestCase::DecisionChecks::check_finished(const std::tr1::shared_ptr<const Decision> & r)
{
    return ! r;
}

std::string
ResolverTestCase::DecisionChecks::check_finished_msg(const std::tr1::shared_ptr<const Decision> & r)
{
    return check_generic_msg("finished", r);
}

bool
ResolverTestCase::DecisionChecks::check_change(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & d)
{
    if (! d)
        return false;

    return simple_visitor_cast<const ChangesToMakeDecision>(*d) && d->resolvent().package() == q;
}

bool
ResolverTestCase::DecisionChecks::check_breaking(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & d)
{
    if (! d)
        return false;

    return simple_visitor_cast<const BreakDecision>(*d) && d->resolvent().package() == q;
}

bool
ResolverTestCase::DecisionChecks::check_remove(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & d)
{
    if (! d)
        return false;

    return simple_visitor_cast<const RemoveDecision>(*d) && d->resolvent().package() == q;
}

bool
ResolverTestCase::DecisionChecks::check_unable(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & d)
{
    if (! d)
        return false;

    return simple_visitor_cast<const UnableToMakeDecision>(*d) && d->resolvent().package() == q;
}

std::string
ResolverTestCase::DecisionChecks::check_change_msg(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & r)
{
    return check_generic_msg(stringify(q), r);
}

std::string
ResolverTestCase::DecisionChecks::check_breaking_msg(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & r)
{
    return check_generic_msg("break " + stringify(q), r);
}

std::string
ResolverTestCase::DecisionChecks::check_remove_msg(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & r)
{
    return check_generic_msg("remove " + stringify(q), r);
}

std::string
ResolverTestCase::DecisionChecks::check_unable_msg(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & r)
{
    return check_generic_msg("unable " + stringify(q), r);
}

namespace
{
    struct DecisionStringifier
    {
        std::string visit(const NothingNoChangeDecision &) const
        {
            return "NothingNoChangeDecision";
        }

        std::string visit(const UnableToMakeDecision & d) const
        {
            return "UnableToMakeDecision(" + stringify(d.resolvent()) + ")";
        }

        std::string visit(const ChangesToMakeDecision & d) const
        {
            return "ChangesToMakeDecision(" + stringify(*d.origin_id()) + ")";
        }

        std::string visit(const ExistingNoChangeDecision &) const
        {
            return "ExistingNoChangeDecision";
        }

        std::string visit(const BreakDecision &) const
        {
            return "BreakDecision";
        }

        std::string visit(const RemoveDecision & d) const
        {
            return "RemoveDecision(" + join(indirect_iterator(d.ids()->begin()), indirect_iterator(d.ids()->end()), ", ") + ")";
        }
    };
}

std::string
ResolverTestCase::DecisionChecks::check_generic_msg(const std::string & q, const std::tr1::shared_ptr<const Decision> & r)
{
    if (! r)
        return "Expected " + stringify(q) + " but got finished";
    else
        return "Expected " + stringify(q) + " but got " + r->accept_returning<std::string>(DecisionStringifier());
}

const std::tr1::shared_ptr<FakePackageID>
ResolverTestCase::install(const std::string & c, const std::string & p, const std::string & v)
{
    return fake_inst_repo->add_version(c, p, v);
}

