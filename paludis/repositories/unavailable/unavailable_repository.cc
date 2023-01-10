/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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
#include <paludis/util/pimp-impl.hh>
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
#include <vector>
#include <list>

using namespace paludis;
using namespace paludis::unavailable_repository;

namespace
{
    std::shared_ptr<UnavailableRepositoryStore>
    make_store(const UnavailableRepository * const repo, const UnavailableRepositoryParams & p)
    {
        return std::make_shared<UnavailableRepositoryStore>(p.environment(), repo, p.location());
    }
}

namespace paludis
{
    template <>
    struct Imp<UnavailableRepository>
    {
        const UnavailableRepositoryParams params;

        const std::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        const std::shared_ptr<LiteralMetadataValueKey<FSPath> > location_key;
        const std::shared_ptr<LiteralMetadataStringStringMapKey> sync_key;
        const std::shared_ptr<LiteralMetadataStringStringMapKey> sync_options_key;
        const std::shared_ptr<Map<std::string, std::string> > sync_hosts;
        const std::shared_ptr<LiteralMetadataStringStringMapKey> sync_host_key;

        const ActiveObjectPtr<DeferredConstructionPtr<
            std::shared_ptr<UnavailableRepositoryStore> > > store;

        const std::shared_ptr<CategoryNamePartSet> unmasked_category_names;
        const std::shared_ptr<PackageIDSequence> no_package_ids;

        Imp(const UnavailableRepository * const repo, const UnavailableRepositoryParams & p) :
            params(p),
            format_key(std::make_shared<LiteralMetadataValueKey<std::string> >("format", "format",
                        mkt_significant, "unavailable")),
            location_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("location", "location",
                        mkt_significant, params.location())),
            sync_key(std::make_shared<LiteralMetadataStringStringMapKey>(
                        "sync", "sync", mkt_normal, params.sync())),
            sync_options_key(std::make_shared<LiteralMetadataStringStringMapKey>(
                        "sync_options", "sync_options", mkt_normal, params.sync_options())),
            sync_hosts(std::make_shared<Map<std::string, std::string> >()),
            sync_host_key(std::make_shared<LiteralMetadataStringStringMapKey>("sync_host", "sync_host", mkt_internal, sync_hosts)),
            store(DeferredConstructionPtr<std::shared_ptr<UnavailableRepositoryStore> > (
                        std::bind(&make_store, repo, std::cref(params)))),
            unmasked_category_names(std::make_shared<CategoryNamePartSet>()),
            no_package_ids(std::make_shared<PackageIDSequence>())
        {
            for (const auto & i : *params.sync())
                sync_hosts->insert(i.first, extract_host_from_url(i.second));

            unmasked_category_names->insert(CategoryNamePart("repository"));
        }
    };
}

UnavailableRepositoryConfigurationError::UnavailableRepositoryConfigurationError(const std::string & s) noexcept :
    ConfigurationError("UnavailableRepository configuration error: " + s)
{
}

UnavailableRepository::UnavailableRepository(const UnavailableRepositoryParams & p) :
    Repository(
            p.environment(),
            p.name(),
            make_named_values<RepositoryCapabilities>(
                n::destination_interface() = static_cast<RepositoryDestinationInterface *>(nullptr),
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(nullptr),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(nullptr)
                )),
    _imp(this, p)
{
    _add_metadata_keys();
}

UnavailableRepository::~UnavailableRepository() = default;

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

const std::shared_ptr<const MetadataValueKey<std::string> >
UnavailableRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
UnavailableRepository::location_key() const
{
    return _imp->location_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
UnavailableRepository::installed_root_key() const
{
    return nullptr;
}

void
UnavailableRepository::invalidate()
{
    _imp.reset(new Imp<UnavailableRepository>(this, _imp->params));
    _add_metadata_keys();
}

bool
UnavailableRepository::has_category_named(const CategoryNamePart & c, const RepositoryContentMayExcludes & x) const
{
    Context context("When checking for category named '" + stringify(c) + "' in '" + stringify(name()) + ":");

    if (x[rcme_not_installed])
        return false;

    if (x[rcme_masked])
    {
        /* we're only interested in unmasked packages, which are in repository/ */
        return c == CategoryNamePart("repository");
    }

    return _imp->store->has_category_named(c);
}

bool
UnavailableRepository::has_package_named(const QualifiedPackageName & q, const RepositoryContentMayExcludes & x) const
{
    Context context("When checking for package named '" + stringify(q) + "' in '" + stringify(name()) + ":");

    if (x[rcme_not_installed])
        return false;

    if (x[rcme_masked])
    {
        /* we're only interested in unmasked packages, which are in repository/ */
        if (q.category() != CategoryNamePart("repository"))
            return false;
    }

    return _imp->store->has_package_named(q);
}

std::shared_ptr<const CategoryNamePartSet>
UnavailableRepository::category_names(const RepositoryContentMayExcludes & x) const
{
    Context context("When checking for categories in '" + stringify(name()) + ":");

    if (x[rcme_not_installed])
    {
        /* nothing, but we're allowed to return extras */
        return _imp->unmasked_category_names;
    }

    if (x[rcme_masked])
        return _imp->unmasked_category_names;

    return _imp->store->category_names();
}

std::shared_ptr<const CategoryNamePartSet>
UnavailableRepository::unimportant_category_names(const RepositoryContentMayExcludes &) const
{
    Context context("When checking for unimportant categories in '" + stringify(name()) + ":");

    return _imp->store->unimportant_category_names();
}

std::shared_ptr<const CategoryNamePartSet>
UnavailableRepository::category_names_containing_package(const PackageNamePart & p, const RepositoryContentMayExcludes & x) const
{
    return Repository::category_names_containing_package(p, x);
}

std::shared_ptr<const QualifiedPackageNameSet>
UnavailableRepository::package_names(const CategoryNamePart & c, const RepositoryContentMayExcludes &) const
{
    Context context("When checking for packages in '" + stringify(c) + "' in '" + stringify(name()) + ":");

    return _imp->store->package_names(c);
}

std::shared_ptr<const PackageIDSequence>
UnavailableRepository::package_ids(const QualifiedPackageName & p, const RepositoryContentMayExcludes & x) const
{
    Context context("When checking for package IDs in '" + stringify(p) + "' in '" + stringify(name()) + ":");

    if (x[rcme_not_installed])
        return _imp->no_package_ids;

    if (x[rcme_masked] && p.category() != CategoryNamePart("repository"))
        return _imp->no_package_ids;

    return _imp->store->package_ids(p);
}

bool
UnavailableRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    return a.make_accept_returning(
            [&] (const SupportsActionTest<InstallAction> &)      { return true; },
            [&] (const SupportsActionTest<FetchAction> &)        { return false; },
            [&] (const SupportsActionTest<PretendFetchAction> &) { return false; },
            [&] (const SupportsActionTest<ConfigAction> &)       { return false; },
            [&] (const SupportsActionTest<PretendAction> &)      { return false; },
            [&] (const SupportsActionTest<InfoAction> &)         { return false; },
            [&] (const SupportsActionTest<UninstallAction> &)    { return false; }
            );
}

bool
UnavailableRepository::some_ids_might_not_be_masked() const
{
    return true;
}

bool
UnavailableRepository::sync(
        const std::string & source,
        const std::string & revision,
        const std::shared_ptr<OutputManager> & output_manager) const
{
    Context context("When syncing repository '" + stringify(name()) + "':");

    std::string sync_uri;
    std::string sync_options;
    if (_imp->params.sync()->end() != _imp->params.sync()->find(source))
        sync_uri = _imp->params.sync()->find(source)->second;
    if (sync_uri.empty())
        return false;

    if (_imp->params.sync_options()->end() != _imp->params.sync_options()->find(source))
        sync_options = _imp->params.sync_options()->find(source)->second;

    std::list<std::string> sync_list;
    tokenise_whitespace(sync_uri, std::back_inserter(sync_list));

    bool ok(false);
    for (const auto & remote : sync_list)
    {
        DefaultSyncer syncer(make_named_values<SyncerParams>(
                    n::environment() = _imp->params.environment(),
                    n::local() = stringify(_imp->params.location()),
                    n::remote() = remote,
                    n::revision() = revision
                    ));

        SyncOptions opts(make_named_values<SyncOptions>(
                    n::filter_file() = FSPath("/dev/null"),
                    n::options() = sync_options,
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
        throw SyncFailedError(stringify(_imp->params.location()), sync_uri);

    return true;
}

std::shared_ptr<Repository>
UnavailableRepository::repository_factory_create(
        Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    Context context("When making unavailable repository from repo_file '" + f("repo_file") + "':");

    std::string name_str(f("name"));
    if (name_str.empty())
        name_str = "unavailable";

    std::string location(f("location"));
    if (location.empty())
        throw UnavailableRepositoryConfigurationError("Key 'location' not specified or empty");

    auto sync(std::make_shared<Map<std::string, std::string> >());
    std::vector<std::string> sync_tokens;
    tokenise_whitespace(f("sync"), std::back_inserter(sync_tokens));
    std::string source;
    for (auto & sync_token : sync_tokens)
        if ((! sync_token.empty()) && (':' == sync_token.at(sync_token.length() - 1)))
            source = sync_token.substr(0, sync_token.length() - 1);
        else
        {
            std::string v;
            if (sync->end() != sync->find(source))
                v = sync->find(source)->second + " ";
            sync->erase(source);
            sync->insert(source, v + sync_token);
        }

    auto sync_options(std::make_shared<Map<std::string, std::string> >());
    std::vector<std::string> sync_options_tokens;
    tokenise_whitespace(f("sync_options"), std::back_inserter(sync_options_tokens));
    source = "";
    for (auto & sync_options_token : sync_options_tokens)
        if ((! sync_options_token.empty()) && (':' == sync_options_token.at(sync_options_token.length() - 1)))
            source = sync_options_token.substr(0, sync_options_token.length() - 1);
        else
        {
            std::string v;
            if (sync_options->end() != sync_options->find(source))
                v = sync_options->find(source)->second + " ";
            sync_options->erase(source);
            sync_options->insert(source, v + sync_options_token);
        }

    return std::make_shared<UnavailableRepository>(
            make_named_values<UnavailableRepositoryParams>(
                n::environment() = env,
                n::location() = location,
                n::name() = RepositoryName(name_str),
                n::sync() = sync,
                n::sync_options() = sync_options
                ));
}

RepositoryName
UnavailableRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> & f)
{
    if (f("name").empty())
        return RepositoryName("unavailable");
    else
        return RepositoryName(f("name"));
}

std::shared_ptr<const RepositoryNameSet>
UnavailableRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

void
UnavailableRepository::populate_sets() const
{
}

HookResult
UnavailableRepository::perform_hook(const Hook &, const std::shared_ptr<OutputManager> &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > >
UnavailableRepository::sync_host_key() const
{
    return _imp->sync_host_key;
}

const std::shared_ptr<const Set<std::string> >
UnavailableRepository::maybe_expand_licence_nonrecursively(const std::string &) const
{
    return nullptr;
}

namespace paludis
{
    template class Pimp<unavailable_repository::UnavailableRepository>;
}
