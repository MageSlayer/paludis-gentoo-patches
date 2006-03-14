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

#include "config.h"

#include <paludis/dep_atom.hh>
#include <paludis/dep_atom_flattener.hh>
#include <paludis/dep_parser.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/config_file.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/portage_repository.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/system.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>

#include <map>
#include <fstream>
#include <functional>
#include <algorithm>
#include <vector>
#include <deque>
#include <limits>
#include <iostream>
#include <strings.h>

using namespace paludis;

typedef MakeHashedMap<QualifiedPackageName, VersionSpecCollection::Pointer>::Type VersionsMap;

typedef MakeHashedMap<QualifiedPackageName, PackageDepAtom::ConstPointer>::Type VirtualsMap;

typedef MakeHashedMap<QualifiedPackageName, std::deque<PackageDepAtom::ConstPointer> >::Type RepositoryMaskMap;

typedef MakeHashedMap<CategoryNamePart, bool>::Type CategoryMap;

typedef MakeHashedMap<QualifiedPackageName, bool>::Type PackagesMap;

typedef MakeHashedMap<UseFlagName, UseFlagState>::Type UseMap;

typedef MakeHashedSet<UseFlagName>::Type UseMaskSet;

typedef MakeHashedSet<UseFlagName>::Type UseFlagSet;

typedef MakeHashedSet<std::string>::Type MirrorSet;

typedef MakeHashedMap<std::pair<QualifiedPackageName, VersionSpec>, VersionMetadata::Pointer>::Type MetadataMap;

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

        /// Our owning env.
        const Environment * const env;

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
        mutable CategoryMap category_names;

        /// Our package names, and whether we have a fully loaded list of
        /// version specs for that category.
        mutable PackagesMap package_names;

        /// Our version specs for each package.
        mutable VersionsMap version_specs;

        /// Metadata cache.
        mutable MetadataMap metadata;

        /// Repository mask.
        mutable RepositoryMaskMap repo_mask;

        /// Have repository mask?
        mutable bool has_repo_mask;

        /// Use mask.
        mutable UseMaskSet use_mask;

        /// Use.
        mutable UseMap use;

        /// Old style virtuals name mapping.
        mutable VirtualsMap virtuals_map;

        /// Have we loaded our profile yet?
        mutable bool has_profile;

        /// Arch flags
        mutable UseFlagSet arch_list;

        /// Arch flags
        mutable UseFlagSet expand_list;

        /// Do we have arch_list?
        mutable bool has_arch_list;

        /// Do we have mirrors?
        mutable bool has_mirrors;

        mutable MirrorSet mirrors;

        /// Constructor.
        Implementation(const Environment * const,
                const PackageDatabase * const d, const FSEntry & l, const FSEntry & p,
                const FSEntry & c);

        /// Destructor.
        ~Implementation();

        /// Add a use.mask, use from a profile directory, recursive.
        void add_profile(const FSEntry & f) const;
    };
}

Implementation<PortageRepository>::Implementation(const Environment * const env,
        const PackageDatabase * const d,
        const FSEntry & l, const FSEntry & p, const FSEntry & c) :
    db(d),
    env(env),
    location(l),
    profile(p),
    cache(c),
    has_category_names(false),
    has_repo_mask(false),
    has_profile(false),
    has_arch_list(false),
    has_mirrors(false)
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
    {
        Log::get_instance()->message(ll_warning, "Profile component '" + stringify(f) +
                "' is not a directory");
        return;
    }

    if ((f / "parent").exists())
    {
        Context context_local("When reading parent file:");

        LineConfigFile parent(f / "parent");
        if (parent.begin() != parent.end())
            add_profile((f / *parent.begin()).realpath());
        else
        {
            Log::get_instance()->message(ll_warning, "Profile parent file in '" +
                    stringify(f) + "' cannot be read");
            return;
        }
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

        tokeniser.tokenise(make_defaults_f.get("USE_EXPAND"), create_inserter<UseFlagName>(
                    std::inserter(expand_list, expand_list.begin())));
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

PortageRepository::PortageRepository(
        const Environment * const e, const PackageDatabase * const d,
        const FSEntry & location, const FSEntry & profile,
        const FSEntry & cache) :
    Repository(PortageRepository::fetch_repo_name(location)),
    PrivateImplementationPattern<PortageRepository>(new Implementation<PortageRepository>(e,
                d, location, profile, cache))
{
    _info.insert(std::make_pair(std::string("location"), location));
    _info.insert(std::make_pair(std::string("profile"), profile));
    _info.insert(std::make_pair(std::string("cache"), cache));
    _info.insert(std::make_pair(std::string("format"), std::string("portage")));
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
    return _imp->category_names.end() !=
        _imp->category_names.find(c);
}

bool
PortageRepository::do_has_package_named(const CategoryNamePart & c,
        const PackageNamePart & p) const
{
    Context context("When checking for package '" + stringify(c) + "/"
            + stringify(p) + "' in " + stringify(name()) + ":");

    need_category_names();
    need_virtual_names();

    CategoryMap::iterator cat_iter(
            _imp->category_names.find(c));

    if (_imp->category_names.end() == cat_iter)
        return false;

    const QualifiedPackageName n(c, p);

    if (cat_iter->second)
        return _imp->package_names.find(n) !=
            _imp->package_names.end();
    else
    {
        if (_imp->package_names.find(n) !=
                _imp->package_names.end())
            return true;

        FSEntry fs(_imp->location);
        fs /= stringify(c);
        fs /= stringify(p);
        if (! fs.is_directory())
            return false;
        _imp->package_names.insert(std::make_pair(n, false));
        return true;
    }
}

#ifndef DOXYGEN
struct CategoryFilter :
    std::unary_function<bool, QualifiedPackageName>
{
    CategoryNamePart category;

    CategoryFilter(const CategoryNamePart & c) :
        category(c)
    {
    }

    bool operator() (const QualifiedPackageName & a) const
    {
        return a.get<qpn_category>() == category;
    }
};
#endif

CategoryNamePartCollection::ConstPointer
PortageRepository::do_category_names() const
{
    Context context("When fetching category names in " + stringify(name()) + ":");

    need_category_names();

    CategoryNamePartCollection::Pointer result(new CategoryNamePartCollection);
    CategoryMap::const_iterator i(_imp->category_names.begin()),
        i_end(_imp->category_names.end());
    for ( ; i != i_end ; ++i)
        result->insert(i->first);
    return result;
}

QualifiedPackageNameCollection::ConstPointer
PortageRepository::do_package_names(const CategoryNamePart & c) const
{
    /* this isn't particularly fast because it isn't called very often. avoid
     * changing the data structures used to make this faster at the expense of
     * slowing down single item queries. */

    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    need_category_names();
    need_virtual_names();

    if (_imp->category_names.end() == _imp->category_names.find(c))
        return QualifiedPackageNameCollection::Pointer(new QualifiedPackageNameCollection);

    for (DirIterator d(_imp->location / stringify(c)), d_end ; d != d_end ; ++d)
    {
        if (! d->is_directory())
            continue;
        if (DirIterator() == std::find_if(DirIterator(*d), DirIterator(),
                    IsFileWithExtension(".ebuild")))
            continue;

        try
        {
            _imp->package_names.insert(std::make_pair(
                        QualifiedPackageName(c, PackageNamePart(d->basename())), false));
        }
        catch (const NameError &)
        {
            Log::get_instance()->message(ll_warning, "Skipping entry '" +
                    d->basename() + "' in category '" + stringify(c) + "' in repository '"
                    + stringify(name()) + "'");
        }
    }

    _imp->category_names[c] = true;

    QualifiedPackageNameCollection::Pointer result(new QualifiedPackageNameCollection);

    std::copy(_imp->package_names.begin(), _imp->package_names.end(),
            transform_inserter(filter_inserter(result->inserter(), CategoryFilter(c)),
                    SelectFirst<QualifiedPackageName, bool>()));

    return result;
}

VersionSpecCollection::ConstPointer
PortageRepository::do_version_specs(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    if (has_package_named(n))
    {
        need_version_names(n);
        return _imp->version_specs.find(n)->second;
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
                _imp->version_specs.find(QualifiedPackageName(c, p))->second);
        return vv->end() != vv->find(v);
    }
    else
        return false;
}

void
PortageRepository::need_category_names() const
{
    if (_imp->has_category_names)
        return;

    Context context("When loading category names for " + stringify(name()) + ":");

    LineConfigFile cats(_imp->location / "profiles" / "categories");

    for (LineConfigFile::Iterator line(cats.begin()), line_end(cats.end()) ;
            line != line_end ; ++line)
        _imp->category_names.insert(std::make_pair(CategoryNamePart(*line), false));

    _imp->has_category_names = true;
}

void
PortageRepository::need_version_names(const QualifiedPackageName & n) const
{
    need_virtual_names();

    if (_imp->package_names[n])
        return;

    Context context("When loading versions for '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    VersionSpecCollection::Pointer v(new VersionSpecCollection);

    FSEntry path(_imp->location / stringify(n.get<qpn_category>()) /
            stringify(n.get<qpn_package>()));
    if (CategoryNamePart("virtual") == n.get<qpn_category>() && ! path.exists())
    {
        VirtualsMap::iterator i(
                _imp->virtuals_map.find(n));
        need_version_names(i->second->package());

        VersionSpecCollection::ConstPointer versions(version_specs(i->second->package()));
        for (VersionSpecCollection::Iterator vv(versions->begin()), vv_end(versions->end()) ;
                vv != vv_end ; ++vv)
        {
            PackageDatabaseEntry e(i->second->package(), *vv, name());
            if (! match_package(_imp->db, i->second, e))
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


    _imp->version_specs.insert(std::make_pair(n, v));
    _imp->package_names[n] = true;
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
    Log::get_instance()->message(ll_qa, "Couldn't open repo_name file in '"
            + location + "/profiles/'. Falling back to a generated name.");

    std::string modified_location(location);
    std::replace(modified_location.begin(), modified_location.end(), '/', '-');
    return RepositoryName("x-" + modified_location);
}

namespace
{
    std::string
    log_level_string()
    {
        switch (Log::get_instance()->log_level())
        {
            case ll_qa:
                return "qa";

            case ll_warning:
                return "warning";

            case ll_debug:
                return "debug";

            case ll_silent:
                return "silent";

            case last_ll:
                ;
        };

        throw InternalError(PALUDIS_HERE, "Bad log level");
    }
}

VersionMetadata::ConstPointer
PortageRepository::do_version_metadata(
        const CategoryNamePart & c, const PackageNamePart & p, const VersionSpec & v) const
{
    if (_imp->metadata.end() != _imp->metadata.find(
                std::make_pair(QualifiedPackageName(c, p), v)))
            return _imp->metadata.find(std::make_pair(QualifiedPackageName(c, p), v))->second;

    Context context("When fetching metadata for " + stringify(c) + "/" + stringify(p) +
            "-" + stringify(v));

    if (! has_version(c, p, v))
    {
        Log::get_instance()->message(ll_warning, "has_version failed for request for '" +
                stringify(c) + "/" + stringify(p) + "-" + stringify(v) + "' in repository '" +
                stringify(name()) + "'");
        return VersionMetadata::ConstPointer(new VersionMetadata);
    }

    VersionMetadata::Pointer result(new VersionMetadata);

    FSEntry cache_file(_imp->cache);
    cache_file /= stringify(c);
    cache_file /= stringify(p) + "-" + stringify(v);

    bool ok(false);
    VirtualsMap::iterator vi(_imp->virtuals_map.end());
    if (cache_file.is_regular_file())
    {
        std::ifstream cache(std::string(cache_file).c_str());
        std::string line;

        if (cache)
        {
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

            ok = true;
        }
        else
            Log::get_instance()->message(ll_warning, "Couldn't read the cache file at '"
                    + stringify(cache_file) + "'");
    }
    else if (_imp->virtuals_map.end() != ((vi = _imp->virtuals_map.find(
                QualifiedPackageName(c, p)))))
    {
        VersionMetadata::ConstPointer m(version_metadata(vi->second->package(), v));
        result->set(vmk_slot, m->get(vmk_slot));
        result->set(vmk_keywords, m->get(vmk_keywords));
        result->set(vmk_eapi, m->get(vmk_eapi));
        result->set(vmk_virtual, stringify(vi->second->package()));
        result->set(vmk_depend, "=" + stringify(vi->second->package()) + "-" + stringify(v));
        ok = true;
    }

    if (! ok)
    {
        if (_imp->cache.basename() != "empty")
            Log::get_instance()->message(ll_warning, "No cache entry for '" + stringify(c) + "/" +
                    stringify(p) + "-" + stringify(v) + "' in '" + stringify(name()) + "'");

        std::string cmd(
                "env P='" + stringify(p) + "-" + stringify(v.remove_revision()) + "' " +
                "PV='" + stringify(v.remove_revision()) + "' " +
                "PR='" + v.revision_only() + "' " +
                "PN='" + stringify(p) + "' " +
                "PVR='" + stringify(v.remove_revision()) + "-" + v.revision_only() + "' " +
                "PF='" + stringify(p) + "-" + stringify(v) + "' " +
                "A='' " +
                "CATEGORY='" + stringify(c) + "' " +
                "FILESDIR='" + stringify(_imp->location) + "/" + stringify(c) + "/" +
                    stringify(p) + "/files/' " +
                "ECLASSDIR='" + stringify(_imp->location) + "/eclass/' " +
                "PORTDIR='" + stringify(_imp->location) + "/' " +
                "DISTDIR='" + stringify(_imp->location) + "/distfiles/' " +
                "WORKDIR='/dev/null' " +
                "PALUDIS_TMPDIR='/dev/null' " +
                "KV='" + kernel_version() + "' " +
                "PALUDIS_EBUILD_LOG_LEVEL='" + log_level_string() + "' " +
                getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis") +
                "/ebuild.bash '" +
                stringify(_imp->location) + "/" + stringify(c) + "/" + stringify(p) + "/" +
                stringify(p) + "-" + stringify(v) + ".ebuild' metadata");

        PStream prog(cmd);
        KeyValueConfigFile f(&prog);

        result->set(vmk_depend,      f.get("DEPEND"));
        result->set(vmk_rdepend,     f.get("RDEPEND"));
        result->set(vmk_slot,        f.get("SLOT"));
        result->set(vmk_src_uri,     f.get("SRC_URI"));
        result->set(vmk_restrict,    f.get("RESTRICT"));
        result->set(vmk_homepage,    f.get("HOMEPAGE"));
        result->set(vmk_license,     f.get("LICENSE"));
        result->set(vmk_description, f.get("DESCRIPTION"));
        result->set(vmk_keywords,    f.get("KEYWORDS"));
        result->set(vmk_inherited,   f.get("INHERITED"));
        result->set(vmk_iuse,        f.get("IUSE"));
        result->set(vmk_pdepend,     f.get("PDEPEND"));
        result->set(vmk_provide,     f.get("PROVIDE"));
        result->set(vmk_eapi,        f.get("EAPI"));
        result->set(vmk_virtual, "");
        result->set(vmk_e_keywords,  f.get("E_KEYWORDS"));

        if (prog.exit_status())
        {
            Log::get_instance()->message(ll_warning, "Could not generate cache for '"
                    + stringify(c) + "/" + stringify(p) + "-" + stringify(v) + "' in repository '"
                    + stringify(name()) + "'");
            result->set(vmk_eapi, "UNKNOWN");
        }
        else
            ok = true;
    }

    _imp->metadata.insert(std::make_pair(std::make_pair(QualifiedPackageName(c, p), v), result));
    return result;
}

bool
PortageRepository::do_query_repository_masks(const CategoryNamePart & c,
        const PackageNamePart & p, const VersionSpec & v) const
{
    if (! _imp->has_repo_mask)
    {
        Context context("When querying repository mask for '" + stringify(c) + "/" + stringify(p) + "-"
                + stringify(v) + "':");

        FSEntry fff(_imp->location / "profiles" / "package.mask");
        if (fff.exists())
        {
            LineConfigFile ff(fff);
            for (LineConfigFile::Iterator line(ff.begin()), line_end(ff.end()) ;
                    line != line_end ; ++line)
            {
                PackageDepAtom::ConstPointer a(new PackageDepAtom(*line));
                _imp->repo_mask[a->package()].push_back(a);
            }
        }

        _imp->has_repo_mask = true;
    }

    RepositoryMaskMap::iterator r(
            _imp->repo_mask.find(QualifiedPackageName(c, p)));
    if (_imp->repo_mask.end() == r)
        return false;
    else
        for (IndirectIterator<std::deque<PackageDepAtom::ConstPointer>::const_iterator, const PackageDepAtom>
                k(r->second.begin()), k_end(r->second.end()) ; k != k_end ; ++k)
            if (match_package(_imp->db, *k, PackageDatabaseEntry(
                            QualifiedPackageName(c, p), v, name())))
                return true;

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
    if (! _imp->has_profile)
    {
        Context context("When checking USE state for '" + stringify(f) + "':");
        _imp->add_profile(_imp->profile.realpath());
        _imp->has_profile = true;
    }

    UseMap::iterator p(_imp->use.end());
    if (_imp->use.end() == ((p = _imp->use.find(f))))
        return use_unspecified;
    else
        return p->second;
}

bool
PortageRepository::do_query_use_mask(const UseFlagName & u) const
{
    if (! _imp->has_profile)
    {
        Context context("When checking USE mask for '" + stringify(u) + "':");
        _imp->add_profile(_imp->profile.realpath());
        _imp->has_profile = true;
    }

    return _imp->use_mask.end() != _imp->use_mask.find(u);
}

void
PortageRepository::need_virtual_names() const
{
    if (! _imp->has_profile)
    {
        Context context("When loading virtual names:");
        _imp->add_profile(_imp->profile.realpath());
        _imp->has_profile = true;

        for (VirtualsMap::const_iterator
                v(_imp->virtuals_map.begin()), v_end(_imp->virtuals_map.end()) ;
                v != v_end ; ++v)
            _imp->package_names.insert(
                    std::make_pair(v->first, false));
    }
}

CountedPtr<Repository>
PortageRepository::make_portage_repository(
        const Environment * const env,
        const PackageDatabase * const db,
        const std::map<std::string, std::string> & m)
{
    Context context("When making Portage repository from repo_file '" +
            (m.end() == m.find("repo_file") ? std::string("?") : m.find("repo_file")->second) + "':");

    std::string location;
    if (m.end() == m.find("location") || ((location = m.find("location")->second)).empty())
        throw PortageRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string profile;
    if (m.end() == m.find("profile") || ((profile = m.find("profile")->second)).empty())
        throw PortageRepositoryConfigurationError("Key 'profile' not specified or empty");

    std::string cache;
    if (m.end() == m.find("cache") || ((cache = m.find("cache")->second)).empty())
        cache = location + "/metadata/cache";

    return CountedPtr<Repository>(new PortageRepository(env, db, location, profile, cache));
}

PortageRepositoryConfigurationError::PortageRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("Portage repository configuration error: " + msg)
{
}

bool
PortageRepository::do_is_arch_flag(const UseFlagName & u) const
{
    if (! _imp->has_arch_list)
    {
        Context context("When checking arch list for '" + stringify(u) + "':");

        LineConfigFile archs(_imp->location / "profiles" / "arch.list");
        std::copy(archs.begin(), archs.end(), create_inserter<UseFlagName>(
                    std::inserter(_imp->arch_list, _imp->arch_list.begin())));

        _imp->has_arch_list = true;
    }

    return _imp->arch_list.end() != _imp->arch_list.find(u);
}

bool
PortageRepository::do_is_expand_flag(const UseFlagName & u) const
{
    if (! _imp->has_profile)
    {
        Context context("When checking USE_EXPAND list for '" + stringify(u) + "':");
        _imp->add_profile(_imp->profile.realpath());
        _imp->has_profile = true;
    }

    /// \todo VV no need for this to be linear
    for (UseFlagSet::const_iterator i(_imp->expand_list.begin()),
            i_end(_imp->expand_list.end()) ; i != i_end ; ++i)
        if (0 == strncasecmp(
                    stringify(u).c_str(),
                    (stringify(*i) + "_").c_str(),
                    stringify(*i).length() + 1))
            return true;

    return false;
}

bool
PortageRepository::do_is_licence(const std::string & s) const
{
    FSEntry l(_imp->location);
    l /= "licenses";

    if (! l.is_directory())
        return false;

    l /= s;
    return l.exists() && l.is_regular_file();
}

bool
PortageRepository::do_is_mirror(const std::string & s) const
{
    if (! _imp->has_mirrors)
    {
        static Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");

        LineConfigFile mirrors(_imp->location / "profiles" / "thirdpartymirrors");
        for (LineConfigFile::Iterator line(mirrors.begin()) ; line != mirrors.end() ; ++line)
        {
            std::vector<std::string> entries;
            tokeniser.tokenise(*line, std::back_inserter(entries));
            if (! entries.empty())
                _imp->mirrors.insert(entries.at(0));
        }
        _imp->has_mirrors = true;
    }

    return _imp->mirrors.end() != _imp->mirrors.find(s);
}

void
PortageRepository::do_install(const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (! has_version(q, v))
        throw InternalError(PALUDIS_HERE, "TODO"); /// \todo fixme

    VersionMetadata::ConstPointer metadata(version_metadata(q, v));

    std::string archives;
    PackageDatabaseEntry e(q, v, name());
    DepAtomFlattener f(_imp->env, &e,
            DepParser::parse(metadata->get(vmk_src_uri),
                DepParserPolicy<PlainTextDepAtom, false>::get_instance()));

    for (DepAtomFlattener::Iterator ff(f.begin()), ff_end(f.end()) ; ff != ff_end ; ++ff)
    {
        std::string::size_type p((*ff)->text().rfind('/'));
        if (std::string::npos == p)
            archives.append((*ff)->text());
        else
            archives.append((*ff)->text().substr(p + 1));
        archives.append(" ");
    }

    std::string cmd(
            "env P='" + stringify(q.get<qpn_package>()) + "-" + stringify(v.remove_revision()) + "' " +
            "PV='" + stringify(v.remove_revision()) + "' " +
            "PR='" + v.revision_only() + "' " +
            "PN='" + stringify(q.get<qpn_package>()) + "' " +
            "PVR='" + stringify(v.remove_revision()) + "-" + v.revision_only() + "' " +
            "PF='" + stringify(q.get<qpn_package>()) + "-" + stringify(v) + "' " +
            "A='" + archives + "' " +
            "CATEGORY='" + stringify(q.get<qpn_category>()) + "' " +
            "FILESDIR='" + stringify(_imp->location) + "/" + stringify(q.get<qpn_category>()) + "/" +
                stringify(q.get<qpn_package>()) + "/files/' " +
            "ECLASSDIR='" + stringify(_imp->location) + "/eclass/' " +
            "PORTDIR='" + stringify(_imp->location) + "/' " +
            "DISTDIR='" + stringify(_imp->location) + "/distfiles/' " +
            "PALUDIS_TMPDIR='" BIGTEMPDIR "/paludis/' " +
            "KV='" + kernel_version() + "' " +
            "PALUDIS_EBUILD_LOG_LEVEL='" + log_level_string() + "' " +
            "PALUDIS_EBUILD_DIR='" + getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis") + "' " +
            getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis") +
            "/ebuild.bash '" +
            stringify(_imp->location) + "/" + stringify(q.get<qpn_category>()) + "/" +
                stringify(q.get<qpn_package>()) + "/" +
            stringify(q.get<qpn_package>()) + "-" + stringify(v) + ".ebuild' "
            "init setup unpack compile test install preinst merge postinst tidyup");

    PStream prog(cmd);
    std::copy((std::istreambuf_iterator<char>(prog)), std::istreambuf_iterator<char>(),
            std::ostreambuf_iterator<char>(std::cout));
    if (prog.exit_status())
        throw InternalError(PALUDIS_HERE, "TODO"); /// \todo fixme
}

