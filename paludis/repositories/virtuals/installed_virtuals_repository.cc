/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/virtuals/installed_virtuals_repository.hh>
#include <paludis/repositories/virtuals/package_id.hh>

#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/package_database.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <functional>
#include <unordered_map>
#include <algorithm>
#include <vector>

using namespace paludis;

typedef std::unordered_map<QualifiedPackageName, std::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> > IDMap;

namespace paludis
{
    template<>
    struct Imp<InstalledVirtualsRepository>
    {
        const Environment * const env;
        const FSPath root;

        const std::shared_ptr<Mutex> ids_mutex;
        mutable IDMap ids;
        mutable bool has_ids;

        std::shared_ptr<const MetadataValueKey<FSPath> > root_key;
        std::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Imp(const Environment * const e, const FSPath & r, std::shared_ptr<Mutex> m = std::make_shared<Mutex>()) :
            env(e),
            root(r),
            ids_mutex(m),
            has_ids(false),
            root_key(std::make_shared<LiteralMetadataValueKey<FSPath> >(
                        "root", "root", mkt_normal, root)),
            format_key(std::make_shared<LiteralMetadataValueKey<std::string> >(
                        "format", "format", mkt_significant, "installed_virtuals"))
        {
        }
    };
}

namespace
{
    struct MakeSafe
    {
        char operator() (const char & c) const
        {
            static const std::string allow(
                    "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "0123456789_-");

            if (std::string::npos == allow.find(c))
                return '-';
            else
                return c;
        }
    };

    RepositoryName
    make_name(const FSPath & r)
    {
        if (FSPath("/") == r)
            return RepositoryName("installed-virtuals");
        else
        {
            std::string n("installed-virtuals-" + stringify(r)), result;
            std::transform(n.begin(), n.end(), std::back_inserter(result), MakeSafe());
            return RepositoryName(result);
        }
    }
}

InstalledVirtualsRepository::InstalledVirtualsRepository(const Environment * const env,
        const FSPath & r) :
    Repository(env, RepositoryName(make_name(r)), make_named_values<RepositoryCapabilities>(
                n::destination_interface() = static_cast<RepositoryDestinationInterface *>(this),
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(0),
                n::make_virtuals_interface() = static_cast<RepositoryMakeVirtualsInterface *>(0),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(0),
                n::provides_interface() = static_cast<RepositoryProvidesInterface *>(0),
                n::virtuals_interface() = static_cast<RepositoryVirtualsInterface *>(0)
            )),
    Pimp<InstalledVirtualsRepository>(env, r),
    _imp(Pimp<InstalledVirtualsRepository>::_imp)
{
    add_metadata_key(_imp->root_key);
    add_metadata_key(_imp->format_key);
}

InstalledVirtualsRepository::~InstalledVirtualsRepository()
{
}

void
InstalledVirtualsRepository::need_ids() const
{
    Lock l(*_imp->ids_mutex);

    if (_imp->has_ids)
        return;

    /* Populate our _imp->entries. We need to iterate over each repository in
     * our env's package database, see if it has a provides interface, and if it
     * does create an entry for each provided package. */
    for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (**r).provides_interface())
            continue;

        std::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence> pp(
                (**r).provides_interface()->provided_packages());

        for (RepositoryProvidesInterface::ProvidesSequence::ConstIterator p(
                    pp->begin()), p_end(pp->end()) ; p != p_end ; ++p)
        {
            IDMap::iterator i(_imp->ids.find((*p).virtual_name()));
            if (i == _imp->ids.end())
                i = _imp->ids.insert(std::make_pair((*p).virtual_name(), std::make_shared<PackageIDSequence>())).first;

            std::shared_ptr<const PackageID> id(std::make_shared<virtuals::VirtualsPackageID>(
                        _imp->env, shared_from_this(), (*p).virtual_name(), (*p).provided_by(), false));
            i->second->push_back(id);
        }
    }

    _imp->has_ids = true;
}

std::shared_ptr<const PackageIDSequence>
InstalledVirtualsRepository::package_ids(const QualifiedPackageName & q) const
{
    if (q.category().value() != "virtual")
        return std::make_shared<PackageIDSequence>();

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(q));
    if (i == _imp->ids.end())
        return std::make_shared<PackageIDSequence>();

    return i->second;
}

std::shared_ptr<const QualifiedPackageNameSet>
InstalledVirtualsRepository::package_names(const CategoryNamePart & c) const
{
    if (c.value() != "virtual")
        return std::make_shared<QualifiedPackageNameSet>();

    need_ids();

    std::shared_ptr<QualifiedPackageNameSet> result(std::make_shared<QualifiedPackageNameSet>());
    std::transform(_imp->ids.begin(), _imp->ids.end(), result->inserter(),
            std::mem_fn(&std::pair<const QualifiedPackageName, std::shared_ptr<PackageIDSequence> >::first));

    return result;
}

std::shared_ptr<const CategoryNamePartSet>
InstalledVirtualsRepository::category_names() const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());
    result->insert(CategoryNamePart("virtual"));
    return result;
}

bool
InstalledVirtualsRepository::has_package_named(const QualifiedPackageName & q) const
{
    if (q.category().value() != "virtual")
        return false;

    need_ids();

    return _imp->ids.end() != _imp->ids.find(q);
}

bool
InstalledVirtualsRepository::has_category_named(const CategoryNamePart & c) const
{
    return (c.value() == "virtual");
}

void
InstalledVirtualsRepository::invalidate()
{
    _imp.reset(new Imp<InstalledVirtualsRepository>(_imp->env, _imp->root, _imp->ids_mutex));
}

void
InstalledVirtualsRepository::invalidate_masks()
{
}

HookResult
InstalledVirtualsRepository::perform_hook(const Hook & hook, const std::shared_ptr<OutputManager> &)
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

bool
InstalledVirtualsRepository::can_be_favourite_repository() const
{
    return false;
}

const bool
InstalledVirtualsRepository::is_unimportant() const
{
    return false;
}

namespace
{
    struct SupportsActionQuery
    {
        bool visit(const SupportsActionTest<InstallAction> &) const
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

        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
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
InstalledVirtualsRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

bool
InstalledVirtualsRepository::some_ids_might_not_be_masked() const
{
    return true;
}

std::shared_ptr<const CategoryNamePartSet>
InstalledVirtualsRepository::unimportant_category_names() const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());
    result->insert(CategoryNamePart("virtual"));
    return result;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
InstalledVirtualsRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
InstalledVirtualsRepository::location_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSPath> >();
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
InstalledVirtualsRepository::installed_root_key() const
{
    return _imp->root_key;
}

void
InstalledVirtualsRepository::need_keys_added() const
{
}

RepositoryName
InstalledVirtualsRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> & f)
{
    return make_name(FSPath(f("root")));
}

std::shared_ptr<Repository>
InstalledVirtualsRepository::repository_factory_create(
        const Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    if (f("root").empty())
        throw ConfigurationError("Key 'root' unspecified or empty");

    return std::make_shared<InstalledVirtualsRepository>(env, FSPath(f("root")));
}

std::shared_ptr<const RepositoryNameSet>
InstalledVirtualsRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

bool
InstalledVirtualsRepository::is_suitable_destination_for(const std::shared_ptr<const PackageID> & e) const
{
    std::string f(e->repository()->format_key() ? e->repository()->format_key()->value() : "");
    return f == "virtuals";

}

bool
InstalledVirtualsRepository::is_default_destination() const
{
    return false;
}

bool
InstalledVirtualsRepository::want_pre_post_phases() const
{
    return false;
}

void
InstalledVirtualsRepository::merge(const MergeParams &)
{
    throw InternalError(PALUDIS_HERE, "can't merge to installed virtuals");
}

void
InstalledVirtualsRepository::populate_sets() const
{
}

bool
InstalledVirtualsRepository::sync(
        const std::string &,
        const std::shared_ptr<OutputManager> &) const
{
    return false;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
InstalledVirtualsRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > >
InstalledVirtualsRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

