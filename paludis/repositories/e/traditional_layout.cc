/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/config_file.hh>
#include <paludis/package_id.hh>
#include <paludis/package_database.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/map.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/set.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_shared_ptr.hh>
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

namespace paludis
{
    template<>
    struct Implementation<TraditionalLayout>
    {
        const ERepository * const repository;
        const FSEntry tree_root;

        mutable Mutex big_nasty_mutex;

        mutable bool has_category_names;
        mutable CategoryMap category_names;
        mutable PackagesMap package_names;
        mutable IDMap ids;

        mutable std::shared_ptr<CategoryNamePartSet> category_names_collection;

        std::shared_ptr<FSEntrySequence> arch_list_files;
        std::shared_ptr<FSEntrySequence> repository_mask_files;
        std::shared_ptr<FSEntrySequence> profiles_desc_files;
        std::shared_ptr<FSEntrySequence> mirror_files;
        std::shared_ptr<FSEntrySequence> info_packages_files;
        std::shared_ptr<FSEntrySequence> info_variables_files;
        std::shared_ptr<UseDescFileInfoSequence> use_desc_files;

        Implementation(const ERepository * const r, const FSEntry & t) :
            repository(r),
            tree_root(t),
            has_category_names(false),
            arch_list_files(new FSEntrySequence),
            repository_mask_files(new FSEntrySequence),
            profiles_desc_files(new FSEntrySequence),
            mirror_files(new FSEntrySequence),
            info_packages_files(new FSEntrySequence),
            info_variables_files(new FSEntrySequence),
            use_desc_files(new UseDescFileInfoSequence)
        {
        }
    };
}

TraditionalLayout::TraditionalLayout(const ERepository * const repo, const FSEntry & tree_root,
        const std::shared_ptr<const FSEntrySequence> & f) :
    Layout(f),
    PrivateImplementationPattern<TraditionalLayout>(new Implementation<TraditionalLayout>(repo, tree_root))
{
    if (master_repositories_locations())
    {
        for (FSEntrySequence::ConstIterator l(master_repositories_locations()->begin()), l_end(master_repositories_locations()->end()) ;
                l != l_end ; ++l)
        {
            _imp->arch_list_files->push_back(*l / "profiles" / "arch.list");
            _imp->repository_mask_files->push_back(*l / "profiles" / "package.mask");
            _imp->profiles_desc_files->push_back(*l / "profiles" / "profiles.desc");
            _imp->mirror_files->push_back(*l / "profiles" / "thirdpartymirrors");
            _imp->info_variables_files->push_back(*l / "profiles" / "info_vars");

            _imp->use_desc_files->push_back(std::make_pair(*l / "profiles" / "use.desc", ChoicePrefixName("")));
            _imp->use_desc_files->push_back(std::make_pair(*l / "profiles" / "use.local.desc", ChoicePrefixName("")));
            FSEntry descs(*l / "profiles" / "desc");
            if (descs.is_directory_or_symlink_to_directory())
            {
                for (DirIterator d(descs), d_end ; d != d_end ; ++d)
                {
                    if (! is_file_with_extension(*d, ".desc", IsFileWithOptions()))
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

    _imp->use_desc_files->push_back(std::make_pair(_imp->tree_root / "profiles" / "use.desc", ""));
    _imp->use_desc_files->push_back(std::make_pair(_imp->tree_root / "profiles" / "use.local.desc", ""));
    FSEntry descs(_imp->tree_root / "profiles" / "desc");
    if (descs.is_directory_or_symlink_to_directory())
    {
        for (DirIterator d(descs), d_end ; d != d_end ; ++d)
        {
            if (! is_file_with_extension(*d, ".desc", IsFileWithOptions()))
                continue;
            _imp->use_desc_files->push_back(std::make_pair(*d, strip_trailing_string(d->basename(), ".desc")));
        }
    }
}

TraditionalLayout::~TraditionalLayout()
{
}

FSEntry
TraditionalLayout::categories_file() const
{
    return _imp->tree_root / "profiles" / "categories";
}

void
TraditionalLayout::need_category_names() const
{
    Lock l(_imp->big_nasty_mutex);

    if (_imp->has_category_names)
        return;

    Context context("When loading category names for " + stringify(_imp->repository->name()) + ":");

    Log::get_instance()->message("e.traditional_layout.need_category_names", ll_debug, lc_context) << "need_category_names";

    bool found_one(false);

    std::list<FSEntry> cats_list;
    if (_imp->repository->params().master_repositories())
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
            cats_list.push_back((*e)->layout()->categories_file());
    cats_list.push_back(categories_file());

    for (std::list<FSEntry>::const_iterator i(cats_list.begin()), i_end(cats_list.end()) ;
            i != i_end ; ++i)
    {
        if (! i->exists())
            continue;

        LineConfigFile cats(*i, LineConfigFileOptions() + lcfo_disallow_continuations);

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
        for (DirIterator d(_imp->tree_root, DirIteratorOptions() + dio_inode_sort), d_end ; d != d_end ; ++d)
        {
            if (! d->is_directory_or_symlink_to_directory())
                continue;

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
    Lock l(_imp->big_nasty_mutex);

    using namespace std::placeholders;

    if (_imp->package_names[n])
        return;

    Context context("When loading versions for '" + stringify(n) + "' in "
            + stringify(_imp->repository->name()) + ":");

    std::shared_ptr<PackageIDSequence> v(new PackageIDSequence);

    FSEntry path(_imp->tree_root / stringify(n.category()) / stringify(n.package()));

    for (DirIterator e(path, DirIteratorOptions() + dio_inode_sort), e_end ; e != e_end ; ++e)
    {
        if (! _imp->repository->is_package_file(n, *e))
            continue;

        try
        {
            std::shared_ptr<const PackageID> id(_imp->repository->make_id(n, *e));
            if (indirect_iterator(v->end()) != std::find_if(indirect_iterator(v->begin()), indirect_iterator(v->end()),
                        std::bind(std::equal_to<VersionSpec>(), id->version(), std::bind(std::mem_fn(&PackageID::version), _1))))
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
    Lock l(_imp->big_nasty_mutex);

    Context context("When checking for category '" + stringify(c) + "' in '" + stringify(_imp->repository->name()) + "':");

    need_category_names();
    return _imp->category_names.end() != _imp->category_names.find(c);
}

bool
TraditionalLayout::has_package_named(const QualifiedPackageName & q) const
{
    Lock l(_imp->big_nasty_mutex);

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

        FSEntry fs(_imp->tree_root);
        fs /= stringify(q.category());
        fs /= stringify(q.package());
        if (! fs.is_directory_or_symlink_to_directory())
            return false;
        _imp->package_names.insert(std::make_pair(q, false));
        return true;
    }
}

void
TraditionalLayout::need_category_names_collection() const
{
    Lock l(_imp->big_nasty_mutex);

    if (_imp->category_names_collection)
        return;

    need_category_names();

    _imp->category_names_collection.reset(new CategoryNamePartSet);
    std::transform(_imp->category_names.begin(), _imp->category_names.end(),
            _imp->category_names_collection->inserter(),
            std::mem_fn(&std::pair<const CategoryNamePart, bool>::first));
}

std::shared_ptr<const CategoryNamePartSet>
TraditionalLayout::category_names() const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When fetching category names in " + stringify(stringify(_imp->repository->name())) + ":");

    need_category_names_collection();
    return _imp->category_names_collection;
}

std::shared_ptr<const QualifiedPackageNameSet>
TraditionalLayout::package_names(const CategoryNamePart & c) const
{
    Lock l(_imp->big_nasty_mutex);

    using namespace std::placeholders;

    /* this isn't particularly fast because it isn't called very often. avoid
     * changing the data structures used to make this faster at the expense of
     * slowing down single item queries. */

    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(_imp->repository->name()) + ":");

    need_category_names();

    if (_imp->category_names.end() == _imp->category_names.find(c))
        return std::shared_ptr<QualifiedPackageNameSet>(new QualifiedPackageNameSet);

    if ((_imp->tree_root / stringify(c)).is_directory_or_symlink_to_directory())
        for (DirIterator d(_imp->tree_root / stringify(c), DirIteratorOptions() + dio_inode_sort), d_end ; d != d_end ; ++d)
        {
            try
            {
                if (! d->is_directory_or_symlink_to_directory())
                    continue;

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

    std::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);

    for (PackagesMap::const_iterator p(_imp->package_names.begin()), p_end(_imp->package_names.end()) ;
            p != p_end ; ++p)
        if (p->first.category() == c)
            result->insert(p->first);

    return result;
}

std::shared_ptr<const PackageIDSequence>
TraditionalLayout::package_ids(const QualifiedPackageName & n) const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When fetching versions of '" + stringify(n) + "' in " + stringify(_imp->repository->name()) + ":");

    if (has_package_named(n))
    {
        need_package_ids(n);
        return _imp->ids.find(n)->second;
    }
    else
        return std::shared_ptr<PackageIDSequence>(new PackageIDSequence);
}

const std::shared_ptr<const FSEntrySequence>
TraditionalLayout::info_packages_files() const
{
    return _imp->info_packages_files;
}

const std::shared_ptr<const FSEntrySequence>
TraditionalLayout::info_variables_files() const
{
    return _imp->info_variables_files;
}

FSEntry
TraditionalLayout::package_directory(const QualifiedPackageName & qpn) const
{
    return _imp->tree_root / stringify(qpn.category()) / stringify(qpn.package());
}

FSEntry
TraditionalLayout::category_directory(const CategoryNamePart & cat) const
{
    return _imp->tree_root / stringify(cat);
}

std::shared_ptr<const FSEntrySequence>
TraditionalLayout::arch_list_files() const
{
    return _imp->arch_list_files;
}

std::shared_ptr<const FSEntrySequence>
TraditionalLayout::repository_mask_files() const
{
    return _imp->repository_mask_files;
}

std::shared_ptr<const FSEntrySequence>
TraditionalLayout::profiles_desc_files() const
{
    return _imp->profiles_desc_files;
}

std::shared_ptr<const FSEntrySequence>
TraditionalLayout::mirror_files() const
{
    return _imp->mirror_files;
}

std::shared_ptr<const UseDescFileInfoSequence>
TraditionalLayout::use_desc_files() const
{
    return _imp->use_desc_files;
}

FSEntry
TraditionalLayout::profiles_base_dir() const
{
    if (master_repositories_locations() && ! master_repositories_locations()->empty())
        return *master_repositories_locations()->begin() / "profiles";
    else
        return _imp->tree_root / "profiles";
}

std::shared_ptr<const FSEntrySequence>
TraditionalLayout::exlibsdirs(const QualifiedPackageName & q) const
{
    std::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    std::shared_ptr<const FSEntrySequence> global(exlibsdirs_global());
    std::copy(global->begin(), global->end(), result->back_inserter());

    std::shared_ptr<const FSEntrySequence> category(exlibsdirs_category(q.category()));
    std::copy(category->begin(), category->end(), result->back_inserter());

    std::shared_ptr<const FSEntrySequence> package(exlibsdirs_package(q));
    std::copy(package->begin(), package->end(), result->back_inserter());

    return result;
}

std::shared_ptr<const FSEntrySequence>
TraditionalLayout::exlibsdirs_global() const
{
    std::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    if (_imp->repository->params().master_repositories())
    {
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
        {
            std::shared_ptr<const FSEntrySequence> master((*e)->layout()->exlibsdirs_global());
            std::copy(master->begin(), master->end(), result->back_inserter());
        }
    }
    result->push_back(_imp->tree_root / "exlibs");

    return result;
}

std::shared_ptr<const FSEntrySequence>
TraditionalLayout::exlibsdirs_category(const CategoryNamePart & c) const
{
    std::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    if (_imp->repository->params().master_repositories())
    {
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
        {
            std::shared_ptr<const FSEntrySequence> master((*e)->layout()->exlibsdirs_category(c));
            std::copy(master->begin(), master->end(), result->back_inserter());
        }
    }
    result->push_back(category_directory(c) / "exlibs");

    return result;
}

std::shared_ptr<const FSEntrySequence>
TraditionalLayout::exlibsdirs_package(const QualifiedPackageName & q) const
{
    std::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    if (_imp->repository->params().master_repositories())
    {
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
        {
            std::shared_ptr<const FSEntrySequence> master((*e)->layout()->exlibsdirs_package(q));
            std::copy(master->begin(), master->end(), result->back_inserter());
        }
    }
    result->push_back(package_directory(q));

    return result;
}

std::shared_ptr<const FSEntrySequence>
TraditionalLayout::licenses_dirs() const
{
    std::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    if (_imp->repository->params().master_repositories())
    {
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
        {
            std::shared_ptr<const FSEntrySequence> master((*e)->layout()->licenses_dirs());
           std::copy(master->begin(), master->end(), result->back_inserter());
        }
    }
    result->push_back(_imp->tree_root / "licenses");

    return result;
}

namespace
{
    void aux_files_helper(const FSEntry & d,
            std::shared_ptr<Map<FSEntry, std::string> > & m,
            const QualifiedPackageName & qpn)
    {
        if (! d.exists())
            return;

        std::list<FSEntry> files((DirIterator(d, DirIteratorOptions() + dio_inode_sort)),
                DirIterator());
        for (std::list<FSEntry>::iterator f(files.begin()) ;
                f != files.end() ; ++f)
        {
            if (f->is_directory())
            {
                if ("CVS" != f->basename())
                    aux_files_helper((*f), m, qpn);
            }
            else
            {
                if (! f->is_regular_file())
                    continue;
                if (is_file_with_prefix_extension((*f),
                            ("digest-"+stringify(qpn.package())), "",
                            IsFileWithOptions()))
                    continue;
                m->insert((*f), "AUX");
            }
        }
    }
}

std::shared_ptr<Map<FSEntry, std::string> >
TraditionalLayout::manifest_files(const QualifiedPackageName & qpn) const
{
    std::shared_ptr<Map<FSEntry, std::string> > result(new Map<FSEntry, std::string>);
    FSEntry package_dir = _imp->repository->layout()->package_directory(qpn);

    std::list<FSEntry> package_files((DirIterator(package_dir, DirIteratorOptions() + dio_inode_sort)),
            DirIterator());
    for (std::list<FSEntry>::iterator f(package_files.begin()) ;
            f != package_files.end() ; ++f)
    {
        if (! (*f).is_regular_file() || ((*f).basename() == "Manifest") )
            continue;

        std::string file_type("MISC");
        if (_imp->repository->is_package_file(qpn, (*f)))
            file_type=_imp->repository->get_package_file_manifest_key((*f), qpn);

        result->insert((*f), file_type);
    }

    aux_files_helper((package_dir / "files"), result, qpn);

    return result;
}

FSEntry
TraditionalLayout::sync_filter_file() const
{
    return FSEntry(DATADIR "/paludis/traditional.exclude");
}

void
TraditionalLayout::invalidate_masks()
{
    Lock l(_imp->big_nasty_mutex);

    for (IDMap::iterator it(_imp->ids.begin()), it_end(_imp->ids.end()); it_end != it; ++it)
        for (PackageIDSequence::ConstIterator it2(it->second->begin()), it2_end(it->second->end());
             it2_end != it2; ++it2)
            (*it2)->invalidate_masks();
}

FSEntry
TraditionalLayout::binary_ebuild_location(const QualifiedPackageName & q, const VersionSpec & v,
        const std::string & eapi) const
{
    return package_directory(q) / _imp->repository->binary_ebuild_name(q, v, eapi);
}

std::shared_ptr<MetadataValueKey<FSEntry> >
TraditionalLayout::accounts_repository_data_location_key() const
{
    return make_null_shared_ptr();
}

std::shared_ptr<MetadataValueKey<FSEntry> >
TraditionalLayout::e_updates_location_key() const
{
    if ((_imp->tree_root / "profiles" / "updates").exists())
        return make_shared_ptr(new LiteralMetadataValueKey<FSEntry>("e_updates_location",
                    "VDBRepository updates data location", mkt_internal, _imp->tree_root / "profiles" / "updates"));
    else
        return make_null_shared_ptr();
}

