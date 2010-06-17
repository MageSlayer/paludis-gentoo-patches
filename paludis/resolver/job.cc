/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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
#include <paludis/resolver/job_requirements.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/sequence.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/package_id.hh>
#include <paludis/name.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<PretendJob>
    {
        const std::tr1::shared_ptr<const PackageID> origin_id;

        Implementation(const std::tr1::shared_ptr<const PackageID> & o) :
            origin_id(o)
        {
        }
    };
}

PretendJob::PretendJob(const std::tr1::shared_ptr<const PackageID> & o) :
    PrivateImplementationPattern<PretendJob>(new Implementation<PretendJob>(o))
{
}

PretendJob::~PretendJob()
{
}

const std::tr1::shared_ptr<const PackageID>
PretendJob::origin_id() const
{
    return _imp->origin_id;
}

const std::tr1::shared_ptr<PretendJob>
PretendJob::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "PretendJob");
    return make_shared_ptr(new PretendJob(
                v.member<std::tr1::shared_ptr<const PackageID> >("origin_id")
                ));
}

void
PretendJob::serialise(Serialiser & s) const
{
    s.object("PretendJob")
        .member(SerialiserFlags<serialise::might_be_null>(), "origin_id", origin_id())
        ;
}

ExecuteJob::~ExecuteJob()
{
}

const std::tr1::shared_ptr<ExecuteJob>
ExecuteJob::deserialise(Deserialisation & d)
{
    if (d.class_name() == "FetchJob")
        return FetchJob::deserialise(d);
    else if (d.class_name() == "InstallJob")
        return InstallJob::deserialise(d);
    else if (d.class_name() == "UninstallJob")
        return UninstallJob::deserialise(d);
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");
}

namespace paludis
{
    template <>
    struct Implementation<FetchJob>
    {
        const std::tr1::shared_ptr<const JobRequirements> requirements;
        const std::tr1::shared_ptr<const PackageID> origin_id;
        std::tr1::shared_ptr<JobState> state;

        Implementation(
                const std::tr1::shared_ptr<const JobRequirements> & r,
                const std::tr1::shared_ptr<const PackageID> & o) :
            requirements(r),
            origin_id(o)
        {
        }
    };
}

FetchJob::FetchJob(
        const std::tr1::shared_ptr<const JobRequirements> & r,
        const std::tr1::shared_ptr<const PackageID> & o) :
    PrivateImplementationPattern<FetchJob>(new Implementation<FetchJob>(r, o))
{
}

FetchJob::~FetchJob()
{
}

const std::tr1::shared_ptr<const PackageID>
FetchJob::origin_id() const
{
    return _imp->origin_id;
}

const std::tr1::shared_ptr<JobState>
FetchJob::state() const
{
    return _imp->state;
}

void
FetchJob::set_state(const std::tr1::shared_ptr<JobState> & s)
{
    _imp->state = s;
}

const std::tr1::shared_ptr<const JobRequirements>
FetchJob::requirements() const
{
    return _imp->requirements;
}

const std::tr1::shared_ptr<FetchJob>
FetchJob::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "FetchJob");

    std::tr1::shared_ptr<JobRequirements> requirements(new JobRequirements);
    {
        Deserialisator vv(*v.find_remove_member("requirements"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            requirements->push_back(vv.member<JobRequirement>(stringify(n)));
    }

    return make_shared_ptr(new FetchJob(
                requirements,
                v.member<std::tr1::shared_ptr<const PackageID> >("origin_id")
                ));
}

void
FetchJob::serialise(Serialiser & s) const
{
    s.object("FetchJob")
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "requirements", requirements())
        .member(SerialiserFlags<serialise::might_be_null>(), "origin_id", origin_id())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<InstallJob>
    {
        const std::tr1::shared_ptr<const JobRequirements> requirements;
        const std::tr1::shared_ptr<const PackageID> origin_id;
        const RepositoryName destination_repository_name;
        const DestinationType destination_type;
        const std::tr1::shared_ptr<const PackageIDSequence> replacing;

        std::tr1::shared_ptr<JobState> state;

        Implementation(
                const std::tr1::shared_ptr<const JobRequirements> & q,
                const std::tr1::shared_ptr<const PackageID> & o,
                const RepositoryName & d,
                const DestinationType t,
                const std::tr1::shared_ptr<const PackageIDSequence> & r
                ) :
            requirements(q),
            origin_id(o),
            destination_repository_name(d),
            destination_type(t),
            replacing(r)
        {
        }
    };
}

InstallJob::InstallJob(
        const std::tr1::shared_ptr<const JobRequirements> & q,
        const std::tr1::shared_ptr<const PackageID> & o,
        const RepositoryName & d,
        const DestinationType t,
        const std::tr1::shared_ptr<const PackageIDSequence> & r
        ) :
    PrivateImplementationPattern<InstallJob>(new Implementation<InstallJob>(q, o, d, t, r))
{
}

InstallJob::~InstallJob()
{
}

const std::tr1::shared_ptr<const PackageID>
InstallJob::origin_id() const
{
    return _imp->origin_id;
}

const RepositoryName
InstallJob::destination_repository_name() const
{
    return _imp->destination_repository_name;
}

DestinationType
InstallJob::destination_type() const
{
    return _imp->destination_type;
}

const std::tr1::shared_ptr<const PackageIDSequence>
InstallJob::replacing() const
{
    return _imp->replacing;
}

const std::tr1::shared_ptr<JobState>
InstallJob::state() const
{
    return _imp->state;
}

void
InstallJob::set_state(const std::tr1::shared_ptr<JobState> & s)
{
    _imp->state = s;
}

const std::tr1::shared_ptr<const JobRequirements>
InstallJob::requirements() const
{
    return _imp->requirements;
}

const std::tr1::shared_ptr<InstallJob>
InstallJob::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "InstallJob");

    std::tr1::shared_ptr<PackageIDSequence> replacing(new PackageIDSequence);
    {
        Deserialisator vv(*v.find_remove_member("replacing"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            replacing->push_back(vv.member<std::tr1::shared_ptr<const PackageID> >(stringify(n)));
    }

    std::tr1::shared_ptr<JobRequirements> requirements(new JobRequirements);
    {
        Deserialisator vv(*v.find_remove_member("requirements"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            requirements->push_back(vv.member<JobRequirement>(stringify(n)));
    }

    return make_shared_ptr(new InstallJob(
                requirements,
                v.member<std::tr1::shared_ptr<const PackageID> >("origin_id"),
                RepositoryName(v.member<std::string>("destination_repository_name")),
                destringify<DestinationType>(v.member<std::string>("destination_type")),
                replacing
                ));
}

void
InstallJob::serialise(Serialiser & s) const
{
    s.object("InstallJob")
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "requirements", requirements())
        .member(SerialiserFlags<serialise::might_be_null>(), "origin_id", origin_id())
        .member(SerialiserFlags<>(), "destination_repository_name", stringify(destination_repository_name()))
        .member(SerialiserFlags<>(), "destination_type", stringify(destination_type()))
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "replacing", replacing())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<UninstallJob>
    {
        const std::tr1::shared_ptr<const JobRequirements> requirements;
        const std::tr1::shared_ptr<const PackageIDSequence> ids_to_remove;

        std::tr1::shared_ptr<JobState> state;

        Implementation(
                const std::tr1::shared_ptr<const JobRequirements> & q,
                const std::tr1::shared_ptr<const PackageIDSequence> & r
                ) :
            requirements(q),
            ids_to_remove(r)
        {
        }
    };
}

UninstallJob::UninstallJob(
        const std::tr1::shared_ptr<const JobRequirements> & q,
        const std::tr1::shared_ptr<const PackageIDSequence> & r
        ) :
    PrivateImplementationPattern<UninstallJob>(new Implementation<UninstallJob>(q, r))
{
}

UninstallJob::~UninstallJob()
{
}

const std::tr1::shared_ptr<const PackageIDSequence>
UninstallJob::ids_to_remove() const
{
    return _imp->ids_to_remove;
}

const std::tr1::shared_ptr<JobState>
UninstallJob::state() const
{
    return _imp->state;
}

void
UninstallJob::set_state(const std::tr1::shared_ptr<JobState> & s)
{
    _imp->state = s;
}

const std::tr1::shared_ptr<const JobRequirements>
UninstallJob::requirements() const
{
    return _imp->requirements;
}

const std::tr1::shared_ptr<UninstallJob>
UninstallJob::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "UninstallJob");

    std::tr1::shared_ptr<PackageIDSequence> ids_to_remove(new PackageIDSequence);
    {
        Deserialisator vv(*v.find_remove_member("ids_to_remove"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            ids_to_remove->push_back(vv.member<std::tr1::shared_ptr<const PackageID> >(stringify(n)));
    }

    std::tr1::shared_ptr<JobRequirements> requirements(new JobRequirements);
    {
        Deserialisator vv(*v.find_remove_member("requirements"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            requirements->push_back(vv.member<JobRequirement>(stringify(n)));
    }

    return make_shared_ptr(new UninstallJob(
                requirements,
                ids_to_remove
                ));
}

void
UninstallJob::serialise(Serialiser & s) const
{
    s.object("UninstallJob")
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "requirements", requirements())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "ids_to_remove", ids_to_remove())
        ;
}

