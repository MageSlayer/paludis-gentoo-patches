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

#include <paludis/resolver/job.hh>
#include <paludis/resolver/arrow.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/stringify.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<NoChangeJob>
    {
        const std::tr1::shared_ptr<const Resolution> resolution;
        const std::tr1::shared_ptr<ArrowSequence> arrows;

        Implementation(const std::tr1::shared_ptr<const Resolution> & r) :
            resolution(r),
            arrows(new ArrowSequence)
        {
        }
    };

    template <>
    struct Implementation<PretendJob>
    {
        const std::tr1::shared_ptr<const Resolution> resolution;
        const std::tr1::shared_ptr<const ChangesToMakeDecision> decision;
        const std::tr1::shared_ptr<ArrowSequence> arrows;

        Implementation(const std::tr1::shared_ptr<const Resolution> & r,
                const std::tr1::shared_ptr<const ChangesToMakeDecision> & d) :
            resolution(r),
            decision(d),
            arrows(new ArrowSequence)
        {
        }
    };

    template <>
    struct Implementation<FetchJob>
    {
        const std::tr1::shared_ptr<const Resolution> resolution;
        const std::tr1::shared_ptr<const ChangesToMakeDecision> decision;
        const std::tr1::shared_ptr<ArrowSequence> arrows;

        Implementation(const std::tr1::shared_ptr<const Resolution> & r,
                const std::tr1::shared_ptr<const ChangesToMakeDecision> & d) :
            resolution(r),
            decision(d),
            arrows(new ArrowSequence)
        {
        }
    };

    template <>
    struct Implementation<SimpleInstallJob>
    {
        const std::tr1::shared_ptr<const Resolution> resolution;
        const std::tr1::shared_ptr<const ChangesToMakeDecision> decision;
        const std::tr1::shared_ptr<ArrowSequence> arrows;

        Implementation(const std::tr1::shared_ptr<const Resolution> & r,
                const std::tr1::shared_ptr<const ChangesToMakeDecision> & d) :
            resolution(r),
            decision(d),
            arrows(new ArrowSequence)
        {
        }
    };

    template <>
    struct Implementation<UntakenInstallJob>
    {
        const std::tr1::shared_ptr<const Resolution> resolution;
        const std::tr1::shared_ptr<ArrowSequence> arrows;

        Implementation(const std::tr1::shared_ptr<const Resolution> & r) :
            resolution(r),
            arrows(new ArrowSequence)
        {
        }
    };

    template <>
    struct Implementation<SyncPointJob>
    {
        const SyncPoint sync_point;
        const std::tr1::shared_ptr<ArrowSequence> arrows;

        Implementation(const SyncPoint s) :
            sync_point(s),
            arrows(new ArrowSequence)
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
}

const std::tr1::shared_ptr<Job>
Job::deserialise(Deserialisation & d)
{
    std::tr1::shared_ptr<Job> result;

    if (d.class_name() == "NoChangeJob")
    {
        Deserialisator v(d, "NoChangeJob");
        result.reset(new NoChangeJob(
                    v.member<std::tr1::shared_ptr<Resolution> >("resolution")
                    ));
        do_arrows(result, v);
    }
    else if (d.class_name() == "SimpleInstallJob")
    {
        Deserialisator v(d, "SimpleInstallJob");
        result.reset(new SimpleInstallJob(
                    v.member<std::tr1::shared_ptr<Resolution> >("resolution"),
                    v.member<std::tr1::shared_ptr<ChangesToMakeDecision> >("decision")
                    ));
        do_arrows(result, v);
    }
    else if (d.class_name() == "PretendJob")
    {
        Deserialisator v(d, "PretendJob");
        result.reset(new PretendJob(
                    v.member<std::tr1::shared_ptr<Resolution> >("resolution"),
                    v.member<std::tr1::shared_ptr<ChangesToMakeDecision> >("decision")
                    ));
        do_arrows(result, v);
    }
    else if (d.class_name() == "FetchJob")
    {
        Deserialisator v(d, "FetchJob");
        result.reset(new FetchJob(
                    v.member<std::tr1::shared_ptr<Resolution> >("resolution"),
                    v.member<std::tr1::shared_ptr<ChangesToMakeDecision> >("decision")
                    ));
        do_arrows(result, v);
    }
    else if (d.class_name() == "UntakenInstallJob")
    {
        Deserialisator v(d, "UntakenInstallJob");
        result.reset(new UntakenInstallJob(
                    v.member<std::tr1::shared_ptr<Resolution> >("resolution")
                    ));
        do_arrows(result, v);
    }
    else if (d.class_name() == "SyncPointJob")
    {
        Deserialisator v(d, "SyncPointJob");
        result.reset(new SyncPointJob(
                    destringify<SyncPoint>(v.member<std::string>("sync_point"))
                    ));
        do_arrows(result, v);
    }
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");

    return result;
}

NoChangeJob::NoChangeJob(const std::tr1::shared_ptr<const Resolution> & r) :
    PrivateImplementationPattern<NoChangeJob>(new Implementation<NoChangeJob>(r))
{
}

NoChangeJob::~NoChangeJob()
{
}

const std::tr1::shared_ptr<const Resolution>
NoChangeJob::resolution() const
{
    return _imp->resolution;
}

const std::tr1::shared_ptr<ArrowSequence>
NoChangeJob::arrows() const
{
    return _imp->arrows;
}

const JobID
NoChangeJob::id() const
{
    return make_named_values<JobID>(
            value_for<n::string_id>("n:" + stringify(resolution()->resolvent()))
            );
}

void
NoChangeJob::serialise(Serialiser & s) const
{
    s.object("NoChangeJob")
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "arrows", arrows())
        .member(SerialiserFlags<serialise::might_be_null>(), "resolution", resolution())
        ;
}

PretendJob::PretendJob(const std::tr1::shared_ptr<const Resolution> & r,
        const std::tr1::shared_ptr<const ChangesToMakeDecision> & d) :
    PrivateImplementationPattern<PretendJob>(new Implementation<PretendJob>(r, d))
{
}

PretendJob::~PretendJob()
{
}

const std::tr1::shared_ptr<const Resolution>
PretendJob::resolution() const
{
    return _imp->resolution;
}

const std::tr1::shared_ptr<const ChangesToMakeDecision>
PretendJob::decision() const
{
    return _imp->decision;
}

const std::tr1::shared_ptr<ArrowSequence>
PretendJob::arrows() const
{
    return _imp->arrows;
}

const JobID
PretendJob::id() const
{
    return make_named_values<JobID>(
            value_for<n::string_id>("p:" + stringify(resolution()->resolvent()))
            );
}

void
PretendJob::serialise(Serialiser & s) const
{
    s.object("PretendJob")
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "arrows", arrows())
        .member(SerialiserFlags<serialise::might_be_null>(), "decision", decision())
        .member(SerialiserFlags<serialise::might_be_null>(), "resolution", resolution())
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
FetchJob::decision() const
{
    return _imp->decision;
}

const std::tr1::shared_ptr<ArrowSequence>
FetchJob::arrows() const
{
    return _imp->arrows;
}

const JobID
FetchJob::id() const
{
    return make_named_values<JobID>(
            value_for<n::string_id>("f:" + stringify(resolution()->resolvent()))
            );
}

void
FetchJob::serialise(Serialiser & s) const
{
    s.object("FetchJob")
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "arrows", arrows())
        .member(SerialiserFlags<serialise::might_be_null>(), "decision", decision())
        .member(SerialiserFlags<serialise::might_be_null>(), "resolution", resolution())
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
SimpleInstallJob::decision() const
{
    return _imp->decision;
}

const std::tr1::shared_ptr<ArrowSequence>
SimpleInstallJob::arrows() const
{
    return _imp->arrows;
}

const JobID
SimpleInstallJob::id() const
{
    return make_named_values<JobID>(
            value_for<n::string_id>("i:" + stringify(resolution()->resolvent()))
            );
}

void
SimpleInstallJob::serialise(Serialiser & s) const
{
    s.object("SimpleInstallJob")
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "arrows", arrows())
        .member(SerialiserFlags<serialise::might_be_null>(), "decision", decision())
        .member(SerialiserFlags<serialise::might_be_null>(), "resolution", resolution())
        ;
}

UntakenInstallJob::UntakenInstallJob(const std::tr1::shared_ptr<const Resolution> & r) :
    PrivateImplementationPattern<UntakenInstallJob>(new Implementation<UntakenInstallJob>(r))
{
}

UntakenInstallJob::~UntakenInstallJob()
{
}

const std::tr1::shared_ptr<const Resolution>
UntakenInstallJob::resolution() const
{
    return _imp->resolution;
}

const std::tr1::shared_ptr<ArrowSequence>
UntakenInstallJob::arrows() const
{
    return _imp->arrows;
}

const JobID
UntakenInstallJob::id() const
{
    return make_named_values<JobID>(
            value_for<n::string_id>("u:" + stringify(resolution()->resolvent()))
            );
}

void
UntakenInstallJob::serialise(Serialiser & s) const
{
    s.object("UntakenInstallJob")
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "arrows", arrows())
        .member(SerialiserFlags<serialise::might_be_null>(), "resolution", resolution())
        ;
}

SyncPointJob::SyncPointJob(const SyncPoint s) :
    PrivateImplementationPattern<SyncPointJob>(new Implementation<SyncPointJob>(s))
{
}

SyncPointJob::~SyncPointJob()
{
}

SyncPoint
SyncPointJob::sync_point() const
{
    return _imp->sync_point;
}

const std::tr1::shared_ptr<ArrowSequence>
SyncPointJob::arrows() const
{
    return _imp->arrows;
}

const JobID
SyncPointJob::id() const
{
    return make_named_values<JobID>(
            value_for<n::string_id>("s:" + stringify(sync_point()))
            );
}

void
SyncPointJob::serialise(Serialiser & s) const
{
    s.object("SyncPointJob")
        .member(SerialiserFlags<serialise::might_be_null, serialise::container>(), "arrows", arrows())
        .member(SerialiserFlags<>(), "sync_point", stringify(sync_point()))
        ;
}

template class PrivateImplementationPattern<resolver::NoChangeJob>;
template class PrivateImplementationPattern<resolver::PretendJob>;
template class PrivateImplementationPattern<resolver::FetchJob>;
template class PrivateImplementationPattern<resolver::SimpleInstallJob>;
template class PrivateImplementationPattern<resolver::SyncPointJob>;
template class PrivateImplementationPattern<resolver::UntakenInstallJob>;

