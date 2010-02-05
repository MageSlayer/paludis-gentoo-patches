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

#include <paludis/resolver/orderer.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/decider.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/arrow.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolver_lists.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/jobs.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/match_package.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/notifier_callback.hh>
#include <algorithm>
#include <set>

using namespace paludis;
using namespace paludis::resolver;

typedef std::set<JobID> ToOrder;

namespace paludis
{
    template <>
    struct Implementation<Orderer>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const Decider> decider;

        const std::tr1::shared_ptr<ResolverLists> lists;

        ToOrder to_order;

        Implementation(const Environment * const e,
                const std::tr1::shared_ptr<const Decider> & d,
                const std::tr1::shared_ptr<ResolverLists> & l) :
            env(e),
            decider(d),
            lists(l)
        {
        }
    };
}

Orderer::Orderer(const Environment * const e, const std::tr1::shared_ptr<const Decider> & d,
        const std::tr1::shared_ptr<ResolverLists> & l) :
    PrivateImplementationPattern<Orderer>(new Implementation<Orderer>(e, d, l))
{
}

Orderer::~Orderer()
{
}

void
Orderer::resolve()
{
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Building Jobs"));
    _resolve_jobs();
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Building Arrows"));
    _resolve_jobs_dep_arrows();
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Ordering"));
    _resolve_order();
}

namespace paludis
{
    namespace n
    {
        struct done_installs;
        struct done_pretends;
    }
}

namespace
{
    struct DecisionHandler
    {
        const std::tr1::shared_ptr<Resolution> resolution;
        const std::tr1::shared_ptr<ResolverLists> lists;
        ToOrder & to_order;

        DecisionHandler(
                const std::tr1::shared_ptr<Resolution> & r,
                const std::tr1::shared_ptr<ResolverLists> & l,
                ToOrder & o) :
            resolution(r),
            lists(l),
            to_order(o)
        {
        }

        void visit(const NothingNoChangeDecision & d)
        {
            if (d.taken())
            {
                Log::get_instance()->message("resolver.orderer.job.nothing_no_change", ll_debug, lc_no_context)
                    << "taken " << resolution->resolvent() << " nothing no change";

                const std::tr1::shared_ptr<UsableJob> usable_job(new UsableJob(resolution));
                lists->jobs()->add(usable_job);
                to_order.insert(usable_job->id());
            }
            else
            {
                Log::get_instance()->message("resolver.orderer.job.nothing_no_change", ll_debug, lc_no_context)
                    << "untaken " << resolution->resolvent() << " nothing no change";
            }
        }

        void visit(const ExistingNoChangeDecision & d)
        {
            if (d.taken())
            {
                Log::get_instance()->message("resolver.orderer.job.existing_no_change", ll_debug, lc_no_context)
                    << "taken " << resolution->resolvent() << " existing no change";

                const std::tr1::shared_ptr<UsableJob> usable_job(new UsableJob(resolution));
                lists->jobs()->add(usable_job);
                to_order.insert(usable_job->id());
            }
            else
            {
                Log::get_instance()->message("resolver.orderer.job.existing_no_change", ll_debug, lc_no_context)
                    << "untaken " << resolution->resolvent() << " existing no change";
            }
        }

        void visit(const ChangesToMakeDecision & d)
        {
            if (d.taken())
            {
                Log::get_instance()->message("resolver.orderer.job.changes_to_make", ll_debug, lc_no_context)
                    << "taken " << resolution->resolvent() << " changes to make";

                const std::tr1::shared_ptr<FetchJob> fetch_job(new FetchJob(resolution,
                            d.shared_from_this()));
                lists->jobs()->add(fetch_job);
                to_order.insert(fetch_job->id());

                const std::tr1::shared_ptr<SimpleInstallJob> install_job(new SimpleInstallJob(resolution,
                            d.shared_from_this()));
                lists->jobs()->add(install_job);
                to_order.insert(install_job->id());

                const std::tr1::shared_ptr<UsableJob> usable_job(new UsableJob(resolution));
                lists->jobs()->add(usable_job);
                to_order.insert(usable_job->id());

                /* we can't install until we've fetched */
                install_job->arrows()->push_back(make_named_values<Arrow>(
                            value_for<n::comes_after>(fetch_job->id()),
                            value_for<n::failure_kinds>(FailureKinds()),
                            value_for<n::maybe_reason>(make_null_shared_ptr())
                            ));

                /* we aren't usable until we've been installed */
                usable_job->arrows()->push_back(make_named_values<Arrow>(
                            value_for<n::comes_after>(install_job->id()),
                            value_for<n::failure_kinds>(FailureKinds()),
                            value_for<n::maybe_reason>(make_null_shared_ptr())
                            ));
            }
            else
            {
                Log::get_instance()->message("resolver.orderer.job.changes_to_make", ll_debug, lc_no_context)
                    << "untaken " << resolution->resolvent() << " changes to make";

                const std::tr1::shared_ptr<SimpleInstallJob> install_job(new SimpleInstallJob(resolution, d.shared_from_this()));
                lists->jobs()->add(install_job);
                lists->untaken_job_ids()->push_back(install_job->id());
            }
        }

        void visit(const UnableToMakeDecision & d)
        {
            if (d.taken())
            {
                Log::get_instance()->message("resolver.orderer.job.unable_to_make", ll_debug, lc_no_context)
                    << "taken " << resolution->resolvent() << " unable to make";

                const std::tr1::shared_ptr<ErrorJob> error_job(new ErrorJob(resolution, d.shared_from_this()));
                lists->jobs()->add(error_job);
                lists->taken_error_job_ids()->push_back(error_job->id());
            }
            else
            {
                Log::get_instance()->message("resolver.orderer.job.unable_to_make", ll_debug, lc_no_context)
                    << "untaken " << resolution->resolvent() << " unable to make";

                const std::tr1::shared_ptr<ErrorJob> error_job(new ErrorJob(resolution, d.shared_from_this()));
                lists->jobs()->add(error_job);
                lists->untaken_error_job_ids()->push_back(error_job->id());
            }
        }

        void visit(const RemoveDecision & d)
        {
            if (d.taken())
            {
                Log::get_instance()->message("resolver.orderer.job.remove_decision", ll_debug, lc_no_context)
                    << "taken " << resolution->resolvent() << " remove decision";

                const std::tr1::shared_ptr<UninstallJob> uninstall_job(new UninstallJob(resolution,
                            d.shared_from_this()));
                lists->jobs()->add(uninstall_job);
                to_order.insert(uninstall_job->id());
            }
            else
            {
                Log::get_instance()->message("resolver.orderer.job.remove_decision", ll_debug, lc_no_context)
                    << "untaken " << resolution->resolvent() << " remove decision";

                const std::tr1::shared_ptr<UninstallJob> uninstall_job(new UninstallJob(resolution, d.shared_from_this()));
                lists->jobs()->add(uninstall_job);
                lists->untaken_job_ids()->push_back(uninstall_job->id());
            }
        }
    };
}

void
Orderer::_resolve_jobs()
{
    for (Resolutions::ConstIterator i(_imp->lists->all_resolutions()->begin()),
            i_end(_imp->lists->all_resolutions()->end()) ;
            i != i_end ; ++i)
    {
        DecisionHandler d(*i, _imp->lists, _imp->to_order);
        (*i)->decision()->accept(d);
    }
}

namespace
{
    struct LabelArrowsInfo
    {
        bool build;
        bool run;

        LabelArrowsInfo() :
            build(false),
            run(false)
        {
        }

        void visit(const DependenciesBuildLabel &)
        {
            build = true;
        }

        void visit(const DependenciesInstallLabel &)
        {
            build = true;
        }

        void visit(const DependenciesFetchLabel &)
        {
            build = true;
        }

        void visit(const DependenciesRunLabel &)
        {
            run = true;
        }

        void visit(const DependenciesTestLabel &)
        {
            build = true;
        }

        void visit(const DependenciesPostLabel &)
        {
        }

        void visit(const DependenciesSuggestionLabel &)
        {
        }

        void visit(const DependenciesRecommendationLabel &)
        {
        }

        void visit(const DependenciesCompileAgainstLabel &)
        {
            build = true;
        }
    };

    struct DepArrowsAdder
    {
        const std::tr1::shared_ptr<Jobs> jobs;
        const JobID our_identifier;
        const bool is_usable;
        const std::tr1::shared_ptr<const Reason> reason;

        DepArrowsAdder(
                const std::tr1::shared_ptr<Jobs> & j,
                const JobID & i,
                const bool u,
                const std::tr1::shared_ptr<const Reason> & r) :
            jobs(j),
            our_identifier(i),
            is_usable(u),
            reason(r)
        {
        }

        void visit(const TargetReason &) const
        {
        }

        void visit(const PresetReason &) const
        {
        }

        void visit(const SetReason & r) const
        {
            r.reason_for_set()->accept(*this);
        }

        void visit(const DependencyReason & r) const
        {
            Context context("When adding arrows for job '" + stringify(our_identifier.string_id())
                    + "' with reason '" + stringify(r.sanitised_dependency().spec())
                    + "' from '" + stringify(r.from_resolvent()) + "':");

            FailureKinds failure_kinds;
            if (r.already_met())
                failure_kinds += fk_ignorable_if_satisfied;

            if (r.sanitised_dependency().spec().if_block())
            {
                /* only strong blockers impose arrows. todo: maybe weak
                 * blockers should impose some kind of weak arrow? or would
                 * that make matters worse with silly Gentoo KDE blockers? */
                if (r.sanitised_dependency().spec().if_block()->strong())
                {
                    /* we might not be changing anything (e.g. for a blocker), or we
                     * might be a dependency that got cancelled out later when
                     * something was changed from a decision to an error. */
                    if (! jobs->have_job_for_installed(r.from_resolvent()))
                    {
                        Log::get_instance()->message("resolver.orderer.job.no_job_for_installed", ll_warning, lc_context)
                            << "No job for building '" << r.from_resolvent() << "'. Verify manually that this is sane for now.";
                        return;
                    }

                    /* todo: this should probably only cause an arrow if the
                     * blocker is currently met */
                    jobs->fetch(jobs->find_id_for_installed(r.from_resolvent()))->arrows()->push_back(
                            make_named_values<Arrow>(
                                value_for<n::comes_after>(our_identifier),
                                value_for<n::failure_kinds>(failure_kinds),
                                value_for<n::maybe_reason>(reason)
                                ));
                }
            }
            else
            {
                LabelArrowsInfo v;
                for (DependenciesLabelSequence::ConstIterator l(r.sanitised_dependency().active_dependency_labels()->begin()),
                        l_end(r.sanitised_dependency().active_dependency_labels()->end()) ;
                        l != l_end ; ++l)
                    (*l)->accept(v);

                if (v.build || v.run)
                {
                    /* we might not be changing anything (e.g. for a blocker), or we
                     * might be a dependency that got cancelled out later when
                     * something was changed from a decision to an error. */
                    if (! jobs->have_job_for_usable(r.from_resolvent()))
                    {
                        Log::get_instance()->message("resolver.orderer.job.no_job_for_usable", ll_warning, lc_context)
                            << "No job for building '" << r.from_resolvent() << "'. Verify manually that this is sane for now.";
                        return;
                    }

                    if (v.build)
                    {
                        /* build: we must be usable before the other package is built */
                        if (is_usable)
                            jobs->fetch(jobs->find_id_for_installed(r.from_resolvent()))->arrows()->push_back(
                                    make_named_values<Arrow>(
                                        value_for<n::comes_after>(our_identifier),
                                        value_for<n::failure_kinds>(failure_kinds),
                                        value_for<n::maybe_reason>(reason)
                                        ));
                    }

                    if (v.run)
                    {
                        /* run: we must be usable before the other package is usable */
                        if (is_usable)
                            jobs->fetch(jobs->find_id_for_usable(r.from_resolvent()))->arrows()->push_back(
                                    make_named_values<Arrow>(
                                        value_for<n::comes_after>(our_identifier),
                                        value_for<n::failure_kinds>(failure_kinds),
                                        value_for<n::maybe_reason>(reason)
                                        ));
                    }
                }
            }
        }
    };

    struct DepArrowHandler
    {
        const std::tr1::shared_ptr<Jobs> jobs;
        const JobID our_identifier;

        DepArrowHandler(
                const std::tr1::shared_ptr<Jobs> & j,
                const JobID & i) :
            jobs(j),
            our_identifier(i)
        {
        }

        void add_dep_arrows(const bool u, const std::tr1::shared_ptr<const Resolution> & r)
        {
            for (Constraints::ConstIterator c(r->constraints()->begin()), c_end(r->constraints()->end()) ;
                    c != c_end ; ++c)
                if ((*c)->reason())
                    (*c)->reason()->accept(DepArrowsAdder(jobs, our_identifier, u, (*c)->reason()));
        }

        void visit(const SimpleInstallJob & c)
        {
            add_dep_arrows(false, c.resolution());
        }

        void visit(const UsableJob & c)
        {
            add_dep_arrows(true, c.resolution());
        }

        void visit(const UninstallJob &)
        {
        }

        void visit(const UsableGroupJob &)
        {
        }

        void visit(const FetchJob &)
        {
        }

        void visit(const ErrorJob &)
        {
        }
    };
}

void
Orderer::_resolve_jobs_dep_arrows()
{
    Context context("When resolving dep arrows:");

    for (ToOrder::const_iterator i(_imp->to_order.begin()),
            i_end(_imp->to_order.end()) ;
            i != i_end ; ++i)
    {
        DepArrowHandler d(_imp->lists->jobs(), *i);
        _imp->lists->jobs()->fetch(*i)->accept(d);
    }
}

void
Orderer::_resolve_order()
{
    while (true)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Ordering"));

        /* anything left? */
        if (_imp->to_order.empty())
            break;

        /* first attempt: nothing fancy */
        if (_order_some(false, false))
            continue;

        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Ordering Usable Cycles"));

        /* second attempt: remove cycles containing only usable jobs */
        if (_remove_usable_cycles())
            continue;

        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Ordering Using Existing"));

        /* third attempt: remove an install job whose deps are already met */
        if (_order_some(true, true))
            continue;

        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Ordering Using Any Existing"));

        if (_order_some(true, false))
            continue;

        _cycle_error();
    }
}

bool
Orderer::_order_some(const bool desperate, const bool installs_only)
{
    bool result(false);

    for (ToOrder::iterator i(_imp->to_order.begin()), i_next(i), i_end(_imp->to_order.end()) ;
            i != i_end ; i = i_next)
    {
        /* i might end up invalidated by the time we're done */
        JobID id(*i);
        ++i_next;

        if (installs_only)
        {
            const std::tr1::shared_ptr<const Job> job(_imp->lists->jobs()->fetch(*i));
            if (! simple_visitor_cast<const SimpleInstallJob>(*job))
                continue;
        }

        if (! _can_order(id, desperate))
            continue;

        if (desperate)
        {
            const std::tr1::shared_ptr<Job> job(_imp->lists->jobs()->fetch(id));
            for (ArrowSequence::ConstIterator a(job->arrows()->begin()), a_end(job->arrows()->end()) ;
                    a != a_end ; ++a)
                if (_imp->to_order.end() != _imp->to_order.find(a->comes_after()))
                    job->used_existing_packages_when_ordering()->push_back(a->comes_after());
        }

        _order_this(id, false);
        result = true;
    }

    return result;
}

void
Orderer::_order_this(const JobID & i, const bool inside_something_else)
{
    if (! inside_something_else)
        _imp->lists->taken_job_ids()->push_back(i);
    _mark_already_ordered(i);
}

bool
Orderer::_remove_usable_cycles()
{
    /* we only want to remove usable jobs... */
    std::set<JobID> removable;
    for (ToOrder::const_iterator i(_imp->to_order.begin()),
            i_end(_imp->to_order.end()) ;
            i != i_end ; ++i)
    {
        const std::tr1::shared_ptr<const Job> job(_imp->lists->jobs()->fetch(*i));
        if (simple_visitor_cast<const UsableJob>(*job))
            removable.insert(job->id());
    }

    /* but we don't want to remove any job unless it only requires other jobs that
     * we're going to remove. */
    while (true)
    {
        bool need_another_pass(false);
        /* hashes have annoying invalidation rules */
        std::list<JobID> removable_copy(removable.begin(), removable.end());
        for (std::list<JobID>::iterator i(removable_copy.begin()), i_end(removable_copy.end()) ;
                i != i_end ; ++i)
        {
            bool ok(true);
            const std::tr1::shared_ptr<const Job> job(_imp->lists->jobs()->fetch(*i));
            for (ArrowSequence::ConstIterator a(job->arrows()->begin()), a_end(job->arrows()->end()) ;
                    a != a_end ; ++a)
                if (_imp->to_order.end() != _imp->to_order.find(a->comes_after()))
                    if (removable.end() == removable.find(a->comes_after()))
                    {
                        need_another_pass = true;
                        ok = false;
                        break;
                    }

            if (! ok)
                removable.erase(*i);
        }

        if (! need_another_pass)
            break;
    }

    if (! removable.empty())
    {
        const std::tr1::shared_ptr<JobIDSequence> ids(new JobIDSequence);
        std::copy(removable.begin(), removable.end(), ids->back_inserter());

        const std::tr1::shared_ptr<UsableGroupJob> usable_group_job(new UsableGroupJob(ids));
        _imp->lists->jobs()->add(usable_group_job);
        _order_this(usable_group_job->id(), false);

        for (std::set<JobID>::iterator i(removable.begin()), i_next(i),
                i_end(removable.end()) ;
                i != i_end ; i = i_next)
        {
            ++i_next;
            _order_this(*i, true);
        }
    }

    return ! removable.empty();
}

void
Orderer::_cycle_error()
{
    std::stringstream s;
    for (ToOrder::const_iterator i(_imp->to_order.begin()),
            i_end(_imp->to_order.end()) ;
            i != i_end ; ++i)
    {
        s << " {{{" << i->string_id() << " missing";
        const std::tr1::shared_ptr<const Job> job(_imp->lists->jobs()->fetch(*i));
        for (ArrowSequence::ConstIterator a(job->arrows()->begin()), a_end(job->arrows()->end()) ;
                a != a_end ; ++a)
            if (_imp->to_order.end() != _imp->to_order.find(a->comes_after()))
                s << " [" << a->comes_after().string_id() << "]";
        s << " }}}";
    }

    throw InternalError(PALUDIS_HERE, "can't order any of " + s.str());
}

namespace
{
    typedef std::tr1::function<bool (const PackageOrBlockDepSpec &)> AlreadyMetFn;

    struct AlreadyMetDep
    {
        bool visit(const PresetReason &) const
        {
            return false;
        }

        bool visit(const TargetReason &) const
        {
            return false;
        }

        bool visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<bool>(*this);
        }

        bool visit(const DependencyReason & r) const
        {
            return r.already_met();
        }
    };
}

bool
Orderer::_can_order(const JobID & i, const bool desperate) const
{
    const std::tr1::shared_ptr<const Job> job(_imp->lists->jobs()->fetch(i));
    for (ArrowSequence::ConstIterator a(job->arrows()->begin()), a_end(job->arrows()->end()) ;
            a != a_end ; ++a)
    {
        if (_imp->to_order.end() != _imp->to_order.find(a->comes_after()))
        {
            bool skippable(false);

            if ((! skippable) && desperate)
            {
                /* we can also ignore any arrows that are already met (e.g a
                 * dep b, b dep a, a is already installed */
                if (a->maybe_reason() && a->maybe_reason()->accept_returning<bool>(AlreadyMetDep()))
                    skippable = true;
            }

            if (! skippable)
                return false;
        }
    }

    return true;
}

void
Orderer::_mark_already_ordered(const JobID & i)
{
    _imp->to_order.erase(i);
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());
}

