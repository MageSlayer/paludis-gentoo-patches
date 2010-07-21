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

#include <paludis/repositories/unavailable/unavailable_repository.hh>
#include <paludis/repositories/unavailable/unavailable_repository_store.hh>
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
using namespace paludis::unavailable_repository;

namespace
{
    std::tr1::shared_ptr<UnavailableRepositoryStore>
    make_store(const UnavailableRepository * const repo, const UnavailableRepositoryParams & p)
    {
        return make_shared_ptr(new UnavailableRepositoryStore(p.environment(), repo, p.location()));
    }
}

namespace paludis
{
    template <>
    struct Implementation<UnavailableRepository>
    {
        const UnavailableRepositoryParams params;

        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<FSEntry> > location_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > sync_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > sync_options_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > sync_host_key;

        const ActiveObjectPtr<DeferredConstructionPtr<
            std::tr1::shared_ptr<UnavailableRepositoryStore> > > store;

        Implementation(const UnavailableRepository * const repo, const UnavailableRepositoryParams & p) :
            params(p),
            format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                        mkt_significant, "unavailable")),
            location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                        mkt_significant, params.location())),
            sync_key(new LiteralMetadataValueKey<std::string> (
                        "sync", "sync", mkt_normal, params.sync())),
            sync_options_key(new LiteralMetadataValueKey<std::string> (
                        "sync_options", "sync_options", mkt_normal, params.sync_options())),
            sync_host_key(new LiteralMetadataValueKey<std::string> ("sync_host", "sync_host", mkt_internal, extract_host_from_url(params.sync()))),
            store(DeferredConstructionPtr<std::tr1::shared_ptr<UnavailableRepositoryStore> > (
                        std::tr1::bind(&make_store, repo, std::tr1::cref(params))))
        {
        }
    };
}

UnavailableRepositoryConfigurationError::UnavailableRepositoryConfigurationError(const std::string & s) throw () :
    ConfigurationError("UnavailableRepository configuration error: " + s)
{
}

UnavailableRepository::UnavailableRepository(const UnavailableRepositoryParams & p) :
    PrivateImplementationPattern<UnavailableRepository>(new Implementation<UnavailableRepository>(this, p)),
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
    _imp(PrivateImplementationPattern<UnavailableRepository>::_imp)
{
    _add_metadata_keys();
}

UnavailableRepository::~UnavailableRepository()
{
}

bool
UnavailableRepository::can_be_favourite_repository() const
{
    return false;
}

const bool
UnavailableRepository::is_unimportant() const
{
    return true;
}

void
UnavailableRepository::_add_metadata_keys()
{
    clear_metadata_keys();
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->sync_key);
    add_metadata_key(_imp->sync_options_key);
    add_metadata_key(_imp->sync_host_key);
}

void
UnavailableRepository::need_keys_added() const
{
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnavailableRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
UnavailableRepository::location_key() const
{
    return _imp->location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
UnavailableRepository::installed_root_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

void
UnavailableRepository::invalidate()
{
    _imp.reset(new Implementation<UnavailableRepository>(this, _imp->params));
    _add_metadata_keys();
}

void
UnavailableRepository::invalidate_masks()
{
}

bool
UnavailableRepository::has_category_named(const CategoryNamePart & c) const
{
    return _imp->store->has_category_named(c);
}

bool
UnavailableRepository::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->store->has_package_named(q);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
UnavailableRepository::category_names() const
{
    return _imp->store->category_names();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
UnavailableRepository::unimportant_category_names() const
{
    return _imp->store->unimportant_category_names();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
UnavailableRepository::category_names_containing_package(const PackageNamePart & p) const
{
    return Repository::category_names_containing_package(p);
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
UnavailableRepository::package_names(const CategoryNamePart & c) const
{
    return _imp->store->package_names(c);
}

std::tr1::shared_ptr<const PackageIDSequence>
UnavailableRepository::package_ids(const QualifiedPackageName & p) const
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
UnavailableRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

bool
UnavailableRepository::some_ids_might_not_be_masked() const
{
    return false;
}

bool
UnavailableRepository::sync(const std::tr1::shared_ptr<OutputManager> & output_manager) const
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
                    n::filter_file() = FSEntry("/dev/null"),
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

std::tr1::shared_ptr<Repository>
UnavailableRepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    Context context("When making unavailable repository from repo_file '" + f("repo_file") + "':");

    std::string name_str(f("name"));
    if (name_str.empty())
        name_str = "unavailable";

    std::string location(f("location"));
    if (location.empty())
        throw UnavailableRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string sync(f("sync"));

    std::string sync_options(f("sync_options"));

    return std::tr1::shared_ptr<UnavailableRepository>(new UnavailableRepository(
                make_named_values<UnavailableRepositoryParams>(
                    n::environment() = env,
                    n::location() = location,
                    n::name() = RepositoryName(name_str),
                    n::sync() = sync,
                    n::sync_options() = sync_options
                )));
}

RepositoryName
UnavailableRepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    if (f("name").empty())
        return RepositoryName("unavailable");
    else
        return RepositoryName(f("name"));
}

std::tr1::shared_ptr<const RepositoryNameSet>
UnavailableRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

void
UnavailableRepository::populate_sets() const
{
}

HookResult
UnavailableRepository::perform_hook(const Hook &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnavailableRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnavailableRepository::sync_host_key() const
{
    return _imp->sync_host_key;
}

template class PrivateImplementationPattern<unavailable_repository::UnavailableRepository>;

