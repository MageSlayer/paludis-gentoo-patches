/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/repositories/unavailable/unavailable_id.hh>
#include <paludis/repositories/unavailable/unavailable_repository.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/kc.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/hashes.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>

using namespace paludis;
using namespace paludis::unavailable_repository;

namespace paludis
{
    template <>
    struct Implementation<UnavailableID>
    {
        const QualifiedPackageName name;
        const VersionSpec version;
        const SlotName slot;
        const UnavailableRepository * const repo;

        const std::tr1::shared_ptr<const MetadataValueKey<std::string> > description_key;
        const std::tr1::shared_ptr<const MetadataValueKey<std::string> > owning_repository_key, repository_homepage_key, repository_description_key;
        const std::tr1::shared_ptr<const Mask> mask;

        Implementation(
                const UnavailableIDParams & e) :
            name(e[k::name()]),
            version(e[k::version()]),
            slot(e[k::slot()]),
            repo(e[k::repository()]),
            description_key(e[k::description()]),
            owning_repository_key(e[k::owning_repository()]),
            repository_homepage_key(e[k::repository_homepage()]),
            repository_description_key(e[k::repository_description()]),
            mask(e[k::mask()])
        {
        }
    };
}

UnavailableID::UnavailableID(const UnavailableIDParams & entry) :
    PrivateImplementationPattern<UnavailableID>(new Implementation<UnavailableID>(entry)),
    _imp(PrivateImplementationPattern<UnavailableID>::_imp)
{
    add_metadata_key(_imp->description_key);
    add_metadata_key(_imp->owning_repository_key);
    if (_imp->repository_homepage_key)
        add_metadata_key(_imp->repository_homepage_key);
    if (_imp->repository_description_key)
        add_metadata_key(_imp->repository_description_key);
    add_mask(_imp->mask);
}

UnavailableID::~UnavailableID()
{
}

void
UnavailableID::need_keys_added() const
{
}

void
UnavailableID::need_masks_added() const
{
}

const std::string
UnavailableID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) +
                ":" + stringify(_imp->slot) + "::" + stringify(_imp->repo->name()) +
                " (in ::" + stringify(_imp->owning_repository_key->value()) + ")";

        case idcf_no_version:
            return stringify(_imp->name) + ":" + stringify(_imp->slot) +
                "::" + stringify(_imp->repo->name()) +
                " (in ::" + stringify(_imp->owning_repository_key->value()) + ")";

        case idcf_version:
            return stringify(_imp->version) +
                " (in ::" + stringify(_imp->owning_repository_key->value()) + ")";

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

const QualifiedPackageName
UnavailableID::name() const
{
    return _imp->name;
}

const VersionSpec
UnavailableID::version() const
{
    return _imp->version;
}

const SlotName
UnavailableID::slot() const
{
    return _imp->slot;
}

const std::tr1::shared_ptr<const Repository>
UnavailableID::repository() const
{
    return _imp->repo->shared_from_this();
}

bool
UnavailableID::supports_action(const SupportsActionTestBase & a) const
{
    return visitor_cast<const SupportsActionTest<InstallAction> >(a);
}

void
UnavailableID::perform_action(Action & a) const
{
    throw UnsupportedActionError(*this, a);
}

std::tr1::shared_ptr<const Set<std::string> >
UnavailableID::breaks_portage() const
{
    return make_shared_ptr(new Set<std::string>);
}

bool
UnavailableID::arbitrary_less_than_comparison(const PackageID & other) const
{
    if (slot() < other.slot())
        return true;
    if (slot() > other.slot())
        return false;

    UnavailableID::MetadataConstIterator k(other.find_metadata("OWNING_REPOSITORY"));
    if (other.end_metadata() == k)
        throw InternalError(PALUDIS_HERE, "other has no OWNING_REPOSITORY");

    const MetadataValueKey<std::string> * const kk(visitor_cast<const MetadataValueKey<std::string> >(**k));
    if (! kk)
        throw InternalError(PALUDIS_HERE, "other has bad OWNING_REPOSITORY");

    return _imp->owning_repository_key->value() < kk->value();
}

std::size_t
UnavailableID::extra_hash_value() const
{
    return Hash<std::pair<SlotName, std::string> >()(std::make_pair(
                slot(), _imp->owning_repository_key->value()));
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
UnavailableID::contains_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
UnavailableID::contained_in_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
UnavailableID::fs_location_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<bool> >
UnavailableID::transient_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<bool> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
UnavailableID::virtual_for_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
UnavailableID::keywords_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<IUseFlagSet> >
UnavailableID::iuse_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<IUseFlagSet> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
UnavailableID::provide_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnavailableID::build_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnavailableID::run_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnavailableID::post_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnavailableID::suggested_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnavailableID::short_description_key() const
{
    return _imp->description_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnavailableID::long_description_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
UnavailableID::fetches_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
UnavailableID::homepage_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
UnavailableID::contents_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >();
}

const std::tr1::shared_ptr<const MetadataTimeKey>
UnavailableID::installed_time_key() const
{
    return std::tr1::shared_ptr<const MetadataTimeKey>();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnavailableID::source_origin_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnavailableID::binary_origin_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

template class PrivateImplementationPattern<UnavailableID>;

