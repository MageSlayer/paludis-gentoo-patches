/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/unavailable/unavailable_repository_store.hh>
#include <paludis/repositories/unavailable/unavailable_repository_file.hh>
#include <paludis/repositories/unavailable/unavailable_package_id.hh>
#include <paludis/repositories/unavailable/unavailable_repository_id.hh>
#include <paludis/repositories/unavailable/unavailable_repository_dependencies_key.hh>
#include <paludis/repositories/unavailable/unavailable_mask.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <set>

using namespace paludis;
using namespace paludis::unavailable_repository;

typedef std::unordered_map<CategoryNamePart,
        std::shared_ptr<QualifiedPackageNameSet>,
        Hash<CategoryNamePart> > PackageNames;

typedef std::unordered_map<QualifiedPackageName,
        std::shared_ptr<PackageIDSequence>,
        Hash<QualifiedPackageName> > IDs;

namespace paludis
{
    template <>
    struct Implementation<UnavailableRepositoryStore>
    {
        const UnavailableRepository * const repo;
        mutable std::shared_ptr<CategoryNamePartSet> categories;
        mutable PackageNames package_names;
        mutable IDs ids;

        std::set<std::string> seen_repo_names;

        Implementation(const UnavailableRepository * const r) :
            repo(r),
            categories(new CategoryNamePartSet)
        {
        }
    };
}

UnavailableRepositoryStore::UnavailableRepositoryStore(
        const Environment * const env,
        const UnavailableRepository * const repo,
        const FSEntry & f) :
    PrivateImplementationPattern<UnavailableRepositoryStore>(repo)
{
    _populate(env, f);
}

UnavailableRepositoryStore::~UnavailableRepositoryStore()
{
}

void
UnavailableRepositoryStore::_populate(const Environment * const env, const FSEntry & f)
{
    Context context("When populating UnavailableRepository from directory '" + stringify(f) + "':");

    using namespace std::placeholders;
    std::for_each(DirIterator(f), DirIterator(), std::bind(
                &UnavailableRepositoryStore::_populate_one, this, env, _1));
}

void
UnavailableRepositoryStore::_populate_one(const Environment * const env, const FSEntry & f)
{
    if (! is_file_with_extension(f, ".repository", IsFileWithOptions()))
        return;

    Context context("When populating UnavailableRepository from file '" + stringify(f) + "':");

    UnavailableRepositoryFile file(f);

    if (env->package_database()->has_repository_named(RepositoryName(file.repo_name())))
    {
        Log::get_instance()->message("unavailable_repository.file.present", ll_debug, lc_context)
            << "Skipping file '" << f << "' due to present repo_name '" << file.repo_name() << "'";
        return;
    }

    if (! _imp->seen_repo_names.insert(file.repo_name()).second)
    {
        Log::get_instance()->message("unavailable_repository.file.duplicate", ll_warning, lc_context)
            << "Skipping file '" << f << "' due to duplicate repo_name '" << file.repo_name() << "'";
        return;
    }

    std::shared_ptr<MetadataValueKey<std::string> > repository_homepage, repository_description,
        repository_format, repository_sync;
    if (! file.homepage().empty())
        repository_homepage.reset(new LiteralMetadataValueKey<std::string>(
                "REPOSITORY_HOMEPAGE", "Repository homepage", mkt_normal, file.homepage()));
    if (! file.description().empty())
        repository_description.reset(new LiteralMetadataValueKey<std::string>(
                "REPOSITORY_DESCRIPTION", "Repository description", mkt_normal, file.description()));
    if (! file.repo_format().empty())
        repository_format.reset(new LiteralMetadataValueKey<std::string>(
                "REPOSITORY_FORMAT", "Repository format", mkt_normal, file.repo_format()));
    if (! file.sync().empty())
        repository_sync.reset(new LiteralMetadataValueKey<std::string>(
                "REPOSITORY_SYNC", "Repository sync", mkt_normal, file.sync()));

    {
        std::shared_ptr<Mask> mask(new UnavailableMask);
        std::shared_ptr<Set<std::string> > from_repositories_set(new Set<std::string>);
        from_repositories_set->insert(file.repo_name());
        std::shared_ptr<MetadataCollectionKey<Set<std::string> > > from_repositories(
                new LiteralMetadataStringSetKey("OWNING_REPOSITORY", "Owning repository",
                    mkt_significant, from_repositories_set));

        QualifiedPackageName old_name("x/x");
        std::shared_ptr<QualifiedPackageNameSet> pkgs;
        std::shared_ptr<PackageIDSequence> ids;
        for (UnavailableRepositoryFile::ConstIterator i(file.begin()), i_end(file.end()) ;
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

            ids->push_back(std::make_shared<UnavailablePackageID>(make_named_values<UnavailablePackageIDParams>(
                                n::description() = (*i).description(),
                                n::environment() = env,
                                n::from_repositories() = from_repositories,
                                n::mask() = mask,
                                n::name() = (*i).name(),
                                n::repository() = _imp->repo,
                                n::repository_description() = repository_description,
                                n::repository_homepage() = repository_homepage,
                                n::slot() = (*i).slot(),
                                n::version() = (*i).version()
                            )));

            old_name = (*i).name();
        }
    }

    if (file.autoconfigurable())
    {
        const std::shared_ptr<NoConfigurationInformationMask> no_configuration_mask(new NoConfigurationInformationMask);
        std::shared_ptr<UnavailableRepositoryDependenciesKey> deps;
        if (! file.dependencies().empty())
            deps.reset(new UnavailableRepositoryDependenciesKey(env, "dependencies", "Dependencies", mkt_dependencies,
                        file.dependencies()));

        const std::shared_ptr<UnavailableRepositoryID> id(new UnavailableRepositoryID(
                    make_named_values<UnavailableRepositoryIDParams>(
                        n::dependencies() = deps,
                        n::description() = repository_description,
                        n::environment() = env,
                        n::format() = repository_format,
                        n::homepage() = repository_homepage,
                        n::mask() = repository_sync && repository_format ?
                            make_null_shared_ptr() : no_configuration_mask,
                        n::name() = CategoryNamePart("repository") + PackageNamePart(file.repo_name()),
                        n::repository() = _imp->repo,
                        n::sync() = repository_sync
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
}

bool
UnavailableRepositoryStore::has_category_named(const CategoryNamePart & c) const
{
    return _imp->categories->end() != _imp->categories->find(c);
}

bool
UnavailableRepositoryStore::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->ids.end() != _imp->ids.find(q);
}

std::shared_ptr<const CategoryNamePartSet>
UnavailableRepositoryStore::category_names() const
{
    return _imp->categories;
}

std::shared_ptr<const CategoryNamePartSet>
UnavailableRepositoryStore::unimportant_category_names() const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());
    result->insert(CategoryNamePart("virtual"));
    result->insert(CategoryNamePart("repository"));
    return result;
}

std::shared_ptr<const QualifiedPackageNameSet>
UnavailableRepositoryStore::package_names(const CategoryNamePart & c) const
{
    PackageNames::iterator p(_imp->package_names.find(c));
    if (_imp->package_names.end() == p)
        return std::make_shared<QualifiedPackageNameSet>();
    else
        return p->second;
}

std::shared_ptr<const PackageIDSequence>
UnavailableRepositoryStore::package_ids(const QualifiedPackageName & p) const
{
    IDs::iterator i(_imp->ids.find(p));
    if (_imp->ids.end() == i)
        return std::make_shared<PackageIDSequence>();
    else
        return i->second;
}

template class PrivateImplementationPattern<unavailable_repository::UnavailableRepositoryStore>;

