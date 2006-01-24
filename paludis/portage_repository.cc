/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "dir_iterator.hh"
#include "filter_insert_iterator.hh"
#include "fs_entry.hh"
#include "indirect_iterator.hh"
#include "is_file_with_extension.hh"
#include "key_value_config_file.hh"
#include "line_config_file.hh"
#include "match_package.hh"
#include "package_database.hh"
#include "package_dep_atom.hh"
#include "portage_repository.hh"
#include "stringify.hh"
#include "strip.hh"
#include "tokeniser.hh"
#include "transform_insert_iterator.hh"
#include <map>
#include <fstream>
#include <functional>
#include <algorithm>
#include <vector>
#include <deque>

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for a PortageRepository.
     */
    template <>
    struct Implementation<PortageRepository> :
        InternalCounted<Implementation<PortageRepository> >
    {
        /// Our owning db.
        const PackageDatabase * const db;

        /// Our base location.
        FSEntry location;

        /// Our profile.
        FSEntry profile;

        /// Our cache.
        FSEntry cache;

        /// Have we loaded our category names?
        mutable bool has_category_names;

        /// Our category names, and whether we have a fully loaded list
        /// of package names for that category.
        mutable std::map<CategoryNamePart, bool> category_names;

        /// Our package names, and whether we have a fully loaded list of
        /// version specs for that category.
        mutable std::map<QualifiedPackageName, bool> package_names;

        /// Our version specs for each package.
        mutable std::map<QualifiedPackageName, VersionSpecCollection::Pointer> version_specs;

        /// Metadata cache.
        mutable std::map<std::pair<QualifiedPackageName, VersionSpec>, VersionMetadata::Pointer> metadata;

        /// Repository mask.
        mutable std::map<QualifiedPackageName, std::deque<PackageDepAtom::ConstPointer> > repo_mask;

        /// Have repository mask?
        mutable bool has_repo_mask;

        /// Use mask.
        mutable std::set<UseFlagName> use_mask;

        /// Use.
        mutable std::map<UseFlagName, UseFlagState> use;

        /// Old style virtuals name mapping.
        mutable std::map<QualifiedPackageName, PackageDepAtom::ConstPointer> virtuals_map;

        /// Have we loaded our profile yet?
        mutable bool has_profile;

        /// Constructor.
        Implementation(const PackageDatabase * const d, const FSEntry & l, const FSEntry & p,
                const FSEntry & c);

        /// Destructor.
        ~Implementation();

        /// Add a use.mask, use from a profile directory, recursive.
        void add_profile(const FSEntry & f) const;
    };
}

Implementation<PortageRepository>::Implementation(const PackageDatabase * const d,
        const FSEntry & l, const FSEntry & p, const FSEntry & c) :
    db(d),
    location(l),
    profile(p),
    cache(c),
    has_category_names(false),
    has_repo_mask(false),
    has_profile(false)
{
}

Implementation<PortageRepository>::~Implementation()
{
}

void
Implementation<PortageRepository>::add_profile(const FSEntry & f) const
{
    Context context("When reading profile directory '" + stringify(f) + "':");

    static Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");

    if (! f.is_directory())
        throw InternalError(PALUDIS_HERE, "todo"); /// \bug exception

    if ((f / "parent").exists())
    {
        Context context_local("When reading parent file:");

        LineConfigFile parent(f / "parent");
        if (parent.begin() != parent.end())
            add_profile((f / *parent.begin()).realpath());
        else
            throw InternalError(PALUDIS_HERE, "todo"); /// \bug exception
    }

    if ((f / "make.defaults").exists())
    {
        Context context_local("When reading make.defaults file:");

        KeyValueConfigFile make_defaults_f(f / "make.defaults");
        std::deque<std::string> uses;
        tokeniser.tokenise(make_defaults_f.get("USE"), std::back_inserter(uses));
        for (std::deque<std::string>::const_iterator u(uses.begin()), u_end(uses.end()) ;
                u != u_end ; ++u)
        {
            if ('-' == u->at(0))
                use[UseFlagName(u->substr(1))] = use_disabled;
            else
                use[UseFlagName(*u)] = use_enabled;
        }
    }

    if ((f / "use.mask").exists())
    {
        Context context_local("When reading use.mask file:");

        LineConfigFile use_mask_f(f / "use.mask");
        for (LineConfigFile::Iterator line(use_mask_f.begin()), line_end(use_mask_f.end()) ;
                line != line_end ; ++line)
            if ('-' == line->at(0))
                use_mask.erase(UseFlagName(line->substr(1)));
            else
                use_mask.insert(UseFlagName(*line));
    }

    if ((f / "virtuals").exists())
    {
        Context context_local("When reading virtuals file:");

        LineConfigFile virtuals_f(f / "virtuals");
        for (LineConfigFile::Iterator line(virtuals_f.begin()), line_end(virtuals_f.end()) ;
                line != line_end ; ++line)
        {
            std::deque<std::string> tokens;
            tokeniser.tokenise(*line, std::back_inserter(tokens));
            if (tokens.size() < 2)
                continue;
            virtuals_map.erase(QualifiedPackageName(tokens[0]));
            virtuals_map.insert(std::make_pair(QualifiedPackageName(tokens[0]),
                        PackageDepAtom::Pointer(new PackageDepAtom(tokens[1]))));
        }
    }
}

PortageRepository::PortageRepository(const PackageDatabase * const d,
        const FSEntry & location, const FSEntry & profile,
        const FSEntry & cache) :
    Repository(PortageRepository::fetch_repo_name(location)),
    PrivateImplementationPattern<PortageRepository>(new Implementation<PortageRepository>(
                d, location, profile, cache))
{
    _info.insert(std::make_pair("location", location));
    _info.insert(std::make_pair("profile", profile));
    _info.insert(std::make_pair("cache", cache));
    _info.insert(std::make_pair("format", "portage"));
}

PortageRepository::~PortageRepository()
{
}

bool
PortageRepository::do_has_category_named(const CategoryNamePart & c) const
{
    Context context("When checking for category '" + stringify(c) +
            "' in " + stringify(name()) + ":");

    need_category_names();
    return _implementation->category_names.end() !=
        _implementation->category_names.find(c);
}

bool
PortageRepository::do_has_package_named(const CategoryNamePart & c,
        const PackageNamePart & p) const
{
    Context context("When checking for package '" + stringify(c) + "/"
            + stringify(p) + "' in " + stringify(name()) + ":");

    need_category_names();
    need_virtual_names();

    std::map<CategoryNamePart, bool>::const_iterator cat_iter(
            _implementation->category_names.find(c));

    if (_implementation->category_names.end() == cat_iter)
        return false;

    const QualifiedPackageName n(c, p);

    if (cat_iter->second)
        return _implementation->package_names.find(n) !=
            _implementation->package_names.end();
    else
    {
        if (_implementation->package_names.find(n) !=
                _implementation->package_names.end())
            return true;

        FSEntry fs(_implementation->location);
        fs /= stringify(c);
        fs /= stringify(p);
        if (! fs.is_directory())
            return false;
        _implementation->package_names.insert(std::make_pair(n, false));
        return true;
    }
}

CategoryNamePartCollection::ConstPointer
PortageRepository::do_category_names() const
{
    Context context("When fetching category names in " + stringify(name()) + ":");

    need_category_names();

    CategoryNamePartCollection::Pointer result(new CategoryNamePartCollection);
    std::map<CategoryNamePart, bool>::const_iterator i(_implementation->category_names.begin()),
        i_end(_implementation->category_names.end());
    for ( ; i != i_end ; ++i)
        result->insert(i->first);
    return result;
}

QualifiedPackageNameCollection::ConstPointer
PortageRepository::do_package_names(const CategoryNamePart & c) const
{
    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    need_category_names();
    need_virtual_names();

    /// \todo
    throw InternalError(PALUDIS_HERE, "not implemented");
    return QualifiedPackageNameCollection::Pointer(new QualifiedPackageNameCollection);
}

VersionSpecCollection::ConstPointer
PortageRepository::do_version_specs(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    if (has_package_named(n))
    {
        need_version_names(n);
        return _implementation->version_specs.find(n)->second;
    }
    else
        return VersionSpecCollection::Pointer(new VersionSpecCollection);
}

bool
PortageRepository::do_has_version(const CategoryNamePart & c,
        const PackageNamePart & p, const VersionSpec & v) const
{
    Context context("When checking for version '" + stringify(v) + "' in '"
            + stringify(c) + "/" + stringify(p) + "' in " + stringify(name()) + ":");

    if (has_package_named(c, p))
    {
        need_version_names(QualifiedPackageName(c, p));
        VersionSpecCollection::Pointer vv(
                _implementation->version_specs.find(QualifiedPackageName(c, p))->second);
        return vv->end() != vv->find(v);
    }
    else
        return false;
}

void
PortageRepository::need_category_names() const
{
    if (_implementation->has_category_names)
        return;

    Context context("When loading category names for " + stringify(name()) + ":");

    LineConfigFile cats(_implementation->location / "profiles" / "categories");

    for (LineConfigFile::Iterator line(cats.begin()), line_end(cats.end()) ;
            line != line_end ; ++line)
        _implementation->category_names.insert(std::make_pair(CategoryNamePart(*line), false));

    _implementation->has_category_names = true;
}

void
PortageRepository::need_version_names(const QualifiedPackageName & n) const
{
    need_virtual_names();

    if (_implementation->package_names[n])
        return;

    Context context("When loading versions for '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    VersionSpecCollection::Pointer v(new VersionSpecCollection);

    FSEntry path(_implementation->location / stringify(n.get<qpn_category>()) /
            stringify(n.get<qpn_package>()));
    if (CategoryNamePart("virtual") == n.get<qpn_category>() && ! path.exists())
    {
        std::map<QualifiedPackageName, PackageDepAtom::ConstPointer>::iterator i(
                _implementation->virtuals_map.find(n));
        need_version_names(i->second->package());

        VersionSpecCollection::ConstPointer versions(version_specs(i->second->package()));
        for (VersionSpecCollection::Iterator vv(versions->begin()), vv_end(versions->end()) ;
                vv != vv_end ; ++vv)
        {
            PackageDatabaseEntry e(i->second->package(), *vv, name());
            if (! match_package(_implementation->db, i->second, e))
                continue;

            v->insert(*vv);
        }
    }
    else
        std::copy(DirIterator(path), DirIterator(),
                filter_inserter(
                    transform_inserter(
                        transform_inserter(
                            transform_inserter(v->inserter(),
                                StripTrailingString(".ebuild")),
                            StripLeadingString(stringify(n.get<qpn_package>()) + "-")),
                        std::mem_fun_ref(&FSEntry::basename)),
                    IsFileWithExtension(stringify(n.get<qpn_package>()) + "-", ".ebuild")));


    _implementation->version_specs.insert(std::make_pair(n, v));
    _implementation->package_names[n] = true;
}

RepositoryName
PortageRepository::fetch_repo_name(const std::string & location)
{
    try
    {
        do
        {
            FSEntry name_file(location);
            name_file /= "profiles";
            name_file /= "repo_name";

            if (! name_file.is_regular_file())
                break;

            LineConfigFile f(name_file);
            if (f.begin() == f.end())
                break;
            return RepositoryName(*f.begin());

        } while (false);
    }
    catch (...)
    {
    }
    return RepositoryName("x-" + location);
}

VersionMetadata::ConstPointer
PortageRepository::do_version_metadata(
        const CategoryNamePart & c, const PackageNamePart & p, const VersionSpec & v) const
{
    if (_implementation->metadata.end() != _implementation->metadata.find(
                std::make_pair(QualifiedPackageName(c, p), v)))
            return _implementation->metadata.find(std::make_pair(QualifiedPackageName(c, p), v))->second;

    if (! has_version(c, p, v))
        throw InternalError(PALUDIS_HERE, "todo: has_version failed for do_version_metadata"); /// \bug todo

    VersionMetadata::Pointer result(new VersionMetadata);

    FSEntry cache_file(_implementation->cache);
    cache_file /= stringify(c);
    cache_file /= stringify(p) + "-" + stringify(v);

    std::map<QualifiedPackageName, PackageDepAtom::ConstPointer>::iterator vi;
    if (cache_file.is_regular_file())
    {
        std::ifstream cache(std::string(cache_file).c_str());
        std::string line;

        if (! cache)
            throw InternalError(PALUDIS_HERE, "todo");

        /// \bug this lot
        std::getline(cache, line); result->set(vmk_depend,      line);
        std::getline(cache, line); result->set(vmk_rdepend,     line);
        std::getline(cache, line); result->set(vmk_slot,        line);
        std::getline(cache, line); result->set(vmk_src_uri,     line);
        std::getline(cache, line); result->set(vmk_restrict,    line);
        std::getline(cache, line); result->set(vmk_homepage,    line);
        std::getline(cache, line); result->set(vmk_license,     line);
        std::getline(cache, line); result->set(vmk_description, line);
        std::getline(cache, line); result->set(vmk_keywords,    line);
        std::getline(cache, line); result->set(vmk_inherited,   line);
        std::getline(cache, line); result->set(vmk_iuse,        line);
        std::getline(cache, line);
        std::getline(cache, line); result->set(vmk_pdepend,     line);
        std::getline(cache, line); result->set(vmk_provide,     line);
        std::getline(cache, line); result->set(vmk_eapi,        line);
        result->set(vmk_virtual, "");
    }
    else if (_implementation->virtuals_map.end() != ((vi = _implementation->virtuals_map.find(
                QualifiedPackageName(c, p)))))
    {
        VersionMetadata::ConstPointer m(version_metadata(vi->second->package(), v));
        result->set(vmk_slot, m->get(vmk_slot));
        result->set(vmk_keywords, m->get(vmk_keywords));
        result->set(vmk_eapi, m->get(vmk_eapi));
        result->set(vmk_virtual, stringify(vi->second->package()));
        result->set(vmk_depend, "=" + stringify(vi->second->package()) + "-" + stringify(v));
    }
    else
        throw InternalError(PALUDIS_HERE, "no cache handling not implemented"); /// \todo

    _implementation->metadata.insert(std::make_pair(std::make_pair(QualifiedPackageName(c, p), v), result));
    return result;
}

bool
PortageRepository::do_query_repository_masks(const CategoryNamePart & c,
        const PackageNamePart & p, const VersionSpec & v) const
{
    if (! _implementation->has_repo_mask)
    {
        Context context("When querying repository mask for '" + stringify(c) + "/" + stringify(p) + "-"
                + stringify(v) + "':");

        LineConfigFile ff(_implementation->location / "profiles" / "package.mask");
        for (LineConfigFile::Iterator line(ff.begin()), line_end(ff.end()) ;
                line != line_end ; ++line)
        {
            PackageDepAtom::ConstPointer a(new PackageDepAtom(*line));
            _implementation->repo_mask[a->package()].push_back(a);
        }

        _implementation->has_repo_mask = true;
    }

    std::map<QualifiedPackageName, std::deque<PackageDepAtom::ConstPointer> >::const_iterator r(
            _implementation->repo_mask.find(QualifiedPackageName(c, p)));
    if (_implementation->repo_mask.end() == r)
        return false;
    else
    {
        for (IndirectIterator<std::deque<PackageDepAtom::ConstPointer>::const_iterator, const PackageDepAtom>
                k(r->second.begin()), k_end(r->second.end()) ; k != k_end ; ++k)
        {
            if (k->package() != QualifiedPackageName(c, p))
                continue;
            if (k->version_spec_ptr() && ! ((v.*
                            (k->version_operator().as_version_spec_operator()))
                        (*k->version_spec_ptr())))
                continue;
            /// \bug slot

            return true;
        }
    }

    return false;
}

bool
PortageRepository::do_query_profile_masks(const CategoryNamePart &,
        const PackageNamePart &, const VersionSpec &) const
{
    /// \todo
    return false;
}

UseFlagState
PortageRepository::do_query_use(const UseFlagName & f) const
{
    if (! _implementation->has_profile)
    {
        Context context("When checking USE state for '" + stringify(f) + "':");
        _implementation->add_profile(_implementation->profile.realpath());
        _implementation->has_profile = true;
    }

    std::map<UseFlagName, UseFlagState>::const_iterator p;
    if (_implementation->use.end() == ((p = _implementation->use.find(f))))
        return use_unspecified;
    else
        return p->second;
}

bool
PortageRepository::do_query_use_mask(const UseFlagName & u) const
{
    if (! _implementation->has_profile)
    {
        Context context("When checking USE mask for '" + stringify(u) + "':");
        _implementation->add_profile(_implementation->profile.realpath());
        _implementation->has_profile = true;
    }

    return _implementation->use_mask.end() != _implementation->use_mask.find(u);
}

void
PortageRepository::need_virtual_names() const
{
    if (! _implementation->has_profile)
    {
        Context context("When loading virtual names:");
        _implementation->add_profile(_implementation->profile.realpath());
        _implementation->has_profile = true;

        for (std::map<QualifiedPackageName, PackageDepAtom::ConstPointer>::const_iterator
                v(_implementation->virtuals_map.begin()), v_end(_implementation->virtuals_map.end()) ;
                v != v_end ; ++v)
            _implementation->package_names.insert(
                    std::make_pair(v->first, false));
    }
}

