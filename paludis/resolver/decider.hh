/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_DECIDER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_DECIDER_HH 1

#include <paludis/resolver/decider-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/destination_types-fwd.hh>
#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/destination-fwd.hh>
#include <paludis/resolver/unsuitable_candidates-fwd.hh>
#include <paludis/resolver/spec_rewriter-fwd.hh>
#include <paludis/resolver/resolver_functions-fwd.hh>
#include <paludis/resolver/resolver-fwd.hh>
#include <paludis/resolver/any_child_score-fwd.hh>
#include <paludis/resolver/change_type-fwd.hh>
#include <paludis/resolver/package_or_block_dep_spec-fwd.hh>
#include <paludis/resolver/resolutions_by_resolvent-fwd.hh>
#include <paludis/resolver/change_by_resolvent-fwd.hh>
#include <paludis/resolver/why_changed_choices-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/tribool-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/filtered_generator-fwd.hh>
#include <paludis/generator-fwd.hh>
#include <paludis/changed_choices-fwd.hh>
#include <paludis/name-fwd.hh>
#include <tuple>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE Decider
        {
            private:
                Pimp<Decider> _imp;

            private:
                typedef std::tuple<std::shared_ptr<const PackageID>, std::shared_ptr<const WhyChangedChoices>, bool> FoundID;

                const std::shared_ptr<Resolution> _create_resolution_for_resolvent(const Resolvent &) const;
                const std::shared_ptr<Resolution> _resolution_for_resolvent(const Resolvent &, const Tribool);

                const std::shared_ptr<const Resolvents> _get_resolvents_for_blocker(const BlockDepSpec &,
                        const std::shared_ptr<const Reason> & reason) const;

                const DestinationTypes _get_destination_types_for_blocker(const BlockDepSpec &,
                        const std::shared_ptr<const Reason> &) const;

                const std::pair<std::shared_ptr<const Resolvents>, bool> _get_resolvents_for(
                        const PackageDepSpec & spec,
                        const std::shared_ptr<const Reason> & reason) const;

                const DestinationTypes _get_destination_types_for_error(
                        const PackageDepSpec & spec,
                        const std::shared_ptr<const Reason> &) const;

                const std::shared_ptr<const Resolvents> _get_error_resolvents_for(
                            const PackageDepSpec & spec,
                            const std::shared_ptr<const Reason> & reason) const;

                const std::shared_ptr<ConstraintSequence> _make_constraints_from_target(
                        const std::shared_ptr<const Resolution> &,
                        const PackageOrBlockDepSpec &,
                        const std::shared_ptr<const Reason> &) const;

                const std::shared_ptr<ConstraintSequence> _make_constraints_from_dependency(
                        const std::shared_ptr<const Resolution> &,
                        const SanitisedDependency &,
                        const std::shared_ptr<const Reason> &,
                        const SpecInterest) const;

                const std::shared_ptr<Constraint> _make_constraint_from_package_dependency(
                        const std::shared_ptr<const Resolution> &,
                        const SanitisedDependency &,
                        const std::shared_ptr<const Reason> &,
                        const SpecInterest) const;

                const std::shared_ptr<ConstraintSequence> _make_constraints_from_blocker(
                        const std::shared_ptr<const Resolution> &,
                        const BlockDepSpec & dep,
                        const std::shared_ptr<const Reason> & reason) const;

                const std::shared_ptr<ConstraintSequence> _make_constraints_from_other_destination(
                        const std::shared_ptr<const Resolution> & resolution,
                        const std::shared_ptr<const Resolution> & from_resolution,
                        const std::shared_ptr<const Constraint> & from_constraint) const;

                void _apply_resolution_constraint(
                        const std::shared_ptr<Resolution> &,
                        const std::shared_ptr<const Constraint> &);

                bool _check_constraint(
                        const std::shared_ptr<const Constraint> & constraint,
                        const std::shared_ptr<const Decision> & decision) const;

                bool _verify_new_constraint(
                        const std::shared_ptr<const Resolution> &,
                        const std::shared_ptr<const Constraint> &);

                void _made_wrong_decision(
                        const std::shared_ptr<Resolution> & resolution,
                        const std::shared_ptr<const Constraint> & constraint);

                bool _allowed_to_restart(
                        const std::shared_ptr<const Resolution> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                void _suggest_restart_with(
                        const std::shared_ptr<const Resolution> & resolution,
                        const std::shared_ptr<const Constraint> & constraint,
                        const std::shared_ptr<const Decision> & decision) const PALUDIS_ATTRIBUTE((noreturn));

                const std::shared_ptr<const Constraint> _make_constraint_for_preloading(
                        const std::shared_ptr<const Decision> & d,
                        const std::shared_ptr<const Constraint> & c) const;

                const PackageDepSpec _make_spec_for_preloading(
                        const PackageDepSpec & spec,
                        const std::shared_ptr<const ChangedChoices> &) const;

                const std::shared_ptr<const PackageIDSequence> _find_replacing(
                        const std::shared_ptr<const PackageID> &,
                        const std::shared_ptr<const Repository> &) const;

                void _resolve_decide_with_dependencies();
                bool _resolve_vias() PALUDIS_ATTRIBUTE((warn_unused_result));
                bool _resolve_dependents() PALUDIS_ATTRIBUTE((warn_unused_result));
                bool _resolve_purges() PALUDIS_ATTRIBUTE((warn_unused_result));
                void _resolve_destinations();
                void _resolve_confirmations();

                bool _via_binary(const std::shared_ptr<const Resolution> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                void _fixup_changes_to_make_decision(
                        const std::shared_ptr<const Resolution> &,
                        ChangesToMakeDecision &) const;

                const std::shared_ptr<Destination> _make_destination_for(
                        const std::shared_ptr<const Resolution> & resolution,
                        const ChangesToMakeDecision &) const;

                const ChangeType _make_change_type_for(
                        const std::shared_ptr<const Resolution> & resolution,
                        const ChangesToMakeDecision &) const;

                void _decide(const std::shared_ptr<Resolution> & resolution);
                void _copy_other_destination_constraints(const std::shared_ptr<Resolution> & resolution);

                const std::shared_ptr<Decision> _try_to_find_decision_for(
                        const std::shared_ptr<const Resolution> & resolution,
                        const bool also_try_option_changes,
                        const bool try_option_changes_this_time,
                        const bool also_try_masked,
                        const bool try_masked_this_time,
                        const bool try_removes_if_allowed) const;

                const std::shared_ptr<Decision> _cannot_decide_for(
                        const std::shared_ptr<const Resolution> & resolution) const;

                void _add_dependencies_if_necessary(
                        const std::shared_ptr<Resolution> & our_resolution);

                SpecInterest _interest_in_spec(
                        const std::shared_ptr<const Resolution> &,
                        const std::shared_ptr<const PackageID> &,
                        const SanitisedDependency &) const;

                const std::shared_ptr<Constraints> _initial_constraints_for(const Resolvent &) const;

                const std::shared_ptr<const PackageID> _find_existing_id_for(
                        const std::shared_ptr<const Resolution> &) const;

                const std::shared_ptr<const PackageIDSequence> _find_installable_id_candidates_for(
                        const QualifiedPackageName &,
                        const Filter &,
                        const bool include_errors,
                        const bool include_unmaskable) const;

                const FoundID _find_installable_id_for(
                        const std::shared_ptr<const Resolution> &,
                        const bool include_option_changes,
                        const bool include_unmaskable) const;

                const FoundID _find_id_for_from(
                        const std::shared_ptr<const Resolution> &,
                        const std::shared_ptr<const PackageIDSequence> &,
                        const bool try_changing_choices,
                        const bool trying_changing_choices) const;

                const std::shared_ptr<const Constraints> _get_unmatching_constraints(
                        const std::shared_ptr<const Resolution> &,
                        const std::shared_ptr<const PackageID> &,
                        const bool existing) const PALUDIS_ATTRIBUTE((warn_unused_result));

                UnsuitableCandidate _make_unsuitable_candidate(
                        const std::shared_ptr<const Resolution> &,
                        const std::shared_ptr<const PackageID> &,
                        const bool existing) const;

                bool _package_dep_spec_already_met(
                        const PackageDepSpec &,
                        const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                bool _block_dep_spec_already_met(
                        const BlockDepSpec &,
                        const std::shared_ptr<const PackageID> &,
                        const Resolvent &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                bool _installed_but_allowed_to_remove(
                        const std::shared_ptr<const Resolution> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::shared_ptr<const PackageIDSequence> _installed_ids(
                        const std::shared_ptr<const Resolution> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                bool _allowed_to_remove(
                        const std::shared_ptr<const Resolution> &,
                        const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                bool _remove_if_dependent(const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::pair<
                    std::shared_ptr<const ChangeByResolventSequence>,
                    std::shared_ptr<const ChangeByResolventSequence> > _collect_changing() const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::shared_ptr<const PackageIDSequence> _collect_staying(
                        const std::shared_ptr<const ChangeByResolventSequence> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                void _confirm(const std::shared_ptr<const Resolution> & resolution);

                bool _can_use(
                        const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            public:
                Decider(const Environment * const,
                        const ResolverFunctions &,
                        const std::shared_ptr<ResolutionsByResolvent> &);
                ~Decider();

                void resolve();

                void add_target_with_reason(const PackageOrBlockDepSpec &, const std::shared_ptr<const Reason> &);

                void purge();

                std::pair<AnyChildScore, OperatorScore> find_any_score(
                        const std::shared_ptr<const Resolution> &,
                        const std::shared_ptr<const PackageID> &,
                        const SanitisedDependency &) const;

                const std::shared_ptr<const RewrittenSpec> rewrite_if_special(const PackageOrBlockDepSpec &,
                        const std::shared_ptr<const Resolvent> & maybe_from) const;
        };
    }
}

#endif
