/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/hashed_containers.hh>
#include <paludis/hook.hh>
#include <paludis/package_database.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/map.hh>
#include <paludis/util/mutex.hh>

#include <algorithm>
#include <vector>

using namespace paludis;

typedef MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type IDMap;

namespace paludis
{
    template<>
    struct Implementation<InstalledVirtualsRepository>
    {
        const Environment * const env;
        const FSEntry root;

        const tr1::shared_ptr<Mutex> ids_mutex;
        mutable IDMap ids;
        mutable bool has_ids;

        tr1::shared_ptr<const MetadataValueKey<FSEntry> > root_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Implementation(const Environment * const e, const FSEntry & r, tr1::shared_ptr<Mutex> m = make_shared_ptr(new Mutex)) :
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
    Repository(RepositoryName(make_name(r)), RepositoryCapabilities::named_create()
            (k::use_interface(), static_cast<RepositoryUseInterface *>(0))
            (k::sets_interface(), static_cast<RepositorySetsInterface *>(0))
            (k::syncable_interface(), static_cast<RepositorySyncableInterface *>(0))
            (k::mirrors_interface(), static_cast<RepositoryMirrorsInterface *>(0))
            (k::environment_variable_interface(), static_cast<RepositoryEnvironmentVariableInterface *>(0))
            (k::provides_interface(), static_cast<RepositoryProvidesInterface *>(0))
            (k::virtuals_interface(), static_cast<RepositoryVirtualsInterface *>(0))
            (k::destination_interface(), static_cast<RepositoryDestinationInterface *>(0))
            (k::e_interface(), static_cast<RepositoryEInterface *>(0))
            (k::make_virtuals_interface(), static_cast<RepositoryMakeVirtualsInterface *>(0))
            (k::qa_interface(), static_cast<RepositoryQAInterface *>(0))
            (k::hook_interface(), this)
            (k::manifest_interface(), static_cast<RepositoryManifestInterface *>(0))),
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
        if (! (**r)[k::provides_interface()])
            continue;

        tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence> pp(
                (**r)[k::provides_interface()]->provided_packages());

        for (RepositoryProvidesInterface::ProvidesSequence::ConstIterator p(
                    pp->begin()), p_end(pp->end()) ; p != p_end ; ++p)
        {
            IDMap::iterator i(_imp->ids.find((*p)[k::virtual_name()]));
            if (i == _imp->ids.end())
                i = _imp->ids.insert(std::make_pair((*p)[k::virtual_name()], make_shared_ptr(new PackageIDSequence))).first;

            tr1::shared_ptr<const PackageID> id(new virtuals::VirtualsPackageID(
                        _imp->env, shared_from_this(), (*p)[k::virtual_name()], (*p)[k::provided_by()], false));
            i->second->push_back(id);
        }
    }

    _imp->has_ids = true;
}

tr1::shared_ptr<Repository>
InstalledVirtualsRepository::make_installed_virtuals_repository(
        Environment * const env,
        tr1::shared_ptr<const Map<std::string, std::string> > k)
{
    std::string root_str;

    if (k && (k->end() != k->find("root")))
        root_str = k->find("root")->second;

    if (root_str.empty())
        throw ConfigurationError("No root specified for InstalledVirtualsRepository");

    return tr1::shared_ptr<Repository>(new InstalledVirtualsRepository(env, FSEntry(root_str)));
}

tr1::shared_ptr<const PackageIDSequence>
InstalledVirtualsRepository::package_ids(const QualifiedPackageName & q) const
{
    if (q.category.data() != "virtual")
        return tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(q));
    if (i == _imp->ids.end())
        return tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);

    return i->second;
}

tr1::shared_ptr<const QualifiedPackageNameSet>
InstalledVirtualsRepository::package_names(const CategoryNamePart & c) const
{
    if (c.data() != "virtual")
        return tr1::shared_ptr<QualifiedPackageNameSet>(new QualifiedPackageNameSet);

    need_ids();

    tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
    std::transform(_imp->ids.begin(), _imp->ids.end(), result->inserter(),
            tr1::mem_fn(&std::pair<const QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::first));

    return result;
}

tr1::shared_ptr<const CategoryNamePartSet>
InstalledVirtualsRepository::category_names() const
{
    tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
    result->insert(CategoryNamePart("virtual"));
    return result;
}

bool
InstalledVirtualsRepository::has_package_named(const QualifiedPackageName & q) const
{
    if (q.category.data() != "virtual")
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
InstalledVirtualsRepository::perform_hook(const Hook & hook) const
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    return HookResult(0, "");
}

bool
InstalledVirtualsRepository::can_be_favourite_repository() const
{
    return false;
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
            result = true;
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
        }

        void visit(const SupportsActionTest<PretendFetchAction> &)
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
InstalledVirtualsRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

tr1::shared_ptr<const CategoryNamePartSet>
InstalledVirtualsRepository::unimportant_category_names() const
{
    tr1::shared_ptr<CategoryNamePartSet> result(make_shared_ptr(new CategoryNamePartSet));
    result->insert(CategoryNamePart("virtual"));
    return result;
}

const tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledVirtualsRepository::format_key() const
{
    return _imp->format_key;
}

const tr1::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledVirtualsRepository::installed_root_key() const
{
    return _imp->root_key;
}

void
InstalledVirtualsRepository::need_keys_added() const
{
}

