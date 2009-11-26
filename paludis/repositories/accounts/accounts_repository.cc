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

#include <paludis/repositories/accounts/accounts_repository.hh>
#include <paludis/repositories/accounts/accounts_exceptions.hh>
#include <paludis/repositories/accounts/accounts_repository_store.hh>
#include <paludis/repositories/accounts/dummy_accounts_handler.hh>
#include <paludis/repositories/accounts/passwd_accounts_handler.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/set.hh>
#include <paludis/util/active_object_ptr.hh>
#include <paludis/util/deferred_construction_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/dep_tag.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/package_id.hh>
#include <paludis/environment.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/hook.hh>

using namespace paludis;
using namespace paludis::accounts_repository;

namespace
{
    std::tr1::shared_ptr<AccountsHandler> make_handler(const std::string & handler)
    {
        if (handler == "dummy")
            return make_shared_ptr(new DummyAccountsHandler);
        else if (handler == "passwd")
            return make_shared_ptr(new PasswdAccountsHandler);
        else
            throw AccountsRepositoryConfigurationError("Unknown accounts handler '" + handler + "'");
    }

    std::tr1::shared_ptr<AccountsRepositoryStore>
    make_store(const AccountsRepository * const repo, const AccountsRepositoryParams & p)
    {
        return make_shared_ptr(new AccountsRepositoryStore(p.environment(), repo, false));
    }

    std::tr1::shared_ptr<AccountsRepositoryStore>
    make_installed_store(const AccountsRepository * const repo, const InstalledAccountsRepositoryParams & p)
    {
        return make_shared_ptr(new AccountsRepositoryStore(p.environment(), repo, true));
    }
}

namespace paludis
{
    template <>
    struct Implementation<AccountsRepository>
    {
        const std::tr1::shared_ptr<const AccountsRepositoryParams> params_if_not_installed;
        const std::tr1::shared_ptr<const InstalledAccountsRepositoryParams> params_if_installed;
        const std::tr1::shared_ptr<AccountsHandler> handler_if_installed;

        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > handler_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<FSEntry> > installed_root_key;

        const ActiveObjectPtr<DeferredConstructionPtr<std::tr1::shared_ptr<AccountsRepositoryStore> > > store;

        Implementation(AccountsRepository * const repo, const AccountsRepositoryParams & p) :
            params_if_not_installed(new AccountsRepositoryParams(p)),
            format_key(new LiteralMetadataValueKey<std::string> ("format", "format", mkt_significant, "accounts")),
            store(DeferredConstructionPtr<std::tr1::shared_ptr<AccountsRepositoryStore> > (
                        std::tr1::bind(&make_store, repo, std::tr1::cref(*params_if_not_installed))))
        {
        }

        Implementation(AccountsRepository * const repo, const InstalledAccountsRepositoryParams & p) :
            params_if_installed(new InstalledAccountsRepositoryParams(p)),
            handler_if_installed(make_handler(p.handler())),
            format_key(new LiteralMetadataValueKey<std::string> ("format", "format", mkt_significant, "installed-accounts")),
            handler_key(new LiteralMetadataValueKey<std::string> ("handler", "handler", mkt_normal, p.handler())),
            installed_root_key(new LiteralMetadataValueKey<FSEntry>("root", "root", mkt_normal, p.root())),
            store(DeferredConstructionPtr<std::tr1::shared_ptr<AccountsRepositoryStore> > (
                        std::tr1::bind(&make_installed_store, repo, std::tr1::cref(*params_if_installed))))
        {
        }
    };
}

AccountsRepository::AccountsRepository(const AccountsRepositoryParams & p) :
    PrivateImplementationPattern<AccountsRepository>(new Implementation<AccountsRepository>(this, p)),
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
    _imp(PrivateImplementationPattern<AccountsRepository>::_imp)
{
    _add_metadata_keys();
}

AccountsRepository::AccountsRepository(const InstalledAccountsRepositoryParams & p) :
    PrivateImplementationPattern<AccountsRepository>(new Implementation<AccountsRepository>(this, p)),
    Repository(
            p.environment(),
            p.name(),
            make_named_values<RepositoryCapabilities>(
                value_for<n::destination_interface>(this),
                value_for<n::environment_variable_interface>(static_cast<RepositoryEnvironmentVariableInterface *>(0)),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(static_cast<RepositoryManifestInterface *>(0)),
                value_for<n::provides_interface>(static_cast<RepositoryProvidesInterface *>(0)),
                value_for<n::virtuals_interface>(static_cast<RepositoryVirtualsInterface *>(0))
                )),
    _imp(PrivateImplementationPattern<AccountsRepository>::_imp)
{
    _add_metadata_keys();
}

void
AccountsRepository::need_keys_added() const
{
}

void
AccountsRepository::_add_metadata_keys()
{
    clear_metadata_keys();
    add_metadata_key(_imp->format_key);

    if (_imp->handler_key)
        add_metadata_key(_imp->handler_key);
}

AccountsRepository::~AccountsRepository()
{
}

std::tr1::shared_ptr<Repository>
AccountsRepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    Context context("When making accounts repository from repo_file '" + f("repo_file") + "':");

    std::string name_str(f("name"));
    if (name_str.empty())
        name_str = "accounts";

    return std::tr1::shared_ptr<AccountsRepository>(new AccountsRepository(
                make_named_values<AccountsRepositoryParams>(
                    value_for<n::environment>(env),
                    value_for<n::name>(RepositoryName(name_str))
                )));
}

std::tr1::shared_ptr<Repository>
AccountsRepository::repository_factory_installed_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    Context context("When making accounts repository from repo_file '" + f("repo_file") + "':");

    std::string name_str(f("name"));
    if (name_str.empty())
        name_str = "installed-accounts";

    std::string handler(f("handler"));
    if (handler.empty())
        throw AccountsRepositoryConfigurationError("Key 'handler' not specified or empty");

    std::string root_str(f("root"));
    if (root_str.empty())
        root_str = "/";

    if (root_str != "/")
        throw AccountsRepositoryConfigurationError("Values other than '/' for 'root' not yet supported");

    return std::tr1::shared_ptr<AccountsRepository>(new AccountsRepository(
                make_named_values<InstalledAccountsRepositoryParams>(
                    value_for<n::environment>(env),
                    value_for<n::handler>(handler),
                    value_for<n::name>(RepositoryName(name_str)),
                    value_for<n::root>(FSEntry(root_str))
                )));
}

RepositoryName
AccountsRepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    if (f("name").empty())
        return RepositoryName("accounts");
    else
        return RepositoryName(f("name"));
}

RepositoryName
AccountsRepository::repository_factory_installed_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    if (f("name").empty())
        return RepositoryName("installed-accounts");
    else
        return RepositoryName(f("name"));
}

std::tr1::shared_ptr<const RepositoryNameSet>
AccountsRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

std::tr1::shared_ptr<const RepositoryNameSet>
AccountsRepository::repository_factory_installed_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
AccountsRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
AccountsRepository::location_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
AccountsRepository::installed_root_key() const
{
    return _imp->installed_root_key;
}

void
AccountsRepository::invalidate()
{
    if (_imp->params_if_not_installed)
        _imp.reset(new Implementation<AccountsRepository>(this, *_imp->params_if_not_installed));
    else
        _imp.reset(new Implementation<AccountsRepository>(this, *_imp->params_if_installed));
    _add_metadata_keys();
}

void
AccountsRepository::invalidate_masks()
{
}

void
AccountsRepository::regenerate_cache() const
{
}

bool
AccountsRepository::has_category_named(const CategoryNamePart & c) const
{
    return _imp->store->has_category_named(c);
}

bool
AccountsRepository::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->store->has_package_named(q);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
AccountsRepository::category_names() const
{
    return _imp->store->category_names();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
AccountsRepository::unimportant_category_names() const
{
    return _imp->store->unimportant_category_names();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
AccountsRepository::category_names_containing_package(const PackageNamePart & p) const
{
    return Repository::category_names_containing_package(p);
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
AccountsRepository::package_names(const CategoryNamePart & c) const
{
    return _imp->store->package_names(c);
}

std::tr1::shared_ptr<const PackageIDSequence>
AccountsRepository::package_ids(const QualifiedPackageName & p) const
{
    return _imp->store->package_ids(p);
}

namespace
{
    struct SupportsActionQuery
    {
        bool installed;

        SupportsActionQuery(bool b) :
            installed(b)
        {
        }

        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return ! installed;
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
AccountsRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q(_imp->params_if_installed);
    return a.accept_returning<bool>(q);
}

bool
AccountsRepository::is_suitable_destination_for(const PackageID & id) const
{
    std::string f(id.repository()->format_key() ? id.repository()->format_key()->value() : "");
    return _imp->handler_if_installed && f == "accounts";
}

bool
AccountsRepository::is_default_destination() const
{
    return _imp->handler_if_installed &&
        _imp->params_if_installed->environment()->root() == installed_root_key()->value();
}

bool
AccountsRepository::want_pre_post_phases() const
{
    return true;
}

void
AccountsRepository::merge(const MergeParams & m)
{
    if (! _imp->handler_if_installed)
        throw ActionFailedError("Can't merge to here");

    _imp->handler_if_installed->merge(m);
}

namespace
{
    std::tr1::shared_ptr<SetSpecTree> get_everything_set(
            const Environment * const env,
            const AccountsRepository * const repo)
    {
        Context context("When making 'everything' set from '" + stringify(repo->name()) + "':");

        std::tr1::shared_ptr<SetSpecTree> result(new SetSpecTree(make_shared_ptr(new AllDepSpec)));

        std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(
                    generator::InRepository(repo->name()))]);
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
            result->root()->append(make_shared_ptr(new PackageDepSpec(
                            make_package_dep_spec(PartiallyMadePackageDepSpecOptions())
                            .package((*i)->name())
                            )));

        return result;
    }
}

void
AccountsRepository::populate_sets() const
{
    if (_imp->params_if_not_installed)
    {
        /* no sets */
    }
    else
    {
        /* everything */
        _imp->params_if_installed->environment()->add_set(
                SetName("everything"),
                SetName("everything::" + stringify(name())),
                std::tr1::bind(get_everything_set, _imp->params_if_installed->environment(), this),
                true);
    }
}

HookResult
AccountsRepository::perform_hook(const Hook &)
{
    return make_named_values<HookResult>(value_for<n::max_exit_status>(0), value_for<n::output>(""));
}

bool
AccountsRepository::sync(const std::tr1::shared_ptr<OutputManager> &) const
{
    return false;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
AccountsRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

template class PrivateImplementationPattern<AccountsRepository>;

