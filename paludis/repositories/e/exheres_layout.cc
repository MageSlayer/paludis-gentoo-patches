/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/util/config_file.hh>
#include <paludis/package_id.hh>
#include <paludis/package_database.hh>
#include <paludis/util/pimp-impl.hh>
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
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/choice.hh>
#include <paludis/literal_metadata_key.hh>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::unordered_map<CategoryNamePart, bool, Hash<CategoryNamePart> > CategoryMap;
typedef std::unordered_map<QualifiedPackageName, bool, Hash<QualifiedPackageName> > PackagesMap;
typedef std::unordered_map<QualifiedPackageName, std::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> > IDMap;

namespace paludis
{
    template<>
    struct Imp<ExheresLayout>
    {
        const ERepository * const repository;
        const FSPath tree_root;

        mutable Mutex big_nasty_mutex;

        mutable bool has_category_names;
        mutable CategoryMap category_names;
        mutable PackagesMap package_names;
        mutable IDMap ids;

        mutable std::shared_ptr<CategoryNamePartSet> category_names_collection;

        std::shared_ptr<FSPathSequence> arch_list_files;
        std::shared_ptr<FSPathSequence> repository_mask_files;
        std::shared_ptr<FSPathSequence> profiles_desc_files;
        std::shared_ptr<FSPathSequence> mirror_files;
        std::shared_ptr<FSPathSequence> info_variables_files;
        std::shared_ptr<FSPathSequence> info_packages_files;
        std::shared_ptr<UseDescFileInfoSequence> use_desc_files;

        Imp(const ERepository * const n, const FSPath & t) :
            repository(n),
            tree_root(t),
            has_category_names(false),
            arch_list_files(std::make_shared<FSPathSequence>()),
            repository_mask_files(std::make_shared<FSPathSequence>()),
            profiles_desc_files(std::make_shared<FSPathSequence>()),
            mirror_files(std::make_shared<FSPathSequence>()),
            info_variables_files(std::make_shared<FSPathSequence>()),
            info_packages_files(std::make_shared<FSPathSequence>()),
            use_desc_files(std::make_shared<UseDescFileInfoSequence>())
        {
        }
    };
}

ExheresLayout::ExheresLayout(const ERepository * const r, const FSPath & tree_root,
        const std::shared_ptr<const FSPathSequence> & f) :
    Layout(f),
    _imp(r, tree_root)
{
    if (master_repositories_locations())
    {
        for (FSPathSequence::ConstIterator l(master_repositories_locations()->begin()), l_end(master_repositories_locations()->end()) ;
                l != l_end ; ++l)
        {
            /* don't also follow our masters' masters. Otherwise things like masters = arbor x11 will
             * get weird... */
            _imp->arch_list_files->push_back(*l / "metadata" / "arch.conf");
            _imp->repository_mask_files->push_back(*l / "metadata" / "repository_mask.conf");
            _imp->profiles_desc_files->push_back(*l / "metadata" / "profiles_desc.conf");
            _imp->mirror_files->push_back(*l / "metadata" / "mirrors.conf");
            _imp->info_variables_files->push_back(*l / "metadata" / "info" / "variables.conf");

            FSPath descs(*l / "metadata" / "options" / "descriptions");
            if (descs.stat().is_directory_or_symlink_to_directory())
            {
                for (FSIterator d(descs, { }), d_end ; d != d_end ; ++d)
                {
                    if (! is_file_with_extension(*d, ".conf", { }))
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

    FSPath descs(_imp->tree_root / "metadata" / "options" / "descriptions");
    if (descs.stat().is_directory_or_symlink_to_directory())
    {
        for (FSIterator d(descs, { }), d_end ; d != d_end ; ++d)
        {
            if (! is_file_with_extension(*d, ".conf", { }))
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

FSPath
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

    std::list<FSPath> cats_list;
    cats_list.push_back(categories_file());

    for (std::list<FSPath>::const_iterator i(cats_list.begin()), i_end(cats_list.end()) ;
            i != i_end ; ++i)
    {
        if (! i->stat().exists())
            continue;

        LineConfigFile cats(*i, { });

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

    using namespace std::placeholders;

    if (_imp->package_names[n])
        return;

    Context context("When loading versions for '" + stringify(n) + "' in "
            + stringify(_imp->repository->name()) + ":");

    std::shared_ptr<PackageIDSequence> v(std::make_shared<PackageIDSequence>());

    FSPath path(_imp->tree_root / "packages" / stringify(n.category()) / stringify(n.package()));

    for (FSIterator e(path, { }), e_end ; e != e_end ; ++e)
    {
        if (! _imp->repository->is_package_file(n, *e))
            continue;

        try
        {
            std::shared_ptr<const PackageID> id(_imp->repository->make_id(n, *e));
            if (indirect_iterator(v->end()) != std::find_if(indirect_iterator(v->begin()), indirect_iterator(v->end()),
                        std::bind(std::equal_to<VersionSpec>(), id->version(), std::bind(std::mem_fn(&PackageID::version), _1))))
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

        FSPath fs(_imp->tree_root);
        fs /= "packages";
        fs /= stringify(q.category());
        fs /= stringify(q.package());
        if (! fs.stat().is_directory_or_symlink_to_directory())
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

    _imp->category_names_collection = std::make_shared<CategoryNamePartSet>();
    std::transform(_imp->category_names.begin(), _imp->category_names.end(),
            _imp->category_names_collection->inserter(),
            std::mem_fn(&std::pair<const CategoryNamePart, bool>::first));
}

std::shared_ptr<const CategoryNamePartSet>
ExheresLayout::category_names() const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When fetching category names in " + stringify(stringify(_imp->repository->name())) + ":");

    need_category_names_collection();
    return _imp->category_names_collection;
}

std::shared_ptr<const QualifiedPackageNameSet>
ExheresLayout::package_names(const CategoryNamePart & c) const
{
    Lock l(_imp->big_nasty_mutex);

    using namespace std::placeholders;

    /* this isn't particularly fast because it isn't called very often. avoid
     * changing the data structures used to make this faster at the expense of
     * slowing down single item queries. */

    Context context("When fetching package names in category '" + stringify(c)
            + "' in '" + stringify(_imp->repository->name()) + "':");

    need_category_names();

    if (_imp->category_names.end() == _imp->category_names.find(c))
        return std::make_shared<QualifiedPackageNameSet>();

    if ((_imp->tree_root / "packages" / stringify(c)).stat().is_directory_or_symlink_to_directory())
        for (FSIterator d(_imp->tree_root / "packages" / stringify(c), { fsio_want_directories, fsio_deref_symlinks_for_wants }), d_end ;
                d != d_end ; ++d)
        {
            try
            {
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

    std::shared_ptr<QualifiedPackageNameSet> result(std::make_shared<QualifiedPackageNameSet>());

    for (PackagesMap::const_iterator p(_imp->package_names.begin()), p_end(_imp->package_names.end()) ;
            p != p_end ; ++p)
        if (p->first.category() == c)
            result->insert(p->first);

    return result;
}

std::shared_ptr<const PackageIDSequence>
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
        return std::make_shared<PackageIDSequence>();
}

const std::shared_ptr<const FSPathSequence>
ExheresLayout::info_packages_files() const
{
    return _imp->info_packages_files;
}

const std::shared_ptr<const FSPathSequence>
ExheresLayout::info_variables_files() const
{
    return _imp->info_variables_files;
}

FSPath
ExheresLayout::package_directory(const QualifiedPackageName & qpn) const
{
    return _imp->tree_root / "packages" / stringify(qpn.category()) / stringify(qpn.package());
}

FSPath
ExheresLayout::category_directory(const CategoryNamePart & cat) const
{
    return _imp->tree_root / "packages" / stringify(cat);
}

std::shared_ptr<const FSPathSequence>
ExheresLayout::arch_list_files() const
{
    return _imp->arch_list_files;
}

std::shared_ptr<const FSPathSequence>
ExheresLayout::repository_mask_files() const
{
    return _imp->repository_mask_files;
}

std::shared_ptr<const FSPathSequence>
ExheresLayout::profiles_desc_files() const
{
    return _imp->profiles_desc_files;
}

std::shared_ptr<const FSPathSequence>
ExheresLayout::mirror_files() const
{
    return _imp->mirror_files;
}

std::shared_ptr<const UseDescFileInfoSequence>
ExheresLayout::use_desc_files() const
{
    return _imp->use_desc_files;
}

FSPath
ExheresLayout::profiles_base_dir() const
{
    if (master_repositories_locations() && ! master_repositories_locations()->empty())
        return *master_repositories_locations()->begin() / "profiles";
    else
        return _imp->tree_root / "profiles";
}

std::shared_ptr<const FSPathSequence>
ExheresLayout::exlibsdirs(const QualifiedPackageName & q) const
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
ExheresLayout::exlibsdirs_global() const
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
ExheresLayout::exlibsdirs_category(const CategoryNamePart & c) const
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
ExheresLayout::exlibsdirs_package(const QualifiedPackageName & q) const
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
ExheresLayout::licenses_dirs() const
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
    result->push_back(_imp->tree_root / "licences");

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

        std::list<FSPath> files((FSIterator(d, { })), FSIterator());
        for (std::list<FSPath>::iterator f(files.begin()) ;
                f != files.end() ; ++f)
        {
            FSStat f_stat(f->stat());
            if (f_stat.is_directory())
            {
                if ("CVS" != f->basename())
                    aux_files_helper((*f), m, qpn);
            }
            else
            {
                if (! f_stat.is_regular_file())
                    continue;
                if (is_file_with_prefix_extension((*f),
                            ("digest-"+stringify(qpn.package())), "",
                            { }))
                    continue;
                m->insert((*f), "AUX");
            }
        }
    }
}

std::shared_ptr<Map<FSPath, std::string, FSPathComparator> >
ExheresLayout::manifest_files(const QualifiedPackageName & qpn) const
{
    auto result(std::make_shared<Map<FSPath, std::string, FSPathComparator>>());
    FSPath package_dir = _imp->repository->layout()->package_directory(qpn);

    std::list<FSPath> package_files((FSIterator(package_dir, { })), FSIterator());
    for (std::list<FSPath>::iterator f(package_files.begin()) ;
            f != package_files.end() ; ++f)
    {
        FSStat f_stat(f->stat());
        if (! f_stat.is_regular_file() || ((*f).basename() == "Manifest") )
            continue;

        std::string file_type("MISC");
        if (_imp->repository->is_package_file(qpn, (*f)))
            file_type=_imp->repository->get_package_file_manifest_key((*f), qpn);

        result->insert((*f), file_type);
    }

    aux_files_helper((package_dir / "files"), result, qpn);

    return result;
}

FSPath
ExheresLayout::binary_ebuild_directory(const QualifiedPackageName & q) const
{
    return package_directory(q);
}

std::shared_ptr<MetadataValueKey<FSPath> >
ExheresLayout::accounts_repository_data_location_key() const
{
    if ((_imp->tree_root / "metadata" / "accounts").stat().exists())
        return std::make_shared<LiteralMetadataValueKey<FSPath>>("accounts_repository_data_location",
                    "AccountsRepository data location", mkt_internal, _imp->tree_root / "metadata" / "accounts");
    else
        return make_null_shared_ptr();
}

std::shared_ptr<MetadataValueKey<FSPath> >
ExheresLayout::e_updates_location_key() const
{
    return make_null_shared_ptr();
}

