/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/repositories/unpackaged/exceptions.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/destringify.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/hook.hh>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace paludis
{
    template <>
    struct Imp<UnpackagedRepository>
    {
        const UnpackagedRepositoryParams params;
        std::shared_ptr<const PackageID> id;
        std::shared_ptr<PackageIDSequence> ids;
        std::shared_ptr<QualifiedPackageNameSet> package_names;
        std::shared_ptr<CategoryNamePartSet> category_names;

        std::shared_ptr<const MetadataValueKey<FSPath> > location_key;
        std::shared_ptr<const MetadataValueKey<FSPath> > install_under_key;
        std::shared_ptr<const MetadataValueKey<long> > rewrite_ids_over_to_root_key;
        std::shared_ptr<const MetadataValueKey<std::string> > name_key;
        std::shared_ptr<const MetadataValueKey<std::string> > slot_key;
        std::shared_ptr<const MetadataValueKey<std::string> > format_key;
        std::shared_ptr<const MetadataValueKey<std::string> > build_dependencies_key;
        std::shared_ptr<const MetadataValueKey<std::string> > run_dependencies_key;
        std::shared_ptr<const MetadataValueKey<std::string> > description_key;

        Imp(const RepositoryName & n,
                const UnpackagedRepositoryParams & p) :
            params(p),
            id(std::make_shared<UnpackagedID>(params.environment(), params.name(), params.version(), params.slot(), n, params.location(),
                        params.build_dependencies(), params.run_dependencies(), params.description(), params.strip(), params.preserve_work())),
            ids(std::make_shared<PackageIDSequence>()),
            package_names(std::make_shared<QualifiedPackageNameSet>()),
            category_names(std::make_shared<CategoryNamePartSet>()),
            location_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("location", "location",
                        mkt_significant, params.location())),
            install_under_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("install_under", "install_under",
                        mkt_significant, params.install_under())),
            rewrite_ids_over_to_root_key(std::make_shared<LiteralMetadataValueKey<long> >("rewrite_ids_over_to_root", "rewrite_ids_over_to_root",
                        mkt_normal, params.rewrite_ids_over_to_root())),
            name_key(std::make_shared<LiteralMetadataValueKey<std::string> >("name", "name",
                        mkt_normal, stringify(params.name()))),
            slot_key(std::make_shared<LiteralMetadataValueKey<std::string> >("slot", "slot",
                        mkt_normal, stringify(params.slot()))),
            format_key(std::make_shared<LiteralMetadataValueKey<std::string> >(
                        "format", "format", mkt_significant, "unpackaged")),
            build_dependencies_key(std::make_shared<LiteralMetadataValueKey<std::string> >(
                        "build_dependencies", "build_dependencies", mkt_normal, params.build_dependencies())),
            run_dependencies_key(std::make_shared<LiteralMetadataValueKey<std::string> >(
                        "run_dependencies", "run_dependencies", mkt_normal, params.run_dependencies())),
            description_key(std::make_shared<LiteralMetadataValueKey<std::string> >(
                        "description", "description", mkt_normal, params.description()))
        {
            ids->push_back(id);
            package_names->insert(id->name());
            category_names->insert(id->name().category());
        }
    };
}

UnpackagedRepository::UnpackagedRepository(const RepositoryName & n,
        const UnpackagedRepositoryParams & params) :
    Repository(params.environment(), n, make_named_values<RepositoryCapabilities>(
                n::destination_interface() = static_cast<RepositoryDestinationInterface *>(nullptr),
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(nullptr),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(nullptr)
            )),
    _imp(n, params)
{
    _add_metadata_keys();
}

UnpackagedRepository::~UnpackagedRepository() = default;

void
UnpackagedRepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->install_under_key);
    add_metadata_key(_imp->rewrite_ids_over_to_root_key);
    add_metadata_key(_imp->name_key);
    add_metadata_key(_imp->slot_key);
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->build_dependencies_key);
    add_metadata_key(_imp->run_dependencies_key);
    add_metadata_key(_imp->description_key);
}

std::shared_ptr<const PackageIDSequence>
UnpackagedRepository::package_ids(const QualifiedPackageName & n, const RepositoryContentMayExcludes &) const
{
    return n == _imp->id->name() ? _imp->ids : std::make_shared<PackageIDSequence>();
}

std::shared_ptr<const QualifiedPackageNameSet>
UnpackagedRepository::package_names(const CategoryNamePart & c, const RepositoryContentMayExcludes &) const
{
    return c == _imp->id->name().category() ? _imp->package_names : std::make_shared<QualifiedPackageNameSet>();
}

std::shared_ptr<const CategoryNamePartSet>
UnpackagedRepository::category_names(const RepositoryContentMayExcludes &) const
{
    return _imp->category_names;
}

std::shared_ptr<const CategoryNamePartSet>
UnpackagedRepository::category_names_containing_package(const PackageNamePart & p, const RepositoryContentMayExcludes &) const
{
    return p == _imp->id->name().package() ? _imp->category_names : std::make_shared<CategoryNamePartSet>();
}

bool
UnpackagedRepository::has_package_named(const QualifiedPackageName & q, const RepositoryContentMayExcludes &) const
{
    return q == _imp->id->name();
}

bool
UnpackagedRepository::has_category_named(const CategoryNamePart & c, const RepositoryContentMayExcludes &) const
{
    return c == _imp->id->name().category();
}

bool
UnpackagedRepository::some_ids_might_support_action(const SupportsActionTestBase & test) const
{
    return _imp->id->supports_action(test);
}

bool
UnpackagedRepository::some_ids_might_not_be_masked() const
{
    return true;
}

const bool
UnpackagedRepository::is_unimportant() const
{
    return false;
}

void
UnpackagedRepository::invalidate()
{
    _imp.reset(new Imp<UnpackagedRepository>(name(), _imp->params));
    _add_metadata_keys();
}

void
UnpackagedRepository::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<std::string>>
UnpackagedRepository::cross_compile_host_key() const
{
    return nullptr;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
UnpackagedRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
UnpackagedRepository::location_key() const
{
    return _imp->location_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
UnpackagedRepository::installed_root_key() const
{
    return nullptr;
}

std::shared_ptr<Repository>
UnpackagedRepository::repository_factory_create(
        Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    Context context("When creating UnpackagedRepository:");

    std::string location(f("location"));
    if (location.empty())
        throw unpackaged_repositories::RepositoryConfigurationError("Key 'location' not specified or empty");

    std::string install_under(f("install_under"));
    if (install_under.empty())
        install_under = "/";

    std::string name(f("name"));
    if (name.empty())
        throw unpackaged_repositories::RepositoryConfigurationError("Key 'name' not specified or empty");

    std::string version(f("version"));
    if (version.empty())
        throw unpackaged_repositories::RepositoryConfigurationError("Key 'version' not specified or empty");

    std::string slot(f("slot"));
    if (slot.empty())
        throw unpackaged_repositories::RepositoryConfigurationError("Key 'slot' not specified or empty");

    std::string build_dependencies(f("build_dependencies"));
    std::string run_dependencies(f("run_dependencies"));
    std::string description(f("description"));

    int rewrite_ids_over_to_root(-1);
    if (! f("rewrite_ids_over_to_root").empty())
    {
        Context item_context("When handling rewrite_ids_over_to_root key:");
        rewrite_ids_over_to_root = destringify<int>(f("rewrite_ids_over_to_root"));
    }

    Tribool strip(indeterminate);
    if (! f("strip").empty())
        strip = destringify<bool>(f("strip"));

    Tribool preserve_work(indeterminate);
    if (! f("preserve_work").empty())
        preserve_work = destringify<bool>(f("preserve_work"));

    return std::make_shared<UnpackagedRepository>(RepositoryName("unpackaged"),
                make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                    n::build_dependencies() = build_dependencies,
                    n::description() = description,
                    n::environment() = env,
                    n::install_under() = install_under,
                    n::location() = location,
                    n::name() = QualifiedPackageName(name),
                    n::preserve_work() = preserve_work,
                    n::rewrite_ids_over_to_root() = rewrite_ids_over_to_root,
                    n::run_dependencies() = run_dependencies,
                    n::slot() = SlotName(slot),
                    n::strip() = strip,
                    n::version() = VersionSpec(version, user_version_spec_options())
                ));
}

RepositoryName
UnpackagedRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> & f)
{
    return RepositoryName(f("name"));
}

std::shared_ptr<const RepositoryNameSet>
UnpackagedRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

void
UnpackagedRepository::populate_sets() const
{
}

HookResult
UnpackagedRepository::perform_hook(const Hook &, const std::shared_ptr<OutputManager> &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

bool
UnpackagedRepository::sync(
        const std::string &,
        const std::string &,
        const std::shared_ptr<OutputManager> &) const
{
    return false;
}

const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > >
UnpackagedRepository::sync_host_key() const
{
    return nullptr;
}

const std::shared_ptr<const MetadataValueKey<std::string>>
UnpackagedRepository::tool_prefix_key() const
{
    return nullptr;
}

const std::shared_ptr<const Set<std::string> >
UnpackagedRepository::maybe_expand_licence_nonrecursively(const std::string &) const
{
    return nullptr;
}

