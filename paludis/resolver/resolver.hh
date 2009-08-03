/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_HH 1

#include <paludis/resolver/resolver-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/qpn_s-fwd.hh>
#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/use_installed-fwd.hh>
#include <paludis/resolver/desire_strength-fwd.hh>
#include <paludis/resolver/destinations-fwd.hh>
#include <paludis/resolver/resolver_functions-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/name.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE Resolver :
            private PrivateImplementationPattern<Resolver>
        {
            private:
                template <typename I_> void _find_qpn_s_s_for_spec(const PackageDepSpec &, I_) const;

                QualifiedPackageName _package_from_spec(const PackageDepSpec &) const;

                const std::tr1::shared_ptr<Resolution> _create_resolution_for_qpn_s(const QPN_S &) const;
                const std::tr1::shared_ptr<Resolution> _resolution_for_qpn_s(const QPN_S &, const bool create);
                const std::tr1::shared_ptr<Resolution> _resolution_for_qpn_s(const QPN_S &) const;

                const std::tr1::shared_ptr<Constraint> _make_constraint_from_target(
                        const QPN_S &,
                        const PackageDepSpec &,
                        const std::tr1::shared_ptr<const Reason> &) const;

                const std::tr1::shared_ptr<Constraint> _make_constraint_from_dependency(
                        const QPN_S &, const SanitisedDependency &) const;

                const std::tr1::shared_ptr<Decision> _decision_from_package_id(
                        const QPN_S &, const std::tr1::shared_ptr<const PackageID> &) const;

                void _apply_resolution_constraint(const QPN_S &,
                        const std::tr1::shared_ptr<Resolution> &,
                        const std::tr1::shared_ptr<const Constraint> &);

                void _verify_new_constraint(const QPN_S &,
                        const std::tr1::shared_ptr<const Resolution> &,
                        const std::tr1::shared_ptr<const Constraint> &);

                void _made_wrong_decision(const QPN_S &,
                        const std::tr1::shared_ptr<const Resolution> &,
                        const std::tr1::shared_ptr<const Constraint> &) PALUDIS_ATTRIBUTE((noreturn));

                bool _constraint_matches(
                        const std::tr1::shared_ptr<const Constraint> &,
                        const std::tr1::shared_ptr<const Decision> &) const;

                const std::tr1::shared_ptr<Destinations> _make_destinations_for(const QPN_S &,
                        const std::tr1::shared_ptr<const Resolution> &) const;

                const std::tr1::shared_ptr<Destination> _make_slash_destination_for(const QPN_S &,
                        const std::tr1::shared_ptr<const Resolution> &) const;

                const std::tr1::shared_ptr<const PackageIDSequence> _find_replacing(
                        const std::tr1::shared_ptr<const PackageID> &,
                        const std::tr1::shared_ptr<const Repository> &) const;

                void _resolve_dependencies();
                void _resolve_destinations();
                void _resolve_arrows();
                void _resolve_order();

                void _decide(const QPN_S &, const std::tr1::shared_ptr<Resolution> & resolution);

                const std::tr1::shared_ptr<Decision> _try_to_find_decision_for(
                        const QPN_S &, const std::tr1::shared_ptr<const Resolution> & resolution) const;

                const std::tr1::shared_ptr<const PackageID> _best_installed_id_for(
                        const QPN_S &, const std::tr1::shared_ptr<const Resolution> &) const;

                const std::tr1::shared_ptr<const PackageID> _best_installable_id_for(
                        const QPN_S &, const std::tr1::shared_ptr<const Resolution> &) const;

                const std::tr1::shared_ptr<const PackageID> _best_id_from(
                        const std::tr1::shared_ptr<const PackageIDSequence> &,
                        const QPN_S &, const std::tr1::shared_ptr<const Resolution> &) const;

                void _add_dependencies(const QPN_S & our_qpn_s, const std::tr1::shared_ptr<Resolution> & our_resolution);

                bool _care_about_dependency_spec(const QPN_S &, const std::tr1::shared_ptr<const Resolution> &,
                        const SanitisedDependency &) const;

                bool _causes_pre_arrow(const DependencyReason &) const;

                bool _can_order_now(const QPN_S &, const std::tr1::shared_ptr<const Resolution> & resolution) const;

                void _do_order(const QPN_S &, const std::tr1::shared_ptr<Resolution> & resolution);

                void _unable_to_decide(const QPN_S &,
                        const std::tr1::shared_ptr<const Resolution> &) const PALUDIS_ATTRIBUTE((noreturn));

                void _unable_to_order_more() const PALUDIS_ATTRIBUTE((noreturn));

                const std::tr1::shared_ptr<Constraints> _initial_constraints_for(const QPN_S &) const;

                void _suggest_restart_with(const QPN_S &,
                        const std::tr1::shared_ptr<const Resolution> &,
                        const std::tr1::shared_ptr<const Constraint> &,
                        const std::tr1::shared_ptr<const Decision> &) const PALUDIS_ATTRIBUTE((noreturn));

                const std::tr1::shared_ptr<const Constraint> _make_constraint_for_preloading(
                        const QPN_S &,
                        const std::tr1::shared_ptr<const Decision> &) const;

                DesireStrength _desire_strength_from_sanitised_dependency(
                        const QPN_S &, const SanitisedDependency &) const;

                bool _dependency_to_destination_slash(
                        const QPN_S &, const SanitisedDependency &) const;

                bool _same_slot(const std::tr1::shared_ptr<const PackageID> & a,
                        const std::tr1::shared_ptr<const PackageID> & b) const;

            public:
                Resolver(
                        const Environment * const,
                        const ResolverFunctions &);
                ~Resolver();

                void add_target_with_reason(const PackageDepSpec &, const std::tr1::shared_ptr<const Reason> &);
                void add_target(const PackageDepSpec &);
                void add_target(const SetName &);

                void resolve();
                void dump(std::ostream &, const bool deps) const;

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag,
                        const std::tr1::shared_ptr<const Resolution> > ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                int find_any_score(const QPN_S &, const SanitisedDependency &) const;
                QPN_S qpn_s_from_id(const std::tr1::shared_ptr<const PackageID> &) const;

        };
    }
}

#endif
