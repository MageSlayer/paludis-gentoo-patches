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

#include <paludis/resolver/job.hh>
#include <paludis/resolver/arrow.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<UsableJob>
    {
        const std::tr1::shared_ptr<const Resolution> resolution;
        const std::tr1::shared_ptr<ArrowSequence> arrows;
        const std::tr1::shared_ptr<JobIDSequence> used_existing_packages_when_ordering;

        Implementation(const std::tr1::shared_ptr<const Resolution> & r) :
            resolution(r),
            arrows(new ArrowSequence),
            used_existing_packages_when_ordering(new JobIDSequence)
        {
        }
    };

    template <>
    struct Implementation<UsableGroupJob>
    {
        const std::tr1::shared_ptr<const JobIDSequence> ids;
        const std::tr1::shared_ptr<ArrowSequence> arrows;
        const std::tr1::shared_ptr<JobIDSequence> used_existing_packages_when_ordering;

        Implementation(const std::tr1::shared_ptr<const JobIDSequence> & i) :
            ids(i),
            arrows(new ArrowSequence),
            used_existing_packages_when_ordering(new JobIDSequence)
        {
        }
    };

    template <>
    struct Implementation<FetchJob>
    {
        const std::tr1::shared_ptr<const Resolution> resolution;
        const std::tr1::shared_ptr<const ChangesToMakeDecision> decision;
        const std::tr1::shared_ptr<ArrowSequence> arrows;
        const std::tr1::shared_ptr<JobIDSequence> used_existing_packages_when_ordering;

        Implementation(const std::tr1::shared_ptr<const Resolution> & r,
                const std::tr1::shared_ptr<const ChangesToMakeDecision> & d) :
            resolution(r),
            decision(d),
            arrows(new ArrowSequence),
            used_existing_packages_when_ordering(new JobIDSequence)
        {
        }
    };

    template <>
    struct Implementation<SimpleInstallJob>
    {
        const std::tr1::shared_ptr<const Resolution> resolution;
        const std::tr1::shared_ptr<const ChangesToMakeDecision> decision;
        const std::tr1::shared_ptr<ArrowSequence> arrows;
        const std::tr1::shared_ptr<JobIDSequence> used_existing_packages_when_ordering;

        Implementation(const std::tr1::shared_ptr<const Resolution> & r,
                const std::tr1::shared_ptr<const ChangesToMakeDecision> & d) :
            resolution(r),
            decision(d),
            arrows(new ArrowSequence),
            used_existing_packages_when_ordering(new JobIDSequence)
        {
        }
    };

    template <>
    struct Implementation<ErrorJob>
    {
        const std::tr1::shared_ptr<const Resolution> resolution;
        const std::tr1::shared_ptr<const UnableToMakeDecision> decision;
        const std::tr1::shared_ptr<ArrowSequence> arrows;
        const std::tr1::shared_ptr<JobIDSequence> used_existing_packages_when_ordering;

        Implementation(const std::tr1::shared_ptr<const Resolution> & r,
                const std::tr1::shared_ptr<const UnableToMakeDecision> & d) :
            resolution(r),
            decision(d),
            arrows(new ArrowSequence),
            used_existing_packages_when_ordering(new JobIDSequence)
        {
        }
    };
}

Job::~Job()
{
}

namespace
{
    void do_arrows(
            const std::tr1::shared_ptr<Job> & result,
            Deserialisator & v)
    {
        Deserialisator vv(*v.find_remove_member("arrows"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            result->arrows()->push_back(vv.member<Arrow>(stringify(n)));
    }

    void do_existing(
            const std::tr1::shared_ptr<Job> & result,
            Deserialisator & v)
    {
        Deserialisator vv(*v.find_remove_member("used_existing_packages_when_ordering"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            result->used_existing_packages_when_ordering()->push_back(vv.member<JobID>(stringify(n)));
    }
}

const std::tr1::shared_ptr<Job>
Job::deserialise(Deserialisation & d)
{
    std::tr1::shared_ptr<Job> result;

    if (d.class_name() == "UsableJob")
    {
        Deserialisator v(d, "UsableJob");
        result.reset(new UsableJob(
                    v.member<std::tr1::shared_ptr<Resolution> >("resolution")
                    ));
        do_arrows(result, v);
        do_existing(result, v);
    }
    else if (d.class_name() == "UsableGroupJob")
    {
        Deserialisator v(d, "UsableGroupJob");

        Deserialisator vv(*v.find_remove_member("job_ids"), "c");
        const std::tr1::shared_ptr<JobIDSequence> ids(new JobIDSequence);
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            ids->push_back(vv.member<JobID>(stringify(n)));

        result.reset(new UsableGroupJob(ids));
        do_arrows(result, v);
        do_existing(result, v);
    }
    else if (d.class_name() == "SimpleInstallJob")
    {
        Deserialisator v(d, "SimpleInstallJob");
        result.reset(new SimpleInstallJob(
                    v.member<std::tr1::shared_ptr<Resolution> >("resolution"),
                    v.member<std::tr1::shared_ptr<ChangesToMakeDecision> >("changes_to_make_decision")
                    ));
        do_arrows(result, v);
        do_existing(result, v);
    }
    else if (d.class_name() == "FetchJob")
    {
        Deserialisator v(d, "FetchJob");
        result.reset(new FetchJob(
                    v.member<std::tr1::shared_ptr<Resolution> >("resolution"),
                    v.member<std::tr1::shared_ptr<ChangesToMakeDecision> >("changes_to_make_decision")
                    ));
        do_arrows(result, v);
        do_existing(result, v);
    }
    else if (d.class_name() == "ErrorJob")
    {
        Deserialisator v(d, "ErrorJob");
        result.reset(new ErrorJob(
                    v.member<std::tr1::shared_ptr<Resolution> >("resolution"),
                    v.member<std::tr1::shared_ptr<UnableToMakeDecision> >("unable_to_make_decision")
                    ));
        do_arrows(result, v);
        do_existing(result, v);
    }
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");

    return result;
}

UsableJob::UsableJob(const std::tr1::shared_ptr<const Resolution> & r) :
    PrivateImplementationPattern<UsableJob>(new Implementation<UsableJob>(r))
{
}

UsableJob::~UsableJob()
{
}

const std::tr1::shared_ptr<const Resolution>
UsableJob::resolution() const
{
    return _imp->resolution;
}

const std::tr1::shared_ptr<const ArrowSequence>
UsableJob::arrows() const
{
    return _imp->arrows;
}

const std::tr1::shared_ptr<ArrowSequence>
UsableJob::arrows()
{
    return _imp->arrows;
}

const std::tr1::shared_ptr<const JobIDSequence>
UsableJob::used_existing_packages_when_ordering() const
{
    return _imp->used_existing_packages_when_ordering;
}

const std::tr1::shared_ptr<JobIDSequence>
UsableJob::used_existing_packages_when_ordering()
{
    return _imp->used_existing_packages_when_ordering;
}

const JobID
UsableJob::id() const
{
    return make_named_values<JobID>(
            value_for<n::string_id>("usable:" + stringify(resolution()->resolvent()))
            );
}

void
UsableJob::serialise(Serialiser & s) const
{
    s.object("UsableJob")
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "arrows", arrows())
        .member(SerialiserFlags<serialise::might_be_null>(), "resolution", resolution())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(),
                "used_existing_packages_when_ordering", used_existing_packages_when_ordering())
        ;
}

UsableGroupJob::UsableGroupJob(const std::tr1::shared_ptr<const JobIDSequence> & r) :
    PrivateImplementationPattern<UsableGroupJob>(new Implementation<UsableGroupJob>(r))
{
}

UsableGroupJob::~UsableGroupJob()
{
}

const std::tr1::shared_ptr<const JobIDSequence>
UsableGroupJob::job_ids() const
{
    return _imp->ids;
}

const std::tr1::shared_ptr<const ArrowSequence>
UsableGroupJob::arrows() const
{
    return _imp->arrows;
}

const std::tr1::shared_ptr<ArrowSequence>
UsableGroupJob::arrows()
{
    return _imp->arrows;
}

const std::tr1::shared_ptr<const JobIDSequence>
UsableGroupJob::used_existing_packages_when_ordering() const
{
    return _imp->used_existing_packages_when_ordering;
}

const std::tr1::shared_ptr<JobIDSequence>
UsableGroupJob::used_existing_packages_when_ordering()
{
    return _imp->used_existing_packages_when_ordering;
}

namespace
{
    std::string stringify_job_id(const JobID & i)
    {
        return i.string_id();
    }
}

const JobID
UsableGroupJob::id() const
{
    return make_named_values<JobID>(
            value_for<n::string_id>("usable_group:" + join(_imp->ids->begin(), _imp->ids->end(), "+", &stringify_job_id))
            );
}

void
UsableGroupJob::serialise(Serialiser & s) const
{
    s.object("UsableGroupJob")
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "arrows", arrows())
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "job_ids", job_ids())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(),
                "used_existing_packages_when_ordering", used_existing_packages_when_ordering())
        ;
}

FetchJob::FetchJob(const std::tr1::shared_ptr<const Resolution> & r,
        const std::tr1::shared_ptr<const ChangesToMakeDecision> & d) :
    PrivateImplementationPattern<FetchJob>(new Implementation<FetchJob>(r, d))
{
}

FetchJob::~FetchJob()
{
}

const std::tr1::shared_ptr<const Resolution>
FetchJob::resolution() const
{
    return _imp->resolution;
}

const std::tr1::shared_ptr<const ChangesToMakeDecision>
FetchJob::changes_to_make_decision() const
{
    return _imp->decision;
}

const std::tr1::shared_ptr<const ArrowSequence>
FetchJob::arrows() const
{
    return _imp->arrows;
}

const std::tr1::shared_ptr<ArrowSequence>
FetchJob::arrows()
{
    return _imp->arrows;
}

const std::tr1::shared_ptr<const JobIDSequence>
FetchJob::used_existing_packages_when_ordering() const
{
    return _imp->used_existing_packages_when_ordering;
}

const std::tr1::shared_ptr<JobIDSequence>
FetchJob::used_existing_packages_when_ordering()
{
    return _imp->used_existing_packages_when_ordering;
}

const JobID
FetchJob::id() const
{
    return make_named_values<JobID>(
            value_for<n::string_id>("fetch:" + stringify(resolution()->resolvent()))
            );
}

void
FetchJob::serialise(Serialiser & s) const
{
    s.object("FetchJob")
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "arrows", arrows())
        .member(SerialiserFlags<serialise::might_be_null>(), "changes_to_make_decision", changes_to_make_decision())
        .member(SerialiserFlags<serialise::might_be_null>(), "resolution", resolution())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(),
                "used_existing_packages_when_ordering", used_existing_packages_when_ordering())
        ;
}

SimpleInstallJob::SimpleInstallJob(const std::tr1::shared_ptr<const Resolution> & r,
        const std::tr1::shared_ptr<const ChangesToMakeDecision> & d) :
    PrivateImplementationPattern<SimpleInstallJob>(new Implementation<SimpleInstallJob>(r, d))
{
}

SimpleInstallJob::~SimpleInstallJob()
{
}

const std::tr1::shared_ptr<const Resolution>
SimpleInstallJob::resolution() const
{
    return _imp->resolution;
}

const std::tr1::shared_ptr<const ChangesToMakeDecision>
SimpleInstallJob::changes_to_make_decision() const
{
    return _imp->decision;
}

const std::tr1::shared_ptr<const ArrowSequence>
SimpleInstallJob::arrows() const
{
    return _imp->arrows;
}

const std::tr1::shared_ptr<ArrowSequence>
SimpleInstallJob::arrows()
{
    return _imp->arrows;
}

const std::tr1::shared_ptr<const JobIDSequence>
SimpleInstallJob::used_existing_packages_when_ordering() const
{
    return _imp->used_existing_packages_when_ordering;
}

const std::tr1::shared_ptr<JobIDSequence>
SimpleInstallJob::used_existing_packages_when_ordering()
{
    return _imp->used_existing_packages_when_ordering;
}

const JobID
SimpleInstallJob::id() const
{
    return make_named_values<JobID>(
            value_for<n::string_id>("install:" + stringify(resolution()->resolvent()))
            );
}

void
SimpleInstallJob::serialise(Serialiser & s) const
{
    s.object("SimpleInstallJob")
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "arrows", arrows())
        .member(SerialiserFlags<serialise::might_be_null>(), "changes_to_make_decision", changes_to_make_decision())
        .member(SerialiserFlags<serialise::might_be_null>(), "resolution", resolution())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(),
                "used_existing_packages_when_ordering", used_existing_packages_when_ordering())
        ;
}

ErrorJob::ErrorJob(const std::tr1::shared_ptr<const Resolution> & r, const std::tr1::shared_ptr<const UnableToMakeDecision> & d) :
    PrivateImplementationPattern<ErrorJob>(new Implementation<ErrorJob>(r, d))
{
}

ErrorJob::~ErrorJob()
{
}

const std::tr1::shared_ptr<const Resolution>
ErrorJob::resolution() const
{
    return _imp->resolution;
}

const std::tr1::shared_ptr<const ArrowSequence>
ErrorJob::arrows() const
{
    return _imp->arrows;
}

const std::tr1::shared_ptr<ArrowSequence>
ErrorJob::arrows()
{
    return _imp->arrows;
}

const std::tr1::shared_ptr<const JobIDSequence>
ErrorJob::used_existing_packages_when_ordering() const
{
    return _imp->used_existing_packages_when_ordering;
}

const std::tr1::shared_ptr<JobIDSequence>
ErrorJob::used_existing_packages_when_ordering()
{
    return _imp->used_existing_packages_when_ordering;
}

const JobID
ErrorJob::id() const
{
    return make_named_values<JobID>(
            value_for<n::string_id>("error:" + stringify(resolution()->resolvent()))
            );
}

void
ErrorJob::serialise(Serialiser & s) const
{
    s.object("ErrorJob")
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "arrows", arrows())
        .member(SerialiserFlags<serialise::might_be_null>(), "unable_to_make_decision", unable_to_make_decision())
        .member(SerialiserFlags<serialise::might_be_null>(), "resolution", resolution())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(),
                "used_existing_packages_when_ordering", used_existing_packages_when_ordering())
        ;
}

const std::tr1::shared_ptr<const UnableToMakeDecision>
ErrorJob::unable_to_make_decision() const
{
    return _imp->decision;
}
template class PrivateImplementationPattern<resolver::UsableJob>;
template class PrivateImplementationPattern<resolver::UsableGroupJob>;
template class PrivateImplementationPattern<resolver::FetchJob>;
template class PrivateImplementationPattern<resolver::SimpleInstallJob>;
template class PrivateImplementationPattern<resolver::ErrorJob>;

