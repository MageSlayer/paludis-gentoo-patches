/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/active_object_ptr.hh>
#include <paludis/util/deferred_construction_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/extract_host_from_url.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/syncer.hh>
#include <paludis/hook.hh>
#include <list>

using namespace paludis;
using namespace paludis::unwritten_repository;

namespace
{
    std::tr1::shared_ptr<UnwrittenRepositoryStore>
    make_store(const UnwrittenRepository * const repo, const UnwrittenRepositoryParams & p)
    {
        return make_shared_ptr(new UnwrittenRepositoryStore(p.environment(), repo, p.location()));
    }
}

namespace paludis
{
    template <>
    struct Implementation<UnwrittenRepository>
    {
        const UnwrittenRepositoryParams params;

        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<FSEntry> > location_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > sync_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > sync_options_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > sync_host_key;

        const ActiveObjectPtr<DeferredConstructionPtr<
            std::tr1::shared_ptr<UnwrittenRepositoryStore> > > store;

        Implementation(const UnwrittenRepository * const repo, const UnwrittenRepositoryParams & p) :
            params(p),
            format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                        mkt_significant, "unwritten")),
            location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                        mkt_significant, params.location())),
            sync_key(new LiteralMetadataValueKey<std::string> (
                        "sync", "sync", mkt_normal, params.sync())),
            sync_options_key(new LiteralMetadataValueKey<std::string> (
                        "sync_options", "sync_options", mkt_normal, params.sync_options())),
            sync_host_key(new LiteralMetadataValueKey<std::string> ("sync_host", "sync_host", mkt_internal, extract_host_from_url(params.sync()))),
            store(DeferredConstructionPtr<std::tr1::shared_ptr<UnwrittenRepositoryStore> > (
                        std::tr1::bind(&make_store, repo, std::tr1::cref(params))))
        {
        }
    };
}

UnwrittenRepositoryConfigurationError::UnwrittenRepositoryConfigurationError(const std::string & s) throw () :
    ConfigurationError("UnwrittenRepository configuration error: " + s)
{
}

UnwrittenRepository::UnwrittenRepository(const UnwrittenRepositoryParams & p) :
    PrivateImplementationPattern<UnwrittenRepository>(new Implementation<UnwrittenRepository>(this, p)),
    Repository(
            p.environment(),
            p.name(),
            make_named_values<RepositoryCapabilities>(
                value_for<n::destination_interface>(static_cast<RepositoryDestinationInterface *>(0)),
                value_for<n::environment_variable_interface>(static_cast<RepositoryEnvironmentVariableInterface *>(0)),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(static_cast<RepositoryManifestInterface *>(0)),
                value_for<n::provides_interface>(static_cast<RepositoryProvidesInterface *>(0)),
                value_for<n::virtuals_interface>(static_cast<RepositoryVirtualsInterface *>(0))
                )),
    _imp(PrivateImplementationPattern<UnwrittenRepository>::_imp)
{
    _add_metadata_keys();
}

UnwrittenRepository::~UnwrittenRepository()
{
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

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnwrittenRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
UnwrittenRepository::location_key() const
{
    return _imp->location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
UnwrittenRepository::installed_root_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

void
UnwrittenRepository::invalidate()
{
    _imp.reset(new Implementation<UnwrittenRepository>(this, _imp->params));
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

std::tr1::shared_ptr<const CategoryNamePartSet>
UnwrittenRepository::category_names() const
{
    return _imp->store->category_names();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
UnwrittenRepository::unimportant_category_names() const
{
    return _imp->store->unimportant_category_names();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
UnwrittenRepository::category_names_containing_package(const PackageNamePart & p) const
{
    return Repository::category_names_containing_package(p);
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
UnwrittenRepository::package_names(const CategoryNamePart & c) const
{
    return _imp->store->package_names(c);
}

std::tr1::shared_ptr<const PackageIDSequence>
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
UnwrittenRepository::sync(const std::tr1::shared_ptr<OutputManager> & output_manager) const
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
                    value_for<n::environment>(_imp->params.environment()),
                    value_for<n::local>(stringify(_imp->params.location())),
                    value_for<n::remote>(*s)
                    ));
        SyncOptions opts(make_named_values<SyncOptions>(
                    value_for<n::filter_file>(FSEntry("/dev/null")),
                    value_for<n::options>(_imp->params.sync_options()),
                    value_for<n::output_manager>(output_manager)
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

std::tr1::shared_ptr<Repository>
UnwrittenRepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
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

    return std::tr1::shared_ptr<UnwrittenRepository>(new UnwrittenRepository(
                make_named_values<UnwrittenRepositoryParams>(
                    value_for<n::environment>(env),
                    value_for<n::location>(location),
                    value_for<n::name>(RepositoryName(name_str)),
                    value_for<n::sync>(sync),
                    value_for<n::sync_options>(sync_options)
                )));
}

RepositoryName
UnwrittenRepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    if (f("name").empty())
        return RepositoryName("unwritten");
    else
        return RepositoryName(f("name"));
}

std::tr1::shared_ptr<const RepositoryNameSet>
UnwrittenRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

void
UnwrittenRepository::populate_sets() const
{
}

HookResult
UnwrittenRepository::perform_hook(const Hook &)
{
    return make_named_values<HookResult>(value_for<n::max_exit_status>(0), value_for<n::output>(""));
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnwrittenRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnwrittenRepository::sync_host_key() const
{
    return _imp->sync_host_key;
}

template class PrivateImplementationPattern<unwritten_repository::UnwrittenRepository>;

