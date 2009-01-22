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

#include <paludis/repositories/accounts/installed_accounts_id.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/options.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/repository.hh>
#include <paludis/action.hh>

using namespace paludis;
using namespace paludis::accounts_repository;

namespace paludis
{
    template <>
    struct Implementation<InstalledAccountsID>
    {
        const Environment * const env;

        const QualifiedPackageName name;
        const VersionSpec version;
        const SlotName slot;
        const std::tr1::shared_ptr<const Repository> repository;

        const std::tr1::shared_ptr<const LiteralMetadataValueKey<bool> > transient_key;

        const bool is_user;

        Implementation(const Environment * const e,
                const QualifiedPackageName & q, const std::tr1::shared_ptr<const Repository> & r,
                const bool u) :
            env(e),
            name(q),
            version("0"),
            slot("0"),
            repository(r),
            transient_key(new LiteralMetadataValueKey<bool>("transient", "Transient", mkt_internal, true)),
            is_user(u)
        {
        }
    };
}

InstalledAccountsID::InstalledAccountsID(const Environment * const e,
        const QualifiedPackageName & q, const std::tr1::shared_ptr<const Repository> & r, const bool u) :
    PrivateImplementationPattern<InstalledAccountsID>(new Implementation<InstalledAccountsID>(e, q, r, u)),
    _imp(PrivateImplementationPattern<InstalledAccountsID>::_imp)
{
    add_metadata_key(_imp->transient_key);
}

InstalledAccountsID::~InstalledAccountsID()
{
}

void
InstalledAccountsID::need_keys_added() const
{
}

void
InstalledAccountsID::clear_metadata_keys() const
{
}

void
InstalledAccountsID::need_masks_added() const
{
}

const QualifiedPackageName
InstalledAccountsID::name() const
{
    return _imp->name;
}

const VersionSpec
InstalledAccountsID::version() const
{
    return _imp->version;
}

const SlotName
InstalledAccountsID::slot() const
{
    return _imp->slot;
}

const std::tr1::shared_ptr<const Repository>
InstalledAccountsID::repository() const
{
    return _imp->repository;
}

const std::string
InstalledAccountsID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(name()) + "-" + stringify(version()) + ":" + stringify(slot()) + "::" + stringify(repository()->name());

        case idcf_no_version:
            return stringify(name()) + ":" + stringify(slot()) + "::" + stringify(repository()->name());

        case idcf_version:
            return stringify(version());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
InstalledAccountsID::virtual_for_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
InstalledAccountsID::keywords_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
InstalledAccountsID::provide_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
InstalledAccountsID::contains_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
InstalledAccountsID::contained_in_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledAccountsID::build_dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledAccountsID::run_dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledAccountsID::post_dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledAccountsID::suggested_dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
InstalledAccountsID::fetches_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
InstalledAccountsID::homepage_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledAccountsID::short_description_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledAccountsID::long_description_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
InstalledAccountsID::contents_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataTimeKey>
InstalledAccountsID::installed_time_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
InstalledAccountsID::from_repositories_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledAccountsID::fs_location_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<bool> >
InstalledAccountsID::transient_key() const
{
    return _imp->transient_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
InstalledAccountsID::choices_key() const
{
    return make_null_shared_ptr();
}

std::tr1::shared_ptr<const Set<std::string> >
InstalledAccountsID::breaks_portage() const
{
    return make_shared_ptr(new Set<std::string>);
}

bool
InstalledAccountsID::arbitrary_less_than_comparison(const PackageID & other) const
{
    if (slot() < other.slot())
        return true;

    return false;
}

std::size_t
InstalledAccountsID::extra_hash_value() const
{
    return Hash<SlotName>()(slot());
}

bool
InstalledAccountsID::supports_action(const SupportsActionTestBase & test) const
{
    return simple_visitor_cast<const SupportsActionTest<InstalledAction> >(test);
}

void
InstalledAccountsID::perform_action(Action & action) const
{
    throw UnsupportedActionError(*this, action);
}

