/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/accounts/accounts_repository_store.hh>
#include <paludis/repositories/accounts/accounts_repository.hh>
#include <paludis/repositories/accounts/accounts_id.hh>
#include <paludis/repositories/accounts/installed_accounts_id.hh>
#include <paludis/repositories/accounts/accounts_exceptions.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/log.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/options.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/name.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/literal_metadata_key.hh>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

using namespace paludis;
using namespace paludis::accounts_repository;

typedef std::unordered_map<CategoryNamePart,
        std::shared_ptr<QualifiedPackageNameSet>,
        Hash<CategoryNamePart> > PackageNames;

typedef std::unordered_map<QualifiedPackageName,
        std::shared_ptr<PackageIDSequence>,
        Hash<QualifiedPackageName> > IDs;

namespace paludis
{
    template <>
    struct Imp<AccountsRepositoryStore>
    {
        const Environment * const env;
        const RepositoryName repository_name;
        const bool installed;

        std::shared_ptr<CategoryNamePartSet> categories;
        mutable PackageNames package_names;
        mutable IDs ids;

        Imp(const Environment * const e, const RepositoryName & r, const bool i) :
            env(e),
            repository_name(r),
            installed(i),
            categories(std::make_shared<CategoryNamePartSet>())
        {
            categories->insert(CategoryNamePart("user"));
            categories->insert(CategoryNamePart("group"));
        }
    };
}

AccountsRepositoryStore::AccountsRepositoryStore(
        const Environment * const env,
        const RepositoryName & r,
        const bool installed) :
    Pimp<AccountsRepositoryStore>(env, r, installed)
{
    _load(r);
}

AccountsRepositoryStore::~AccountsRepositoryStore()
{
}

void
AccountsRepositoryStore::_load(const RepositoryName & repository_name)
{
    Context context("When loading data for AccountsRepository:");

    for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        Context r_context("When loading data for repository '" + stringify((*r)->name()) + ":");

        Repository::MetadataConstIterator k_iter((*r)->find_metadata("accounts_repository_data_location"));
        if (k_iter == (*r)->end_metadata())
        {
            Log::get_instance()->message("accounts.no_key_from_repository", ll_debug, lc_context) <<
                "Repository " << (*r)->name() << " defines no accounts_repository_data_location key";
            continue;
        }

        const MetadataValueKey<FSPath> * k(simple_visitor_cast<const MetadataValueKey<FSPath> >(**k_iter));
        if (! k)
        {
            Log::get_instance()->message("accounts.bad_key_from_repository", ll_warning, lc_context) <<
                "Repository " << (*r)->name() << " defines an accounts_repository_data_location key, but it is not an FSPath key";
            continue;
        }

        FSPath dir(k->value());
        if (! dir.stat().is_directory_or_symlink_to_directory())
        {
            Log::get_instance()->message("accounts.empty_key_from_repository", ll_warning, lc_context) <<
                "Repository " << (*r)->name() << " has accounts_repository_data_location " << dir << ", but this is not a directory";
            continue;
        }

        std::shared_ptr<Set<std::string> > r_set(std::make_shared<Set<std::string>>());
        r_set->insert(stringify((*r)->name()));
        std::shared_ptr<LiteralMetadataStringSetKey> r_key(std::make_shared<LiteralMetadataStringSetKey>("defined_by", "Defined by repository", mkt_internal, r_set));
        _load_one(repository_name, r_key, dir);
    }
}

void
AccountsRepositoryStore::_load_one(
        const RepositoryName & repository_name,
        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & from_repo,
        const FSPath & dir)
{
    Context context("When loading accounts from directory '" + stringify(dir) + "':");

    FSPath users_dir(dir / "users");
    if (users_dir.stat().is_directory_or_symlink_to_directory())
        _load_one_users(repository_name, from_repo, users_dir);
    else
        Log::get_instance()->message("accounts.no_users", ll_debug, lc_context) <<
            "Repository " << repository_name << " has accounts_repository_data_location " << dir << ", but no users subdirectory";

    FSPath groups_dir(dir / "groups");
    if (groups_dir.stat().is_directory_or_symlink_to_directory())
        _load_one_groups(repository_name, from_repo, groups_dir);
    else
        Log::get_instance()->message("accounts.no_groups", ll_debug, lc_context) <<
            "Repository " << repository_name << " has accounts_repository_data_location " << dir << ", but no groups subdirectory";

}

void
AccountsRepositoryStore::_load_one_users(
        const RepositoryName & repository_name,
        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & from_repo,
        const FSPath & dir)
{
    for (FSIterator d(dir, { fsio_inode_sort }), d_end ; d != d_end ; ++d)
        if (is_file_with_extension(*d, ".conf", { }))
            _load_one_user(repository_name, from_repo, *d);
        else
            Log::get_instance()->message("accounts.unknown_file", ll_debug, lc_context) <<
                "Don't know what to do with '" << *d << "'";
}

void
AccountsRepositoryStore::_load_one_user(
        const RepositoryName & repository_name,
        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & from_repo,
        const FSPath & filename)
{
    CategoryNamePart cat("user");
    PackageNamePart pkg(strip_trailing_string(filename.basename(), ".conf"));
    QualifiedPackageName qpn(cat + pkg);
    std::string username(stringify(pkg));

    bool masked(false);

    if (_imp->installed)
    {
        if (! getpwnam(username.c_str()))
            return;
    }
    else
    {
        if (getpwnam(username.c_str()))
            masked = true;
    }

    PackageNames::iterator p(_imp->package_names.find(cat));
    if (p == _imp->package_names.end())
        p = _imp->package_names.insert(std::make_pair(cat, std::make_shared<QualifiedPackageNameSet>())).first;

    p->second->insert(qpn);

    IDs::iterator q(_imp->ids.find(qpn));
    if (q == _imp->ids.end())
        q = _imp->ids.insert(std::make_pair(qpn, std::make_shared<PackageIDSequence>())).first;
    else
        q->second = std::make_shared<PackageIDSequence>();

    if (_imp->installed)
        q->second->push_back(std::make_shared<InstalledAccountsID>(_imp->env, qpn, repository_name, true));
    else
        q->second->push_back(std::make_shared<AccountsID>(_imp->env, qpn, repository_name, from_repo, filename, true, masked));
}

void
AccountsRepositoryStore::_load_one_groups(
        const RepositoryName & repository_name,
        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & from_repo,
        const FSPath & dir)
{
    for (FSIterator d(dir, { fsio_inode_sort }), d_end ; d != d_end ; ++d)
        if (is_file_with_extension(*d, ".conf", { }))
            _load_one_group(repository_name, from_repo, *d);
        else
            Log::get_instance()->message("accounts.unknown_file", ll_debug, lc_context) <<
                "Don't know what to do with '" << *d << "'";
}

void
AccountsRepositoryStore::_load_one_group(
        const RepositoryName & repository_name,
        const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > & from_repo,
        const FSPath & filename)
{
    CategoryNamePart cat("group");
    PackageNamePart pkg(strip_trailing_string(filename.basename(), ".conf"));
    QualifiedPackageName qpn(cat + pkg);
    std::string groupname(stringify(pkg));

    bool masked(false);

    if (_imp->installed)
    {
        if (! getgrnam(groupname.c_str()))
            return;
    }
    else
    {
        if (getgrnam(groupname.c_str()))
            masked = true;
    }

    PackageNames::iterator p(_imp->package_names.find(cat));
    if (p == _imp->package_names.end())
        p = _imp->package_names.insert(std::make_pair(cat, std::make_shared<QualifiedPackageNameSet>())).first;

    p->second->insert(qpn);

    IDs::iterator q(_imp->ids.find(qpn));
    if (q == _imp->ids.end())
        q = _imp->ids.insert(std::make_pair(qpn, std::make_shared<PackageIDSequence>())).first;
    else
        q->second = std::make_shared<PackageIDSequence>();

    if (_imp->installed)
        q->second->push_back(std::make_shared<InstalledAccountsID>(_imp->env, qpn, repository_name, false));
    else
        q->second->push_back(std::make_shared<AccountsID>(_imp->env, qpn, repository_name, from_repo, filename, false, masked));
}

bool
AccountsRepositoryStore::has_category_named(const CategoryNamePart & c) const
{
    return _imp->categories->end() != _imp->categories->find(c);
}

bool
AccountsRepositoryStore::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->ids.end() != _imp->ids.find(q);
}

std::shared_ptr<const CategoryNamePartSet>
AccountsRepositoryStore::category_names() const
{
    return _imp->categories;
}

std::shared_ptr<const CategoryNamePartSet>
AccountsRepositoryStore::unimportant_category_names() const
{
    return _imp->categories;
}

std::shared_ptr<const QualifiedPackageNameSet>
AccountsRepositoryStore::package_names(const CategoryNamePart & c) const
{
    PackageNames::iterator p(_imp->package_names.find(c));
    if (_imp->package_names.end() == p)
        return std::make_shared<QualifiedPackageNameSet>();
    else
        return p->second;
}

std::shared_ptr<const PackageIDSequence>
AccountsRepositoryStore::package_ids(const QualifiedPackageName & p) const
{
    IDs::iterator i(_imp->ids.find(p));
    if (_imp->ids.end() == i)
        return std::make_shared<PackageIDSequence>();
    else
        return i->second;
}

template class Pimp<accounts_repository::AccountsRepositoryStore>;

