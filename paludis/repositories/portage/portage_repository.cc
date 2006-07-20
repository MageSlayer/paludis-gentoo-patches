/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "config.h"

#include <paludis/repositories/portage/portage_repository.hh>
#include <paludis/repositories/portage/portage_repository_profile.hh>
#include <paludis/repositories/portage/portage_repository_exceptions.hh>

#include <paludis/config_file.hh>
#include <paludis/dep_atom.hh>
#include <paludis/dep_atom_flattener.hh>
#include <paludis/ebuild.hh>
#include <paludis/environment.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/syncer.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/random.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>

#include <map>
#include <fstream>
#include <functional>
#include <algorithm>
#include <vector>
#include <limits>

#include <strings.h>
#include <ctype.h>

/** \file
 * Implementation of PortageRepository.
 *
 * \ingroup grpportagerepository
 */

using namespace paludis;

namespace paludis
{
    typedef MakeHashedSet<UseFlagName>::Type UseFlagSet;

    /// Map for versions.
    typedef MakeHashedMap<QualifiedPackageName, VersionSpecCollection::Pointer>::Type VersionsMap;

    /// Map for repository masks.
    typedef MakeHashedMap<QualifiedPackageName, std::list<PackageDepAtom::ConstPointer> >::Type RepositoryMaskMap;

    /// Map for categories.
    typedef MakeHashedMap<CategoryNamePart, bool>::Type CategoryMap;

    /// Map for packages.
    typedef MakeHashedMap<QualifiedPackageName, bool>::Type PackagesMap;

    /// Map for mirrors.
    typedef MakeHashedMap<std::string, std::list<std::string> >::Type MirrorMap;

    /// Map for metadata.
    typedef MakeHashedMap<std::pair<QualifiedPackageName, VersionSpec>,
            VersionMetadata::Pointer>::Type MetadataMap;

    /// Map for virtuals.
    typedef MakeHashedMap<QualifiedPackageName, PackageDepAtom::ConstPointer>::Type VirtualsMap;

    /**
     * Implementation data for a PortageRepository.
     *
     * \ingroup grpportagerepository
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

        /// Our profile locations.
        FSEntryCollection::Pointer profile_locations;

        /// Our cache.
        FSEntry cache;

        /// Eclass dir
        FSEntryCollection::Pointer eclassdirs;

        /// Distfiles dir
        FSEntry distdir;

        /// Sets dir
        FSEntry setsdir;

        /// Security dir
        FSEntry securitydir;

        /// News dir
        FSEntry newsdir;

        /// Sync URL
        std::string sync;

        /// Sync exclude file
        std::string sync_exclude;

        /// Root location
        FSEntry root;

        /// Build root
        FSEntry buildroot;

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

        /// Have virtual names?
        mutable bool has_virtuals;

        /// Arch flags
        mutable UseFlagSet arch_list;

        /// Do we have arch_list?
        mutable bool has_arch_list;

        /// Do we have mirrors?
        mutable bool has_mirrors;

        /// Mirrors.
        mutable MirrorMap mirrors;

        /// Constructor.
        Implementation(const PortageRepositoryParams &);

        /// Destructor.
        ~Implementation();

        /// Invalidate our cache.
        void invalidate() const;

        /// (Empty) provides map.
        const std::map<QualifiedPackageName, QualifiedPackageName> provide_map;

        /// Load profiles, if we haven't already.
        inline void need_profiles() const;

        /// Our profile.
        mutable PortageRepositoryProfile::Pointer profile_ptr;

        /// Our virtuals
        mutable VirtualsMap our_virtuals;

        /// Have we loaded our virtuals?
        bool has_our_virtuals;
    };

    Implementation<PortageRepository>::Implementation(const PortageRepositoryParams & p) :
        db(p.get<prpk_package_database>()),
        env(p.get<prpk_environment>()),
        location(p.get<prpk_location>()),
        profile_locations(p.get<prpk_profiles>()),
        cache(p.get<prpk_cache>()),
        eclassdirs(p.get<prpk_eclassdirs>()),
        distdir(p.get<prpk_distdir>()),
        setsdir(p.get<prpk_setsdir>()),
        securitydir(p.get<prpk_securitydir>()),
        newsdir(p.get<prpk_newsdir>()),
        sync(p.get<prpk_sync>()),
        sync_exclude(p.get<prpk_sync_exclude>()),
        root(p.get<prpk_root>()),
        buildroot(p.get<prpk_buildroot>()),
        has_category_names(false),
        has_repo_mask(false),
        has_virtuals(false),
        has_arch_list(false),
        has_mirrors(false),
        profile_ptr(0),
        has_our_virtuals(false)
    {
    }

    Implementation<PortageRepository>::~Implementation()
    {
    }

    void
    Implementation<PortageRepository>::need_profiles() const
    {
        if (profile_ptr)
            return;

        profile_ptr.assign(new PortageRepositoryProfile(env, *profile_locations));
    }

    void
    Implementation<PortageRepository>::invalidate() const
    {
        profile_ptr.zero();
        has_category_names = false;
        category_names.clear();
        package_names.clear();
        version_specs.clear();
        metadata.clear();
        repo_mask.clear();
        has_repo_mask = false;
        has_virtuals = false;
        arch_list.clear();
        has_arch_list = false;
        has_mirrors = false;
        mirrors.clear();
    }
}

PortageRepository::PortageRepository(const PortageRepositoryParams & p) :
    Repository(PortageRepository::fetch_repo_name(stringify(p.get<prpk_location>())),
            RepositoryCapabilities::create((
                    param<repo_mask>(this),
                    param<repo_installable>(this),
                    param<repo_installed>(static_cast<InstalledInterface *>(0)),
                    param<repo_news>(this),
                    param<repo_sets>(this),
                    param<repo_syncable>(this),
                    param<repo_uninstallable>(static_cast<UninstallableInterface *>(0)),
                    param<repo_use>(this),
                    param<repo_world>(static_cast<WorldInterface *>(0)),
                    param<repo_environment_variable>(this)
                    ))),
    PrivateImplementationPattern<PortageRepository>(new Implementation<PortageRepository>(p))
{
    // the info_vars and info_pkgs info is only added on demand, since it's
    // fairly slow to calculate.
    RepositoryInfoSection::Pointer config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("location", stringify(_imp->location));
    config_info->add_kv("profiles", join(_imp->profile_locations->begin(),
                _imp->profile_locations->end(), " "));
    config_info->add_kv("eclassdirs", join(_imp->eclassdirs->begin(),
                _imp->eclassdirs->end(), " "));
    config_info->add_kv("cache", stringify(_imp->cache));
    config_info->add_kv("distdir", stringify(_imp->distdir));
    config_info->add_kv("securitydir", stringify(_imp->securitydir));
    config_info->add_kv("setsdir", stringify(_imp->setsdir));
    config_info->add_kv("newsdir", stringify(_imp->newsdir));
    config_info->add_kv("format", "portage");
    config_info->add_kv("root", stringify(_imp->root));
    config_info->add_kv("buildroot", stringify(_imp->buildroot));
    config_info->add_kv("sync", _imp->sync);
    config_info->add_kv("sync_exclude", _imp->sync_exclude);

    _info->add_section(config_info);
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
PortageRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    Context context("When checking for package '" + stringify(q) + "' in " +
            stringify(name()) + ":");

    need_category_names();

    if (q.get<qpn_category>() == CategoryNamePart("virtual"))
        need_virtual_names();

    CategoryMap::iterator cat_iter(_imp->category_names.find(q.get<qpn_category>()));

    if (_imp->category_names.end() == cat_iter)
        return false;

    if (cat_iter->second)
        return _imp->package_names.find(q) !=
            _imp->package_names.end();
    else
    {
        if (_imp->package_names.find(q) !=
                _imp->package_names.end())
            return true;

        FSEntry fs(_imp->location);
        fs /= stringify(q.get<qpn_category>());
        fs /= stringify(q.get<qpn_package>());
        if (! fs.is_directory())
            return false;
        _imp->package_names.insert(std::make_pair(q, false));
        return true;
    }
}

namespace
{
    /**
     * Filter QualifiedPackageName instances by category.
     *
     * \ingroup grpportagerepository
     */
    struct CategoryFilter :
        std::unary_function<bool, QualifiedPackageName>
    {
        /// Our category.
        CategoryNamePart category;

        /// Constructor.
        CategoryFilter(const CategoryNamePart & c) :
            category(c)
        {
        }

        /// Predicate.
        bool operator() (const QualifiedPackageName & a) const
        {
            return a.get<qpn_category>() == category;
        }
    };
}

CategoryNamePartCollection::ConstPointer
PortageRepository::do_category_names() const
{
    Context context("When fetching category names in " + stringify(name()) + ":");

    need_category_names();

    CategoryNamePartCollection::Pointer result(new CategoryNamePartCollection::Concrete);
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
    if (c == CategoryNamePart("virtual"))
        need_virtual_names();

    if (_imp->category_names.end() == _imp->category_names.find(c))
        return QualifiedPackageNameCollection::Pointer(new QualifiedPackageNameCollection::Concrete);

    if ((_imp->location / stringify(c)).is_directory())
        for (DirIterator d(_imp->location / stringify(c)), d_end ; d != d_end ; ++d)
        {
            if (! d->is_directory())
                continue;
            if (DirIterator() == std::find_if(DirIterator(*d), DirIterator(),
                        IsFileWithExtension(".ebuild")))
                continue;

            try
            {
                _imp->package_names.insert(std::make_pair(c + PackageNamePart(d->basename()), false));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Skipping entry '" +
                        d->basename() + "' in category '" + stringify(c) + "' in repository '"
                        + stringify(name()) + "' (" + e.message() + ")");
            }
        }

    _imp->category_names[c] = true;

    QualifiedPackageNameCollection::Pointer result(new QualifiedPackageNameCollection::Concrete);

    std::copy(_imp->package_names.begin(), _imp->package_names.end(),
            transform_inserter(filter_inserter(result->inserter(), CategoryFilter(c)),
                    SelectFirst<const QualifiedPackageName, bool>()));

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
        return VersionSpecCollection::Pointer(new VersionSpecCollection::Concrete);
}

bool
PortageRepository::do_has_version(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    Context context("When checking for version '" + stringify(v) + "' in '"
            + stringify(q) + "' in " + stringify(name()) + ":");

    if (has_package_named(q))
    {
        need_version_names(q);
        VersionSpecCollection::Pointer vv(_imp->version_specs.find(q)->second);
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
    if (n.get<qpn_category>() == CategoryNamePart("virtual"))
        need_virtual_names();

    if (_imp->package_names[n])
        return;

    Context context("When loading versions for '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    VersionSpecCollection::Pointer v(new VersionSpecCollection::Concrete);

    FSEntry path(_imp->location / stringify(n.get<qpn_category>()) /
            stringify(n.get<qpn_package>()));
    if (CategoryNamePart("virtual") == n.get<qpn_category>() && ! path.exists())
    {
        VirtualsMap::iterator i(_imp->our_virtuals.find(n));
        need_version_names(i->second->package());

        VersionSpecCollection::ConstPointer versions(version_specs(i->second->package()));
        for (VersionSpecCollection::Iterator vv(versions->begin()), vv_end(versions->end()) ;
                vv != vv_end ; ++vv)
        {
            PackageDatabaseEntry e(i->second->package(), *vv, name());
            if (! match_package(_imp->env, i->second, e))
                continue;

            v->insert(*vv);
        }
    }
    else
    {
        for (DirIterator e(path), e_end ; e != e_end ; ++e)
        {
            if (! IsFileWithExtension(stringify(n.get<qpn_package>()) + "-", ".ebuild")(*e))
                continue;

            try
            {
                v->insert(strip_leading_string(
                            strip_trailing_string(e->basename(), ".ebuild"),
                            stringify(n.get<qpn_package>()) + "-"));
            }
            catch (const NameError &)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Skipping entry '"
                        + stringify(*e) + "' for '" + stringify(n) + "' in repository '"
                        + stringify(name()) + "'");
            }
        }
    }

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
    Log::get_instance()->message(ll_qa, lc_no_context, "Couldn't open repo_name file in '"
            + location + "/profiles/'. Falling back to a generated name.");

    std::string modified_location(FSEntry(location).basename());
    std::replace(modified_location.begin(), modified_location.end(), '/', '-');
    return RepositoryName("x-" + modified_location);
}

VersionMetadata::ConstPointer
PortageRepository::do_version_metadata(
        const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (_imp->metadata.end() != _imp->metadata.find(
                std::make_pair(q, v)))
            return _imp->metadata.find(std::make_pair(q, v))->second;

    Context context("When fetching metadata for '" + stringify(q) +
            "-" + stringify(v) + "':");

    if (! has_version(q, v))
    {
        Log::get_instance()->message(ll_warning, lc_context, "has_version failed for request for '" +
                stringify(q) + "-" + stringify(v) + "' in repository '" +
                stringify(name()) + "'");
        return VersionMetadata::ConstPointer(new VersionMetadata::Ebuild(
                    PortageDepParser::parse_depend));
    }

    VersionMetadata::Pointer result(new VersionMetadata::Ebuild(PortageDepParser::parse_depend));

    FSEntry cache_file(_imp->cache);
    cache_file /= stringify(q.get<qpn_category>());
    cache_file /= stringify(q.get<qpn_package>()) + "-" + stringify(v);

    bool ok(false);
    VirtualsMap::iterator vi(_imp->our_virtuals.end());
    if (cache_file.is_regular_file())
    {
        std::ifstream cache(stringify(cache_file).c_str());
        std::string line;

        if (cache)
        {
            std::getline(cache, line); result->get<vm_deps>().set<vmd_build_depend_string>(line);
            std::getline(cache, line); result->get<vm_deps>().set<vmd_run_depend_string>(line);
            std::getline(cache, line); result->set<vm_slot>(SlotName(line));
            std::getline(cache, line); result->get_ebuild_interface()->set<evm_src_uri>(line);
            std::getline(cache, line); result->get_ebuild_interface()->set<evm_restrict>(line);
            std::getline(cache, line); result->set<vm_homepage>(line);
            std::getline(cache, line); result->set<vm_license>(line);
            std::getline(cache, line); result->set<vm_description>(line);
            std::getline(cache, line); result->get_ebuild_interface()->set<evm_keywords>(line);
            std::getline(cache, line); result->get_ebuild_interface()->set<evm_inherited>(line);
            std::getline(cache, line); result->get_ebuild_interface()->set<evm_iuse>(line);
            std::getline(cache, line);
            std::getline(cache, line); result->get<vm_deps>().set<vmd_post_depend_string>(line);
            std::getline(cache, line); result->get_ebuild_interface()->set<evm_provide>(line);
            std::getline(cache, line); result->set<vm_eapi>(line);
            result->get_ebuild_interface()->set<evm_virtual>("");

            // check mtimes
            time_t cache_time(cache_file.mtime());
            ok = true;

            if ((_imp->location / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>()) /
                        (stringify(q.get<qpn_package>()) + "-" + stringify(v)
                            + ".ebuild")).mtime() > cache_time)
                ok = false;
            else
            {
                FSEntry timestamp(_imp->location / "metadata" / "timestamp");
                if (timestamp.exists())
                    cache_time = timestamp.mtime();

                std::list<std::string> inherits;
                WhitespaceTokeniser::get_instance()->tokenise(
                        stringify(result->get_ebuild_interface()->get<evm_inherited>()),
                        std::back_inserter(inherits));
                for (FSEntryCollection::Iterator e(_imp->eclassdirs->begin()),
                        e_end(_imp->eclassdirs->end()) ; e != e_end ; ++e)
                    for (std::list<std::string>::const_iterator i(inherits.begin()),
                            i_end(inherits.end()) ; i != i_end ; ++i)
                    {
                        if ((*e / (*i + ".eclass")).exists())
                            if (((*e / (*i + ".eclass"))).mtime() > cache_time)
                                ok = false;
                    }
            }

            if (! ok)
                Log::get_instance()->message(ll_warning, lc_no_context, "Stale cache file at '"
                        + stringify(cache_file) + "'");
        }
        else
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "Couldn't read the cache file at '"
                    + stringify(cache_file) + "'");
    }
    else if (_imp->our_virtuals.end() != ((vi = _imp->our_virtuals.find(q))))
    {
        VersionMetadata::ConstPointer m(version_metadata(vi->second->package(), v));
        result->set<vm_slot>(m->get<vm_slot>());
        result->get_ebuild_interface()->set<evm_keywords>(m->get_ebuild_interface()->get<evm_keywords>());
        result->set<vm_eapi>(m->get<vm_eapi>());
        result->get_ebuild_interface()->set<evm_virtual>(stringify(vi->second->package()));
        result->get<vm_deps>().set<vmd_build_depend_string>(
                "=" + stringify(vi->second->package()) + "-" + stringify(v));
        ok = true;
    }

    if (! ok)
    {
        if (_imp->cache.basename() != "empty")
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "No usable cache entry for '" + stringify(q) +
                    "-" + stringify(v) + "' in '" + stringify(name()) + "'");

        PackageDatabaseEntry e(q, v, name());
        EbuildMetadataCommand cmd(EbuildCommandParams::create((
                        param<ecpk_environment>(_imp->env),
                        param<ecpk_db_entry>(&e),
                        param<ecpk_ebuild_dir>(_imp->location / stringify(q.get<qpn_category>()) /
                            stringify(q.get<qpn_package>())),
                        param<ecpk_files_dir>(_imp->location / stringify(q.get<qpn_category>()) /
                            stringify(q.get<qpn_package>()) / "files"),
                        param<ecpk_eclassdirs>(_imp->eclassdirs),
                        param<ecpk_portdir>(_imp->location),
                        param<ecpk_distdir>(_imp->distdir),
                        param<ecpk_buildroot>(_imp->buildroot)
                        )));
        if (! cmd())
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "No usable metadata for '" + stringify(q)
                    + "-" + stringify(v) + "' in '" + stringify(name()) + "'");

        if (0 == ((result = cmd.metadata())))
            throw InternalError(PALUDIS_HERE, "cmd.metadata() is zero pointer???");
    }

    _imp->metadata.insert(std::make_pair(std::make_pair(q, v), result));
    return result;
}

bool
PortageRepository::do_query_repository_masks(const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (! _imp->has_repo_mask)
    {
        Context context("When querying repository mask for '" + stringify(q) + "-"
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

    RepositoryMaskMap::iterator r(_imp->repo_mask.find(q));
    if (_imp->repo_mask.end() == r)
        return false;
    else
        for (IndirectIterator<std::list<PackageDepAtom::ConstPointer>::const_iterator, const PackageDepAtom>
                k(r->second.begin()), k_end(r->second.end()) ; k != k_end ; ++k)
            if (match_package(_imp->env, *k, PackageDatabaseEntry(q, v, name())))
                return true;

    return false;
}

bool
PortageRepository::do_query_profile_masks(const QualifiedPackageName &,
        const VersionSpec &) const
{
    return false;
}

UseFlagState
PortageRepository::do_query_use(const UseFlagName & f, const PackageDatabaseEntry *) const
{
    _imp->need_profiles();
    return _imp->profile_ptr->use_state_ignoring_masks(f);
}

bool
PortageRepository::do_query_use_mask(const UseFlagName & u, const PackageDatabaseEntry *e) const
{
    _imp->need_profiles();
    return _imp->profile_ptr->use_masked(u, e);
}

bool
PortageRepository::do_query_use_force(const UseFlagName & u, const PackageDatabaseEntry *e) const
{
    _imp->need_profiles();
    return _imp->profile_ptr->use_forced(u, e);
}

void
PortageRepository::need_virtual_names() const
{
    if (_imp->has_virtuals)
        return;

    _imp->has_virtuals = true;

    try
    {
        _imp->need_profiles();
        need_category_names();

        // don't use std::copy!
        for (PortageRepositoryProfile::VirtualsIterator i(_imp->profile_ptr->begin_virtuals()),
                i_end(_imp->profile_ptr->end_virtuals()) ; i != i_end ; ++i)
            _imp->our_virtuals.insert(*i);

        for (Environment::ProvideMapIterator p(_imp->env->begin_provide_map()),
                p_end(_imp->env->end_provide_map()) ; p != p_end ; ++p)
        {
            if (! has_package_named(p->second))
                continue;

            _imp->our_virtuals.erase(p->first);
            _imp->our_virtuals.insert(std::make_pair(p->first, PackageDepAtom::Pointer(
                            new PackageDepAtom(p->second))));
        }

        for (VirtualsMap::const_iterator
                v(_imp->our_virtuals.begin()), v_end(_imp->our_virtuals.end()) ;
                v != v_end ; ++v)
            _imp->package_names.insert(std::make_pair(v->first, false));
    }
    catch (...)
    {
        _imp->has_virtuals = false;
        throw;
    }
}

CountedPtr<Repository>
PortageRepository::make_portage_repository(
        const Environment * const env,
        const PackageDatabase * const db,
        AssociativeCollection<std::string, std::string>::ConstPointer m)
{
    std::string repo_file(m->end() == m->find("repo_file") ? std::string("?") :
            m->find("repo_file")->second);

    Context context("When making Portage repository from repo_file '" + repo_file + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw PortageRepositoryConfigurationError("Key 'location' not specified or empty");

    FSEntryCollection::Pointer profiles(new FSEntryCollection::Concrete);
    if (m->end() != m->find("profiles"))
        WhitespaceTokeniser::get_instance()->tokenise(m->find("profiles")->second,
                create_inserter<FSEntry>(std::back_inserter(*profiles)));
    if (m->end() != m->find("profile") && ! m->find("profile")->second.empty())
    {
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Key 'profile' in '" + repo_file + "' is deprecated, "
                "use 'profiles = " + m->find("profile")->second + "' instead");
        if (profiles->empty())
            profiles->append(m->find("profile")->second);
        else
            throw PortageRepositoryConfigurationError("Both 'profile' and 'profiles' keys are present");
    }
    if (profiles->empty())
        throw PortageRepositoryConfigurationError("No profiles have been specified");

    FSEntryCollection::Pointer eclassdirs(new FSEntryCollection::Concrete);
    if (m->end() != m->find("eclassdirs"))
        WhitespaceTokeniser::get_instance()->tokenise(m->find("eclassdirs")->second,
                create_inserter<FSEntry>(std::back_inserter(*eclassdirs)));
    if (m->end() != m->find("eclassdir") && ! m->find("eclassdir")->second.empty())
    {
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Key 'eclassdir' in '" + repo_file + "' is deprecated, "
                "use 'eclassdirs = " + m->find("eclassdir")->second + "' instead");
        if (eclassdirs->empty())
            eclassdirs->append(m->find("eclassdir")->second);
        else
            throw PortageRepositoryConfigurationError("Both 'eclassdir' and 'eclassdirs' keys are present");
    }
    if (eclassdirs->empty())
        eclassdirs->append(location + "/eclass");

    std::string distdir;
    if (m->end() == m->find("distdir") || ((distdir = m->find("distdir")->second)).empty())
        distdir = location + "/distfiles";

    std::string setsdir;
    if (m->end() == m->find("setsdir") || ((setsdir = m->find("setsdir")->second)).empty())
        setsdir = location + "/sets";

    std::string securitydir;
    if (m->end() == m->find("securitydir") || ((securitydir = m->find("securitydir")->second)).empty())
        securitydir = location + "/metadata/security";

    std::string newsdir;
    if (m->end() == m->find("newsdir") || ((newsdir = m->find("newsdir")->second)).empty())
        newsdir = location + "/metadata/news";

    std::string cache;
    if (m->end() == m->find("cache") || ((cache = m->find("cache")->second)).empty())
        cache = location + "/metadata/cache";

    std::string sync;
    if (m->end() == m->find("sync") || ((sync = m->find("sync")->second)).empty())
        ; // nothing

    std::string sync_exclude;
    if (m->end() == m->find("sync_exclude") || ((sync_exclude = m->find("sync_exclude")->second)).empty())
        ; // nothing

    std::string root;
    if (m->end() == m->find("root") || ((root = m->find("root")->second)).empty())
        root = "/";

    std::string buildroot;
    if (m->end() == m->find("buildroot") || ((buildroot = m->find("buildroot")->second)).empty())
        buildroot = "/var/tmp/paludis";

    return CountedPtr<Repository>(new PortageRepository(PortageRepositoryParams::create((
                        param<prpk_environment>(env),
                        param<prpk_package_database>(db),
                        param<prpk_location>(location),
                        param<prpk_profiles>(profiles),
                        param<prpk_cache>(cache),
                        param<prpk_eclassdirs>(eclassdirs),
                        param<prpk_distdir>(distdir),
                        param<prpk_securitydir>(securitydir),
                        param<prpk_setsdir>(setsdir),
                        param<prpk_newsdir>(newsdir),
                        param<prpk_sync>(sync),
                        param<prpk_sync_exclude>(sync_exclude),
                        param<prpk_root>(root),
                        param<prpk_buildroot>(buildroot)))));
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
    _imp->need_profiles();

    for (PortageRepositoryProfile::UseExpandIterator i(_imp->profile_ptr->begin_use_expand()),
            i_end(_imp->profile_ptr->end_use_expand()) ; i != i_end ; ++i)
        if (0 == strncasecmp(
                    stringify(u).c_str(),
                    (stringify(*i) + "_").c_str(),
                    stringify(*i).length() + 1))
            return true;

    return false;
}

bool
PortageRepository::do_is_expand_hidden_flag(const UseFlagName & u) const
{
    _imp->need_profiles();

    for (PortageRepositoryProfile::UseExpandIterator i(_imp->profile_ptr->begin_use_expand_hidden()),
            i_end(_imp->profile_ptr->end_use_expand_hidden()) ; i != i_end ; ++i)
        if (0 == strncasecmp(
                    stringify(u).c_str(),
                    (stringify(*i) + "_").c_str(),
                    stringify(*i).length() + 1))
            return true;

    return false;
}

std::string::size_type
PortageRepository::do_expand_flag_delim_pos(const UseFlagName & u) const
{
    _imp->need_profiles();

    for (PortageRepositoryProfile::UseExpandIterator i(_imp->profile_ptr->begin_use_expand_hidden()),
            i_end(_imp->profile_ptr->end_use_expand_hidden()) ; i != i_end ; ++i)
        if (0 == strncasecmp(
                    stringify(u).c_str(),
                    (stringify(*i) + "_").c_str(),
                    stringify(*i).length() + 1))
            return stringify(*i).length();

    throw InternalError(PALUDIS_HERE, "Use flag '" +
            stringify(u) + "' not an expand flag?");
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
        if ((_imp->location / "profiles" / "thirdpartymirrors").exists())
        {
            LineConfigFile mirrors(_imp->location / "profiles" / "thirdpartymirrors");
            for (LineConfigFile::Iterator line(mirrors.begin()) ; line != mirrors.end() ; ++line)
            {
                std::vector<std::string> entries;
                WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(entries));
                if (! entries.empty())
                {
                    /* pick up to five random mirrors only */
                    static Random r;
                    std::random_shuffle(next(entries.begin()), entries.end(), r);
                    if (entries.size() > 6)
                        entries.resize(6);
                    _imp->mirrors.insert(std::make_pair(
                                entries.at(0),
                                std::list<std::string>(next(entries.begin()), entries.end())));
                }
            }
        }
        else
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "No thirdpartymirrors file found in '"
                    + stringify(_imp->location / "profiles") + "', so mirror:// SRC_URI "
                    "components cannot be fetched");

        _imp->has_mirrors = true;
    }

    return _imp->mirrors.end() != _imp->mirrors.find(s);
}

namespace
{
    class AAFinder :
        private InstantiationPolicy<AAFinder, instantiation_method::NonCopyableTag>,
        protected DepAtomVisitorTypes::ConstVisitor
    {
        private:
            mutable std::list<const StringDepAtom *> _atoms;

        protected:
            void visit(const AllDepAtom * a)
            {
                std::for_each(a->begin(), a->end(), accept_visitor(
                            static_cast<DepAtomVisitorTypes::ConstVisitor *>(this)));
            }

            void visit(const AnyDepAtom *) PALUDIS_ATTRIBUTE((noreturn))
            {
                throw InternalError(PALUDIS_HERE, "Found unexpected AnyDepAtom");
            }

            void visit(const UseDepAtom * a)
            {
                std::for_each(a->begin(), a->end(), accept_visitor(
                            static_cast<DepAtomVisitorTypes::ConstVisitor *>(this)));
            }

            void visit(const PlainTextDepAtom * a)
            {
                _atoms.push_back(a);
            }

            void visit(const PackageDepAtom * a)
            {
                _atoms.push_back(a);
            }

            void visit(const BlockDepAtom * a)
            {
                _atoms.push_back(a);
            }

        public:
            AAFinder(const DepAtom::ConstPointer a)
            {
                a->accept(static_cast<DepAtomVisitorTypes::ConstVisitor *>(this));
            }

            typedef std::list<const StringDepAtom *>::const_iterator Iterator;

            Iterator begin()
            {
                return _atoms.begin();
            }

            Iterator end() const
            {
                return _atoms.end();
            }
    };

}

void
PortageRepository::do_install(const QualifiedPackageName & q, const VersionSpec & v,
        const InstallOptions & o) const
{
    _imp->need_profiles();

    if (! _imp->root.is_directory())
        throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                + stringify(v) + "' since root ('" + stringify(_imp->root) + "') isn't a directory");

    VersionMetadata::ConstPointer metadata(0);
    if (! has_version(q, v))
    {
        if (q.get<qpn_category>() == CategoryNamePart("virtual"))
        {
            VersionMetadata::Ebuild::Pointer m(new VersionMetadata::Ebuild(PortageDepParser::parse_depend));
            m->set<vm_slot>(SlotName("0"));
            m->get_ebuild_interface()->set<evm_virtual>(" ");
            metadata = m;
        }
        else
            throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                    + stringify(v) + "' since has_version failed");
    }
    else
        metadata = version_metadata(q, v);

    PackageDatabaseEntry e(q, v, name());

    bool fetch_restrict(false), no_mirror(false);
    {
        std::list<std::string> restricts;
        WhitespaceTokeniser::get_instance()->tokenise(
                metadata->get_ebuild_interface()->get<evm_restrict>(), std::back_inserter(restricts));
        fetch_restrict = (restricts.end() != std::find(restricts.begin(), restricts.end(), "fetch")) ||
            (restricts.end() != std::find(restricts.begin(), restricts.end(), "nofetch"));
        no_mirror = (restricts.end() != std::find(restricts.begin(), restricts.end(), "mirror")) ||
            (restricts.end() != std::find(restricts.begin(), restricts.end(), "nomirror"));
    }

    std::string archives, all_archives, flat_src_uri;
    {
        std::set<std::string> already_in_archives;

        /* make A and FLAT_SRC_URI */
        DepAtom::ConstPointer f_atom(PortageDepParser::parse(metadata->get_ebuild_interface()->get<evm_src_uri>(),
                    PortageDepParserPolicy<PlainTextDepAtom, false>::get_instance()));
        DepAtomFlattener f(_imp->env, &e, f_atom);

        for (DepAtomFlattener::Iterator ff(f.begin()), ff_end(f.end()) ; ff != ff_end ; ++ff)
        {
            std::string::size_type p((*ff)->text().rfind('/'));
            if (std::string::npos == p)
            {
                if (already_in_archives.end() == already_in_archives.find((*ff)->text()))
                {
                    archives.append((*ff)->text());
                    already_in_archives.insert((*ff)->text());
                }
            }
            else
            {
                if (already_in_archives.end() == already_in_archives.find((*ff)->text().substr(p + 1)))
                {
                    archives.append((*ff)->text().substr(p + 1));
                    already_in_archives.insert((*ff)->text().substr(p + 1));
                }
            }
            archives.append(" ");

            /* add * mirror entries */
            for (Environment::MirrorIterator
                    m(_imp->env->begin_mirrors("*")),
                    m_end(_imp->env->end_mirrors("*")) ;
                    m != m_end ; ++m)
                flat_src_uri.append(m->second + "/" + (*ff)->text().substr(p + 1) + " ");

            if (0 == (*ff)->text().compare(0, 9, "mirror://"))
            {
                std::string mirror((*ff)->text().substr(9));
                std::string::size_type q(mirror.find('/'));

                if (std::string::npos == q)
                    throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                            + stringify(v) + "' since SRC_URI is broken");

                if (! is_mirror(mirror.substr(0, q)))
                    throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                            + stringify(v) + "' since SRC_URI references unknown mirror:// '" +
                            mirror.substr(0, q) + "'");

                for (Environment::MirrorIterator
                        m(_imp->env->begin_mirrors(mirror.substr(0, q))),
                        m_end(_imp->env->end_mirrors(mirror.substr(0, q))) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(m->second + "/" + mirror.substr(q + 1) + " ");

                for (std::list<std::string>::iterator
                        m(_imp->mirrors.find(mirror.substr(0, q))->second.begin()),
                        m_end(_imp->mirrors.find(mirror.substr(0, q))->second.end()) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(*m + "/" + mirror.substr(q + 1) + " ");
            }
            else
                flat_src_uri.append((*ff)->text());
            flat_src_uri.append(" ");

            /* add mirror://gentoo/ entries */
            std::string master_mirror(strip_trailing_string(stringify(name()), "x-"));
            if (is_mirror(master_mirror) && ! no_mirror)
            {
                for (Environment::MirrorIterator
                        m(_imp->env->begin_mirrors(master_mirror)),
                        m_end(_imp->env->end_mirrors(master_mirror)) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(m->second + "/" + (*ff)->text().substr(p + 1) + " ");

                for (std::list<std::string>::iterator
                        m(_imp->mirrors.find(master_mirror)->second.begin()),
                        m_end(_imp->mirrors.find(master_mirror)->second.end()) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(*m + "/" + (*ff)->text().substr(p + 1) + " ");
            }
        }

        /* make AA */
        DepAtom::ConstPointer g_atom(PortageDepParser::parse(
                    metadata->get_ebuild_interface()->get<evm_src_uri>(),
                    PortageDepParserPolicy<PlainTextDepAtom, false>::get_instance()));
        AAFinder g(g_atom);
        std::set<std::string> already_in_all_archives;

        for (AAFinder::Iterator gg(g.begin()), gg_end(g.end()) ; gg != gg_end ; ++gg)
        {
            std::string::size_type p((*gg)->text().rfind('/'));
            if (std::string::npos == p)
            {
                if (already_in_all_archives.end() == already_in_all_archives.find((*gg)->text()))
                {
                    all_archives.append((*gg)->text());
                    already_in_all_archives.insert((*gg)->text());
                }
            }
            else
            {
                if (already_in_all_archives.end() == already_in_all_archives.find((*gg)->text().substr(p + 1)))
                {
                    all_archives.append((*gg)->text().substr(p + 1));
                    already_in_all_archives.insert((*gg)->text().substr(p + 1));
                }
            }
            all_archives.append(" ");
        }
    }

    std::string use;
    {
        std::set<UseFlagName> iuse;
        WhitespaceTokeniser::get_instance()->tokenise(metadata->get_ebuild_interface()->
                get<evm_iuse>(), create_inserter<UseFlagName>(std::inserter(iuse, iuse.begin())));
        for (std::set<UseFlagName>::const_iterator iuse_it(iuse.begin()), iuse_end(iuse.end()) ;
                iuse_it != iuse_end; ++iuse_it)
            if (_imp->env->query_use(*iuse_it, &e))
                use += (*iuse_it).data() + " ";
    }

    use += _imp->profile_ptr->environment_variable("ARCH") + " ";
    for (PortageRepositoryProfile::UseExpandIterator x(_imp->profile_ptr->begin_use_expand()),
            x_end(_imp->profile_ptr->end_use_expand()) ; x != x_end ; ++x)
    {
        std::string lower_x;
        std::transform(x->data().begin(), x->data().end(), std::back_inserter(lower_x),
                &::tolower);

        std::list<std::string> uses;
        WhitespaceTokeniser::get_instance()->tokenise(
                _imp->profile_ptr->environment_variable(stringify(*x)),
                std::back_inserter(uses));

        for (std::list<std::string>::const_iterator u(uses.begin()), u_end(uses.end()) ;
                u != u_end ; ++u)
            use += lower_x + "_" + *u + " ";

        UseFlagNameCollection::Pointer u(_imp->env->query_enabled_use_matching(
                    lower_x + "_", &e));
        for (UseFlagNameCollection::Iterator uu(u->begin()), uu_end(u->end()) ;
                uu != uu_end ; ++uu)
            use += stringify(*uu) + " ";
    }

    AssociativeCollection<std::string, std::string>::Pointer expand_vars(
            new AssociativeCollection<std::string, std::string>::Concrete);
    for (PortageRepositoryProfile::UseExpandIterator
            u(_imp->profile_ptr->begin_use_expand()),
            u_end(_imp->profile_ptr->end_use_expand()) ; u != u_end ; ++u)
    {
        std::string prefix;
        std::transform(u->data().begin(), u->data().end(), std::back_inserter(prefix),
                &::tolower);
        prefix.append("_");

        UseFlagNameCollection::Pointer x(_imp->env->query_enabled_use_matching(prefix, &e));
        std::string value;
        for (UseFlagNameCollection::Iterator xx(x->begin()), xx_end(x->end()) ;
                xx != xx_end ; ++xx)
            value.append(stringify(*xx).erase(0, stringify(*u).length() + 1) + " ");

        expand_vars->insert(stringify(*u), value);
    }

    /* Strip trailing space. Some ebuilds rely upon this. From kde-meta.eclass:
     *     [[ -n ${A/${TARBALL}/} ]] && unpack ${A/${TARBALL}/}
     * Rather annoying.
     */
    archives = strip_trailing(archives, " ");
    all_archives = strip_trailing(all_archives, " ");

    EbuildFetchCommand fetch_cmd(EbuildCommandParams::create((
                    param<ecpk_environment>(_imp->env),
                    param<ecpk_db_entry>(&e),
                    param<ecpk_ebuild_dir>(_imp->location / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>())),
                    param<ecpk_files_dir>(_imp->location / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>()) / "files"),
                    param<ecpk_eclassdirs>(_imp->eclassdirs),
                    param<ecpk_portdir>(_imp->location),
                    param<ecpk_distdir>(_imp->distdir),
                    param<ecpk_buildroot>(_imp->buildroot)
                    )),
            EbuildFetchCommandParams::create((
                    param<ecfpk_a>(archives),
                    param<ecfpk_aa>(all_archives),
                    param<ecfpk_use>(use),
                    param<ecfpk_use_expand>(join(
                            _imp->profile_ptr->begin_use_expand(),
                            _imp->profile_ptr->end_use_expand(), " ")),
                    param<ecfpk_expand_vars>(expand_vars),
                    param<ecfpk_flat_src_uri>(flat_src_uri),
                    param<ecfpk_root>(stringify(_imp->root) + "/"),
                    param<ecfpk_profiles>(_imp->profile_locations),
                    param<ecfpk_no_fetch>(fetch_restrict)
                    )));

    if (metadata->get_ebuild_interface()->get<evm_virtual>().empty())
        fetch_cmd();

    if (o.get<io_fetchonly>())
        return;

    EbuildInstallCommand install_cmd(EbuildCommandParams::create((
                    param<ecpk_environment>(_imp->env),
                    param<ecpk_db_entry>(&e),
                    param<ecpk_ebuild_dir>(_imp->location / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>())),
                    param<ecpk_files_dir>(_imp->location / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>()) / "files"),
                    param<ecpk_eclassdirs>(_imp->eclassdirs),
                    param<ecpk_portdir>(_imp->location),
                    param<ecpk_distdir>(_imp->distdir),
                    param<ecpk_buildroot>(_imp->buildroot)
                    )),
            EbuildInstallCommandParams::create((
                    param<ecipk_use>(use),
                    param<ecipk_a>(archives),
                    param<ecipk_aa>(all_archives),
                    param<ecipk_use_expand>(join(
                            _imp->profile_ptr->begin_use_expand(),
                            _imp->profile_ptr->end_use_expand(), " ")),
                    param<ecipk_expand_vars>(expand_vars),
                    param<ecipk_root>(stringify(_imp->root) + "/"),
                    param<ecipk_profiles>(_imp->profile_locations),
                    param<ecipk_disable_cfgpro>(o.get<io_noconfigprotect>()),
                    param<ecipk_merge_only>(! metadata->get_ebuild_interface()->get<evm_virtual>().empty()),
                    param<ecipk_slot>(SlotName(metadata->get<vm_slot>()))
                    )));

    install_cmd();
}

namespace
{
    inline
    PackageDepAtom::Pointer make_atom(const PackageDatabaseEntry & e)
    {
        QualifiedPackageName n(e.get<pde_name>());
        VersionSpec v(e.get<pde_version>());

        std::string s("=" + stringify(n) + "-" + stringify(v));
        return PackageDepAtom::Pointer(new PackageDepAtom(s));
    }
}

PackageDatabaseEntryCollection::Iterator
PortageRepository::find_best(PackageDatabaseEntryCollection & c, const PackageDatabaseEntry & e) const
{
    Context local("When finding best update for '" + stringify(e.get<pde_name>()) + "-" +
            stringify(e.get<pde_version>()) + "':");
    // Find an entry in c that matches e best. e is not in c.
    QualifiedPackageName n(e.get<pde_name>());
    SlotName s(_imp->env->package_database()->fetch_repository(e.get<pde_repository>())->version_metadata(
                e.get<pde_name>(), e.get<pde_version>())->get<vm_slot>());
    PackageDatabaseEntryCollection::Iterator i(c.begin()), i_end(c.end()), i_best(c.end());
    for ( ; i != i_end; ++i)
    {
        if (n != i->get<pde_name>())
            continue;
        if (s != _imp->env->package_database()->fetch_repository(i->get<pde_repository>())->version_metadata(
                    i->get<pde_name>(), i->get<pde_version>())->get<vm_slot>())
            continue;

        i_best = i;
    }

    return i_best;
}

AdvisoryVisitor::AdvisoryVisitor(const Environment * const env, const CompositeDepAtom & a) :
    _env(env),
    _a(a)
{
    Context c("When flattening the AdvisoryFile line:");
    std::for_each(a.begin(), a.end(), accept_visitor(this));
    if (_atoms.size() == 2)
    {
        VersionOperatorValue v1(_atoms[0]->version_operator().value()),
                v2(_atoms[1]->version_operator().value());

        if ((v1 == vo_equal) || (v2 == vo_equal))
            throw AdvisoryFileError("Broken line: Forbidden 'equal' atom in range");
    }
}

void
AdvisoryVisitor::visit(const AllDepAtom * a)
{
    std::for_each(a->begin(), a->end(), accept_visitor(this));
}

void
AdvisoryVisitor::visit(const AnyDepAtom *)
{
    throw AdvisoryFileError("Unexpected AnyDepAtom in line");
}

void
AdvisoryVisitor::visit(const UseDepAtom * a)
{
    if (_env->query_use(a->flag(), 0) ^ a->inverse())
        std::for_each(a->begin(), a->end(), accept_visitor(this));
}

void
AdvisoryVisitor::visit(const PackageDepAtom * a)
{
    _atoms.push_back(a);
}

void
AdvisoryVisitor::visit(const PlainTextDepAtom *)
{
}

void
AdvisoryVisitor::visit(const BlockDepAtom *)
{
}

DepAtom::Pointer
PortageRepository::do_security_set(const PackageSetOptions & o) const
{
    Context c("When building security package set:");
    AllDepAtom::Pointer security_packages(new AllDepAtom);

    bool list_affected_only(o.get<pso_list_affected_only>());
    InstallState affected_state(list_affected_only ? is_either : is_installed_only);

    if (!_imp->securitydir.is_directory())
        return DepAtom::Pointer(new AllDepAtom);

    std::list<FSEntry> advisories;
    std::copy(DirIterator(_imp->securitydir), DirIterator(),
        filter_inserter(std::back_inserter(advisories),
        IsFileWithExtension("advisory-", ".conf")));

    std::list<FSEntry>::const_iterator f(advisories.begin()),
        f_end(advisories.end());

    std::set<std::pair<PackageDatabaseEntry, std::string> > affected;
    PackageDatabaseEntryCollection::Concrete unaffected;
    std::map<std::string, std::string> advisory_map;

    for ( ; f != f_end; ++f)
    {
        Context c("When parsing security advisory '" + stringify(*f) + "':");

        try
        {
            AdvisoryFile advisory(*f);
            std::string advisory_id(advisory.get("Id"));
            advisory_map[advisory_id] = advisory.get("Title");


            AdvisoryFile::LineIterator a(advisory.begin_affected()), a_end(advisory.end_affected());
            for ( ; a != a_end ; ++a)
            {
                Context c("When parsing line 'Affected: " + *a + "':");

                CompositeDepAtom::ConstPointer line(PortageDepParser::parse(*a));
                AdvisoryVisitor atoms(_imp->env, *line);

                if ((0 == atoms.size()) || (2 < atoms.size()))
                {
                    continue;
                }

                bool is_range(2 == atoms.size());

                PackageDatabaseEntryCollection::ConstPointer affected_collection1(
                        _imp->db->query(*atoms.at(0), affected_state));
                PackageDatabaseEntryCollection::ConstPointer affected_collection2(
                        new PackageDatabaseEntryCollection::Concrete);
                PackageDatabaseEntryCollection::Iterator p(affected_collection1->begin()),
                    p_end(affected_collection1->end());

                if (is_range)
                    affected_collection2 = _imp->db->query(*atoms.at(1), affected_state);

                for ( ; p != p_end ; ++p)
                {
                    if ((affected.end() != affected.find(std::make_pair(*p, advisory_id))))
                        continue;
                    if ((! is_range) || (affected_collection2->end() != affected_collection2->find(*p)))
                        affected.insert(std::make_pair(*p, advisory_id));
                }
            }

            AdvisoryFile::LineIterator u(advisory.begin_unaffected()), u_end(advisory.end_unaffected());
            for ( ; u != u_end ; ++u)
            {
                Context c("When parsing line 'Unaffected: " + *u + "':");

                CompositeDepAtom::ConstPointer line(PortageDepParser::parse(*u));
                AdvisoryVisitor atoms(_imp->env, *line);

                if ((0 == atoms.size()) || (2 < atoms.size()))
                {
                    continue;
                }

                bool is_range(2 == atoms.size());

                PackageDatabaseEntryCollection::ConstPointer unaffected_collection1(_imp->db->query(
                            *atoms.at(0), is_either));
                PackageDatabaseEntryCollection::ConstPointer unaffected_collection2(
                        new PackageDatabaseEntryCollection::Concrete);
                PackageDatabaseEntryCollection::Iterator p(unaffected_collection1->begin()),
                    p_end(unaffected_collection1->end());

                if (is_range)
                    unaffected_collection2 = _imp->db->query(*atoms.at(1), is_either);

                for ( ; p != p_end ; ++p)
                {
                    if ((! is_range) || (unaffected_collection2->end() != unaffected_collection2->find(*p)))
                    {
                        unaffected.insert(*p);
                        std::set<std::pair<PackageDatabaseEntry, std::string> >::iterator
                                a(affected.find(std::make_pair(*p, advisory_id)));
                        if (a != affected.end())
                            affected.erase(a);
                    }
                }
            }
        }
        catch (const AdvisoryFileError & e)
        {
            Log::get_instance()->message(ll_warning, lc_context,
                    "Malformed advisory file '" + stringify(*f) + "': " + e.message());
        }
        catch (const InternalError & e)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message(ll_warning, lc_context,
                    "Exception caught while parsing advisory '" + stringify(*f) +
                    "': " + e.message());
        }

    }

    std::set<std::pair<PackageDatabaseEntry, std::string> >::const_iterator i(affected.begin()), i_end(affected.end());
    if (list_affected_only)
    {
        for ( ; i != i_end ; ++i)
        {
            Context c("When creating adding vulnerable package '" + stringify(i->first) + "':");

            PackageDepAtom::Pointer p(make_atom(i->first));
            p->set_tag(GLSADepTag::Pointer(new GLSADepTag(i->second, advisory_map[i->second])));
            security_packages->add_child(p);
        }
    }
    else
    {
        for ( ; i != i_end ; ++i)
        {
            Context c("When finding best update for package '" + stringify(i->first) + "', affected by '" + i->second + "':");

            PackageDatabaseEntryCollection::Iterator best = find_best(unaffected, i->first);
            if (best == unaffected.end())
                throw AllMaskedError("No best update available for package '" + stringify(i->first) + "':");

            PackageDepAtom::Pointer p(make_atom(*best));
            p->set_tag(GLSADepTag::Pointer(new GLSADepTag(i->second, advisory_map[i->second])));
            security_packages->add_child(p);
        }
    }

    return security_packages;
}

DepAtom::Pointer
PortageRepository::do_package_set(const std::string & s, const PackageSetOptions & o) const
{
    if ("system" == s)
    {
        _imp->need_profiles();
        return _imp->profile_ptr->system_packages();
    }
    else if ("security" == s)
        return do_security_set(o);
    else if ((_imp->setsdir / (s + ".conf")).exists())
    {
        GeneralSetDepTag::Pointer tag(new GeneralSetDepTag(s));

        FSEntry ff(_imp->setsdir / (s + ".conf"));
        Context context("When loading package set '" + s + "' from '" + stringify(ff) + "':");

        AllDepAtom::Pointer result(new AllDepAtom);
        LineConfigFile f(ff);
        for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
                line != line_end ; ++line)
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));
            if (tokens.empty())
                continue;

            if (1 == tokens.size())
            {
                Log::get_instance()->message(ll_warning, lc_context,
                        "Line '" + *line + "' in set file '"
                        + stringify(ff) + "' does not specify '*' or '?', assuming '*'");
                PackageDepAtom::Pointer atom(new PackageDepAtom(tokens.at(0)));
                atom->set_tag(tag);
                result->add_child(atom);
            }
            else if ("*" == tokens.at(0))
            {
                PackageDepAtom::Pointer atom(new PackageDepAtom(tokens.at(1)));
                atom->set_tag(tag);
                result->add_child(atom);
            }
            else if ("?" == tokens.at(0))
            {
                PackageDepAtom::Pointer p(new PackageDepAtom(tokens.at(1)));
                p->set_tag(tag);
                if (! _imp->env->package_database()->query(
                            PackageDepAtom::Pointer(new PackageDepAtom(p->package())),
                            is_installed_only)->empty())
                    result->add_child(p);
            }
            else
                Log::get_instance()->message(ll_warning, lc_context,
                        "Line '" + *line + "' in set file '"
                        + stringify(ff) + "' does not start with '*' or '?' token, skipping");

            if (tokens.size() > 2)
                Log::get_instance()->message(ll_warning, lc_context,
                        "Line '" + *line + "' in set file '"
                        + stringify(ff) + "' has trailing garbage");
        }

        return result;
    }
    else
        return DepAtom::Pointer(0);
}

bool
PortageRepository::do_sync() const
{
    Context context("When syncing repository '" + stringify(name()) + "':");

    if (_imp->sync.empty())
        return false;

    std::string::size_type p(_imp->sync.find("://")), q(_imp->sync.find(":"));
    if (std::string::npos == p)
        throw NoSuchSyncerError(_imp->sync);

    SyncOptions opts(_imp->sync_exclude);

    SyncerMaker::get_instance()->find_maker(_imp->sync.substr(0, std::min(p, q)))(
            stringify(_imp->location), _imp->sync.substr(q < p ? q + 1 : 0))->sync(opts);

    return true;
}

void
PortageRepository::invalidate() const
{
    _imp->invalidate();
}

Repository::ProvideMapIterator
PortageRepository::begin_provide_map() const
{
    return _imp->provide_map.begin();
}

Repository::ProvideMapIterator
PortageRepository::end_provide_map() const
{
    return _imp->provide_map.end();
}

void
PortageRepository::update_news() const
{
    Context context("When updating news for repository '" + stringify(name()) + "':");

    if (! _imp->newsdir.is_directory())
        return;

    std::set<std::string> skip;
    FSEntry
        skip_file(_imp->root / "var" / "lib" / "paludis" / "news" /
                ("news-" + stringify(name()) + ".skip")),
        unread_file(_imp->root / "var" / "lib" / "paludis" / "news" /
                ("news-" + stringify(name()) + ".unread"));

    if (skip_file.is_regular_file())
    {
        Context local_context("When handling news skip file '" + stringify(skip_file) + "':");
        LineConfigFile s(skip_file);
        std::copy(s.begin(), s.end(), std::inserter(skip, skip.end()));
    }

    for (DirIterator d(_imp->newsdir), d_end ; d != d_end ; ++d)
    {
        Context local_context("When handling news entry '" + stringify(*d) + "':");

        if (! d->is_directory())
            continue;
        if (! (*d / (d->basename() + ".en.txt")).is_regular_file())
            continue;

        if (skip.end() != skip.find(d->basename()))
            continue;

        try
        {
            NewsFile news(*d / (d->basename() + ".en.txt"));
            bool show(true);

            if (news.begin_display_if_installed() != news.end_display_if_installed())
            {
                bool local_show(false);
                for (NewsFile::DisplayIfInstalledIterator i(news.begin_display_if_installed()),
                        i_end(news.end_display_if_installed()) ; i != i_end ; ++i)
                    if (! _imp->env->package_database()->query(PackageDepAtom::Pointer(
                                    new PackageDepAtom(*i)), is_installed_only)->empty())
                        local_show = true;
                show &= local_show;
            }

            if (news.begin_display_if_keyword() != news.end_display_if_keyword())
            {
                _imp->need_profiles();

                bool local_show(false);
                for (NewsFile::DisplayIfKeywordIterator i(news.begin_display_if_keyword()),
                        i_end(news.end_display_if_keyword()) ; i != i_end ; ++i)
                    if (_imp->profile_ptr->environment_variable("ARCH") == *i)
                        local_show = true;
                show &= local_show;
            }

            if (news.begin_display_if_profile() != news.end_display_if_profile())
            {
                bool local_show(false);
                for (FSEntryCollection::Iterator p(_imp->profile_locations->begin()),
                        p_end(_imp->profile_locations->end()) ; p != p_end ; ++p)
                {
                    std::string profile(strip_leading_string(strip_trailing_string(
                                strip_leading_string(stringify(p->realpath()),
                                    stringify(p->realpath())), "/"), "/"));
                    Log::get_instance()->message(ll_debug, lc_no_context,
                            "Profile path is '" + profile + "'");
                    for (NewsFile::DisplayIfProfileIterator i(news.begin_display_if_profile()),
                            i_end(news.end_display_if_profile()) ; i != i_end ; ++i)
                        if (profile == *i)
                            local_show = true;
                }
                show &= local_show;
            }

            if (show)
            {
                std::ofstream s(stringify(skip_file).c_str(), std::ios::out | std::ios::app);
                if (! s)
                    Log::get_instance()->message(ll_warning, lc_no_context,
                            "Cannot append to news skip file '"
                            + stringify(skip_file) + "', skipping news item '" + stringify(*d) + "'");

                std::ofstream t(stringify(unread_file).c_str(), std::ios::out | std::ios::app);
                if (! t)
                    Log::get_instance()->message(ll_warning, lc_no_context,
                            "Cannot append to unread file '"
                            + stringify(unread_file) + "', skipping news item '" + stringify(*d) + "'");

                if (s && t)
                {
                    s << d->basename() << std::endl;
                    t << d->basename() << std::endl;
                }
            }
        }
        catch (const ConfigFileError & e)
        {
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "Skipping news item '"
                    + stringify(*d) + "' because of exception '" + e.message() + "' ("
                    + e.what() + ")");
        }
    }
}

std::string
PortageRepository::get_environment_variable(
        const PackageDatabaseEntry & for_package,
        const std::string & var) const
{
    Context context("When fetching environment variable '" + var + "' from repository '"
            + stringify(name()) + "':");

    _imp->need_profiles();

    QualifiedPackageName q(for_package.get<pde_name>());
    EbuildVariableCommand cmd(EbuildCommandParams::create((
                    param<ecpk_environment>(_imp->env),
                    param<ecpk_db_entry>(&for_package),
                    param<ecpk_ebuild_dir>(_imp->location / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>())),
                    param<ecpk_files_dir>(_imp->location / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>()) / "files"),
                    param<ecpk_eclassdirs>(_imp->eclassdirs),
                    param<ecpk_portdir>(_imp->location),
                    param<ecpk_distdir>(_imp->distdir),
                    param<ecpk_buildroot>(_imp->buildroot)
                    )),
            var);

    if (! cmd())
        throw EnvironmentVariableActionError("Couldn't get environment variable '" +
                stringify(var) + "' for package '" + stringify(for_package) + "'");

    return cmd.result();
}

RepositoryInfo::ConstPointer
PortageRepository::info(bool verbose) const
{
    RepositoryInfo::ConstPointer result_non_verbose(Repository::info(verbose));
    if (! verbose)
        return result_non_verbose;

    RepositoryInfo::Pointer result(new RepositoryInfo);

    for (RepositoryInfo::SectionIterator s(result_non_verbose->begin_sections()),
            s_end(result_non_verbose->end_sections()) ; s != s_end ; ++s)
        result->add_section(*s);

    std::set<std::string> info_pkgs;
    if ((_imp->location / "profiles" / "info_pkgs").exists())
    {
        LineConfigFile vars(_imp->location / "profiles" / "info_pkgs");
        info_pkgs.insert(vars.begin(), vars.end());
    }

    if (! info_pkgs.empty())
    {
        RepositoryInfoSection::Pointer package_info(new RepositoryInfoSection("Package information"));
        for (std::set<std::string>::const_iterator i(info_pkgs.begin()),
                i_end(info_pkgs.end()) ; i != i_end ; ++i)
        {
            PackageDatabaseEntryCollection::ConstPointer q(_imp->env->package_database()->query(
                        PackageDepAtom::ConstPointer(new PackageDepAtom(*i)), is_installed_only));
            if (q->empty())
                package_info->add_kv(*i, "(none)");
            else
            {
                std::set<VersionSpec> versions;

                /* don't use std::transform, it breaks g++4.1 */
                // std::transform(q->begin(), q->end(), std::inserter(versions, versions.end()),
                //        std::mem_fun_ref(&PackageDatabaseEntry::get<pde_version>));
                for (PackageDatabaseEntryCollection::Iterator qq(q->begin()), qq_end(q->end()) ;
                        qq != qq_end ; ++qq)
                    versions.insert(qq->get<pde_version>());
                package_info->add_kv(*i, join(versions.begin(), versions.end(), ", "));
            }
        }

        result->add_section(package_info);
    }

    std::set<std::string> info_vars;
    if ((_imp->location / "profiles" / "info_vars").exists())
    {
        LineConfigFile vars(_imp->location / "profiles" / "info_vars");
        info_vars.insert(vars.begin(), vars.end());
    }

    if (! info_vars.empty() && ! info_pkgs.empty() &&
            ! version_specs(QualifiedPackageName(*info_pkgs.begin()))->empty())
    {
        PackageDatabaseEntry e(QualifiedPackageName(*info_pkgs.begin()),
                *version_specs(QualifiedPackageName(*info_pkgs.begin()))->last(),
                name());
        RepositoryInfoSection::Pointer variable_info(new RepositoryInfoSection("Variable information"));
        for (std::set<std::string>::const_iterator i(info_vars.begin()),
                i_end(info_vars.end()) ; i != i_end ; ++i)
            variable_info->add_kv(*i, get_environment_variable(e, *i));

        result->add_section(variable_info);
    }
    else if (! info_vars.empty())
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Skipping info_vars for '" + stringify(name()) +
                "' because info_pkgs is not usable");

    return result;
}

