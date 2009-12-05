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
#include <paludis/util/hashes.hh>
#include <paludis/match_package.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/notifier_callback.hh>
#include <algorithm>
#include <tr1/unordered_set>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<Orderer>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const Decider> decider;

        const std::tr1::shared_ptr<ResolverLists> lists;

        std::tr1::unordered_set<JobID, Hash<JobID> > already_ordered;

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
    struct CommonJobs
    {
        NamedValue<n::done_installs, std::tr1::shared_ptr<Job> > done_installs;
        NamedValue<n::done_pretends, std::tr1::shared_ptr<Job> > done_pretends;
    };

    struct DecisionHandler
    {
        CommonJobs & common_jobs;
        const std::tr1::shared_ptr<Resolution> resolution;
        const std::tr1::shared_ptr<ResolverLists> lists;

        DecisionHandler(
                CommonJobs & c,
                const std::tr1::shared_ptr<Resolution> & r,
                const std::tr1::shared_ptr<ResolverLists> & l) :
            common_jobs(c),
            resolution(r),
            lists(l)
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
                lists->unordered_job_ids()->push_back(usable_job->id());

                /* we want to do all of these as part of the main build process,
                 * not at pretend time */
                usable_job->arrows()->push_back(make_named_values<Arrow>(
                            value_for<n::comes_after>(common_jobs.done_pretends()->id()),
                            value_for<n::failure_kinds>(FailureKinds()),
                            value_for<n::maybe_reason>(make_null_shared_ptr())
                            ));
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
                lists->unordered_job_ids()->push_back(usable_job->id());

                /* we want to do all of these as part of the main build process,
                 * not at pretend time */
                usable_job->arrows()->push_back(make_named_values<Arrow>(
                            value_for<n::comes_after>(common_jobs.done_pretends()->id()),
                            value_for<n::failure_kinds>(FailureKinds()),
                            value_for<n::maybe_reason>(make_null_shared_ptr())
                            ));
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

                const std::tr1::shared_ptr<PretendJob> pretend_job(new PretendJob(resolution,
                            d.shared_from_this()));
                lists->jobs()->add(pretend_job);
                lists->unordered_job_ids()->push_back(pretend_job->id());

                const std::tr1::shared_ptr<FetchJob> fetch_job(new FetchJob(resolution,
                            d.shared_from_this()));
                lists->jobs()->add(fetch_job);
                lists->unordered_job_ids()->push_back(fetch_job->id());

                const std::tr1::shared_ptr<SimpleInstallJob> install_job(new SimpleInstallJob(resolution,
                            d.shared_from_this()));
                lists->jobs()->add(install_job);
                lists->unordered_job_ids()->push_back(install_job->id());

                const std::tr1::shared_ptr<UsableJob> usable_job(new UsableJob(resolution));
                lists->jobs()->add(usable_job);
                lists->unordered_job_ids()->push_back(usable_job->id());

                /* we can't do any fetches or installs until all pretends have passed */
                fetch_job->arrows()->push_back(make_named_values<Arrow>(
                            value_for<n::comes_after>(common_jobs.done_pretends()->id()),
                            value_for<n::failure_kinds>(FailureKinds()),
                            value_for<n::maybe_reason>(make_null_shared_ptr())
                            ));

                install_job->arrows()->push_back(make_named_values<Arrow>(
                            value_for<n::comes_after>(common_jobs.done_pretends()->id()),
                            value_for<n::failure_kinds>(FailureKinds()),
                            value_for<n::maybe_reason>(make_null_shared_ptr())
                            ));

                /* we haven't done all our pretends until we've done our pretend */
                common_jobs.done_pretends()->arrows()->push_back(make_named_values<Arrow>(
                            value_for<n::comes_after>(pretend_job->id()),
                            value_for<n::failure_kinds>(FailureKinds()),
                            value_for<n::maybe_reason>(make_null_shared_ptr())
                            ));

                /* we haven't done all our installs until we're usable
                 * (arguably this should just be install rather than usable,
                 * but the stronger requirement doesn't seem to hurt */
                common_jobs.done_installs()->arrows()->push_back(make_named_values<Arrow>(
                            value_for<n::comes_after>(usable_job->id()),
                            value_for<n::failure_kinds>(FailureKinds()),
                            value_for<n::maybe_reason>(make_null_shared_ptr())
                            ));

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

                const std::tr1::shared_ptr<UntakenInstallJob> install_job(new UntakenInstallJob(resolution));
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

                lists->error_resolutions()->append(resolution);
            }
            else
            {
                Log::get_instance()->message("resolver.orderer.job.unable_to_make", ll_debug, lc_no_context)
                    << "untaken " << resolution->resolvent() << " unable to make";

                const std::tr1::shared_ptr<UntakenInstallJob> install_job(new UntakenInstallJob(resolution));
                lists->jobs()->add(install_job);
                lists->untaken_job_ids()->push_back(install_job->id());
            }
        }
    };
}

void
Orderer::_resolve_jobs()
{
    CommonJobs common_jobs(make_named_values<CommonJobs>(
                value_for<n::done_installs>(make_shared_ptr(new SyncPointJob(sp_done_installs))),
                value_for<n::done_pretends>(make_shared_ptr(new SyncPointJob(sp_done_pretends)))
                ));

    _imp->lists->jobs()->add(common_jobs.done_installs());
    _imp->lists->unordered_job_ids()->push_back(common_jobs.done_installs()->id());

    _imp->lists->jobs()->add(common_jobs.done_pretends());
    _imp->lists->unordered_job_ids()->push_back(common_jobs.done_pretends()->id());

    for (Resolutions::ConstIterator i(_imp->lists->all_resolutions()->begin()),
            i_end(_imp->lists->all_resolutions()->end()) ;
            i != i_end ; ++i)
    {
        DecisionHandler d(common_jobs, *i, _imp->lists);
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

        void visit(const FetchJob &)
        {
        }

        void visit(const PretendJob &)
        {
        }

        void visit(const UntakenInstallJob &)
        {
        }

        void visit(const SyncPointJob &)
        {
        }
    };
}

void
Orderer::_resolve_jobs_dep_arrows()
{
    Context context("When resolving dep arrows:");

    for (JobIDSequence::ConstIterator i(_imp->lists->unordered_job_ids()->begin()),
            i_end(_imp->lists->unordered_job_ids()->end()) ;
            i != i_end ; ++i)
    {
        DepArrowHandler d(_imp->lists->jobs(), *i);
        _imp->lists->jobs()->fetch(*i)->accept(d);
    }
}

void
Orderer::_resolve_order()
{
    bool remaining(true);
    int pass(1);
    while (remaining)
    {
        remaining = false;
        bool any(false);

        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        for (JobIDSequence::ConstIterator i(_imp->lists->unordered_job_ids()->begin()),
                i_end(_imp->lists->unordered_job_ids()->end()) ;
                i != i_end ; ++i)
        {
            if (_already_ordered(*i))
                continue;

            remaining = true;

            if (! _can_order(*i, pass))
                continue;

            _imp->lists->ordered_job_ids()->push_back(*i);
            any = true;
            _mark_already_ordered(*i);

            if (pass >= 2)
            {
                std::stringstream s;
                const std::tr1::shared_ptr<const Job> job(_imp->lists->jobs()->fetch(*i));
                for (ArrowSequence::ConstIterator a(job->arrows()->begin()), a_end(job->arrows()->end()) ;
                        a != a_end ; ++a)
                    if (! _already_ordered(a->comes_after()))
                        s << " " << a->comes_after().string_id();

                Log::get_instance()->message("resolver.orderer.job.broke_cycle", ll_warning, lc_context)
                    << "Had to use cycle breaking to order " << i->string_id() << " (pass " << pass
                    << ", remaining arrows are" << s.str() << ")";

                pass = 1;
                continue;
            }
        }

        if (remaining && ! any)
        {
            if (++pass < 3)
                continue;

            std::stringstream s;
            for (JobIDSequence::ConstIterator i(_imp->lists->unordered_job_ids()->begin()),
                    i_end(_imp->lists->unordered_job_ids()->end()) ;
                    i != i_end ; ++i)
            {
                if (_already_ordered(*i))
                    continue;

                s << " {{{" << i->string_id() << " missing";
                const std::tr1::shared_ptr<const Job> job(_imp->lists->jobs()->fetch(*i));
                for (ArrowSequence::ConstIterator a(job->arrows()->begin()), a_end(job->arrows()->end()) ;
                        a != a_end ; ++a)
                    if (! _already_ordered(a->comes_after()))
                        s << " [" << a->comes_after().string_id() << "]";
                s << " }}}";
            }

            throw InternalError(PALUDIS_HERE, "unbreakable cycle, remaining" + s.str());
        }
    }
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

    struct Pass2Ignorable
    {
        bool visit(const UsableJob &) const
        {
            return true;
        }

        bool visit(const PretendJob &) const
        {
            return false;
        }

        bool visit(const SyncPointJob &) const
        {
            return false;
        }

        bool visit(const FetchJob &) const
        {
            return false;
        }

        bool visit(const UntakenInstallJob &) const
        {
            return false;
        }

        bool visit(const SimpleInstallJob &) const
        {
            return false;
        }
    };
}

bool
Orderer::_can_order(const JobID & i, const int pass) const
{
    const std::tr1::shared_ptr<const Job> job(_imp->lists->jobs()->fetch(i));
    for (ArrowSequence::ConstIterator a(job->arrows()->begin()), a_end(job->arrows()->end()) ;
            a != a_end ; ++a)
    {
        if (! _already_ordered(a->comes_after()))
        {
            bool skippable(false);

            if ((! skippable) && (pass >= 1))
            {
                /* if our job is a NoChangeJob or a UsableJob, and we're
                 * supposed to come after a NoChangeJob or a UsableJob, ignore
                 * the arrow. */
                const std::tr1::shared_ptr<const Job> other_job(_imp->lists->jobs()->fetch(a->comes_after()));
                if (job->accept_returning<bool>(Pass2Ignorable()) &&
                        other_job->accept_returning<bool>(Pass2Ignorable()))
                    skippable = true;
            }

            if ((! skippable) && (pass >= 2))
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

bool
Orderer::_already_ordered(const JobID & i) const
{
    return _imp->already_ordered.end() != _imp->already_ordered.find(i);
}

void
Orderer::_mark_already_ordered(const JobID & i)
{
    if (! _imp->already_ordered.insert(i).second)
        throw InternalError(PALUDIS_HERE, "already already ordered");
}

