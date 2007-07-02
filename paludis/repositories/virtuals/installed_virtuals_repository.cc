/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/repository_info.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tr1_functional.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

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

        mutable IDMap ids;
        mutable bool has_ids;

        Implementation(const Environment * const e, const FSEntry & r) :
            env(e),
            root(r),
            has_ids(false)
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
            return RepositoryName("installed_virtuals");
        else
        {
            std::string n("installed_virtuals-" + stringify(r)), result;
            std::transform(n.begin(), n.end(), std::back_inserter(result), MakeSafe());
            return RepositoryName(result);
        }
    }
}

InstalledVirtualsRepository::InstalledVirtualsRepository(const Environment * const env,
        const FSEntry & r) :
    Repository(RepositoryName(make_name(r)), RepositoryCapabilities::create()
            .installable_interface(0)
            .mask_interface(0)
            .installed_interface(this)
            .use_interface(0)
            .sets_interface(0)
            .syncable_interface(0)
            .uninstallable_interface(0)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .world_interface(0)
            .provides_interface(0)
            .virtuals_interface(0)
            .config_interface(0)
            .destination_interface(0)
            .licenses_interface(0)
            .e_interface(0)
            .make_virtuals_interface(0)
            .pretend_interface(0)
            .qa_interface(0)
            .hook_interface(this),
            "installed_virtuals"),
    PrivateImplementationPattern<InstalledVirtualsRepository>(
            new Implementation<InstalledVirtualsRepository>(env, r))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));
    config_info->add_kv("format", "installed_virtuals");
    _info->add_section(config_info);
}

InstalledVirtualsRepository::~InstalledVirtualsRepository()
{
}

void
InstalledVirtualsRepository::need_ids() const
{
    if (_imp->has_ids)
        return;

    /* Populate our _imp->entries. We need to iterate over each repository in
     * our env's package database, see if it has a provides interface, and if it
     * does create an entry for each provided package. */
    for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (*r)->provides_interface)
            continue;

        tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence> pp(
                (*r)->provides_interface->provided_packages());

        for (RepositoryProvidesInterface::ProvidesSequence::Iterator p(
                    pp->begin()), p_end(pp->end()) ; p != p_end ; ++p)
        {
            IDMap::iterator i(_imp->ids.find(p->virtual_name));
            if (i == _imp->ids.end())
                i = _imp->ids.insert(std::make_pair(p->virtual_name, make_shared_ptr(new PackageIDSequence::Concrete))).first;

            tr1::shared_ptr<const PackageID> id(new virtuals::VirtualsPackageID(shared_from_this(), p->virtual_name, p->provided_by));
            i->second->push_back(id);
        }
    }

    _imp->has_ids = true;
}

tr1::shared_ptr<Repository>
InstalledVirtualsRepository::make_installed_virtuals_repository(
        Environment * const env,
        tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > k)
{
    std::string root_str;

    if (k && (k->end() != k->find("root")))
        root_str = k->find("root")->second;

    if (root_str.empty())
        throw ConfigurationError("No root specified for InstalledVirtualsRepository");

    return tr1::shared_ptr<Repository>(new InstalledVirtualsRepository(env, FSEntry(root_str)));
}

tr1::shared_ptr<const PackageIDSequence>
InstalledVirtualsRepository::do_package_ids(const QualifiedPackageName & q) const
{
    if (q.category.data() != "virtual")
        return tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence::Concrete);

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(q));
    if (i == _imp->ids.end())
        return tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence::Concrete);

    return i->second;
}

tr1::shared_ptr<const QualifiedPackageNameCollection>
InstalledVirtualsRepository::do_package_names(const CategoryNamePart & c) const
{
    if (c.data() != "virtual")
        return tr1::shared_ptr<QualifiedPackageNameCollection>(new QualifiedPackageNameCollection::Concrete);

    need_ids();

    tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);
    std::copy(_imp->ids.begin(), _imp->ids.end(), transform_inserter(result->inserter(),
                tr1::mem_fn(&std::pair<const QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::first)));

    return result;
}

tr1::shared_ptr<const CategoryNamePartCollection>
InstalledVirtualsRepository::do_category_names() const
{
    tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
    result->insert(CategoryNamePart("virtual"));
    return result;
}

bool
InstalledVirtualsRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    if (q.category.data() != "virtual")
        return false;

    need_ids();

    return _imp->ids.end() != _imp->ids.find(q);
}

bool
InstalledVirtualsRepository::do_has_category_named(const CategoryNamePart & c) const
{
    return (c.data() == "virtual");
}

void
InstalledVirtualsRepository::invalidate()
{
    _imp.reset(new Implementation<InstalledVirtualsRepository>(_imp->env, _imp->root));
}

FSEntry
InstalledVirtualsRepository::root() const
{
    return _imp->root;
}

HookResult
InstalledVirtualsRepository::perform_hook(const Hook & hook) const
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    return HookResult(0, "");
}

time_t
InstalledVirtualsRepository::do_installed_time(const PackageID &) const
{
    return 0;
}

bool
InstalledVirtualsRepository::can_be_favourite_repository() const
{
    return false;
}

