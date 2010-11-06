/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/unwritten/unwritten_repository.hh>
#include <paludis/repositories/unwritten/unwritten_repository_store.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/active_object_ptr.hh>
#include <paludis/util/deferred_construction_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/extract_host_from_url.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/syncer.hh>
#include <paludis/hook.hh>
#include <list>

using namespace paludis;
using namespace paludis::unwritten_repository;

namespace
{
    std::shared_ptr<UnwrittenRepositoryStore>
    make_store(const UnwrittenRepository * const repo, const UnwrittenRepositoryParams & p)
    {
        return std::make_shared<UnwrittenRepositoryStore>(p.environment(), repo, p.location());
    }
}

namespace paludis
{
    template <>
    struct Imp<UnwrittenRepository>
    {
        const UnwrittenRepositoryParams params;

        const std::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        const std::shared_ptr<LiteralMetadataValueKey<FSPath> > location_key;
        const std::shared_ptr<LiteralMetadataValueKey<std::string> > sync_key;
        const std::shared_ptr<LiteralMetadataValueKey<std::string> > sync_options_key;
        const std::shared_ptr<LiteralMetadataValueKey<std::string> > sync_host_key;

        const ActiveObjectPtr<DeferredConstructionPtr<
            std::shared_ptr<UnwrittenRepositoryStore> > > store;

        Imp(const UnwrittenRepository * const repo, const UnwrittenRepositoryParams & p) :
            params(p),
            format_key(std::make_shared<LiteralMetadataValueKey<std::string> >("format", "format",
                        mkt_significant, "unwritten")),
            location_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("location", "location",
                        mkt_significant, params.location())),
            sync_key(std::make_shared<LiteralMetadataValueKey<std::string> >(
                        "sync", "sync", mkt_normal, params.sync())),
            sync_options_key(std::make_shared<LiteralMetadataValueKey<std::string> >(
                        "sync_options", "sync_options", mkt_normal, params.sync_options())),
            sync_host_key(std::make_shared<LiteralMetadataValueKey<std::string> >("sync_host", "sync_host", mkt_internal, extract_host_from_url(params.sync()))),
            store(DeferredConstructionPtr<std::shared_ptr<UnwrittenRepositoryStore> > (
                        std::bind(&make_store, repo, std::cref(params))))
        {
        }
    };
}

UnwrittenRepositoryConfigurationError::UnwrittenRepositoryConfigurationError(const std::string & s) throw () :
    ConfigurationError("UnwrittenRepository configuration error: " + s)
{
}

UnwrittenRepository::UnwrittenRepository(const UnwrittenRepositoryParams & p) :
    Pimp<UnwrittenRepository>(this, p),
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
    _imp(Pimp<UnwrittenRepository>::_imp)
{
    _add_metadata_keys();
}

UnwrittenRepository::~UnwrittenRepository()
{
}

const bool
UnwrittenRepository::is_unimportant() const
{
    return true;
}

bool
UnwrittenRepository::can_be_favourite_repository() const
{
    return false;
}

void
UnwrittenRepository::_add_metadata_keys()
{
    clear_metadata_keys();
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->sync_key);
    add_metadata_key(_imp->sync_options_key);
    add_metadata_key(_imp->sync_host_key);
}

void
UnwrittenRepository::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
UnwrittenRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
UnwrittenRepository::location_key() const
{
    return _imp->location_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
UnwrittenRepository::installed_root_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSPath> >();
}

void
UnwrittenRepository::invalidate()
{
    _imp.reset(new Imp<UnwrittenRepository>(this, _imp->params));
    _add_metadata_keys();
}

void
UnwrittenRepository::invalidate_masks()
{
}

bool
UnwrittenRepository::has_category_named(const CategoryNamePart & c) const
{
    return _imp->store->has_category_named(c);
}

bool
UnwrittenRepository::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->store->has_package_named(q);
}

std::shared_ptr<const CategoryNamePartSet>
UnwrittenRepository::category_names() const
{
    return _imp->store->category_names();
}

std::shared_ptr<const CategoryNamePartSet>
UnwrittenRepository::unimportant_category_names() const
{
    return _imp->store->unimportant_category_names();
}

std::shared_ptr<const CategoryNamePartSet>
UnwrittenRepository::category_names_containing_package(const PackageNamePart & p) const
{
    return Repository::category_names_containing_package(p);
}

std::shared_ptr<const QualifiedPackageNameSet>
UnwrittenRepository::package_names(const CategoryNamePart & c) const
{
    return _imp->store->package_names(c);
}

std::shared_ptr<const PackageIDSequence>
UnwrittenRepository::package_ids(const QualifiedPackageName & p) const
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
            return false;
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
UnwrittenRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

bool
UnwrittenRepository::some_ids_might_not_be_masked() const
{
    return false;
}

bool
UnwrittenRepository::sync(const std::shared_ptr<OutputManager> & output_manager) const
{
    Context context("When syncing repository '" + stringify(name()) + "':");

    if (_imp->params.sync().empty())
        return false;

    std::list<std::string> sync_list;
    tokenise_whitespace(_imp->params.sync(), std::back_inserter(sync_list));

    bool ok(false);
    for (std::list<std::string>::const_iterator s(sync_list.begin()),
            s_end(sync_list.end()) ; s != s_end ; ++s)
    {
        DefaultSyncer syncer(make_named_values<SyncerParams>(
                    n::environment() = _imp->params.environment(),
                    n::local() = stringify(_imp->params.location()),
                    n::remote() = *s
                    ));
        SyncOptions opts(make_named_values<SyncOptions>(
                    n::filter_file() = FSPath("/dev/null"),
                    n::options() = _imp->params.sync_options(),
                    n::output_manager() = output_manager
                    ));
        try
        {
            syncer.sync(opts);
        }
        catch (const SyncFailedError &)
        {
            continue;
        }

        ok = true;
        break;
    }

    if (! ok)
        throw SyncFailedError(stringify(_imp->params.location()), _imp->params.sync());

    return true;
}

std::shared_ptr<Repository>
UnwrittenRepository::repository_factory_create(
        Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    Context context("When making unwritten repository from repo_file '" + f("repo_file") + "':");

    std::string name_str(f("name"));
    if (name_str.empty())
        name_str = "unwritten";

    std::string location(f("location"));
    if (location.empty())
        throw UnwrittenRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string sync(f("sync"));

    std::string sync_options(f("sync_options"));

    return std::make_shared<UnwrittenRepository>(
            make_named_values<UnwrittenRepositoryParams>(
                n::environment() = env,
                n::location() = location,
                n::name() = RepositoryName(name_str),
                n::sync() = sync,
                n::sync_options() = sync_options
                ));
}

RepositoryName
UnwrittenRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> & f)
{
    if (f("name").empty())
    {
        UnwrittenRepositoryInformation info(UnwrittenRepositoryStore::repository_information(FSPath(f("location"))));
        return RepositoryName(info.name());
    }
    else
        return RepositoryName(f("name"));
}

std::shared_ptr<const RepositoryNameSet>
UnwrittenRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

void
UnwrittenRepository::populate_sets() const
{
}

HookResult
UnwrittenRepository::perform_hook(const Hook &, const std::shared_ptr<OutputManager> &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

const std::shared_ptr<const MetadataValueKey<std::string> >
UnwrittenRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
UnwrittenRepository::sync_host_key() const
{
    return _imp->sync_host_key;
}

template class Pimp<unwritten_repository::UnwrittenRepository>;

