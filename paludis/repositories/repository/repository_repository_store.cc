/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/repositories/repository/repository_repository_store.hh>
#include <paludis/repositories/repository/repository_id.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <tr1/functional>
#include <tr1/unordered_map>
#include <algorithm>
#include <set>

using namespace paludis;
using namespace paludis::repository_repository;

typedef std::tr1::unordered_map<CategoryNamePart,
        std::tr1::shared_ptr<QualifiedPackageNameSet>,
        Hash<CategoryNamePart> > PackageNames;

typedef std::tr1::unordered_map<QualifiedPackageName,
        std::tr1::shared_ptr<PackageIDSequence>,
        Hash<QualifiedPackageName> > IDs;

namespace paludis
{
    template <>
    struct Implementation<RepositoryRepositoryStore>
    {
        const Environment * const env;
        const RepositoryRepository * const repo;
        mutable std::tr1::shared_ptr<CategoryNamePartSet> categories;
        mutable PackageNames package_names;
        mutable IDs ids;

        Implementation(const Environment * const e, const RepositoryRepository * const r) :
            env(e),
            repo(r),
            categories(new CategoryNamePartSet)
        {
        }
    };
}

RepositoryRepositoryStore::RepositoryRepositoryStore(
        const Environment * const env,
        const RepositoryRepository * const repo) :
    PrivateImplementationPattern<RepositoryRepositoryStore>(new Implementation<RepositoryRepositoryStore>(env, repo))
{
    _populate();
}

RepositoryRepositoryStore::~RepositoryRepositoryStore()
{
}

void
RepositoryRepositoryStore::_populate()
{
    for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
        _populate_one((*r)->name());
}

void
RepositoryRepositoryStore::_populate_one(const RepositoryName & repo_name)
{
    const std::tr1::shared_ptr<RepositoryID> id(new RepositoryID(make_named_values<RepositoryIDParams>(
                    value_for<n::environment>(_imp->env),
                    value_for<n::name>(CategoryNamePart("repository") + PackageNamePart(stringify(repo_name))),
                    value_for<n::repository>(_imp->repo)
                    )));

    _imp->categories->insert(id->name().category());

    PackageNames::iterator p(_imp->package_names.find(id->name().category()));
    if (_imp->package_names.end() == p)
        p = _imp->package_names.insert(std::make_pair(id->name().category(),
                    make_shared_ptr(new QualifiedPackageNameSet))).first;
    p->second->insert(id->name());

    IDs::iterator i(_imp->ids.find(id->name()));
    if (_imp->ids.end() == i)
        i = _imp->ids.insert(std::make_pair(id->name(), make_shared_ptr(new PackageIDSequence))).first;
    i->second->push_back(id);
}

bool
RepositoryRepositoryStore::has_category_named(const CategoryNamePart & c) const
{
    return _imp->categories->end() != _imp->categories->find(c);
}

bool
RepositoryRepositoryStore::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->ids.end() != _imp->ids.find(q);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
RepositoryRepositoryStore::category_names() const
{
    return _imp->categories;
}

std::tr1::shared_ptr<const CategoryNamePartSet>
RepositoryRepositoryStore::unimportant_category_names() const
{
    std::tr1::shared_ptr<CategoryNamePartSet> result(make_shared_ptr(new CategoryNamePartSet));
    result->insert(CategoryNamePart("repository"));
    return result;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
RepositoryRepositoryStore::package_names(const CategoryNamePart & c) const
{
    PackageNames::iterator p(_imp->package_names.find(c));
    if (_imp->package_names.end() == p)
        return make_shared_ptr(new QualifiedPackageNameSet);
    else
        return p->second;
}

std::tr1::shared_ptr<const PackageIDSequence>
RepositoryRepositoryStore::package_ids(const QualifiedPackageName & p) const
{
    IDs::iterator i(_imp->ids.find(p));
    if (_imp->ids.end() == i)
        return make_shared_ptr(new PackageIDSequence);
    else
        return i->second;
}

template class PrivateImplementationPattern<repository_repository::RepositoryRepositoryStore>;

