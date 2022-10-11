/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
 * Copyright (c) 2006 Danny van Dyk
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

#include <paludis/repositories/e/traditional_layout.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/file_suffixes.hh>
#include <paludis/repositories/e/traditional_mask_store.hh>

#include <paludis/util/config_file.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/map.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/active_object_ptr.hh>
#include <paludis/util/deferred_construction_ptr.hh>

#include <paludis/package_id.hh>
#include <paludis/choice.hh>
#include <paludis/literal_metadata_key.hh>

#include <functional>
#include <unordered_map>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::unordered_map<CategoryNamePart, bool, Hash<CategoryNamePart> > CategoryMap;
typedef std::unordered_map<QualifiedPackageName, bool, Hash<QualifiedPackageName> > PackagesMap;
typedef std::unordered_map<QualifiedPackageName, std::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName>  > IDMap;

namespace
{
    std::shared_ptr<TraditionalMaskStore> make_mask_store(
            const Environment * const env,
            const RepositoryName & repo_name,
            const std::shared_ptr<const FSPathSequence> & f,
            const EAPIForFileFunction & e)
    {
        return std::make_shared<TraditionalMaskStore>(env, repo_name, f, e);
    }
}

namespace paludis
{
    template<>
    struct Imp<TraditionalLayout>
    {
        const ERepository * const repository;
        const FSPath tree_root;

        mutable std::recursive_mutex big_nasty_mutex;

        mutable bool has_category_names;
        mutable CategoryMap category_names;
        mutable PackagesMap package_names;
        mutable IDMap ids;

        mutable std::shared_ptr<CategoryNamePartSet> category_names_collection;

        std::shared_ptr<FSPathSequence> arch_list_files;
        std::shared_ptr<FSPathSequence> repository_mask_files;
        std::shared_ptr<FSPathSequence> profiles_desc_files;
        std::shared_ptr<FSPathSequence> mirror_files;
        std::shared_ptr<FSPathSequence> info_packages_files;
        std::shared_ptr<FSPathSequence> info_variables_files;
        std::shared_ptr<UseDescFileInfoSequence> use_desc_files;

        ActiveObjectPtr<DeferredConstructionPtr<std::shared_ptr<TraditionalMaskStore> > > repository_mask_store;

        Imp(
                const Environment * const env,
                const ERepository * const r,
                const FSPath & t) :
            repository(r),
            tree_root(t),
            has_category_names(false),
            arch_list_files(std::make_shared<FSPathSequence>()),
            repository_mask_files(std::make_shared<FSPathSequence>()),
            profiles_desc_files(std::make_shared<FSPathSequence>()),
            mirror_files(std::make_shared<FSPathSequence>()),
            info_packages_files(std::make_shared<FSPathSequence>()),
            info_variables_files(std::make_shared<FSPathSequence>()),
            use_desc_files(std::make_shared<UseDescFileInfoSequence>()),
            repository_mask_store(DeferredConstructionPtr<std::shared_ptr<TraditionalMaskStore> > (
                        std::bind(&make_mask_store, env, r->name(),
                            repository_mask_files, EAPIForFileFunction(std::bind(std::mem_fn(&ERepository::eapi_for_file), r, std::placeholders::_1)))))
        {
        }
    };
}

TraditionalLayout::TraditionalLayout(
        const Environment * const env,
        const ERepository * const repo,
        const FSPath & tree_root,
        const std::shared_ptr<const FSPathSequence> & f) :
    Layout(f),
    _imp(env, repo, tree_root)
{
    if (master_repositories_locations())
    {
        for (FSPathSequence::ConstIterator l(master_repositories_locations()->begin()), l_end(master_repositories_locations()->end()) ;
                l != l_end ; ++l)
        {
            _imp->arch_list_files->push_back(*l / "profiles" / "arch.list");
            _imp->repository_mask_files->push_back(*l / "profiles" / "package.mask");
            _imp->profiles_desc_files->push_back(*l / "profiles" / "profiles.desc");
            _imp->mirror_files->push_back(*l / "profiles" / "thirdpartymirrors");
            _imp->info_variables_files->push_back(*l / "profiles" / "info_vars");

            _imp->use_desc_files->push_back(std::make_pair(*l / "profiles" / "use.desc", ChoicePrefixName("")));
            _imp->use_desc_files->push_back(std::make_pair(*l / "profiles" / "use.local.desc", ChoicePrefixName("")));
            FSPath descs(*l / "profiles" / "desc");
            if (descs.stat().is_directory_or_symlink_to_directory())
            {
                for (FSIterator d(descs, { }), d_end ; d != d_end ; ++d)
                {
                    if (! is_file_with_extension(*d, ".desc", { }))
                        continue;
                    _imp->use_desc_files->push_back(std::make_pair(*d, ChoicePrefixName(strip_trailing_string(d->basename(), ".desc"))));
                }
            }
        }
    }

    _imp->arch_list_files->push_back(_imp->tree_root / "profiles" / "arch.list");
    _imp->repository_mask_files->push_back(_imp->tree_root / "profiles" / "package.mask");
    _imp->profiles_desc_files->push_back(_imp->tree_root / "profiles" / "profiles.desc");
    _imp->mirror_files->push_back(_imp->tree_root / "profiles" / "thirdpartymirrors");
    _imp->info_variables_files->push_back(_imp->tree_root / "profiles" / "info_vars");
    _imp->info_packages_files->push_back(_imp->tree_root / "profiles" / "info_pkgs");

    _imp->use_desc_files->push_back(std::make_pair(_imp->tree_root / "profiles" / "use.desc", ChoicePrefixName("")));
    _imp->use_desc_files->push_back(std::make_pair(_imp->tree_root / "profiles" / "use.local.desc", ChoicePrefixName("")));
    FSPath descs(_imp->tree_root / "profiles" / "desc");
    if (descs.stat().is_directory_or_symlink_to_directory())
    {
        for (FSIterator d(descs, { }), d_end ; d != d_end ; ++d)
        {
            if (! is_file_with_extension(*d, ".desc", { }))
                continue;
            _imp->use_desc_files->push_back(std::make_pair(*d, ChoicePrefixName(strip_trailing_string(d->basename(), ".desc"))));
        }
    }
}

TraditionalLayout::~TraditionalLayout() = default;

FSPath
TraditionalLayout::categories_file() const
{
    return _imp->tree_root / "profiles" / "categories";
}

void
TraditionalLayout::need_category_names() const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->big_nasty_mutex);

    if (_imp->has_category_names)
        return;

    Context context("When loading category names for " + stringify(_imp->repository->name()) + ":");

    Log::get_instance()->message("e.traditional_layout.need_category_names", ll_debug, lc_context) << "need_category_names";

    bool found_one(false);

    std::list<FSPath> cats_list;
    if (_imp->repository->params().master_repositories())
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
            cats_list.push_back((*e)->layout()->categories_file());
    cats_list.push_back(categories_file());

    for (std::list<FSPath>::const_iterator i(cats_list.begin()), i_end(cats_list.end()) ;
            i != i_end ; ++i)
    {
        if (! i->stat().exists())
            continue;

        LineConfigFile cats(*i, { lcfo_disallow_continuations });

        for (LineConfigFile::ConstIterator line(cats.begin()), line_end(cats.end()) ;
                line != line_end ; ++line)
        {
            try
            {
                _imp->category_names.insert(std::make_pair(CategoryNamePart(*line), false));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message("e.traditional_layout.categories.failure", ll_warning, lc_context)
                    << "Skipping line '" << *line << "' in '" << *i << "' due to exception '"
                    << e.message() << "' ('" << e.what() << ")";
            }
        }

        found_one = true;
    }

    if (! found_one)
    {
        Log::get_instance()->message("e.traditional_layout.categories.no_file", ll_qa, lc_context)
            << "No categories file for repository at '" << _imp->tree_root << "', faking it";
        for (FSIterator d(_imp->tree_root, { fsio_inode_sort, fsio_deref_symlinks_for_wants, fsio_want_directories }), d_end ; d != d_end ; ++d)
        {
            std::string n(d->basename());
            if (n == "CVS" || n == "distfiles" || n == "scripts" || n == "eclass" || n == "licenses"
                    || n == "packages")
                continue;

            try
            {
                _imp->category_names.insert(std::make_pair(CategoryNamePart(n), false));
            }
            catch (const NameError &)
            {
            }
        }
    }

    _imp->has_category_names = true;
}

void
TraditionalLayout::need_package_ids(const QualifiedPackageName & n) const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->big_nasty_mutex);

    using namespace std::placeholders;

    if (_imp->package_names[n])
        return;

    Context context("When loading versions for '" + stringify(n) + "' in "
            + stringify(_imp->repository->name()) + ":");

    std::shared_ptr<PackageIDSequence> v(std::make_shared<PackageIDSequence>());

    FSPath path(_imp->tree_root / stringify(n.category()) / stringify(n.package()));

    for (FSIterator e(path, { fsio_inode_sort }), e_end ; e != e_end ; ++e)
    {
        if (! FileSuffixes::get_instance()->is_package_file(n, *e))
            continue;

        try
        {
            std::shared_ptr<const PackageID> id(_imp->repository->make_id(n, *e));
            if (indirect_iterator(v->end()) != std::find_if(indirect_iterator(v->begin()), indirect_iterator(v->end()),
                        std::bind(std::equal_to<>(), id->version(), std::bind(std::mem_fn(&PackageID::version), _1))))
                Log::get_instance()->message("e.traditional_layout.id.duplicate", ll_warning, lc_context)
                    << "Ignoring entry '" << *e << "' for '" << n << "' in repository '" << _imp->repository->name()
                    << "' because another equivalent version already exists";
            else
                v->push_back(id);
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & ee)
        {
            Log::get_instance()->message("e.traditional_layout.id.failure", ll_warning, lc_context)
                << "Skipping entry '" << *e << "' for '" << n << "' in repository '"
                << _imp->repository->name() << "' due to exception '" << ee.message() << "' ("
                << ee.what() << ")'";
        }
    }

    _imp->ids.insert(std::make_pair(n, v));
    _imp->package_names[n] = true;
}

bool
TraditionalLayout::has_category_named(const CategoryNamePart & c) const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->big_nasty_mutex);

    Context context("When checking for category '" + stringify(c) + "' in '" + stringify(_imp->repository->name()) + "':");

    need_category_names();
    return _imp->category_names.end() != _imp->category_names.find(c);
}

bool
TraditionalLayout::has_package_named(const QualifiedPackageName & q) const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->big_nasty_mutex);

    Context context("When checking for package '" + stringify(q) + "' in '" + stringify(_imp->repository->name()) + ":");

    need_category_names();

    CategoryMap::iterator cat_iter(_imp->category_names.find(q.category()));

    if (_imp->category_names.end() == cat_iter)
        return false;

    if (cat_iter->second)
    {
        /* this category's package names are fully loaded */
        return _imp->package_names.find(q) != _imp->package_names.end();
    }
    else
    {
        /* package names are only partially loaded or not loaded */
        if (_imp->package_names.find(q) != _imp->package_names.end())
            return true;

        FSPath fs(_imp->tree_root);
        fs /= stringify(q.category());
        fs /= stringify(q.package());
        if (! fs.stat().is_directory_or_symlink_to_directory())
            return false;
        _imp->package_names.insert(std::make_pair(q, false));
        return true;
    }
}

void
TraditionalLayout::need_category_names_collection() const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->big_nasty_mutex);

    if (_imp->category_names_collection)
        return;

    need_category_names();

    _imp->category_names_collection = std::make_shared<CategoryNamePartSet>();
    std::transform(_imp->category_names.begin(), _imp->category_names.end(),
            _imp->category_names_collection->inserter(),
            std::mem_fn(&std::pair<const CategoryNamePart, bool>::first));
}

std::shared_ptr<const CategoryNamePartSet>
TraditionalLayout::category_names() const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->big_nasty_mutex);

    Context context("When fetching category names in " + stringify(stringify(_imp->repository->name())) + ":");

    need_category_names_collection();
    return _imp->category_names_collection;
}

std::shared_ptr<const QualifiedPackageNameSet>
TraditionalLayout::package_names(const CategoryNamePart & c) const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->big_nasty_mutex);

    using namespace std::placeholders;

    /* this isn't particularly fast because it isn't called very often. avoid
     * changing the data structures used to make this faster at the expense of
     * slowing down single item queries. */

    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(_imp->repository->name()) + ":");

    need_category_names();

    if (_imp->category_names.end() == _imp->category_names.find(c))
        return std::make_shared<QualifiedPackageNameSet>();

    if ((_imp->tree_root / stringify(c)).stat().is_directory_or_symlink_to_directory())
        for (FSIterator d(_imp->tree_root / stringify(c), { fsio_inode_sort, fsio_deref_symlinks_for_wants, fsio_want_directories }), d_end ; d != d_end ; ++d)
        {
            try
            {
                if (d->basename() == "CVS")
                   continue;

                _imp->package_names.insert(std::make_pair(c + PackageNamePart(d->basename()), false));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message("e.traditional_layout.packages.failure", ll_warning, lc_context) << "Skipping entry '" <<
                    d->basename() << "' in category '" << c << "' in repository '" <<
                    stringify(_imp->repository->name()) << "' (" << e.message() << ")";
            }
        }

    _imp->category_names[c] = true;

    std::shared_ptr<QualifiedPackageNameSet> result(std::make_shared<QualifiedPackageNameSet>());

    for (PackagesMap::const_iterator p(_imp->package_names.begin()), p_end(_imp->package_names.end()) ;
            p != p_end ; ++p)
        if (p->first.category() == c)
            result->insert(p->first);

    return result;
}

std::shared_ptr<const PackageIDSequence>
TraditionalLayout::package_ids(const QualifiedPackageName & n) const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->big_nasty_mutex);

    Context context("When fetching versions of '" + stringify(n) + "' in " + stringify(_imp->repository->name()) + ":");

    if (has_package_named(n))
    {
        need_package_ids(n);
        return _imp->ids.find(n)->second;
    }
    else
        return std::make_shared<PackageIDSequence>();
}

const std::shared_ptr<const FSPathSequence>
TraditionalLayout::info_packages_files() const
{
    return _imp->info_packages_files;
}

const std::shared_ptr<const FSPathSequence>
TraditionalLayout::info_variables_files() const
{
    return _imp->info_variables_files;
}

FSPath
TraditionalLayout::package_directory(const QualifiedPackageName & qpn) const
{
    return _imp->tree_root / stringify(qpn.category()) / stringify(qpn.package());
}

FSPath
TraditionalLayout::category_directory(const CategoryNamePart & cat) const
{
    return _imp->tree_root / stringify(cat);
}

std::shared_ptr<const FSPathSequence>
TraditionalLayout::arch_list_files() const
{
    return _imp->arch_list_files;
}

std::shared_ptr<const FSPathSequence>
TraditionalLayout::repository_mask_files() const
{
    return _imp->repository_mask_files;
}

std::shared_ptr<const FSPathSequence>
TraditionalLayout::profiles_desc_files() const
{
    return _imp->profiles_desc_files;
}

std::shared_ptr<const FSPathSequence>
TraditionalLayout::mirror_files() const
{
    return _imp->mirror_files;
}

std::shared_ptr<const UseDescFileInfoSequence>
TraditionalLayout::use_desc_files() const
{
    return _imp->use_desc_files;
}

FSPath
TraditionalLayout::profiles_base_dir() const
{
    if (master_repositories_locations() && ! master_repositories_locations()->empty())
        return *master_repositories_locations()->begin() / "profiles";
    else
        return _imp->tree_root / "profiles";
}

std::shared_ptr<const FSPathSequence>
TraditionalLayout::exlibsdirs(const QualifiedPackageName & q) const
{
    std::shared_ptr<FSPathSequence> result(std::make_shared<FSPathSequence>());

    std::shared_ptr<const FSPathSequence> global(exlibsdirs_global());
    std::copy(global->begin(), global->end(), result->back_inserter());

    std::shared_ptr<const FSPathSequence> category(exlibsdirs_category(q.category()));
    std::copy(category->begin(), category->end(), result->back_inserter());

    std::shared_ptr<const FSPathSequence> package(exlibsdirs_package(q));
    std::copy(package->begin(), package->end(), result->back_inserter());

    return result;
}

std::shared_ptr<const FSPathSequence>
TraditionalLayout::exlibsdirs_global() const
{
    std::shared_ptr<FSPathSequence> result(std::make_shared<FSPathSequence>());

    if (_imp->repository->params().master_repositories())
    {
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
        {
            std::shared_ptr<const FSPathSequence> master((*e)->layout()->exlibsdirs_global());
            std::copy(master->begin(), master->end(), result->back_inserter());
        }
    }
    result->push_back(_imp->tree_root / "exlibs");

    return result;
}

std::shared_ptr<const FSPathSequence>
TraditionalLayout::exlibsdirs_category(const CategoryNamePart & c) const
{
    std::shared_ptr<FSPathSequence> result(std::make_shared<FSPathSequence>());

    if (_imp->repository->params().master_repositories())
    {
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
        {
            std::shared_ptr<const FSPathSequence> master((*e)->layout()->exlibsdirs_category(c));
            std::copy(master->begin(), master->end(), result->back_inserter());
        }
    }
    result->push_back(category_directory(c) / "exlibs");

    return result;
}

std::shared_ptr<const FSPathSequence>
TraditionalLayout::exlibsdirs_package(const QualifiedPackageName & q) const
{
    std::shared_ptr<FSPathSequence> result(std::make_shared<FSPathSequence>());

    if (_imp->repository->params().master_repositories())
    {
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
        {
            std::shared_ptr<const FSPathSequence> master((*e)->layout()->exlibsdirs_package(q));
            std::copy(master->begin(), master->end(), result->back_inserter());
        }
    }
    result->push_back(package_directory(q));

    return result;
}

std::shared_ptr<const FSPathSequence>
TraditionalLayout::licenses_dirs() const
{
    std::shared_ptr<FSPathSequence> result(std::make_shared<FSPathSequence>());

    if (_imp->repository->params().master_repositories())
    {
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
        {
            std::shared_ptr<const FSPathSequence> master((*e)->layout()->licenses_dirs());
           std::copy(master->begin(), master->end(), result->back_inserter());
        }
    }
    result->push_back(_imp->tree_root / "licenses");

    return result;
}

namespace
{
    void aux_files_helper(const FSPath & d,
            std::shared_ptr<Map<FSPath, std::string, FSPathComparator> > & m,
            const QualifiedPackageName & qpn)
    {
        if (! d.stat().exists())
            return;

        std::list<FSPath> files((FSIterator(d, { fsio_inode_sort })), FSIterator());
        for (auto & file : files)
        {
            FSStat f_stat(file.stat());
            if (f_stat.is_directory())
            {
                if ("CVS" != file.basename())
                    aux_files_helper(file, m, qpn);
            }
            else
            {
                if (! f_stat.is_regular_file())
                    continue;
                if (is_file_with_prefix_extension(file,
                            ("digest-"+stringify(qpn.package())), "",
                            { }))
                    continue;
                m->insert(file, "AUX");
            }
        }
    }
}

std::shared_ptr<Map<FSPath, std::string, FSPathComparator> >
TraditionalLayout::manifest_files(const QualifiedPackageName & qpn, const FSPath & package_dir) const
{
    auto result(std::make_shared<Map<FSPath, std::string, FSPathComparator>>());

    std::list<FSPath> package_files((FSIterator(package_dir, { fsio_inode_sort, fsio_want_regular_files })), FSIterator());
    for (auto & package_file : package_files)
    {
        if (package_file.basename() == "Manifest")
            continue;

        std::string file_type("MISC");
        if (FileSuffixes::get_instance()->is_package_file(qpn, package_file))
            file_type = FileSuffixes::get_instance()->get_package_file_manifest_key(package_file, qpn);

        result->insert(package_file, file_type);
    }

    aux_files_helper((package_dir / "files"), result, qpn);

    return result;
}

FSPath
TraditionalLayout::sync_filter_file() const
{
    return FSPath(DATADIR "/paludis/traditional.exclude");
}

FSPath
TraditionalLayout::binary_ebuild_directory(const QualifiedPackageName & q) const
{
    return package_directory(q);
}

std::shared_ptr<MetadataValueKey<FSPath> >
TraditionalLayout::accounts_repository_data_location_key() const
{
    return nullptr;
}

std::shared_ptr<MetadataValueKey<FSPath> >
TraditionalLayout::e_updates_location_key() const
{
    if ((_imp->tree_root / "profiles" / "updates").stat().exists())
        return std::make_shared<LiteralMetadataValueKey<FSPath>>("e_updates_location",
                    "VDBRepository updates data location", mkt_internal, _imp->tree_root / "profiles" / "updates");
    else
        return nullptr;
}

std::shared_ptr<MetadataValueKey<FSPath> >
TraditionalLayout::licence_groups_location_key() const
{
    if ((_imp->tree_root / "profiles" / "license_groups").stat().exists())
        return std::make_shared<LiteralMetadataValueKey<FSPath>>("licence_groups_location",
                    "License groups data location", mkt_internal, _imp->tree_root / "profiles" / "license_groups");
    else
        return nullptr;
}

std::shared_ptr<const MasksInfo>
TraditionalLayout::repository_masks(const std::shared_ptr<const PackageID> & id) const
{
    return _imp->repository_mask_store->query(id);
}


