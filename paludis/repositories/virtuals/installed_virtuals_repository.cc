/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>

#include <tr1/functional>
#include <tr1/unordered_map>
#include <algorithm>
#include <vector>

using namespace paludis;

typedef std::tr1::unordered_map<QualifiedPackageName, std::tr1::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> > IDMap;

namespace paludis
{
    template<>
    struct Implementation<InstalledVirtualsRepository>
    {
        const Environment * const env;
        const FSEntry root;

        const std::tr1::shared_ptr<Mutex> ids_mutex;
        mutable IDMap ids;
        mutable bool has_ids;

        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > root_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Implementation(const Environment * const e, const FSEntry & r, std::tr1::shared_ptr<Mutex> m = make_shared_ptr(new Mutex)) :
            env(e),
            root(r),
            ids_mutex(m),
            has_ids(false),
            root_key(new LiteralMetadataValueKey<FSEntry> (
                        "root", "root", mkt_normal, root)),
            format_key(new LiteralMetadataValueKey<std::string> (
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
    make_name(const FSEntry & r)
    {
        if (FSEntry("/") == r)
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
        const FSEntry & r) :
    Repository(env, RepositoryName(make_name(r)), make_named_values<RepositoryCapabilities>(
                value_for<n::destination_interface>(static_cast<RepositoryDestinationInterface *>(this)),
                value_for<n::environment_variable_interface>(static_cast<RepositoryEnvironmentVariableInterface *>(0)),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(static_cast<RepositoryManifestInterface *>(0)),
                value_for<n::mirrors_interface>(static_cast<RepositoryMirrorsInterface *>(0)),
                value_for<n::provides_interface>(static_cast<RepositoryProvidesInterface *>(0)),
                value_for<n::virtuals_interface>(static_cast<RepositoryVirtualsInterface *>(0))
            )),
    PrivateImplementationPattern<InstalledVirtualsRepository>(
            new Implementation<InstalledVirtualsRepository>(env, r)),
    _imp(PrivateImplementationPattern<InstalledVirtualsRepository>::_imp)
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

        std::tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence> pp(
                (**r).provides_interface()->provided_packages());

        for (RepositoryProvidesInterface::ProvidesSequence::ConstIterator p(
                    pp->begin()), p_end(pp->end()) ; p != p_end ; ++p)
        {
            IDMap::iterator i(_imp->ids.find((*p).virtual_name()));
            if (i == _imp->ids.end())
                i = _imp->ids.insert(std::make_pair((*p).virtual_name(), make_shared_ptr(new PackageIDSequence))).first;

            std::tr1::shared_ptr<const PackageID> id(new virtuals::VirtualsPackageID(
                        _imp->env, shared_from_this(), (*p).virtual_name(), (*p).provided_by(), false));
            i->second->push_back(id);
        }
    }

    _imp->has_ids = true;
}

std::tr1::shared_ptr<const PackageIDSequence>
InstalledVirtualsRepository::package_ids(const QualifiedPackageName & q) const
{
    if (q.category().data() != "virtual")
        return std::tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(q));
    if (i == _imp->ids.end())
        return std::tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);

    return i->second;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
InstalledVirtualsRepository::package_names(const CategoryNamePart & c) const
{
    if (c.data() != "virtual")
        return std::tr1::shared_ptr<QualifiedPackageNameSet>(new QualifiedPackageNameSet);

    need_ids();

    std::tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
    std::transform(_imp->ids.begin(), _imp->ids.end(), result->inserter(),
            std::tr1::mem_fn(&std::pair<const QualifiedPackageName, std::tr1::shared_ptr<PackageIDSequence> >::first));

    return result;
}

std::tr1::shared_ptr<const CategoryNamePartSet>
InstalledVirtualsRepository::category_names() const
{
    std::tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
    result->insert(CategoryNamePart("virtual"));
    return result;
}

bool
InstalledVirtualsRepository::has_package_named(const QualifiedPackageName & q) const
{
    if (q.category().data() != "virtual")
        return false;

    need_ids();

    return _imp->ids.end() != _imp->ids.find(q);
}

bool
InstalledVirtualsRepository::has_category_named(const CategoryNamePart & c) const
{
    return (c.data() == "virtual");
}

void
InstalledVirtualsRepository::invalidate()
{
    _imp.reset(new Implementation<InstalledVirtualsRepository>(_imp->env, _imp->root, _imp->ids_mutex));
}

void
InstalledVirtualsRepository::invalidate_masks()
{
}

HookResult
InstalledVirtualsRepository::perform_hook(const Hook & hook)
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    return make_named_values<HookResult>(value_for<n::max_exit_status>(0), value_for<n::output>(""));
}

bool
InstalledVirtualsRepository::can_be_favourite_repository() const
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

std::tr1::shared_ptr<const CategoryNamePartSet>
InstalledVirtualsRepository::unimportant_category_names() const
{
    std::tr1::shared_ptr<CategoryNamePartSet> result(make_shared_ptr(new CategoryNamePartSet));
    result->insert(CategoryNamePart("virtual"));
    return result;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledVirtualsRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledVirtualsRepository::location_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
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
        const std::tr1::function<std::string (const std::string &)> & f)
{
    return make_name(FSEntry(f("root")));
}

std::tr1::shared_ptr<Repository>
InstalledVirtualsRepository::repository_factory_create(
        const Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    if (f("root").empty())
        throw ConfigurationError("Key 'root' unspecified or empty");

    return make_shared_ptr(new InstalledVirtualsRepository(env, f("root")));
}

std::tr1::shared_ptr<const RepositoryNameSet>
InstalledVirtualsRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

bool
InstalledVirtualsRepository::is_suitable_destination_for(const PackageID & e) const
{
    std::string f(e.repository()->format_key() ? e.repository()->format_key()->value() : "");
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
InstalledVirtualsRepository::sync(const std::tr1::shared_ptr<OutputManager> &) const
{
    return false;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledVirtualsRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

