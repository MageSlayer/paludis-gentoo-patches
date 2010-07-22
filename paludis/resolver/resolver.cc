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

#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/spec_rewriter.hh>
#include <paludis/resolver/decider.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/orderer.hh>
#include <paludis/resolver/decisions.hh>
#include <paludis/resolver/job_list.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/nag.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/environment.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/spec_tree.hh>
#include <paludis/repository.hh>

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <set>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<Resolver>
    {
        const Environment * const env;
        const ResolverFunctions fns;

        const std::shared_ptr<Resolved> resolved;

        const std::shared_ptr<Decider> decider;
        const std::shared_ptr<Orderer> orderer;

        Implementation(const Environment * const e, const ResolverFunctions & f) :
            env(e),
            fns(f),
            resolved(new Resolved(make_named_values<Resolved>(
                            n::job_lists() = make_shared_copy(make_named_values<JobLists>(
                                    n::execute_job_list() = std::make_shared<JobList<ExecuteJob>>(),
                                    n::pretend_job_list() = std::make_shared<JobList<PretendJob>>()
                                    )),
                            n::nag() = std::make_shared<NAG>(),
                            n::resolutions_by_resolvent() = std::make_shared<ResolutionsByResolvent>(),
                            n::taken_change_or_remove_decisions() = std::make_shared<OrderedChangeOrRemoveDecisions>(),
                            n::taken_unable_to_make_decisions() = std::make_shared<Decisions<UnableToMakeDecision>>(),
                            n::taken_unconfirmed_decisions() = std::make_shared<Decisions<ConfirmableDecision>>(),
                            n::taken_unorderable_decisions() = std::make_shared<OrderedChangeOrRemoveDecisions>(),
                            n::untaken_change_or_remove_decisions() = std::make_shared<Decisions<ChangeOrRemoveDecision>>(),
                            n::untaken_unable_to_make_decisions() = std::make_shared<Decisions<UnableToMakeDecision>>()
                            ))),
            decider(new Decider(e, f, resolved->resolutions_by_resolvent())),
            orderer(new Orderer(e, f, resolved))
        {
        }
    };
}

Resolver::Resolver(const Environment * const e, const ResolverFunctions & f) :
    PrivateImplementationPattern<Resolver>(new Implementation<Resolver>(e, f))
{
}

Resolver::~Resolver()
{
}

void
Resolver::add_target(const PackageOrBlockDepSpec & spec, const std::string & extra_information)
{
    _imp->decider->add_target_with_reason(spec, std::make_shared<TargetReason>(extra_information));
}

namespace
{
    typedef std::set<SetName> RecursingNames;

    struct SetExpander
    {
        const Environment * const env;
        const std::shared_ptr<Decider> & decider;
        const std::shared_ptr<const Reason> reason;
        RecursingNames & recurse;

        SetExpander(const Environment * const e,
                const std::shared_ptr<Decider> & d,
                const std::shared_ptr<const Reason> & r,
                RecursingNames & c) :
            env(e),
            decider(d),
            reason(r),
            recurse(c)
        {
        }

        void visit(const SetSpecTree::NodeType<NamedSetDepSpec>::Type & n) const
        {
            if (! recurse.insert(n.spec()->name()).second)
                throw RecursivelyDefinedSetError(stringify(n.spec()->name()));

            const std::shared_ptr<const SetSpecTree> set(env->set(n.spec()->name()));
            if (! set)
                throw NoSuchSetError(stringify(n.spec()->name()));

            set->root()->accept(SetExpander(env, decider,
                        std::make_shared<SetReason>(n.spec()->name(), reason), recurse));

            recurse.erase(n.spec()->name());
        }

        void visit(const SetSpecTree::NodeType<AllDepSpec>::Type & n) const
        {
            std::for_each(indirect_iterator(n.begin()), indirect_iterator(n.end()), accept_visitor(*this));
        }

        void visit(const SetSpecTree::NodeType<PackageDepSpec>::Type & n) const
        {
            decider->add_target_with_reason(*n.spec(), reason);
        }
    };
}

void
Resolver::add_target(const SetName & set_name, const std::string & extra_information)
{
    Context context("When adding set target '" + stringify(set_name) + "':");
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

    const std::shared_ptr<const SetSpecTree> set(_imp->env->set(set_name));
    if (! set)
        throw NoSuchSetError(stringify(set_name));

    RecursingNames recurse;
    set->root()->accept(SetExpander(_imp->env, _imp->decider, std::make_shared<TargetReason>(extra_information), recurse));
}

void
Resolver::purge()
{
    _imp->decider->purge();
}

void
Resolver::resolve()
{
    _imp->decider->resolve();
    _imp->orderer->resolve();
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Done"));
}

const std::shared_ptr<const Resolved>
Resolver::resolved() const
{
    return _imp->resolved;
}

