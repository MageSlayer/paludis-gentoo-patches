/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006,2007 Danny van Dyk <kugelfang@gentoo.org>
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

#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/config_file.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/package_database.hh>
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/repositories/cran/cran_description.hh>
#include <paludis/repositories/cran/cran_repository.hh>
#include <paludis/repositories/cran/cran_version_metadata.hh>
#include <paludis/repositories/repository_maker.hh>
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
    typedef MakeHashedMap<std::pair<QualifiedPackageName, VersionSpec>, std::tr1::shared_ptr<VersionMetadata> >::Type MetadataMap;

    /**
     * Implementation data for a CRANRepository.
     *
     * \ingroup grpportagerepository
     */
    template <>
    struct Implementation<CRANRepository>
    {
        CRANRepositoryParams params;

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

        /// (Empty) provides map.
        const std::map<QualifiedPackageName, QualifiedPackageName> provide_map;
    };
}

Implementation<CRANRepository>::Implementation(const CRANRepositoryParams & p) :
    params(p),
    env(p.environment),
    location(p.location),
    distdir(p.distdir),
    mirror(p.mirror),
    sync(p.sync),
    buildroot(p.buildroot),
    library(p.library),
    has_packages(false),
    has_mirrors(false)
{
}

Implementation<CRANRepository>::~Implementation()
{
}


CRANRepository::CRANRepository(const CRANRepositoryParams & p) :
    Repository(CRANRepository::fetch_repo_name(stringify(p.location)),
            RepositoryCapabilities::create()
            .mask_interface(0)
            .installable_interface(this)
            .installed_interface(0)
            .sets_interface(this)
            .syncable_interface(this)
            .uninstallable_interface(0)
            .use_interface(0)
            .world_interface(0)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            .provides_interface(0)
            .destination_interface(0)
            .virtuals_interface(0)
            .config_interface(0)
            .contents_interface(0)
            .licenses_interface(0)
            .portage_interface(0)
            .hook_interface(0),
            "cran"),
    PrivateImplementationPattern<CRANRepository>(new Implementation<CRANRepository>(p))
{
    std::tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("location", stringify(_imp->location));
    config_info->add_kv("distdir", stringify(_imp->distdir));
    config_info->add_kv("format", "cran");
    config_info->add_kv("buildroot", stringify(_imp->buildroot));
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

std::tr1::shared_ptr<const CategoryNamePartCollection>
CRANRepository::do_category_names() const
{
    Context context("When fetching category names in " + stringify(name()) + ":");

    std::tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
    result->insert(CategoryNamePart("cran"));

    return result;
}

std::tr1::shared_ptr<const QualifiedPackageNameCollection>
CRANRepository::do_package_names(const CategoryNamePart & c) const
{
    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    need_packages();

    std::tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);
    if (! do_has_category_named(c))
        return result;

    PackagesMap::const_iterator n(_imp->package_names.begin()), n_end(_imp->package_names.end());
    for ( ; n != n_end ; ++n)
        result->insert(n->first);

    return result;
}

std::tr1::shared_ptr<const VersionSpecCollection>
CRANRepository::do_version_specs(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    need_packages();

    std::tr1::shared_ptr<VersionSpecCollection> result(new VersionSpecCollection::Concrete);
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

    LineConfigFile packages(FSEntry(_imp->location / "PACKAGES"), LineConfigFileOptions());
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

                CRANDescription d(*i, FSEntry(_imp->location / std::string(last_package_name + ".DESCRIPTION")), false);

                std::string dep(d.metadata->deps_interface->get_raw_build_depend());
                std::string pkg(d.metadata->cran_interface->package);
                if ("" == dep)
                    dep = pkg;
                else
                    dep += "," + pkg;
                d.metadata->deps_interface->set_build_depend(dep);
                d.metadata->cran_interface->is_bundle_member = true;

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

std::tr1::shared_ptr<const VersionMetadata>
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

    std::tr1::shared_ptr<VersionMetadata> result(new CRANVersionMetadata(false));

    FSEntry d(_imp->location);
    PackageNamePart p(q.package);
    std::string n(stringify(p));
    CRANDescription::denormalise_name(n);
    d /= n + ".DESCRIPTION";

    if (d.is_regular_file())
    {
        CRANDescription desc(stringify(p), d, false);
        result = desc.metadata;
    }
    else
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "has_version failed for request for '" +
                stringify(q) + "-" + stringify(v) + "' in repository '" +
                stringify(name()) + "': File '" + n + ".DESCRIPTION' not present.");
        result.reset(new CRANVersionMetadata(false));
        result->eapi = EAPIData::get_instance()->unknown_eapi();
    }

    _imp->metadata.insert(std::make_pair(std::make_pair(q, v), result));
    return result;
}

std::tr1::shared_ptr<Repository>
CRANRepository::make_cran_repository(
        Environment * const env,
        std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m)
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

    return std::tr1::shared_ptr<Repository>(new CRANRepository(CRANRepositoryParams::create()
                .environment(env)
                .location(location)
                .distdir(distdir)
                .sync(sync)
                .buildroot(buildroot)
                .library(library)
                .mirror(mirror)));
}

CRANRepositoryConfigurationError::CRANRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("CRAN repository configuration error: " + msg)
{
}

namespace
{
    FSEntry
    get_root(std::tr1::shared_ptr<const DestinationsCollection> destinations)
    {
        if (destinations)
            for (DestinationsCollection::Iterator d(destinations->begin()), d_end(destinations->end()) ;
                    d != d_end ; ++d)
                if ((*d)->installed_interface)
                    return (*d)->installed_interface->root();

        return FSEntry("/");
    }
}

void
CRANRepository::do_install(const QualifiedPackageName &q, const VersionSpec &vn,
        const InstallOptions &o) const
{
    PackageNamePart pn(q.package);
    CategoryNamePart c("cran");
    std::tr1::shared_ptr<const VersionMetadata> vm(do_version_metadata(q, vn));

    if (vm->cran_interface->is_bundle_member)
        return;

    std::string p(vm->cran_interface->package);
    std::string v(vm->cran_interface->version);

    std::tr1::shared_ptr<const FSEntryCollection> bashrc_files(_imp->env->bashrc_files());

    Command cmd(Command(LIBEXECDIR "/paludis/cran.bash fetch")
            .with_setenv("CATEGORY", "cran")
            .with_setenv("DISTDIR", stringify(_imp->distdir))
            .with_setenv("DISTFILE", std::string(p + "_" + v + ".tar.gz"))
            .with_setenv("PN", stringify(pn))
            .with_setenv("PV", stringify(vn))
            .with_setenv("PALUDIS_CRAN_MIRRORS", _imp->mirror)
            .with_setenv("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"))
            .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
            .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " ")));


    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't fetch sources for '" + stringify(q) + "-" + stringify(vn) + "'");

    if (o.fetch_only)
        return;

    FSEntry image(_imp->buildroot / stringify(q) / "image");
    FSEntry workdir(_imp->buildroot / stringify(q) / "work");

    cmd = Command(LIBEXECDIR "/paludis/cran.bash clean install")
        .with_sandbox()
        .with_setenv("CATEGORY", "cran")
        .with_setenv("DISTDIR", stringify(_imp->distdir))
        .with_setenv("DISTFILE", std::string(p + "_" + v + ".tar.gz"))
        .with_setenv("IMAGE", stringify(image))
        .with_setenv("IS_BUNDLE", (vm->cran_interface->is_bundle ? "yes" : ""))
        .with_setenv("LOCATION", stringify(_imp->location))
        .with_setenv("PN", stringify(pn))
        .with_setenv("PV", stringify(vn))
        .with_setenv("PALUDIS_CRAN_LIBRARY", stringify(_imp->library))
        .with_setenv("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"))
        .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
        .with_setenv("ROOT", stringify(get_root(o.destinations)))
        .with_setenv("WORKDIR", stringify(workdir));


    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't install '" + stringify(q) + "-" + stringify(vn) + "' to '" +
                stringify(image) + "'");

    if (! o.destinations)
        throw PackageInstallActionError("Can't merge '" + stringify(q) + "-" + stringify(vn) +
                "' because no destinations were provided.");

    MergeOptions m(PackageDatabaseEntry(q, vn, name()),
            image,
            FSEntry("/dev/null"));

    for (DestinationsCollection::Iterator d(o.destinations->begin()),
            d_end(o.destinations->end()) ; d != d_end ; ++d)
    {
        if (! (*d)->destination_interface)
            throw PackageInstallActionError("Couldn't install '" + stringify(q) + "-" + stringify(vn) + "' to '" +
                    stringify((*d)->name()) + "' because it does not provide destination_interface");

        (*d)->destination_interface->merge(m);
    }

    cmd = Command(LIBEXECDIR "/paludis/cran.bash clean")
        .with_setenv("IMAGE", stringify(image))
        .with_setenv("PN", p)
        .with_setenv("PV", stringify(vn))
        .with_setenv("PALUDIS_CRAN_LIBRARY", stringify(_imp->library))
        .with_setenv("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"))
        .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
        .with_setenv("ROOT", stringify(get_root(o.destinations)))
        .with_setenv("WORKDIR", stringify(workdir))
        .with_setenv("REPOSITORY", stringify(name()));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't clean '" + stringify(q) + "-" + stringify(vn) + "'");

    return;
}

std::tr1::shared_ptr<DepSpec>
CRANRepository::do_package_set(const SetName & s) const
{
    if ("base" == s.data())
    {
        /**
         * \todo Implement system as all package which are installed
         * by dev-lang/R by default.
         */
        return std::tr1::shared_ptr<AllDepSpec>(new AllDepSpec);
    }
    else
        return std::tr1::shared_ptr<DepSpec>();
}

std::tr1::shared_ptr<const SetNameCollection>
CRANRepository::sets_list() const
{
    Context context("While generating the list of sets:");

    std::tr1::shared_ptr<SetNameCollection> result(new SetNameCollection::Concrete);
    result->insert(SetName("base"));
    return result;
}

bool
CRANRepository::do_sync() const
{
    Context context("When syncing repository '" + stringify(name()) + "':");

    std::string cmd("rsync --delete --recursive --progress --exclude \"*.html\" --exclude \"*.INDEX\" '" +
                    _imp->sync + "/src/contrib/Descriptions/' ./");

    if (0 != run_command(Command(cmd).with_chdir(_imp->location)))
        return false;

    cmd = "rsync --progress '" + _imp->sync + "/src/contrib/PACKAGES' ./";

    if (0 != run_command(Command(cmd).with_chdir(_imp->location)))
        return false;

    cmd = "rsync --progress '" + _imp->sync + "/CRAN_mirrors.csv' ./";

    return 0 == run_command(Command(cmd).with_chdir(_imp->location));
}

void
CRANRepository::invalidate()
{
    _imp.reset(new Implementation<CRANRepository>(_imp->params));
}

