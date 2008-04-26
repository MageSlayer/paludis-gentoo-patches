/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

namespace paludis
{
    template <>
    struct Implementation<UnpackagedRepository>
    {
        const UnpackagedRepositoryParams params;
        std::tr1::shared_ptr<const PackageID> id;
        std::tr1::shared_ptr<PackageIDSequence> ids;
        std::tr1::shared_ptr<QualifiedPackageNameSet> package_names;
        std::tr1::shared_ptr<CategoryNamePartSet> category_names;

        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > install_under_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > name_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > slot_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > build_dependencies_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > run_dependencies_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > description_key;

        Implementation(const RepositoryName & n,
                const UnpackagedRepositoryParams & p) :
            params(p),
            id(new UnpackagedID(params[k::environment()], params[k::name()], params[k::version()], params[k::slot()], n, params[k::location()],
                        params[k::build_dependencies()], params[k::run_dependencies()], params[k::description()])),
            ids(new PackageIDSequence),
            package_names(new QualifiedPackageNameSet),
            category_names(new CategoryNamePartSet),
            location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                        mkt_significant, params[k::location()])),
            install_under_key(new LiteralMetadataValueKey<FSEntry> ("install_under", "install_under",
                        mkt_significant, params[k::install_under()])),
            name_key(new LiteralMetadataValueKey<std::string> ("name", "name",
                        mkt_normal, stringify(params[k::name()]))),
            slot_key(new LiteralMetadataValueKey<std::string> ("slot", "slot",
                        mkt_normal, stringify(params[k::slot()]))),
            format_key(new LiteralMetadataValueKey<std::string> (
                        "format", "format", mkt_significant, "unpackaged")),
            build_dependencies_key(new LiteralMetadataValueKey<std::string> (
                        "build_dependencies", "build_dependencies", mkt_normal, params[k::build_dependencies()])),
            run_dependencies_key(new LiteralMetadataValueKey<std::string> (
                        "run_dependencies", "run_dependencies", mkt_normal, params[k::run_dependencies()])),
            description_key(new LiteralMetadataValueKey<std::string> (
                        "description", "description", mkt_normal, params[k::description()]))
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
    Repository(n, RepositoryCapabilities::named_create()
            (k::sets_interface(), static_cast<RepositorySetsInterface *>(0))
            (k::syncable_interface(), static_cast<RepositorySyncableInterface *>(0))
            (k::use_interface(), static_cast<RepositoryUseInterface *>(0))
            (k::mirrors_interface(), static_cast<RepositoryMirrorsInterface *>(0))
            (k::environment_variable_interface(), static_cast<RepositoryEnvironmentVariableInterface *>(0))
            (k::provides_interface(), static_cast<RepositoryProvidesInterface *>(0))
            (k::virtuals_interface(), static_cast<RepositoryVirtualsInterface *>(0))
            (k::make_virtuals_interface(), static_cast<RepositoryMakeVirtualsInterface *>(0))
            (k::destination_interface(), static_cast<RepositoryDestinationInterface *>(0))
            (k::e_interface(), static_cast<RepositoryEInterface *>(0))
            (k::hook_interface(), static_cast<RepositoryHookInterface *>(0))
            (k::qa_interface(), static_cast<RepositoryQAInterface *>(0))
            (k::manifest_interface(), static_cast<RepositoryManifestInterface *>(0))),
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
    add_metadata_key(_imp->install_under_key);
    add_metadata_key(_imp->name_key);
    add_metadata_key(_imp->slot_key);
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->build_dependencies_key);
    add_metadata_key(_imp->run_dependencies_key);
    add_metadata_key(_imp->description_key);
}

std::tr1::shared_ptr<const PackageIDSequence>
UnpackagedRepository::package_ids(const QualifiedPackageName & n) const
{
    return n == _imp->id->name() ? _imp->ids : make_shared_ptr(new PackageIDSequence);
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
UnpackagedRepository::package_names(const CategoryNamePart & c) const
{
    return c == _imp->id->name().category ? _imp->package_names : make_shared_ptr(new QualifiedPackageNameSet);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
UnpackagedRepository::category_names() const
{
    return _imp->category_names;
}

std::tr1::shared_ptr<const CategoryNamePartSet>
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

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnpackagedRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
UnpackagedRepository::installed_root_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

