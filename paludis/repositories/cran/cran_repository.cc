/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
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

#include <paludis/dep_atom.hh>
#include <paludis/dep_atom_flattener.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/config_file.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/package_database.hh>
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/repositories/cran/cran_description.hh>
#include <paludis/repositories/cran/cran_repository.hh>
#include <paludis/repository_maker.hh>
#include <paludis/syncer.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
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
#include <list>
#include <fstream>
#include <functional>
#include <algorithm>
#include <vector>
#include <deque>
#include <limits>

#include <strings.h>
#include <ctype.h>

/** \file
 * Implementation CRANRepository.
 *
 * \ingroup grpcranrepository
 */

using namespace paludis;

#include <paludis/repositories/cran/cran_repository-sr.cc>

namespace paludis
{
    /// Map for versions.
    typedef MakeHashedMap<QualifiedPackageName, VersionSpec>::Type VersionsMap;

    /// Map for packages.
    typedef MakeHashedMap<QualifiedPackageName, bool>::Type PackagesMap;

    /// Map for mirrors.
    typedef MakeHashedMap<std::string, std::list<std::string> >::Type MirrorMap;

    /// Map for metadata.
    typedef MakeHashedMap<std::pair<QualifiedPackageName, VersionSpec>, VersionMetadata::Pointer>::Type MetadataMap;

    /**
     * Implementation data for a CRANRepository.
     *
     * \ingroup grpportagerepository
     */
    template <>
    struct Implementation<CRANRepository> :
        InternalCounted<Implementation<CRANRepository> >
    {
        /// Our owning db.
        const PackageDatabase * const db;

        /// Our owning env.
        const Environment * const env;

        /// Our base location.
        FSEntry location;

        /// Distfiles dir
        FSEntry distdir;

        /// Mirror URL
        std::string mirror;

        /// Sync URL
        std::string sync;

        /// Build root location
        FSEntry buildroot;

        /// Root location
        FSEntry root;

        /// Library location
        FSEntry library;

        /// Have we loaded our category names?
        mutable bool has_packages;

        /// Our package names, and whether we have a fully loaded list of
        /// version specs for that category.
        mutable PackagesMap package_names;

        /// Our version specs for each package.
        mutable VersionsMap version_specs;

        /// Metadata cache.
        mutable MetadataMap metadata;

        /// Do we have mirrors?
        mutable bool has_mirrors;

        /// Mirrors.
        mutable MirrorMap mirrors;

        /// Constructor.
        Implementation(const CRANRepositoryParams &);

        /// Destructor.
        ~Implementation();

        /// Invalidate our cache.
        void invalidate() const;

        /// (Empty) provides map.
        const std::map<QualifiedPackageName, QualifiedPackageName> provide_map;
    };
}

Implementation<CRANRepository>::Implementation(const CRANRepositoryParams & p) :
    db(p.package_database),
    env(p.environment),
    location(p.location),
    distdir(p.distdir),
    mirror(p.mirror),
    sync(p.sync),
    buildroot(p.buildroot),
    root(p.root),
    library(p.library),
    has_packages(false),
    has_mirrors(false)
{
}

Implementation<CRANRepository>::~Implementation()
{
}

void
Implementation<CRANRepository>::invalidate() const
{
    package_names.clear();
    has_packages = false;
    version_specs.clear();
    metadata.clear();
    has_mirrors = false;
    mirrors.clear();
}


CRANRepository::CRANRepository(const CRANRepositoryParams & p) :
    Repository(CRANRepository::fetch_repo_name(stringify(p.location)),
            RepositoryCapabilities::create()
            .mask_interface(0)
            .installable_interface(this)
            .installed_interface(0)
            .news_interface(0)
            .sets_interface(this)
            .syncable_interface(this)
            .uninstallable_interface(0)
            .use_interface(0)
            .world_interface(0)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            .provides_interface(0)
            .destination_interface(0)
            .virtuals_interface(0),
            "cran"),
    PrivateImplementationPattern<CRANRepository>(new Implementation<CRANRepository>(p))
{
    RepositoryInfoSection::Pointer config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("location", stringify(_imp->location));
    config_info->add_kv("distdir", stringify(_imp->distdir));
    config_info->add_kv("format", "cran");
    config_info->add_kv("buildroot", stringify(_imp->buildroot));
    config_info->add_kv("root", stringify(_imp->root));
    config_info->add_kv("library", stringify(_imp->library));
    config_info->add_kv("sync", _imp->sync);

    _info->add_section(config_info);
}

CRANRepository::~CRANRepository()
{
}

bool
CRANRepository::do_has_category_named(const CategoryNamePart & c) const
{
    Context context("When checking for category '" + stringify(c) +
            "' in " + stringify(name()) + ":");

    return "cran" == stringify(c);
}

bool
CRANRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    Context context("When checking for package '" + stringify(q) + "' in " +
                stringify(name()) + ":");

    need_packages();

    if (! do_has_category_named(q.category))
        return false;

    return _imp->package_names.end() != _imp->package_names.find(q);
}

CategoryNamePartCollection::ConstPointer
CRANRepository::do_category_names() const
{
    Context context("When fetching category names in " + stringify(name()) + ":");

    CategoryNamePartCollection::Pointer result(new CategoryNamePartCollection::Concrete);
    result->insert(CategoryNamePart("cran"));

    return result;
}

QualifiedPackageNameCollection::ConstPointer
CRANRepository::do_package_names(const CategoryNamePart & c) const
{
    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    need_packages();

    QualifiedPackageNameCollection::Pointer result(new QualifiedPackageNameCollection::Concrete);
    if (! do_has_category_named(c))
        return result;

    PackagesMap::const_iterator n(_imp->package_names.begin()), n_end(_imp->package_names.end());
    for ( ; n != n_end ; ++n)
        result->insert(n->first);

    return result;
}

VersionSpecCollection::ConstPointer
CRANRepository::do_version_specs(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    need_packages();

    VersionSpecCollection::Pointer result(new VersionSpecCollection::Concrete);
    if (_imp->version_specs.end() != _imp->version_specs.find(n))
        result->insert(_imp->version_specs.find(n)->second);

    return result;
}

bool
CRANRepository::do_has_version(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    Context context("When checking for version '" + stringify(v) + "' in '"
            + stringify(q) + "' in " + stringify(name()) + ":");

    need_packages();

    if (has_package_named(q))
        return v == _imp->version_specs.find(q)->second;
    else
        return false;
}

void
CRANRepository::need_packages() const
{
    if (_imp->has_packages)
        return;

    Context context("When loading category names for " + stringify(name()) + ":");

    LineConfigFile packages(FSEntry(_imp->location / "PACKAGES"));
    LineConfigFile::Iterator l(packages.begin()), l_end(packages.end());
    std::string last_package_name;
    bool skip_invalid_package = false;
    for ( ; l != l_end ; ++l)
    {
        Context local_context("When parsing line '" + *l + "':");

        std::string line(strip_leading(strip_trailing(*l, " \t"), " \t"));
        if (line.empty())
            continue;

        std::string::size_type pos(line.find(':'));
        if (std::string::npos == pos)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Broken line in PACKAGES file: '" + stringify(_imp->location / "PACKAGES") + "'");
            continue;
        }

        std::string key(line.substr(0, pos)), value(line.substr(pos + 1));
        key = strip_leading(strip_trailing(key, " \t"), " \t");
        value = strip_leading(strip_trailing(value, " \t"), " \t");
        if (("Package" != key) && (skip_invalid_package))
        {
            skip_invalid_package = false;
            continue;
        }

        if ("Package" == key)
        {
            CRANDescription::normalise_name(value);
            last_package_name = value;
            QualifiedPackageName package_name(CategoryNamePart("cran"), PackageNamePart(last_package_name));
            _imp->package_names[package_name] = true;
        }
        else if ("Version" == key)
        {
            QualifiedPackageName package_name(CategoryNamePart("cran"), PackageNamePart(last_package_name));
            CRANDescription::normalise_version(value);
            VersionSpec version(value);
            if (false == _imp->version_specs.insert(VersionsMap::value_type(package_name, version)).second)
            {
                skip_invalid_package = true;
                Log::get_instance()->message(ll_warning, lc_context, "Multiple versions for package '"
                    + last_package_name + "'.");
                continue;
            }
        }
        else if ("Contains" == key)
        {
            std::list<std::string> contains;
            WhitespaceTokeniser::get_instance()->tokenise(value, std::back_inserter(contains));
            std::list<std::string>::const_iterator i(contains.begin()), i_end(contains.end());
            // load metadata immediately
            for ( ; i != i_end ; ++i)
            {
                Context c("When processing 'Contains:' line: '" + stringify(*i) + "':");

                if (*i == last_package_name)
                    continue;

                CRANDescription d(*i, FSEntry(_imp->location / std::string(last_package_name + ".DESCRIPTION")));

                std::string dep(d.metadata->deps.build_depend_string);
                std::string pkg(d.metadata->get_cran_interface()->package);
                if ("" == dep)
                    dep = pkg;
                else
                    dep += "," + pkg;
                d.metadata->deps.build_depend_string = dep;

                _imp->package_names[d.name] = true;
                _imp->metadata.insert(std::make_pair(std::make_pair(d.name, d.version), d.metadata));
                _imp->version_specs.insert(VersionsMap::value_type(d.name, d.version));
            }
        }
    }

    _imp->has_packages = true;
}

RepositoryName
CRANRepository::fetch_repo_name(const std::string & location)
{
    std::string modified_location(FSEntry(location).basename());
    std::replace(modified_location.begin(), modified_location.end(), '/', '-');
    return RepositoryName("cran-" + modified_location);
}

VersionMetadata::ConstPointer
CRANRepository::do_version_metadata(
        const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (_imp->metadata.end() != _imp->metadata.find(
        std::make_pair(q, v)))
            return _imp->metadata.find(std::make_pair(q, v))->second;

    Context context("When fetching metadata for " + stringify(q) +
            "-" + stringify(v));

    if (! has_version(q, v))
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));

    VersionMetadata::Pointer result(new VersionMetadata(CRANDepParser::parse));

    FSEntry d(_imp->location);
    PackageNamePart p(q.package);
    std::string n(stringify(p));
    CRANDescription::denormalise_name(n);
    d /= n + ".DESCRIPTION";

    if (d.is_regular_file())
    {
        CRANDescription desc(stringify(p), d);
        result = desc.metadata;
    }
    else
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "has_version failed for request for '" +
                stringify(q) + "-" + stringify(v) + "' in repository '" +
                stringify(name()) + "': File '" + n + ".DESCRIPTION' not present.");
        result.assign(new VersionMetadata(CRANDepParser::parse));
        result->eapi = "UNKNOWN";
    }

    _imp->metadata.insert(std::make_pair(std::make_pair(q, v), result));
    return result;
}

CountedPtr<Repository>
CRANRepository::make_cran_repository(
        const Environment * const env,
        const PackageDatabase * const db,
        AssociativeCollection<std::string, std::string>::ConstPointer m)
{
    Context context("When making CRAN repository from repo_file '" +
            (m->end() == m->find("repo_file") ? std::string("?") : m->find("repo_file")->second) + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw CRANRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string library;
    if (m->end() == m->find("library") || ((library = m->find("library")->second)).empty())
        throw CRANRepositoryConfigurationError("Key 'library' not specified or empty");

    std::string distdir;
    if (m->end() == m->find("distdir") || ((distdir = m->find("distdir")->second)).empty())
        distdir = location + "/distfiles";

    std::string mirror;
    if (m->end() == m->find("mirror") || ((mirror = m->find("mirror")->second)).empty())
        mirror = "http://cran.r-project.org/";

    std::string sync;
    if (m->end() == m->find("sync") || ((sync = m->find("sync")->second)).empty())
        sync = "rsync://cran.r-project.org/CRAN";

    std::string buildroot;
    if (m->end() == m->find("buildroot") || ((buildroot = m->find("buildroot")->second)).empty())
        buildroot = "/var/tmp/paludis";

    std::string root;
    if (m->end() == m->find("root") || ((root = m->find("root")->second)).empty())
        root = "/";

    return CountedPtr<Repository>(new CRANRepository(CRANRepositoryParams::create()
                .environment(env)
                .package_database(db)
                .location(location)
                .distdir(distdir)
                .sync(sync)
                .buildroot(buildroot)
                .root(root)
                .library(library)
                .mirror(mirror)));
}

CRANRepositoryConfigurationError::CRANRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("CRAN repository configuration error: " + msg)
{
}

bool
CRANRepository::do_is_licence(const std::string &) const
{
    return false;
}

void
CRANRepository::do_install(const QualifiedPackageName &q, const VersionSpec &vn,
        const InstallOptions &o) const
{
    PackageNamePart pn(q.package);
    CategoryNamePart c("cran");
    VersionMetadata::ConstPointer vm(do_version_metadata(q, vn));
    std::string p(vm->get_cran_interface()->package);
    std::string v(vm->get_cran_interface()->version);

    MakeEnvCommand cmd(LIBEXECDIR "/paludis/cran.bash fetch", "");
    cmd = cmd("CATEGORY", "cran");
    cmd = cmd("DISTDIR", stringify(_imp->distdir));
    cmd = cmd("DISTFILE", std::string(p + "_" + v + ".tar.gz"));
    cmd = cmd("PN", stringify(pn));
    cmd = cmd("PV", stringify(vn));
    cmd = cmd("PALUDIS_CRAN_MIRRORS", _imp->mirror);
    cmd = cmd("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"));
    cmd = cmd("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()));
    cmd = cmd("PALUDIS_BASHRC_FILES", _imp->env->bashrc_files());


    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't fetch sources for '" + stringify(q) + "-" + stringify(vn) + "'");

    if (o.fetch_only)
        return;

    std::string image(stringify(_imp->buildroot / stringify(q) / "image"));
    std::string workdir(stringify(_imp->buildroot / stringify(q) / "work"));

    cmd = MakeEnvCommand(make_sandbox_command(LIBEXECDIR "/paludis/cran.bash clean install"), "");
    cmd = cmd("CATEGORY", "cran");
    cmd = cmd("DISTDIR", stringify(_imp->distdir));
    cmd = cmd("DISTFILE", std::string(p + "_" + v + ".tar.gz"));
    cmd = cmd("IMAGE", image);
    cmd = cmd("PN", stringify(pn));
    cmd = cmd("PV", stringify(vn));
    cmd = cmd("PALUDIS_CRAN_LIBRARY", stringify(_imp->library));
    cmd = cmd("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"));
    cmd = cmd("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()));
    cmd = cmd("PALUDIS_BASHRC_FILES", _imp->env->bashrc_files());
    cmd = cmd("ROOT", stringify(_imp->root));
    cmd = cmd("WORKDIR", workdir);


    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't install '" + stringify(q) + "-" + stringify(vn) + "' to '" +
                image + "'");

    cmd = MakeEnvCommand(LIBEXECDIR "/paludis/cran.bash merge clean", "");
    cmd = cmd("IMAGE", image);
    cmd = cmd("PN", p);
    cmd = cmd("PV", stringify(vn));
    cmd = cmd("PALUDIS_CRAN_LIBRARY", stringify(_imp->library));
    cmd = cmd("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"));
    cmd = cmd("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()));
    cmd = cmd("PALUDIS_BASHRC_FILES", _imp->env->bashrc_files());
    cmd = cmd("ROOT", stringify(_imp->root));
    cmd = cmd("WORKDIR", workdir);
    cmd = cmd("REPOSITORY", stringify(name()));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't merge '" + stringify(q) + "-" + stringify(vn) + "' to '" +
                stringify(_imp->root) + "'");

    return;
}

DepAtom::Pointer
CRANRepository::do_package_set(const SetName & s) const
{
    if ("base" == s.data())
    {
        /**
         * \todo Implement system as all package which are installed
         * by dev-lang/R by default.
         */
        return AllDepAtom::Pointer(new AllDepAtom);
    }
    else
        return DepAtom::Pointer(0);
}

SetsCollection::ConstPointer
CRANRepository::sets_list() const
{
    Context context("While generating the list of sets:");

    SetsCollection::Pointer result(new SetsCollection::Concrete);
    result->insert(SetName("base"));
    return result;
}

bool
CRANRepository::do_sync() const
{
    Context context("When syncing repository '" + stringify(name()) + "':");

    std::string cmd("rsync --delete --recursive --progress --exclude \"*.html\" --exclude \"*.INDEX\" '" +
                    _imp->sync + "/src/contrib/Descriptions/' ./");

    if (0 != run_command_in_directory(cmd, _imp->location))
        return false;

    cmd = "rsync --progress '" + _imp->sync + "/src/contrib/PACKAGES' ./";

    if (0 != run_command_in_directory(cmd, _imp->location))
        return false;

    cmd = "rsync --progress '" + _imp->sync + "/CRAN_mirrors.csv' ./";

    return 0 == run_command_in_directory(cmd, _imp->location);
}

void
CRANRepository::invalidate() const
{
    _imp->invalidate();
}
