/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/repositories/unpackaged/unpackaged_id.hh>
#include <paludis/repositories/unpackaged/unpackaged_key.hh>
#include <paludis/repositories/unpackaged/dep_parser.hh>
#include <paludis/repositories/unpackaged/dep_printer.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/hashed_containers.hh>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace paludis
{
    template <>
    struct Implementation<UnpackagedID>
    {
        const Environment * const env;
        const QualifiedPackageName name;
        const VersionSpec version;
        const SlotName slot;
        const RepositoryName repository_name;

        const tr1::shared_ptr<UnpackagedFSEntryKey> fs_location_key;
        const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key;
        const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key;
        const tr1::shared_ptr<const MetadataStringKey> description_key;

        Implementation(const Environment * const e,
                const QualifiedPackageName & q,
                const VersionSpec & v,
                const SlotName & s,
                const RepositoryName & n,
                const FSEntry & l,
                const std::string & b,
                const std::string & r,
                const std::string & d) :
            env(e),
            name(q),
            version(v),
            slot(s),
            repository_name(n),
            fs_location_key(new UnpackagedFSEntryKey("location", "Location", mkt_normal, l)),
            build_dependencies_key(new UnpackagedDependencyKey(env, "build_dependencies", "Build dependencies", mkt_dependencies, b)),
            run_dependencies_key(new UnpackagedDependencyKey(env, "run_dependencies", "Run dependencies", mkt_dependencies, r)),
            description_key(new UnpackagedStringKey("description", "Description", mkt_significant, d))
        {
        }
    };
}

UnpackagedID::UnpackagedID(const Environment * const e, const QualifiedPackageName & q,
        const VersionSpec & v, const SlotName & s, const RepositoryName & n, const FSEntry & l,
        const std::string & b, const std::string & r, const std::string & d) :
    PrivateImplementationPattern<UnpackagedID>(new Implementation<UnpackagedID>(e, q, v, s, n, l, b, r, d)),
    _imp(PrivateImplementationPattern<UnpackagedID>::_imp.get())
{
    add_metadata_key(_imp->fs_location_key);
    add_metadata_key(_imp->build_dependencies_key);
    add_metadata_key(_imp->run_dependencies_key);
    add_metadata_key(_imp->description_key);
}

UnpackagedID::~UnpackagedID()
{
}

void
UnpackagedID::need_keys_added() const
{
}

void
UnpackagedID::need_masks_added() const
{
}

const std::string
UnpackagedID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) + ":" +
                stringify(slot()) + "::" + stringify(_imp->repository_name);

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_version:
            return stringify(_imp->name) + ":" + stringify(slot()) + "::" +
                stringify(_imp->repository_name);

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

const QualifiedPackageName
UnpackagedID::name() const
{
    return _imp->name;
}

const VersionSpec
UnpackagedID::version() const
{
    return _imp->version;
}

const SlotName
UnpackagedID::slot() const
{
    return _imp->slot;
}

const tr1::shared_ptr<const Repository>
UnpackagedID::repository() const
{
    return _imp->env->package_database()->fetch_repository(_imp->repository_name);
}

const tr1::shared_ptr<const MetadataPackageIDKey>
UnpackagedID::virtual_for_key() const
{
    return tr1::shared_ptr<const MetadataPackageIDKey>();
}

const tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >
UnpackagedID::keywords_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >();
}

const tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> >
UnpackagedID::iuse_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
UnpackagedID::provide_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >
UnpackagedID::contains_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >();
}

const tr1::shared_ptr<const MetadataPackageIDKey>
UnpackagedID::contained_in_key() const
{
    return tr1::shared_ptr<const MetadataPackageIDKey>();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::build_dependencies_key() const
{
    return _imp->build_dependencies_key;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::run_dependencies_key() const
{
    return _imp->run_dependencies_key;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::post_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::suggested_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
UnpackagedID::fetches_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
UnpackagedID::homepage_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const tr1::shared_ptr<const MetadataStringKey>
UnpackagedID::short_description_key() const
{
    return _imp->description_key;
}

const tr1::shared_ptr<const MetadataStringKey>
UnpackagedID::long_description_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataContentsKey>
UnpackagedID::contents_key() const
{
    return tr1::shared_ptr<const MetadataContentsKey>();
}

const tr1::shared_ptr<const MetadataTimeKey>
UnpackagedID::installed_time_key() const
{
    return tr1::shared_ptr<const MetadataTimeKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
UnpackagedID::source_origin_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
UnpackagedID::binary_origin_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataFSEntryKey>
UnpackagedID::fs_location_key() const
{
    return _imp->fs_location_key;
}

bool
UnpackagedID::supports_action(const SupportsActionTestBase & test) const
{
    return visitor_cast<const SupportsActionTest<InstallAction> >(test);
}

void
UnpackagedID::perform_action(Action & action) const
{
    const InstallAction * const install_action(visitor_cast<const InstallAction>(action));
    if (! install_action)
        throw UnsupportedActionError(*this, action);

    if (! install_action->options.destination->destination_interface)
        throw InstallActionError("Can't install '" + stringify(*this)
                + "' to destination '" + stringify(install_action->options.destination->name())
                + "' because destination does not provide destination_interface");

    install_action->options.destination->destination_interface->merge(
            MergeOptions::create()
            .package_id(shared_from_this())
            .image_dir(fs_location_key()->value())
            .environment_file(FSEntry("/dev/null"))
            );
}

void
UnpackagedID::invalidate_masks() const
{
}

bool
UnpackagedID::breaks_portage() const
{
    return true;
}

bool
UnpackagedID::arbitrary_less_than_comparison(const PackageID & other) const
{
    return slot().data() < other.slot().data();
}

std::size_t
UnpackagedID::extra_hash_value() const
{
    return CRCHash<SlotName>()(slot());
}

