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
#include <paludis/resolver/resolver_lists.hh>
#include <paludis/resolver/spec_rewriter.hh>
#include <paludis/resolver/decider.hh>
#include <paludis/resolver/orderer.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/jobs.hh>
#include <paludis/resolver/job_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_shared_ptr.hh>
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

        const std::tr1::shared_ptr<ResolverLists> lists;

        const std::tr1::shared_ptr<Decider> decider;
        const std::tr1::shared_ptr<Orderer> orderer;

        Implementation(const Environment * const e, const ResolverFunctions & f) :
            env(e),
            fns(f),
            lists(new ResolverLists(make_named_values<ResolverLists>(
                            value_for<n::all_resolutions>(make_shared_ptr(new Resolutions)),
                            value_for<n::job_ids_needing_confirmation>(make_shared_ptr(new JobIDSequence)),
                            value_for<n::jobs>(make_shared_ptr(new Jobs)),
                            value_for<n::taken_error_job_ids>(make_shared_ptr(new JobIDSequence)),
                            value_for<n::taken_job_ids>(make_shared_ptr(new JobIDSequence)),
                            value_for<n::untaken_error_job_ids>(make_shared_ptr(new JobIDSequence)),
                            value_for<n::untaken_job_ids>(make_shared_ptr(new JobIDSequence))
                            ))),
            decider(new Decider(e, f, lists)),
            orderer(new Orderer(e, f, decider, lists))
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
Resolver::add_target(const PackageOrBlockDepSpec & spec)
{
    _imp->decider->add_target_with_reason(spec, make_shared_ptr(new TargetReason));
}

namespace
{
    typedef std::set<SetName> RecursingNames;

    struct SetExpander
    {
        const Environment * const env;
        const std::tr1::shared_ptr<Decider> & decider;
        const std::tr1::shared_ptr<const Reason> reason;
        RecursingNames & recurse;

        SetExpander(const Environment * const e,
                const std::tr1::shared_ptr<Decider> & d,
                const std::tr1::shared_ptr<const Reason> & r,
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

            const std::tr1::shared_ptr<const SetSpecTree> set(env->set(n.spec()->name()));
            if (! set)
                throw NoSuchSetError(stringify(n.spec()->name()));

            set->root()->accept(SetExpander(env, decider,
                        make_shared_ptr(new SetReason(n.spec()->name(), reason)), recurse));

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
Resolver::add_target(const SetName & set_name)
{
    Context context("When adding set target '" + stringify(set_name) + "':");
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

    const std::tr1::shared_ptr<const SetSpecTree> set(_imp->env->set(set_name));
    if (! set)
        throw InternalError(PALUDIS_HERE, "unimplemented: no such set");

    RecursingNames recurse;
    set->root()->accept(SetExpander(_imp->env, _imp->decider, make_shared_ptr(new TargetReason), recurse));
}

void
Resolver::resolve()
{
    _imp->decider->resolve();
    _imp->orderer->resolve();
}

const std::tr1::shared_ptr<const ResolverLists>
Resolver::lists() const
{
    return _imp->lists;
}

