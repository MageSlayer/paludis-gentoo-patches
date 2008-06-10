/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/syncer.hh>
#include <list>

using namespace paludis;
using namespace paludis::unavailable_repository;

namespace
{
    std::tr1::shared_ptr<UnavailableRepositoryStore>
    make_store(const UnavailableRepository * const repo, const UnavailableRepositoryParams & p)
    {
        return make_shared_ptr(new UnavailableRepositoryStore(p[k::environment()], repo, p[k::location()]));
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

        const ActiveObjectPtr<DeferredConstructionPtr<
            std::tr1::shared_ptr<UnavailableRepositoryStore> > > store;

        Implementation(const UnavailableRepository * const repo, const UnavailableRepositoryParams & p) :
            params(p),
            format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                        mkt_significant, "unavailable")),
            location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                        mkt_significant, params[k::location()])),
            sync_key(new LiteralMetadataValueKey<std::string> (
                        "sync", "sync", mkt_normal, params[k::sync()])),
            sync_options_key(new LiteralMetadataValueKey<std::string> (
                        "sync_options", "sync_options", mkt_normal, params[k::sync_options()])),
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
            RepositoryName("unavailable"),
            RepositoryCapabilities::named_create()
            (k::sets_interface(), static_cast<RepositorySetsInterface *>(0))
            (k::syncable_interface(), this)
            (k::use_interface(), static_cast<RepositoryUseInterface *>(0))
            (k::environment_variable_interface(), static_cast<RepositoryEnvironmentVariableInterface *>(0))
            (k::mirrors_interface(), static_cast<RepositoryMirrorsInterface *>(0))
            (k::virtuals_interface(), static_cast<RepositoryVirtualsInterface *>(0))
            (k::provides_interface(), static_cast<RepositoryProvidesInterface *>(0))
            (k::destination_interface(), static_cast<RepositoryDestinationInterface *>(0))
            (k::make_virtuals_interface(), static_cast<RepositoryMakeVirtualsInterface *>(0))
            (k::e_interface(), static_cast<RepositoryEInterface *>(0))
            (k::qa_interface(), static_cast<RepositoryQAInterface *>(0))
            (k::hook_interface(), static_cast<RepositoryHookInterface *>(0))
            (k::manifest_interface(), static_cast<RepositoryManifestInterface *>(0))
            ),
    _imp(PrivateImplementationPattern<UnavailableRepository>::_imp)
{
    _add_metadata_keys();
}

UnavailableRepository::~UnavailableRepository()
{
}

void
UnavailableRepository::_add_metadata_keys()
{
    clear_metadata_keys();
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->sync_key);
    add_metadata_key(_imp->sync_options_key);
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
    struct SupportsActionQuery :
        ConstVisitor<SupportsActionTestVisitorTypes>
    {
        bool result;

        SupportsActionQuery() :
            result(false)
        {
        }

        void visit(const SupportsActionTest<InstalledAction> &)
        {
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
        }

        void visit(const SupportsActionTest<PretendFetchAction> &)
        {
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
        }

        void visit(const SupportsActionTest<InfoAction> &)
        {
        }

        void visit(const SupportsActionTest<UninstallAction> &)
        {
        }
    };
}

bool
UnavailableRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

bool
UnavailableRepository::sync() const
{
    Context context("When syncing repository '" + stringify(name()) + "':");

    if (_imp->params[k::sync()].empty())
        return false;

    std::list<std::string> sync_list;
    tokenise_whitespace(_imp->params[k::sync()], std::back_inserter(sync_list));

    bool ok(false);
    for (std::list<std::string>::const_iterator s(sync_list.begin()),
            s_end(sync_list.end()) ; s != s_end ; ++s)
    {
        DefaultSyncer syncer(SyncerParams::named_create()
                (k::environment(), _imp->params[k::environment()])
                (k::local(), stringify(_imp->params[k::location()]))
                (k::remote(), *s)
                );
        SyncOptions opts(
                _imp->params[k::sync_options()],
                FSEntry("/dev/null"),
                "sync " + stringify(name()) + "> "
                );
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
        throw SyncFailedError(stringify(_imp->params[k::location()]), _imp->params[k::sync()]);

    return true;
}

template class PrivateImplementationPattern<unavailable_repository::UnavailableRepository>;

