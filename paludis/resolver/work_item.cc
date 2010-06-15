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

#include <paludis/resolver/work_item.hh>
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
    struct Implementation<PretendWorkItem>
    {
        const std::tr1::shared_ptr<const PackageID> origin_id;

        Implementation(const std::tr1::shared_ptr<const PackageID> & o) :
            origin_id(o)
        {
        }
    };
}

PretendWorkItem::PretendWorkItem(const std::tr1::shared_ptr<const PackageID> & o) :
    PrivateImplementationPattern<PretendWorkItem>(new Implementation<PretendWorkItem>(o))
{
}

PretendWorkItem::~PretendWorkItem()
{
}

const std::tr1::shared_ptr<const PackageID>
PretendWorkItem::origin_id() const
{
    return _imp->origin_id;
}

const std::tr1::shared_ptr<PretendWorkItem>
PretendWorkItem::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "PretendWorkItem");
    return make_shared_ptr(new PretendWorkItem(
                v.member<std::tr1::shared_ptr<const PackageID> >("origin_id")
                ));
}

void
PretendWorkItem::serialise(Serialiser & s) const
{
    s.object("PretendWorkItem")
        .member(SerialiserFlags<serialise::might_be_null>(), "origin_id", origin_id())
        ;
}

ExecuteWorkItem::~ExecuteWorkItem()
{
}

const std::tr1::shared_ptr<ExecuteWorkItem>
ExecuteWorkItem::deserialise(Deserialisation & d)
{
    if (d.class_name() == "FetchWorkItem")
        return FetchWorkItem::deserialise(d);
    else if (d.class_name() == "InstallWorkItem")
        return InstallWorkItem::deserialise(d);
    else if (d.class_name() == "UninstallWorkItem")
        return UninstallWorkItem::deserialise(d);
    else
        throw InternalError(PALUDIS_HERE, "unknown class '" + stringify(d.class_name()) + "'");
}

namespace paludis
{
    template <>
    struct Implementation<FetchWorkItem>
    {
        const std::tr1::shared_ptr<const PackageID> origin_id;

        Implementation(const std::tr1::shared_ptr<const PackageID> & o) :
            origin_id(o)
        {
        }
    };
}

FetchWorkItem::FetchWorkItem(const std::tr1::shared_ptr<const PackageID> & o) :
    PrivateImplementationPattern<FetchWorkItem>(new Implementation<FetchWorkItem>(o))
{
}

FetchWorkItem::~FetchWorkItem()
{
}

const std::tr1::shared_ptr<const PackageID>
FetchWorkItem::origin_id() const
{
    return _imp->origin_id;
}

const std::tr1::shared_ptr<FetchWorkItem>
FetchWorkItem::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "FetchWorkItem");
    return make_shared_ptr(new FetchWorkItem(
                v.member<std::tr1::shared_ptr<const PackageID> >("origin_id")
                ));
}

void
FetchWorkItem::serialise(Serialiser & s) const
{
    s.object("FetchWorkItem")
        .member(SerialiserFlags<serialise::might_be_null>(), "origin_id", origin_id())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<InstallWorkItem>
    {
        const std::tr1::shared_ptr<const PackageID> origin_id;
        const RepositoryName destination_repository_name;
        const DestinationType destination_type;
        const std::tr1::shared_ptr<const PackageIDSequence> replacing;

        Implementation(
                const std::tr1::shared_ptr<const PackageID> & o,
                const RepositoryName & d,
                const DestinationType t,
                const std::tr1::shared_ptr<const PackageIDSequence> & r
                ) :
            origin_id(o),
            destination_repository_name(d),
            destination_type(t),
            replacing(r)
        {
        }
    };
}

InstallWorkItem::InstallWorkItem(
        const std::tr1::shared_ptr<const PackageID> & o,
        const RepositoryName & d,
        const DestinationType t,
        const std::tr1::shared_ptr<const PackageIDSequence> & r
        ) :
    PrivateImplementationPattern<InstallWorkItem>(new Implementation<InstallWorkItem>(o, d, t, r))
{
}

InstallWorkItem::~InstallWorkItem()
{
}

const std::tr1::shared_ptr<const PackageID>
InstallWorkItem::origin_id() const
{
    return _imp->origin_id;
}

const RepositoryName
InstallWorkItem::destination_repository_name() const
{
    return _imp->destination_repository_name;
}

DestinationType
InstallWorkItem::destination_type() const
{
    return _imp->destination_type;
}

const std::tr1::shared_ptr<const PackageIDSequence>
InstallWorkItem::replacing() const
{
    return _imp->replacing;
}

const std::tr1::shared_ptr<InstallWorkItem>
InstallWorkItem::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "InstallWorkItem");

    std::tr1::shared_ptr<PackageIDSequence> replacing(new PackageIDSequence);
    {
        Deserialisator vv(*v.find_remove_member("replacing"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            replacing->push_back(vv.member<std::tr1::shared_ptr<const PackageID> >(stringify(n)));
    }

    return make_shared_ptr(new InstallWorkItem(
                v.member<std::tr1::shared_ptr<const PackageID> >("origin_id"),
                RepositoryName(v.member<std::string>("destination_repository_name")),
                destringify<DestinationType>(v.member<std::string>("destination_type")),
                replacing
                ));
}

void
InstallWorkItem::serialise(Serialiser & s) const
{
    s.object("InstallWorkItem")
        .member(SerialiserFlags<serialise::might_be_null>(), "origin_id", origin_id())
        .member(SerialiserFlags<>(), "destination_repository_name", stringify(destination_repository_name()))
        .member(SerialiserFlags<>(), "destination_type", stringify(destination_type()))
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "replacing", replacing())
        ;
}

namespace paludis
{
    template <>
    struct Implementation<UninstallWorkItem>
    {
        const std::tr1::shared_ptr<const PackageIDSequence> ids_to_remove;

        Implementation(
                const std::tr1::shared_ptr<const PackageIDSequence> & r
                ) :
            ids_to_remove(r)
        {
        }
    };
}

UninstallWorkItem::UninstallWorkItem(
        const std::tr1::shared_ptr<const PackageIDSequence> & r
        ) :
    PrivateImplementationPattern<UninstallWorkItem>(new Implementation<UninstallWorkItem>(r))
{
}

UninstallWorkItem::~UninstallWorkItem()
{
}

const std::tr1::shared_ptr<const PackageIDSequence>
UninstallWorkItem::ids_to_remove() const
{
    return _imp->ids_to_remove;
}

const std::tr1::shared_ptr<UninstallWorkItem>
UninstallWorkItem::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "UninstallWorkItem");

    std::tr1::shared_ptr<PackageIDSequence> ids_to_remove(new PackageIDSequence);
    {
        Deserialisator vv(*v.find_remove_member("ids_to_remove"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            ids_to_remove->push_back(vv.member<std::tr1::shared_ptr<const PackageID> >(stringify(n)));
    }

    return make_shared_ptr(new UninstallWorkItem(
                ids_to_remove
                ));
}

void
UninstallWorkItem::serialise(Serialiser & s) const
{
    s.object("UninstallWorkItem")
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "ids_to_remove", ids_to_remove())
        ;
}

