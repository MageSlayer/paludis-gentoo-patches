/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/repositories/e/exheres_layout.hh>
#include <paludis/repositories/e/e_repository_entries.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/util/config_file.hh>
#include <paludis/package_id.hh>
#include <paludis/package_database.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/map.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/choice.hh>
#include <paludis/literal_metadata_key.hh>
#include <tr1/functional>
#include <tr1/unordered_map>
#include <functional>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::tr1::unordered_map<CategoryNamePart, bool, Hash<CategoryNamePart> > CategoryMap;
typedef std::tr1::unordered_map<QualifiedPackageName, bool, Hash<QualifiedPackageName> > PackagesMap;
typedef std::tr1::unordered_map<QualifiedPackageName, std::tr1::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> > IDMap;

namespace paludis
{
    template<>
    struct Implementation<ExheresLayout>
    {
        const ERepository * const repository;
        const FSEntry tree_root;

        mutable Mutex big_nasty_mutex;

        mutable bool has_category_names;
        mutable CategoryMap category_names;
        mutable PackagesMap package_names;
        mutable IDMap ids;

        mutable std::tr1::shared_ptr<CategoryNamePartSet> category_names_collection;
        std::tr1::shared_ptr<const ERepositoryEntries> entries;

        std::tr1::shared_ptr<FSEntrySequence> arch_list_files;
        std::tr1::shared_ptr<FSEntrySequence> repository_mask_files;
        std::tr1::shared_ptr<FSEntrySequence> profiles_desc_files;
        std::tr1::shared_ptr<FSEntrySequence> mirror_files;
        std::tr1::shared_ptr<FSEntrySequence> info_variables_files;
        std::tr1::shared_ptr<FSEntrySequence> info_packages_files;
        std::tr1::shared_ptr<UseDescFileInfoSequence> use_desc_files;

        Implementation(const ERepository * const n, const FSEntry & t,
                std::tr1::shared_ptr<const ERepositoryEntries> e) :
            repository(n),
            tree_root(t),
            has_category_names(false),
            entries(e),
            arch_list_files(new FSEntrySequence),
            repository_mask_files(new FSEntrySequence),
            profiles_desc_files(new FSEntrySequence),
            mirror_files(new FSEntrySequence),
            info_variables_files(new FSEntrySequence),
            info_packages_files(new FSEntrySequence),
            use_desc_files(new UseDescFileInfoSequence)
        {
        }
    };
}

ExheresLayout::ExheresLayout(const ERepository * const r, const FSEntry & tree_root,
        const std::tr1::shared_ptr<const ERepositoryEntries> & e,
        const std::tr1::shared_ptr<const FSEntrySequence> & f) :
    Layout(f),
    PrivateImplementationPattern<ExheresLayout>(new Implementation<ExheresLayout>(r, tree_root, e))
{
    if (master_repositories_locations())
    {
        for (FSEntrySequence::ConstIterator l(master_repositories_locations()->begin()), l_end(master_repositories_locations()->end()) ;
                l != l_end ; ++l)
        {
            /* don't also follow our masters' masters. Otherwise things like masters = arbor x11 will
             * get weird... */
            _imp->arch_list_files->push_back(*l / "metadata" / "arch.conf");
            _imp->repository_mask_files->push_back(*l / "metadata" / "repository_mask.conf");
            _imp->profiles_desc_files->push_back(*l / "metadata" / "profiles_desc.conf");
            _imp->mirror_files->push_back(*l / "metadata" / "mirrors.conf");
            _imp->info_variables_files->push_back(*l / "metadata" / "info" / "variables.conf");

            FSEntry descs(*l / "metadata" / "options" / "descriptions");
            if (descs.is_directory_or_symlink_to_directory())
            {
                for (DirIterator d(descs), d_end ; d != d_end ; ++d)
                {
                    if (! is_file_with_extension(*d, ".conf", IsFileWithOptions()))
                        continue;

                    std::string p(strip_trailing_string(strip_trailing_string(d->basename(), ".conf"), ".local"));
                    if (p == "options")
                        _imp->use_desc_files->push_back(std::make_pair(*d, ChoicePrefixName("")));
                    else
                        _imp->use_desc_files->push_back(std::make_pair(*d, ChoicePrefixName(p)));
                }
            }
        }
    }

    _imp->arch_list_files->push_back(_imp->tree_root / "metadata" / "arch.conf");
    _imp->repository_mask_files->push_back(_imp->tree_root / "metadata" / "repository_mask.conf");
    _imp->profiles_desc_files->push_back(_imp->tree_root / "metadata" / "profiles_desc.conf");
    _imp->mirror_files->push_back(_imp->tree_root / "metadata" / "mirrors.conf");
    _imp->info_variables_files->push_back(_imp->tree_root / "metadata" / "info" / "variables.conf");
    _imp->info_packages_files->push_back(_imp->tree_root / "metadata" / "info" / "packages.conf");

    FSEntry descs(_imp->tree_root / "metadata" / "options" / "descriptions");
    if (descs.is_directory_or_symlink_to_directory())
    {
        for (DirIterator d(descs), d_end ; d != d_end ; ++d)
        {
            if (! is_file_with_extension(*d, ".conf", IsFileWithOptions()))
                continue;

            std::string p(strip_trailing_string(strip_trailing_string(d->basename(), ".conf"), ".local"));
            if (p == "options")
                _imp->use_desc_files->push_back(std::make_pair(*d, ""));
            else
                _imp->use_desc_files->push_back(std::make_pair(*d, p));
        }
    }
}

ExheresLayout::~ExheresLayout()
{
}

FSEntry
ExheresLayout::categories_file() const
{
    return _imp->tree_root / "metadata" / "categories.conf";
}

void
ExheresLayout::need_category_names() const
{
    Lock l(_imp->big_nasty_mutex);

    if (_imp->has_category_names)
        return;

    Context context("When loading category names for " + stringify(_imp->repository->name()) + ":");

    Log::get_instance()->message("e.exheres_layout.need_category_names", ll_debug, lc_context) << "need_category_names";

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

        LineConfigFile cats(*i, LineConfigFileOptions());

        for (LineConfigFile::ConstIterator line(cats.begin()), line_end(cats.end()) ;
                line != line_end ; ++line)
        {
            try
            {
                _imp->category_names.insert(std::make_pair(CategoryNamePart(*line), false));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message("e.exheres_layout.categories.skipping", ll_warning, lc_context)
                    << "Skipping line '" << *line << "' in '" << *i << "' due to exception '"
                    << e.message() << "' ('" << e.what() << ")";
            }
        }

        found_one = true;
    }

    if (! found_one)
        throw ERepositoryConfigurationError("No categories file available for repository '"
                + stringify(_imp->repository->name()) + "', and this layout does not allow auto-generation");

    _imp->has_category_names = true;
}

void
ExheresLayout::need_package_ids(const QualifiedPackageName & n) const
{
    Lock l(_imp->big_nasty_mutex);

    using namespace std::tr1::placeholders;

    if (_imp->package_names[n])
        return;

    Context context("When loading versions for '" + stringify(n) + "' in "
            + stringify(_imp->repository->name()) + ":");

    std::tr1::shared_ptr<PackageIDSequence> v(new PackageIDSequence);

    FSEntry path(_imp->tree_root / "packages" / stringify(n.category()) / stringify(n.package()));

    for (DirIterator e(path), e_end ; e != e_end ; ++e)
    {
        if (! _imp->entries->is_package_file(n, *e))
            continue;

        try
        {
            std::tr1::shared_ptr<const PackageID> id(_imp->entries->make_id(n, *e));
            if (indirect_iterator(v->end()) != std::find_if(indirect_iterator(v->begin()), indirect_iterator(v->end()),
                        std::tr1::bind(std::equal_to<VersionSpec>(), id->version(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1))))
                Log::get_instance()->message("e.exheres_layout.id.duplicate", ll_warning, lc_context)
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
            Log::get_instance()->message("e.exheres_layout.id.failure", ll_warning, lc_context) << "Skipping entry '"
                << *e << "' for '" << n << "' in repository '"
                << _imp->repository->name() << "' due to exception '" << ee.message() << "' ("
                << ee.what() << ")'";
        }
    }

    _imp->ids.insert(std::make_pair(n, v));
    _imp->package_names[n] = true;
}

bool
ExheresLayout::has_category_named(const CategoryNamePart & c) const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When checking for category '" + stringify(c) + "' in '" + stringify(_imp->repository->name()) + "':");

    need_category_names();
    return _imp->category_names.end() != _imp->category_names.find(c);
}

bool
ExheresLayout::has_package_named(const QualifiedPackageName & q) const
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
        fs /= "packages";
        fs /= stringify(q.category());
        fs /= stringify(q.package());
        if (! fs.is_directory_or_symlink_to_directory())
            return false;
        _imp->package_names.insert(std::make_pair(q, false));
        return true;
    }
}

void
ExheresLayout::need_category_names_collection() const
{
    Lock l(_imp->big_nasty_mutex);

    if (_imp->category_names_collection)
        return;

    need_category_names();

    _imp->category_names_collection.reset(new CategoryNamePartSet);
    std::transform(_imp->category_names.begin(), _imp->category_names.end(),
            _imp->category_names_collection->inserter(),
            std::tr1::mem_fn(&std::pair<const CategoryNamePart, bool>::first));
}

std::tr1::shared_ptr<const CategoryNamePartSet>
ExheresLayout::category_names() const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When fetching category names in " + stringify(stringify(_imp->repository->name())) + ":");

    need_category_names_collection();
    return _imp->category_names_collection;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
ExheresLayout::package_names(const CategoryNamePart & c) const
{
    Lock l(_imp->big_nasty_mutex);

    using namespace std::tr1::placeholders;

    /* this isn't particularly fast because it isn't called very often. avoid
     * changing the data structures used to make this faster at the expense of
     * slowing down single item queries. */

    Context context("When fetching package names in category '" + stringify(c)
            + "' in '" + stringify(_imp->repository->name()) + "':");

    need_category_names();

    if (_imp->category_names.end() == _imp->category_names.find(c))
        return std::tr1::shared_ptr<QualifiedPackageNameSet>(new QualifiedPackageNameSet);

    if ((_imp->tree_root / "packages" / stringify(c)).is_directory_or_symlink_to_directory())
        for (DirIterator d(_imp->tree_root / "packages" / stringify(c)), d_end ; d != d_end ; ++d)
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
                Log::get_instance()->message("e.exheres_layout.packages.failure", ll_warning, lc_context)
                    << "Skipping entry '" << d->basename() << "' in category '" << c << "' in repository '"
                    << _imp->repository->name() << "' (" << e.message() << ")";
            }
        }

    _imp->category_names[c] = true;

    std::tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);

    for (PackagesMap::const_iterator p(_imp->package_names.begin()), p_end(_imp->package_names.end()) ;
            p != p_end ; ++p)
        if (p->first.category() == c)
            result->insert(p->first);

    return result;
}

std::tr1::shared_ptr<const PackageIDSequence>
ExheresLayout::package_ids(const QualifiedPackageName & n) const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When fetching versions of '" + stringify(n) + "' in " + stringify(_imp->repository->name()) + ":");

    if (has_package_named(n))
    {
        need_package_ids(n);
        return _imp->ids.find(n)->second;
    }
    else
        return std::tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);
}

const std::tr1::shared_ptr<const FSEntrySequence>
ExheresLayout::info_packages_files() const
{
    return _imp->info_packages_files;
}

const std::tr1::shared_ptr<const FSEntrySequence>
ExheresLayout::info_variables_files() const
{
    return _imp->info_variables_files;
}

FSEntry
ExheresLayout::package_directory(const QualifiedPackageName & qpn) const
{
    return _imp->tree_root / "packages" / stringify(qpn.category()) / stringify(qpn.package());
}

FSEntry
ExheresLayout::category_directory(const CategoryNamePart & cat) const
{
    return _imp->tree_root / "packages" / stringify(cat);
}

std::tr1::shared_ptr<const FSEntrySequence>
ExheresLayout::arch_list_files() const
{
    return _imp->arch_list_files;
}

std::tr1::shared_ptr<const FSEntrySequence>
ExheresLayout::repository_mask_files() const
{
    return _imp->repository_mask_files;
}

std::tr1::shared_ptr<const FSEntrySequence>
ExheresLayout::profiles_desc_files() const
{
    return _imp->profiles_desc_files;
}

std::tr1::shared_ptr<const FSEntrySequence>
ExheresLayout::mirror_files() const
{
    return _imp->mirror_files;
}

std::tr1::shared_ptr<const UseDescFileInfoSequence>
ExheresLayout::use_desc_files() const
{
    return _imp->use_desc_files;
}

FSEntry
ExheresLayout::profiles_base_dir() const
{
    if (master_repositories_locations() && ! master_repositories_locations()->empty())
        return *master_repositories_locations()->begin() / "profiles";
    else
        return _imp->tree_root / "profiles";
}

std::tr1::shared_ptr<const FSEntrySequence>
ExheresLayout::exlibsdirs(const QualifiedPackageName & q) const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    std::tr1::shared_ptr<const FSEntrySequence> global(exlibsdirs_global());
    std::copy(global->begin(), global->end(), result->back_inserter());

    std::tr1::shared_ptr<const FSEntrySequence> category(exlibsdirs_category(q.category()));
    std::copy(category->begin(), category->end(), result->back_inserter());

    std::tr1::shared_ptr<const FSEntrySequence> package(exlibsdirs_package(q));
    std::copy(package->begin(), package->end(), result->back_inserter());

    return result;
}

std::tr1::shared_ptr<const FSEntrySequence>
ExheresLayout::exlibsdirs_global() const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    if (_imp->repository->params().master_repositories())
    {
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
        {
            std::tr1::shared_ptr<const FSEntrySequence> master((*e)->layout()->exlibsdirs_global());
            std::copy(master->begin(), master->end(), result->back_inserter());
        }
    }
    result->push_back(_imp->tree_root / "exlibs");

    return result;
}

std::tr1::shared_ptr<const FSEntrySequence>
ExheresLayout::exlibsdirs_category(const CategoryNamePart & c) const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    if (_imp->repository->params().master_repositories())
    {
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
        {
            std::tr1::shared_ptr<const FSEntrySequence> master((*e)->layout()->exlibsdirs_category(c));
            std::copy(master->begin(), master->end(), result->back_inserter());
        }
    }
    result->push_back(category_directory(c) / "exlibs");

    return result;
}

std::tr1::shared_ptr<const FSEntrySequence>
ExheresLayout::exlibsdirs_package(const QualifiedPackageName & q) const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    if (_imp->repository->params().master_repositories())
    {
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
        {
            std::tr1::shared_ptr<const FSEntrySequence> master((*e)->layout()->exlibsdirs_package(q));
            std::copy(master->begin(), master->end(), result->back_inserter());
        }
    }
    result->push_back(package_directory(q));

    return result;
}

std::tr1::shared_ptr<const FSEntrySequence>
ExheresLayout::licenses_dirs() const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    if (_imp->repository->params().master_repositories())
    {
        for (ERepositorySequence::ConstIterator e(_imp->repository->params().master_repositories()->begin()),
                e_end(_imp->repository->params().master_repositories()->end()) ; e != e_end ; ++e)
        {
            std::tr1::shared_ptr<const FSEntrySequence> master((*e)->layout()->licenses_dirs());
            std::copy(master->begin(), master->end(), result->back_inserter());
        }
    }
    result->push_back(_imp->tree_root / "licences");

    return result;
}

namespace
{
    void aux_files_helper(const FSEntry & d,
            std::tr1::shared_ptr<Map<FSEntry, std::string> > & m,
            const QualifiedPackageName & qpn)
    {
        if (! d.exists())
            return;

        std::list<FSEntry> files((DirIterator(d)),
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

std::tr1::shared_ptr<Map<FSEntry, std::string> >
ExheresLayout::manifest_files(const QualifiedPackageName & qpn) const
{
    std::tr1::shared_ptr<Map<FSEntry, std::string> > result(new Map<FSEntry, std::string>);
    FSEntry package_dir = _imp->repository->layout()->package_directory(qpn);

    std::list<FSEntry> package_files((DirIterator(package_dir)),
            DirIterator());
    for (std::list<FSEntry>::iterator f(package_files.begin()) ;
            f != package_files.end() ; ++f)
    {
        if (! (*f).is_regular_file() || ((*f).basename() == "Manifest") )
            continue;

        std::string file_type("MISC");
        if (_imp->entries->is_package_file(qpn, (*f)))
            file_type=_imp->entries->get_package_file_manifest_key((*f), qpn);

        result->insert((*f), file_type);
    }

    aux_files_helper((package_dir / "files"), result, qpn);

    return result;
}

void
ExheresLayout::invalidate_masks()
{
    Lock l(_imp->big_nasty_mutex);

    for (IDMap::iterator it(_imp->ids.begin()), it_end(_imp->ids.end()); it_end != it; ++it)
        for (PackageIDSequence::ConstIterator it2(it->second->begin()), it2_end(it->second->end());
             it2_end != it2; ++it2)
            (*it2)->invalidate_masks();
}

FSEntry
ExheresLayout::binary_ebuild_location(const QualifiedPackageName & q, const VersionSpec & v,
        const std::string & eapi) const
{
    return package_directory(q) / _imp->entries->binary_ebuild_name(q, v, eapi);
}

std::tr1::shared_ptr<MetadataValueKey<FSEntry> >
ExheresLayout::accounts_repository_data_location_key() const
{
    if ((_imp->tree_root / "metadata" / "accounts").exists())
        return make_shared_ptr(new LiteralMetadataValueKey<FSEntry>("accounts_repository_data_location",
                    "AccountsRepository data location", mkt_internal, _imp->tree_root / "metadata" / "accounts"));
    else
        return make_null_shared_ptr();
}

std::tr1::shared_ptr<MetadataValueKey<FSEntry> >
ExheresLayout::e_updates_location_key() const
{
    return make_null_shared_ptr();
}

