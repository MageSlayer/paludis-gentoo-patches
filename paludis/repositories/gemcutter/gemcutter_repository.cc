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

#include <paludis/repositories/gemcutter/gemcutter_repository.hh>
#include <paludis/repositories/gemcutter/gemcutter_repository_store.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/active_object_ptr.hh>
#include <paludis/util/deferred_construction_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/simple_parser.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/syncer.hh>
#include <paludis/hook.hh>
#include <paludis/package_id.hh>
#include <paludis/output_manager.hh>
#include <paludis/environment.hh>
#include <list>

using namespace paludis;
using namespace paludis::gemcutter_repository;

namespace
{
    std::shared_ptr<GemcutterRepositoryStore>
    make_store(const GemcutterRepository * const repo, const GemcutterRepositoryParams & p)
    {
        return std::make_shared<GemcutterRepositoryStore>(p.environment(), repo, p.location());
    }
}

namespace paludis
{
    template <>
    struct Imp<GemcutterRepository>
    {
        const GemcutterRepositoryParams params;

        const std::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        const std::shared_ptr<LiteralMetadataValueKey<FSPath> > location_key;

        const ActiveObjectPtr<DeferredConstructionPtr<std::shared_ptr<GemcutterRepositoryStore> > > store;

        Imp(const GemcutterRepository * const repo, const GemcutterRepositoryParams & p) :
            params(p),
            format_key(std::make_shared<LiteralMetadataValueKey<std::string> >("format", "format", mkt_significant, "gemcutter")),
            location_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("location", "location", mkt_significant, params.location())),
            store(DeferredConstructionPtr<std::shared_ptr<GemcutterRepositoryStore> > (std::bind(&make_store, repo, std::cref(params))))
        {
        }
    };
}

GemcutterRepositoryConfigurationError::GemcutterRepositoryConfigurationError(const std::string & s) throw () :
    ConfigurationError("GemcutterRepository configuration error: " + s)
{
}

GemcutterRepository::GemcutterRepository(const GemcutterRepositoryParams & p) :
    Repository(
            p.environment(),
            p.name(),
            make_named_values<RepositoryCapabilities>(
                n::destination_interface() = static_cast<RepositoryDestinationInterface *>(0),
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(0),
                n::make_virtuals_interface() = static_cast<RepositoryMakeVirtualsInterface *>(0),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(0),
                n::provides_interface() = static_cast<RepositoryProvidesInterface *>(0),
                n::virtuals_interface() = static_cast<RepositoryVirtualsInterface *>(0)
                )),
    _imp(this, p)
{
    _add_metadata_keys();
}

GemcutterRepository::~GemcutterRepository()
{
}

bool
GemcutterRepository::can_be_favourite_repository() const
{
    return false;
}

const bool
GemcutterRepository::is_unimportant() const
{
    return false;
}

void
GemcutterRepository::_add_metadata_keys()
{
    clear_metadata_keys();
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->location_key);
}

void
GemcutterRepository::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
GemcutterRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
GemcutterRepository::location_key() const
{
    return _imp->location_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
GemcutterRepository::installed_root_key() const
{
    return make_null_shared_ptr();
}

void
GemcutterRepository::invalidate()
{
    _imp.reset(new Imp<GemcutterRepository>(this, _imp->params));
    _add_metadata_keys();
}

void
GemcutterRepository::invalidate_masks()
{
}

bool
GemcutterRepository::has_category_named(const CategoryNamePart & c) const
{
    return _imp->store->has_category_named(c);
}

bool
GemcutterRepository::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->store->has_package_named(q);
}

std::shared_ptr<const CategoryNamePartSet>
GemcutterRepository::category_names() const
{
    return _imp->store->category_names();
}

std::shared_ptr<const CategoryNamePartSet>
GemcutterRepository::unimportant_category_names() const
{
    return _imp->store->unimportant_category_names();
}

std::shared_ptr<const CategoryNamePartSet>
GemcutterRepository::category_names_containing_package(const PackageNamePart & p) const
{
    return Repository::category_names_containing_package(p);
}

std::shared_ptr<const QualifiedPackageNameSet>
GemcutterRepository::package_names(const CategoryNamePart & c) const
{
    return _imp->store->package_names(c);
}

std::shared_ptr<const PackageIDSequence>
GemcutterRepository::package_ids(const QualifiedPackageName & p) const
{
    return _imp->store->package_ids(p);
}

namespace
{
    struct SupportsActionQuery
    {
        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<ConfigAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return false;
        }
    };
}

bool
GemcutterRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

bool
GemcutterRepository::some_ids_might_not_be_masked() const
{
    return true;
}

bool
GemcutterRepository::sync(const std::string &, const std::shared_ptr<OutputManager> &) const
{
    return false;
}

std::shared_ptr<Repository>
GemcutterRepository::repository_factory_create(
        Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    Context context("When making repository repository from repo_file '" + f("repo_file") + "':");

    std::string name_str(f("name"));
    if (name_str.empty())
        name_str = "gemcutter";

    std::string location_str(f("location"));
    if (location_str.empty())
        throw GemcutterRepositoryConfigurationError("Key 'location' not specified or empty");

    return std::make_shared<GemcutterRepository>(
            make_named_values<GemcutterRepositoryParams>(
                n::environment() = env,
                n::location() = FSPath(location_str),
                n::name() = RepositoryName(name_str)
                ));
}

RepositoryName
GemcutterRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> & f)
{
    if (f("name").empty())
        return RepositoryName("gemcutter");
    else
        return RepositoryName(f("name"));
}

std::shared_ptr<const RepositoryNameSet>
GemcutterRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

void
GemcutterRepository::populate_sets() const
{
}

HookResult
GemcutterRepository::perform_hook(const Hook &, const std::shared_ptr<OutputManager> &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

const std::shared_ptr<const MetadataValueKey<std::string> >
GemcutterRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > >
GemcutterRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

template class Pimp<gemcutter_repository::GemcutterRepository>;

