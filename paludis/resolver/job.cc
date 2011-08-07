/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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
#include <paludis/resolver/job_state.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/package_id.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<PretendJob>
    {
        const PackageDepSpec origin_id_spec;
        const RepositoryName destination_repository_name;
        const DestinationType destination_type;

        Imp(const PackageDepSpec & o, const RepositoryName & r, const DestinationType t) :
            origin_id_spec(o),
            destination_repository_name(r),
            destination_type(t)
        {
        }
    };
}

PretendJob::PretendJob(const PackageDepSpec & o, const RepositoryName & r, const DestinationType t) :
    _imp(o, r, t)
{
}

PretendJob::~PretendJob()
{
}

const PackageDepSpec
PretendJob::origin_id_spec() const
{
    return _imp->origin_id_spec;
}

const RepositoryName
PretendJob::destination_repository_name() const
{
    return _imp->destination_repository_name;
}

DestinationType
PretendJob::destination_type() const
{
    return _imp->destination_type;
}

const std::shared_ptr<PretendJob>
PretendJob::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "PretendJob");
    return std::make_shared<PretendJob>(
                parse_user_package_dep_spec(v.member<std::string>("origin_id_spec"),
                    d.deserialiser().environment(), { updso_no_disambiguation }),
                RepositoryName(v.member<std::string>("destination_repository_name")),
                destringify<DestinationType>(v.member<std::string>("destination_type"))
                );
}

void
PretendJob::serialise(Serialiser & s) const
{
    s.object("PretendJob")
        .member(SerialiserFlags<>(), "origin_id_spec", stringify(origin_id_spec()))
        .member(SerialiserFlags<>(), "destination_type", stringify(destination_type()))
        .member(SerialiserFlags<>(), "destination_repository_name", stringify(destination_repository_name()))
        ;
}

ExecuteJob::~ExecuteJob()
{
}

const std::shared_ptr<ExecuteJob>
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
    struct Imp<FetchJob>
    {
        const std::shared_ptr<const JobRequirements> requirements;
        const PackageDepSpec origin_id_spec;
        std::shared_ptr<JobState> state;

        Imp(
                const std::shared_ptr<const JobRequirements> & r,
                const PackageDepSpec & o) :
            requirements(r),
            origin_id_spec(o)
        {
        }
    };
}

FetchJob::FetchJob(
        const std::shared_ptr<const JobRequirements> & r,
        const PackageDepSpec & o) :
    _imp(r, o)
{
}

FetchJob::~FetchJob()
{
}

const PackageDepSpec
FetchJob::origin_id_spec() const
{
    return _imp->origin_id_spec;
}

const std::shared_ptr<JobState>
FetchJob::state() const
{
    return _imp->state;
}

void
FetchJob::set_state(const std::shared_ptr<JobState> & s)
{
    _imp->state = s;
}

const std::shared_ptr<const JobRequirements>
FetchJob::requirements() const
{
    return _imp->requirements;
}

const std::shared_ptr<FetchJob>
FetchJob::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "FetchJob");

    std::shared_ptr<JobRequirements> requirements(std::make_shared<JobRequirements>());
    {
        Deserialisator vv(*v.find_remove_member("requirements"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            requirements->push_back(vv.member<JobRequirement>(stringify(n)));
    }

    std::shared_ptr<FetchJob> result(std::make_shared<FetchJob>(
                requirements,
                parse_user_package_dep_spec(v.member<std::string>("origin_id_spec"),
                    d.deserialiser().environment(), { updso_no_disambiguation })
                ));
    result->set_state(v.member<std::shared_ptr<JobState> >("state"));
    return result;
}

void
FetchJob::serialise(Serialiser & s) const
{
    s.object("FetchJob")
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "requirements", requirements())
        .member(SerialiserFlags<>(), "origin_id_spec", stringify(origin_id_spec()))
        .member(SerialiserFlags<serialise::might_be_null>(), "state", state())
        ;
}

namespace paludis
{
    template <>
    struct Imp<InstallJob>
    {
        const std::shared_ptr<const JobRequirements> requirements;
        const PackageDepSpec origin_id_spec;
        const RepositoryName destination_repository_name;
        const DestinationType destination_type;
        const std::shared_ptr<const Sequence<PackageDepSpec> > replacing_specs;

        std::shared_ptr<JobState> state;

        Imp(
                const std::shared_ptr<const JobRequirements> & q,
                const PackageDepSpec & o,
                const RepositoryName & d,
                const DestinationType t,
                const std::shared_ptr<const Sequence<PackageDepSpec> > & r
                ) :
            requirements(q),
            origin_id_spec(o),
            destination_repository_name(d),
            destination_type(t),
            replacing_specs(r)
        {
        }
    };
}

InstallJob::InstallJob(
        const std::shared_ptr<const JobRequirements> & q,
        const PackageDepSpec & o,
        const RepositoryName & d,
        const DestinationType t,
        const std::shared_ptr<const Sequence<PackageDepSpec> > & r
        ) :
    _imp(q, o, d, t, r)
{
}

InstallJob::~InstallJob()
{
}

const PackageDepSpec
InstallJob::origin_id_spec() const
{
    return _imp->origin_id_spec;
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

const std::shared_ptr<const Sequence<PackageDepSpec> >
InstallJob::replacing_specs() const
{
    return _imp->replacing_specs;
}

const std::shared_ptr<JobState>
InstallJob::state() const
{
    return _imp->state;
}

void
InstallJob::set_state(const std::shared_ptr<JobState> & s)
{
    _imp->state = s;
}

const std::shared_ptr<const JobRequirements>
InstallJob::requirements() const
{
    return _imp->requirements;
}

const std::shared_ptr<InstallJob>
InstallJob::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "InstallJob");

    std::shared_ptr<Sequence<PackageDepSpec> > replacing_specs(std::make_shared<Sequence<PackageDepSpec>>());
    {
        Deserialisator vv(*v.find_remove_member("replacing_specs"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            replacing_specs->push_back(parse_user_package_dep_spec(vv.member<std::string>(stringify(n)),
                        d.deserialiser().environment(), { updso_no_disambiguation }));
    }

    std::shared_ptr<JobRequirements> requirements(std::make_shared<JobRequirements>());
    {
        Deserialisator vv(*v.find_remove_member("requirements"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            requirements->push_back(vv.member<JobRequirement>(stringify(n)));
    }

    std::shared_ptr<InstallJob> result(std::make_shared<InstallJob>(
                requirements,
                parse_user_package_dep_spec(v.member<std::string>("origin_id_spec"),
                    d.deserialiser().environment(), { updso_no_disambiguation }),
                RepositoryName(v.member<std::string>("destination_repository_name")),
                destringify<DestinationType>(v.member<std::string>("destination_type")),
                replacing_specs
                ));

    result->set_state(v.member<std::shared_ptr<JobState> >("state"));
    return result;
}

void
InstallJob::serialise(Serialiser & s) const
{
    std::shared_ptr<Sequence<std::string> > replacing_specs_s(std::make_shared<Sequence<std::string>>());
    for (Sequence<PackageDepSpec>::ConstIterator r(replacing_specs()->begin()),
            r_end(replacing_specs()->end()) ;
            r != r_end ; ++r)
        replacing_specs_s->push_back(stringify(*r));

    s.object("InstallJob")
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "requirements", requirements())
        .member(SerialiserFlags<>(), "origin_id_spec", stringify(origin_id_spec()))
        .member(SerialiserFlags<>(), "destination_repository_name", stringify(destination_repository_name()))
        .member(SerialiserFlags<>(), "destination_type", stringify(destination_type()))
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "replacing_specs", replacing_specs_s)
        .member(SerialiserFlags<serialise::might_be_null>(), "state", state())
        ;
}

namespace paludis
{
    template <>
    struct Imp<UninstallJob>
    {
        const std::shared_ptr<const JobRequirements> requirements;
        const std::shared_ptr<const Sequence<PackageDepSpec> > ids_to_remove_specs;

        std::shared_ptr<JobState> state;

        Imp(
                const std::shared_ptr<const JobRequirements> & q,
                const std::shared_ptr<const Sequence<PackageDepSpec> > & r
                ) :
            requirements(q),
            ids_to_remove_specs(r)
        {
        }
    };
}

UninstallJob::UninstallJob(
        const std::shared_ptr<const JobRequirements> & q,
        const std::shared_ptr<const Sequence<PackageDepSpec> > & r
        ) :
    _imp(q, r)
{
}

UninstallJob::~UninstallJob()
{
}

const std::shared_ptr<const Sequence<PackageDepSpec> >
UninstallJob::ids_to_remove_specs() const
{
    return _imp->ids_to_remove_specs;
}

const std::shared_ptr<JobState>
UninstallJob::state() const
{
    return _imp->state;
}

void
UninstallJob::set_state(const std::shared_ptr<JobState> & s)
{
    _imp->state = s;
}

const std::shared_ptr<const JobRequirements>
UninstallJob::requirements() const
{
    return _imp->requirements;
}

const std::shared_ptr<UninstallJob>
UninstallJob::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "UninstallJob");

    std::shared_ptr<Sequence<PackageDepSpec> > ids_to_remove_specs(std::make_shared<Sequence<PackageDepSpec> >());
    {
        Deserialisator vv(*v.find_remove_member("ids_to_remove_specs"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            ids_to_remove_specs->push_back(parse_user_package_dep_spec(vv.member<std::string>(stringify(n)),
                        d.deserialiser().environment(), { updso_no_disambiguation }));
    }

    std::shared_ptr<JobRequirements> requirements(std::make_shared<JobRequirements>());
    {
        Deserialisator vv(*v.find_remove_member("requirements"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            requirements->push_back(vv.member<JobRequirement>(stringify(n)));
    }

    std::shared_ptr<UninstallJob> result(std::make_shared<UninstallJob>(
                requirements,
                ids_to_remove_specs
                ));

    result->set_state(v.member<std::shared_ptr<JobState> >("state"));
    return result;
}

void
UninstallJob::serialise(Serialiser & s) const
{
    std::shared_ptr<Sequence<std::string> > ids_to_remove_specs_s(std::make_shared<Sequence<std::string>>());
    for (Sequence<PackageDepSpec>::ConstIterator r(ids_to_remove_specs()->begin()),
            r_end(ids_to_remove_specs()->end()) ;
            r != r_end ; ++r)
        ids_to_remove_specs_s->push_back(stringify(*r));

    s.object("UninstallJob")
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "requirements", requirements())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "ids_to_remove_specs", ids_to_remove_specs_s)
        .member(SerialiserFlags<serialise::might_be_null>(), "state", state())
        ;
}

namespace paludis
{
    template class Sequence<PackageDepSpec>;
    template class WrappedForwardIterator<Sequence<PackageDepSpec>::ConstIteratorTag, const PackageDepSpec>;
}
