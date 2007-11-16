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

#include <paludis/repositories/unpackaged/unpackaged_repository.hh>
#include <paludis/repositories/unpackaged/unpackaged_id.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

#include <paludis/repositories/unpackaged/unpackaged_repository-sr.cc>

namespace paludis
{
    template <>
    struct Implementation<UnpackagedRepository>
    {
        const UnpackagedRepositoryParams params;
        tr1::shared_ptr<const PackageID> id;
        tr1::shared_ptr<PackageIDSequence> ids;
        tr1::shared_ptr<QualifiedPackageNameSet> package_names;
        tr1::shared_ptr<CategoryNamePartSet> category_names;

        tr1::shared_ptr<const MetadataFSEntryKey> location_key;
        tr1::shared_ptr<const MetadataStringKey> name_key;
        tr1::shared_ptr<const MetadataStringKey> slot_key;
        tr1::shared_ptr<const MetadataStringKey> format_key;
        tr1::shared_ptr<const MetadataStringKey> build_dependencies_key;
        tr1::shared_ptr<const MetadataStringKey> run_dependencies_key;
        tr1::shared_ptr<const MetadataStringKey> description_key;

        Implementation(const RepositoryName & n,
                const UnpackagedRepositoryParams & p) :
            params(p),
            id(new UnpackagedID(params.environment, params.name, params.version, params.slot, n, params.location,
                        params.build_dependencies, params.run_dependencies, params.description)),
            ids(new PackageIDSequence),
            package_names(new QualifiedPackageNameSet),
            category_names(new CategoryNamePartSet),
            location_key(new LiteralMetadataFSEntryKey("location", "location",
                        mkt_significant, params.location)),
            name_key(new LiteralMetadataStringKey("name", "name",
                        mkt_normal, stringify(params.name))),
            slot_key(new LiteralMetadataStringKey("slot", "slot",
                        mkt_normal, stringify(params.slot))),
            format_key(new LiteralMetadataStringKey(
                        "format", "format", mkt_significant, "unpackaged")),
            build_dependencies_key(new LiteralMetadataStringKey(
                        "build_dependencies", "build_dependencies", mkt_normal, params.build_dependencies)),
            run_dependencies_key(new LiteralMetadataStringKey(
                        "run_dependencies", "run_dependencies", mkt_normal, params.run_dependencies)),
            description_key(new LiteralMetadataStringKey(
                        "description", "description", mkt_normal, params.description))
        {
            ids->push_back(id);
            package_names->insert(id->name());
            category_names->insert(id->name().category);
        }
    };
}

UnpackagedRepository::UnpackagedRepository(const RepositoryName & n,
        const UnpackagedRepositoryParams & params) :
    PrivateImplementationPattern<UnpackagedRepository>(new Implementation<UnpackagedRepository>(n, params)),
    Repository(n, RepositoryCapabilities::create()
            .sets_interface(0)
            .syncable_interface(0)
            .use_interface(0)
            .world_interface(0)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .provides_interface(0)
            .virtuals_interface(0)
            .make_virtuals_interface(0)
            .destination_interface(0)
            .e_interface(0)
            .hook_interface(0)
            .qa_interface(0)
            .manifest_interface(0)),
    _imp(PrivateImplementationPattern<UnpackagedRepository>::_imp)
{
    _add_metadata_keys();
}

UnpackagedRepository::~UnpackagedRepository()
{
}

void
UnpackagedRepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->name_key);
    add_metadata_key(_imp->slot_key);
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->build_dependencies_key);
    add_metadata_key(_imp->run_dependencies_key);
    add_metadata_key(_imp->description_key);
}

tr1::shared_ptr<const PackageIDSequence>
UnpackagedRepository::package_ids(const QualifiedPackageName & n) const
{
    return n == _imp->id->name() ? _imp->ids : make_shared_ptr(new PackageIDSequence);
}

tr1::shared_ptr<const QualifiedPackageNameSet>
UnpackagedRepository::package_names(const CategoryNamePart & c) const
{
    return c == _imp->id->name().category ? _imp->package_names : make_shared_ptr(new QualifiedPackageNameSet);
}

tr1::shared_ptr<const CategoryNamePartSet>
UnpackagedRepository::category_names() const
{
    return _imp->category_names;
}

tr1::shared_ptr<const CategoryNamePartSet>
UnpackagedRepository::category_names_containing_package(const PackageNamePart & p) const
{
    return p == _imp->id->name().package ? _imp->category_names : make_shared_ptr(new CategoryNamePartSet);
}

bool
UnpackagedRepository::has_package_named(const QualifiedPackageName & q) const
{
    return q == _imp->id->name();
}

bool
UnpackagedRepository::has_category_named(const CategoryNamePart & c) const
{
    return c == _imp->id->name().category;
}

bool
UnpackagedRepository::some_ids_might_support_action(const SupportsActionTestBase & test) const
{
    return _imp->id->supports_action(test);
}

void
UnpackagedRepository::invalidate()
{
    _imp.reset(new Implementation<UnpackagedRepository>(name(), _imp->params));
    _add_metadata_keys();
}

void
UnpackagedRepository::invalidate_masks()
{
}

void
UnpackagedRepository::need_keys_added() const
{
}

const tr1::shared_ptr<const MetadataStringKey>
UnpackagedRepository::format_key() const
{
    return _imp->format_key;
}

const tr1::shared_ptr<const MetadataFSEntryKey>
UnpackagedRepository::installed_root_key() const
{
    return tr1::shared_ptr<const MetadataFSEntryKey>();
}

