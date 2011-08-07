/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/unwritten/unwritten_repository_store.hh>
#include <paludis/repositories/unwritten/unwritten_repository_file.hh>
#include <paludis/repositories/unwritten/unwritten_id.hh>
#include <paludis/repositories/unwritten/unwritten_mask.hh>
#include <paludis/repositories/unwritten/unwritten_repository.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/config_file.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/environment.hh>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <set>

using namespace paludis;
using namespace paludis::unwritten_repository;

typedef std::unordered_map<CategoryNamePart,
        std::shared_ptr<QualifiedPackageNameSet>,
        Hash<CategoryNamePart> > PackageNames;

typedef std::unordered_map<QualifiedPackageName,
        std::shared_ptr<PackageIDSequence>,
        Hash<QualifiedPackageName> > IDs;

namespace paludis
{
    template <>
    struct Imp<UnwrittenRepositoryStore>
    {
        const UnwrittenRepository * const repo;
        mutable std::shared_ptr<CategoryNamePartSet> categories;
        mutable PackageNames package_names;
        mutable IDs ids;

        std::string entry_suffix;
        bool is_graveyard;

        Imp(const UnwrittenRepository * const r) :
            repo(r),
            categories(std::make_shared<CategoryNamePartSet>()),
            is_graveyard(false)
        {
        }
    };
}

UnwrittenRepositoryStore::UnwrittenRepositoryStore(
        const Environment * const env,
        const UnwrittenRepository * const repo,
        const FSPath & f) :
    _imp(repo)
{
    UnwrittenRepositoryInformation info(repository_information(f));
    _imp->entry_suffix = info.entry_suffix();

    if (info.role() == "unwritten")
    {
    }
    else if (info.role() == "graveyard")
        _imp->is_graveyard = true;
    else
        Log::get_instance()->message("unwritten_repository.unknown_role", ll_warning, lc_context)
            << "Role '" << info.role() << "' unknown for '" << f << "'";

    if (f.stat().exists())
        _populate(env, f);
}

UnwrittenRepositoryStore::~UnwrittenRepositoryStore()
{
}

void
UnwrittenRepositoryStore::_populate(const Environment * const env, const FSPath & f)
{
    Context context("When populating UnwrittenRepository from directory '" + stringify(f) + "':");

    using namespace std::placeholders;
    std::for_each(FSIterator(f, { fsio_inode_sort }), FSIterator(), std::bind(
                &UnwrittenRepositoryStore::_populate_one, this, env, _1));
}

void
UnwrittenRepositoryStore::_populate_one(const Environment * const env, const FSPath & f)
{
    if (! is_file_with_extension(f, _imp->entry_suffix, { }))
        return;

    Context context("When populating UnwrittenRepository from file '" + stringify(f) + "':");

    UnwrittenRepositoryFile file(f);

    std::shared_ptr<Mask> mask(_imp->is_graveyard ?
            std::shared_ptr<Mask>(std::make_shared<GraveyardMask>()) :
            std::shared_ptr<Mask>(std::make_shared<UnwrittenMask>()));

    QualifiedPackageName old_name("x/x");
    std::shared_ptr<QualifiedPackageNameSet> pkgs;
    std::shared_ptr<PackageIDSequence> ids;
    for (UnwrittenRepositoryFile::ConstIterator i(file.begin()), i_end(file.end()) ;
            i != i_end ; ++i)
    {
        if (old_name.category() != (*i).name().category())
        {
            _imp->categories->insert((*i).name().category());
            PackageNames::iterator p(_imp->package_names.find((*i).name().category()));
            if (_imp->package_names.end() == p)
                p = _imp->package_names.insert(std::make_pair((*i).name().category(),
                            std::make_shared<QualifiedPackageNameSet>())).first;
            pkgs = p->second;
        }

        if (old_name != (*i).name())
        {
            pkgs->insert((*i).name());
            IDs::iterator p(_imp->ids.find((*i).name()));
            if (_imp->ids.end() == p)
                p = _imp->ids.insert(std::make_pair((*i).name(),
                            std::make_shared<PackageIDSequence>())).first;

            ids = p->second;
        }

        ids->push_back(std::make_shared<UnwrittenID>(make_named_values<UnwrittenIDParams>(
                            n::added_by() = (*i).added_by(),
                            n::bug_ids() = (*i).bug_ids(),
                            n::comment() = (*i).comment(),
                            n::commit_id() = (*i).commit_id(),
                            n::description() = (*i).description(),
                            n::environment() = env,
                            n::homepage() = (*i).homepage(),
                            n::mask() = mask,
                            n::name() = (*i).name(),
                            n::remote_ids() = (*i).remote_ids(),
                            n::removed_by() = (*i).removed_by(),
                            n::removed_from() = (*i).removed_from(),
                            n::repository() = _imp->repo->name(),
                            n::slot() = (*i).slot(),
                            n::version() = (*i).version()
                        )));

        old_name = (*i).name();
    }
}

bool
UnwrittenRepositoryStore::has_category_named(const CategoryNamePart & c) const
{
    return _imp->categories->end() != _imp->categories->find(c);
}

bool
UnwrittenRepositoryStore::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->ids.end() != _imp->ids.find(q);
}

std::shared_ptr<const CategoryNamePartSet>
UnwrittenRepositoryStore::category_names() const
{
    return _imp->categories;
}

std::shared_ptr<const CategoryNamePartSet>
UnwrittenRepositoryStore::unimportant_category_names() const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());
    result->insert(CategoryNamePart("virtual"));
    return result;
}

std::shared_ptr<const QualifiedPackageNameSet>
UnwrittenRepositoryStore::package_names(const CategoryNamePart & c) const
{
    PackageNames::iterator p(_imp->package_names.find(c));
    if (_imp->package_names.end() == p)
        return std::make_shared<QualifiedPackageNameSet>();
    else
        return p->second;
}

std::shared_ptr<const PackageIDSequence>
UnwrittenRepositoryStore::package_ids(const QualifiedPackageName & p) const
{
    IDs::iterator i(_imp->ids.find(p));
    if (_imp->ids.end() == i)
        return std::make_shared<PackageIDSequence>();
    else
        return i->second;
}

UnwrittenRepositoryInformation
UnwrittenRepositoryStore::repository_information(const FSPath & p)
{
    UnwrittenRepositoryInformation result(make_named_values<UnwrittenRepositoryInformation>(
                n::entry_suffix() = ".conf",
                n::name() = p.basename(),
                n::role() = "unwritten"
                ));

    if ((p / "repository.conf").stat().exists())
    {
        KeyValueConfigFile file(p / "repository.conf", { },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

        if (! file.get("entry_suffix").empty())
            result.entry_suffix() = "." + file.get("entry_suffix");
        else
            result.entry_suffix() = ".unwritten";

        if (! file.get("name").empty())
            result.name() = file.get("name");

        if (! file.get("role").empty())
            result.role() = file.get("role");
    }

    return result;
}

namespace paludis
{
    template class Pimp<unwritten_repository::UnwrittenRepositoryStore>;
}
