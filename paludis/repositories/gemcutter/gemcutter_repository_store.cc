/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/gemcutter/gemcutter_repository_store.hh>
#include <paludis/repositories/gemcutter/gemcutter_repository.hh>
#include <paludis/repositories/gemcutter/gemcutter_id.hh>
#include <paludis/repositories/gemcutter/json_things_handle.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/environment.hh>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <set>

using namespace paludis;
using namespace paludis::gemcutter_repository;

typedef std::unordered_map<CategoryNamePart,
        std::shared_ptr<QualifiedPackageNameSet>,
        Hash<CategoryNamePart> > PackageNames;

typedef std::unordered_map<QualifiedPackageName,
        std::shared_ptr<PackageIDSequence>,
        Hash<QualifiedPackageName> > IDs;

namespace paludis
{
    template <>
    struct Imp<GemcutterRepositoryStore>
    {
        const Environment * const env;
        const GemcutterRepository * const repo;
        const FSPath location;
        std::shared_ptr<CategoryNamePartSet> categories;
        PackageNames package_names;
        IDs ids;

        Imp(const Environment * const e, const GemcutterRepository * const r, const FSPath & l) :
            env(e),
            repo(r),
            location(l),
            categories(std::make_shared<CategoryNamePartSet>())
        {
        }
    };
}

GemcutterRepositoryStore::GemcutterRepositoryStore(
        const Environment * const env,
        const GemcutterRepository * const repo,
        const FSPath & location) :
    _imp(env, repo, location)
{
    _populate();
}

GemcutterRepositoryStore::~GemcutterRepositoryStore()
{
}

void
GemcutterRepositoryStore::_populate()
{
    Context context("When populating ::" + stringify(_imp->repo->name()) + ":");

    FSPath all_gems(_imp->location / "all-gems.json");
    if (! all_gems.stat().exists())
    {
        Log::get_instance()->message("gemcutter_repository.no_all_gems_file", ll_warning, lc_context)
            << "File '" << all_gems << "' does not exist, and it should, unless you have just created "
            << "the ::" << stringify(_imp->repo->name()) << " repository and not yet synced it";
        return;
    }

    try
    {
        JSONThingsHandle::get_instance()->parse_all_gems(
                all_gems,
                std::bind(&GemcutterRepositoryStore::_populate_one, this, std::placeholders::_1));
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message("gemcutter_repository.not_available", ll_warning, lc_context)
            << "Not loading '" << all_gems << "' for ::" << stringify(_imp->repo->name()) << " due to "
            << "exception '" << e.backtrace(": ") << ": " << e.message() << "' (" << e.what() << ")";

        _imp->categories->clear();
        _imp->package_names.clear();
        _imp->ids.clear();

        return;
    }
}

void
GemcutterRepositoryStore::_populate_one(const GemJSONInfo & info)
{
    Context context("When populating ::" + stringify(_imp->repo->name()) + " " +
            stringify(info.name()) + " " + stringify(info.version()) + ":");

    const std::shared_ptr<GemcutterID> id(std::make_shared<GemcutterID>(make_named_values<GemcutterIDParams>(
                    n::environment() = _imp->env,
                    n::info() = info,
                    n::repository() = _imp->repo->name()
                    )));

    _imp->categories->insert(id->name().category());

    PackageNames::iterator p(_imp->package_names.find(id->name().category()));
    if (_imp->package_names.end() == p)
        p = _imp->package_names.insert(std::make_pair(id->name().category(),
                    std::make_shared<QualifiedPackageNameSet>())).first;
    p->second->insert(id->name());

    IDs::iterator i(_imp->ids.find(id->name()));
    if (_imp->ids.end() == i)
        i = _imp->ids.insert(std::make_pair(id->name(), std::make_shared<PackageIDSequence>())).first;
    i->second->push_back(id);
}

bool
GemcutterRepositoryStore::has_category_named(const CategoryNamePart & c) const
{
    return _imp->categories->end() != _imp->categories->find(c);
}

bool
GemcutterRepositoryStore::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->ids.end() != _imp->ids.find(q);
}

std::shared_ptr<const CategoryNamePartSet>
GemcutterRepositoryStore::category_names() const
{
    return _imp->categories;
}

std::shared_ptr<const CategoryNamePartSet>
GemcutterRepositoryStore::unimportant_category_names() const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());
    return result;
}

std::shared_ptr<const QualifiedPackageNameSet>
GemcutterRepositoryStore::package_names(const CategoryNamePart & c) const
{
    auto p(_imp->package_names.find(c));
    if (_imp->package_names.end() == p)
        return std::make_shared<QualifiedPackageNameSet>();
    else
        return p->second;
}

std::shared_ptr<const PackageIDSequence>
GemcutterRepositoryStore::package_ids(const QualifiedPackageName & p) const
{
    auto i(_imp->ids.find(p));
    if (_imp->ids.end() == i)
        return std::make_shared<PackageIDSequence>();
    else
        return i->second;
}

template class Pimp<gemcutter_repository::GemcutterRepositoryStore>;

