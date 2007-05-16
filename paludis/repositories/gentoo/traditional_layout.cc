/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#include <paludis/repositories/gentoo/traditional_layout.hh>
#include <paludis/repositories/gentoo/portage_repository_entries.hh>
#include <paludis/config_file.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/strip.hh>

#include <tr1/functional>
#include <functional>
#include <algorithm>

using namespace paludis;

typedef MakeHashedMap<CategoryNamePart, bool>::Type CategoryMap;
typedef MakeHashedMap<QualifiedPackageName, bool>::Type PackagesMap;
typedef MakeHashedMap<QualifiedPackageName, std::tr1::shared_ptr<VersionSpecCollection> >::Type VersionsMap;

namespace paludis
{
    template<>
    struct Implementation<TraditionalLayout>
    {
        const RepositoryName name;
        const FSEntry tree_root;

        mutable bool has_category_names;
        mutable CategoryMap category_names;
        mutable PackagesMap package_names;
        mutable VersionsMap version_specs;

        mutable std::tr1::shared_ptr<CategoryNamePartCollection> category_names_collection;
        std::tr1::shared_ptr<const PortageRepositoryEntries> entries;

        Implementation(const RepositoryName & n, const FSEntry & t,
                std::tr1::shared_ptr<const PortageRepositoryEntries> e) :
            name(n),
            tree_root(t),
            has_category_names(false),
            entries(e)
        {
        }
    };
}

TraditionalLayout::TraditionalLayout(const RepositoryName & name, const FSEntry & tree_root,
        std::tr1::shared_ptr<const PortageRepositoryEntries> e) :
    PrivateImplementationPattern<TraditionalLayout>(new Implementation<TraditionalLayout>(name, tree_root, e))
{
}

TraditionalLayout::~TraditionalLayout()
{
}

void
TraditionalLayout::need_category_names() const
{
    if (_imp->has_category_names)
        return;

    Context context("When loading category names for " + stringify(_imp->name) + ":");

    Log::get_instance()->message(ll_debug, lc_context, "need_category_names");

    bool found_one(false);

    for (ProfilesDirsIterator p(begin_profiles_dirs()), p_end(end_profiles_dirs()) ;
            p != p_end ; ++p)
    {
        if (! (*p / "categories").exists())
            continue;

        LineConfigFile cats(*p / "categories", LineConfigFileOptions());

        for (LineConfigFile::Iterator line(cats.begin()), line_end(cats.end()) ;
                line != line_end ; ++line)
        {
            try
            {
                _imp->category_names.insert(std::make_pair(CategoryNamePart(*line), false));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Skipping line '"
                        + *line + "' in '" + stringify(*p / "categories") + "' due to exception '"
                        + stringify(e.message()) + "' ('" + e.what() + ")");
            }
        }

        found_one = true;
    }

    if (! found_one)
    {
        Log::get_instance()->message(ll_qa, lc_context, "No categories file for repository at '"
                + stringify(_imp->tree_root) + "', faking it");
        for (DirIterator d(_imp->tree_root), d_end ; d != d_end ; ++d)
        {
            if (! d->is_directory_or_symlink_to_directory())
                continue;

            std::string n(d->basename());
            if (n == "CVS" || n == "distfiles" || n == "scripts" || n == "eclass" || n == "licences"
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
TraditionalLayout::need_version_specs(const QualifiedPackageName & n) const
{
    if (_imp->package_names[n])
        return;

    Context context("When loading versions for '" + stringify(n) + "' in "
            + stringify(_imp->name) + ":");

    std::tr1::shared_ptr<VersionSpecCollection> v(new VersionSpecCollection::Concrete);

    FSEntry path(_imp->tree_root / stringify(n.category) / stringify(n.package));

    for (DirIterator e(path), e_end ; e != e_end ; ++e)
    {
        if (! _imp->entries->is_package_file(n, *e))
            continue;

        try
        {
            if (! v->insert(VersionSpec(_imp->entries->extract_package_file_version(n, *e))))
                Log::get_instance()->message(ll_warning, lc_context, "Ignoring entry '" + stringify(*e)
                        + "' for '" + stringify(n) + "' in repository '" + stringify(_imp->name)
                        + "' because another equivalent version already exists");
        }
        catch (const Exception & ee)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Skipping entry '"
                    + stringify(*e) + "' for '" + stringify(n) + "' in repository '"
                    + stringify(_imp->name) + "' due to exception '" + ee.message() + "' ("
                    + ee.what() + ")'");
        }
    }

    _imp->version_specs.insert(std::make_pair(n, v));
    _imp->package_names[n] = true;
}

bool
TraditionalLayout::has_category_named(const CategoryNamePart & c) const
{
    Context context("When checking for category '" + stringify(c) + "' in '" + stringify(_imp->name) + "':");

    need_category_names();
    return _imp->category_names.end() != _imp->category_names.find(c);
}

bool
TraditionalLayout::has_package_named(const QualifiedPackageName & q) const
{
    Context context("When checking for package '" + stringify(q) + "' in '" + stringify(_imp->name) + ":");

    need_category_names();

    CategoryMap::iterator cat_iter(_imp->category_names.find(q.category));

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
        fs /= stringify(q.category);
        fs /= stringify(q.package);
        if (! fs.is_directory_or_symlink_to_directory())
            return false;
        _imp->package_names.insert(std::make_pair(q, false));
        return true;
    }
}

void
TraditionalLayout::need_category_names_collection() const
{
    if (_imp->category_names_collection)
        return;

    need_category_names();

    _imp->category_names_collection.reset(new CategoryNamePartCollection::Concrete);
    std::copy(_imp->category_names.begin(), _imp->category_names.end(),
            transform_inserter(_imp->category_names_collection->inserter(),
                SelectFirst<const CategoryNamePart, bool>()));
}

std::tr1::shared_ptr<const CategoryNamePartCollection>
TraditionalLayout::category_names() const
{
    Context context("When fetching category names in " + stringify(stringify(_imp->name)) + ":");

    need_category_names_collection();
    return _imp->category_names_collection;
}

std::tr1::shared_ptr<const QualifiedPackageNameCollection>
TraditionalLayout::package_names(const CategoryNamePart & c) const
{
    using namespace std::tr1::placeholders;

    /* this isn't particularly fast because it isn't called very often. avoid
     * changing the data structures used to make this faster at the expense of
     * slowing down single item queries. */

    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(_imp->name) + ":");

    need_category_names();

    if (_imp->category_names.end() == _imp->category_names.find(c))
        return std::tr1::shared_ptr<QualifiedPackageNameCollection>(new QualifiedPackageNameCollection::Concrete);

    if ((_imp->tree_root / stringify(c)).is_directory_or_symlink_to_directory())
        for (DirIterator d(_imp->tree_root / stringify(c)), d_end ; d != d_end ; ++d)
        {
            try
            {
                if (! d->is_directory_or_symlink_to_directory())
                    continue;

                if (DirIterator() == std::find_if(DirIterator(*d), DirIterator(),
                            std::tr1::bind(&PortageRepositoryEntries::is_package_file, _imp->entries.get(),
                                c + PackageNamePart(d->basename()), _1)))
                    continue;

                _imp->package_names.insert(std::make_pair(c + PackageNamePart(d->basename()), false));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Skipping entry '" +
                        d->basename() + "' in category '" + stringify(c) + "' in repository '"
                        + stringify(_imp->name) + "' (" + e.message() + ")");
            }
        }

    _imp->category_names[c] = true;

    std::tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);

    std::copy(_imp->package_names.begin(), _imp->package_names.end(),
            transform_inserter(filter_inserter(result->inserter(),
                    std::tr1::bind(std::equal_to<CategoryNamePart>(), c,
                        std::tr1::bind(SelectMember<const QualifiedPackageName, CategoryNamePart, &QualifiedPackageName::category>(), _1))),
                SelectFirst<const QualifiedPackageName, bool>()));

    return result;
}

std::tr1::shared_ptr<const VersionSpecCollection>
TraditionalLayout::version_specs(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in " + stringify(_imp->name) + ":");

    if (has_package_named(n))
    {
        need_version_specs(n);
        return _imp->version_specs.find(n)->second;
    }
    else
        return std::tr1::shared_ptr<VersionSpecCollection>(new VersionSpecCollection::Concrete);
}

bool
TraditionalLayout::has_version(const QualifiedPackageName & q, const VersionSpec & v) const
{
    Context context("When checking for version '" + stringify(v) + "' in '"
            + stringify(q) + "' in " + stringify(_imp->name) + ":");

    if (has_package_named(q))
    {
        need_version_specs(q);
        std::tr1::shared_ptr<VersionSpecCollection> vv(_imp->version_specs.find(q)->second);
        return vv->end() != vv->find(v);
    }
    else
        return false;
}

FSEntry
TraditionalLayout::package_mask_file(const FSEntry & dir) const
{
    return dir / "package.mask";
}

FSEntry
TraditionalLayout::arch_list_file(const FSEntry & dir) const
{
    return dir / "arch.list";
}

FSEntry
TraditionalLayout::mirrors_file(const FSEntry & dir) const
{
    return dir / "thirdpartymirrors";
}

FSEntry
TraditionalLayout::info_packages_file(const FSEntry & dir) const
{
    return dir / "info_pkgs";
}

FSEntry
TraditionalLayout::info_variables_file(const FSEntry & dir) const
{
    return dir / "info_vars";
}

