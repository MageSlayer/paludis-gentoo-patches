/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/vdb_merger.hh>
#include <paludis/repositories/e/vdb_unmerger.hh>
#include <paludis/repositories/e/vdb_id.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/extra_distribution_data.hh>
#include <paludis/repositories/e/can_skip_phase.hh>

#include <paludis/action.hh>
#include <paludis/util/config_file.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/dep_tag.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/match_package.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repository_name_cache.hh>
#include <paludis/set_file.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_requirements.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>

#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fast_unique_copy.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/output_manager.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/timestamp.hh>

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>

#include <tr1/unordered_map>
#include <tr1/functional>
#include <functional>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <iostream>
#include <cstring>
#include <cerrno>

using namespace paludis;
using namespace paludis::erepository;

typedef std::tr1::unordered_map<CategoryNamePart, std::tr1::shared_ptr<QualifiedPackageNameSet>, Hash<CategoryNamePart> > CategoryMap;
typedef std::tr1::unordered_map<QualifiedPackageName, std::tr1::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> > IDMap;
typedef std::map<std::pair<QualifiedPackageName, VersionSpec>, std::tr1::shared_ptr<std::list<QualifiedPackageName> > > ProvidesMap;

namespace paludis
{
    template <>
    struct Implementation<VDBRepository>
    {
        VDBRepositoryParams params;

        const std::tr1::shared_ptr<Mutex> big_nasty_mutex;

        mutable CategoryMap categories;
        mutable bool has_category_names;
        mutable IDMap ids;

        mutable std::tr1::shared_ptr<RepositoryProvidesInterface::ProvidesSequence> provides;
        mutable std::tr1::shared_ptr<ProvidesMap> provides_map;
        mutable bool tried_provides_cache, used_provides_cache;
        std::tr1::shared_ptr<RepositoryNameCache> names_cache;

        Implementation(const VDBRepository * const, const VDBRepositoryParams &, std::tr1::shared_ptr<Mutex> = make_shared_ptr(new Mutex));
        ~Implementation();

        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > root_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > provides_cache_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > names_cache_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > builddir_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > eapi_when_unknown_key;
    };

    Implementation<VDBRepository>::Implementation(const VDBRepository * const r,
            const VDBRepositoryParams & p, std::tr1::shared_ptr<Mutex> m) :
        params(p),
        big_nasty_mutex(m),
        has_category_names(false),
        tried_provides_cache(false),
        used_provides_cache(false),
        names_cache(new RepositoryNameCache(p.names_cache(), r)),
        location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                    mkt_significant, params.location())),
        root_key(new LiteralMetadataValueKey<FSEntry> ("root", "root",
                    mkt_normal, params.root())),
        format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                    mkt_significant, "vdb")),
        provides_cache_key(new LiteralMetadataValueKey<FSEntry> ("provides_cache", "provides_cache",
                    mkt_normal, params.provides_cache())),
        names_cache_key(new LiteralMetadataValueKey<FSEntry> ("names_cache", "names_cache",
                    mkt_normal, params.names_cache())),
        builddir_key(new LiteralMetadataValueKey<FSEntry> ("builddir", "builddir",
                    mkt_normal, params.builddir())),
        eapi_when_unknown_key(new LiteralMetadataValueKey<std::string> (
                    "eapi_when_unknown", "eapi_when_unknown", mkt_normal, params.eapi_when_unknown()))
    {
    }

    Implementation<VDBRepository>::~Implementation()
    {
    }
}

VDBRepository::VDBRepository(const VDBRepositoryParams & p) :
    EInstalledRepository(
            make_named_values<EInstalledRepositoryParams>(
                value_for<n::builddir>(p.builddir()),
                value_for<n::environment>(p.environment()),
                value_for<n::root>(p.root())
                ),
            p.name(),
            make_named_values<RepositoryCapabilities>(
                value_for<n::destination_interface>(this),
                value_for<n::environment_variable_interface>(this),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(static_cast<RepositoryManifestInterface *>(0)),
                value_for<n::provides_interface>(this),
                value_for<n::virtuals_interface>(static_cast<RepositoryVirtualsInterface *>(0))
            )),
    PrivateImplementationPattern<VDBRepository>(new Implementation<VDBRepository>(this, p)),
    _imp(PrivateImplementationPattern<VDBRepository>::_imp)
{
    _add_metadata_keys();
}

VDBRepository::~VDBRepository()
{
}

void
VDBRepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->root_key);
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->provides_cache_key);
    add_metadata_key(_imp->names_cache_key);
    add_metadata_key(_imp->builddir_key);
    add_metadata_key(_imp->eapi_when_unknown_key);
}

bool
VDBRepository::has_category_named(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    Context context("When checking for category '" + stringify(c) +
            "' in " + stringify(name()) + ":");

    need_category_names();
    return _imp->categories.end() != _imp->categories.find(c);
}

bool
VDBRepository::has_package_named(const QualifiedPackageName & q) const
{
    Lock l(*_imp->big_nasty_mutex);

    Context context("When checking for package '" + stringify(q) +
            "' in " + stringify(name()) + ":");

    need_category_names();

    CategoryMap::iterator cat_iter(_imp->categories.find(q.category()));
    if (_imp->categories.end() == cat_iter)
        return false;

    need_package_ids(q.category());

    return cat_iter->second->end() != cat_iter->second->find(q);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
VDBRepository::category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    Context context("When fetching category names in " + stringify(name()) + ":");

    need_category_names();

    std::tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);

    std::transform(_imp->categories.begin(), _imp->categories.end(), result->inserter(),
            std::tr1::mem_fn(&std::pair<const CategoryNamePart, std::tr1::shared_ptr<QualifiedPackageNameSet> >::first));

    return result;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
VDBRepository::package_names(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    std::tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);

    need_category_names();
    if (! has_category_named(c))
        return make_shared_ptr(new QualifiedPackageNameSet);

    need_package_ids(c);

    return _imp->categories.find(c)->second;
}

std::tr1::shared_ptr<const PackageIDSequence>
VDBRepository::package_ids(const QualifiedPackageName & n) const
{
    Lock l(*_imp->big_nasty_mutex);

    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");


    need_category_names();
    if (! has_category_named(n.category()))
        return make_shared_ptr(new PackageIDSequence);

    need_package_ids(n.category());
    if (! has_package_named(n))
        return make_shared_ptr(new PackageIDSequence);

    return _imp->ids.find(n)->second;
}

std::tr1::shared_ptr<Repository>
VDBRepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    Context context("When making VDB repository from repo_file '" + f("repo_file") + "':");

    std::string location(f("location"));
    if (location.empty())
        throw VDBRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string root(f("root"));
    if (root.empty())
        root = "/";

    std::string provides_cache(f("provides_cache"));
    if (provides_cache.empty())
    {
        provides_cache = EExtraDistributionData::get_instance()->data_from_distribution(*DistributionData::get_instance()->distribution_from_string(
                    env->distribution()))->default_provides_cache();
        if (provides_cache.empty())
        {
            Log::get_instance()->message("e.vdb.configuration.no_provides_cache", ll_warning, lc_no_context)
                << "The provides_cache key is not set in '" << f("repo_file")
                << "'. You should read the Paludis documentation and select an appropriate value.";
            provides_cache = "/var/empty";
        }
    }

    std::string names_cache(f("names_cache"));
    if (names_cache.empty())
    {
        names_cache = EExtraDistributionData::get_instance()->data_from_distribution(*DistributionData::get_instance()->distribution_from_string(
                    env->distribution()))->default_names_cache();
        if (names_cache.empty())
        {
            Log::get_instance()->message("e.vdb.configuration.no_names_cache", ll_warning, lc_no_context)
                << "The names_cache key is not set in '" << f("repo_file")
                <<"'. You should read the Paludis documentation and select an appropriate value.";
            names_cache = "/var/empty";
        }
    }

    std::string builddir(f("builddir"));
    if (builddir.empty())
        builddir = EExtraDistributionData::get_instance()->data_from_distribution(*DistributionData::get_instance()->distribution_from_string(
                env->distribution()))->default_buildroot();

    std::string name(f("name"));
    if (name.empty())
        name = "installed";

    std::string eapi_when_unknown(f("eapi_when_unknown"));
    if (eapi_when_unknown.empty())
        eapi_when_unknown = EExtraDistributionData::get_instance()->data_from_distribution(
                *DistributionData::get_instance()->distribution_from_string(
                    env->distribution()))->default_eapi_when_unknown();

    return std::tr1::shared_ptr<Repository>(new VDBRepository(make_named_values<VDBRepositoryParams>(
                value_for<n::builddir>(builddir),
                value_for<n::eapi_when_unknown>(eapi_when_unknown),
                value_for<n::environment>(env),
                value_for<n::location>(location),
                value_for<n::name>(RepositoryName(name)),
                value_for<n::names_cache>(names_cache),
                value_for<n::provides_cache>(provides_cache),
                value_for<n::root>(root)
                )));
}

RepositoryName
VDBRepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    std::string name(f("name"));
    if (name.empty())
        return RepositoryName("installed");
    else
        return RepositoryName(name);
}

std::tr1::shared_ptr<const RepositoryNameSet>
VDBRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

VDBRepositoryConfigurationError::VDBRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("VDB repository configuration error: " + msg)
{
}

VDBRepositoryKeyReadError::VDBRepositoryKeyReadError(
        const std::string & msg) throw () :
    ConfigurationError("VDB repository key read error: " + msg)
{
}

namespace
{
    bool ignore_merged(const std::tr1::shared_ptr<const FSEntrySet> & s,
            const FSEntry & f)
    {
        return s->end() != s->find(f);
    }
}

void
VDBRepository::perform_uninstall(
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const UninstallAction & a) const
{
    Context context("When uninstalling '" + stringify(*id) + (a.options.is_overwrite() ? "' for an overwrite:" : "':"));

    if (! _imp->params.root().is_directory())
        throw ActionFailedError("Couldn't uninstall '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root()) + "') is not a directory");

    std::tr1::shared_ptr<OutputManager> output_manager(a.options.make_output_manager()(a));

    std::string reinstalling_str(a.options.is_overwrite() ? "-reinstalling-" : "");

    std::tr1::shared_ptr<FSEntrySequence> eclassdirs(new FSEntrySequence);
    eclassdirs->push_back(FSEntry(_imp->params.location() / stringify(id->name().category()) /
                (reinstalling_str + stringify(id->name().package()) + "-" + stringify(id->version()))));

    FSEntry pkg_dir(_imp->params.location() / stringify(id->name().category()) / (reinstalling_str +
                stringify(id->name().package()) + "-" + stringify(id->version())));

    std::tr1::shared_ptr<FSEntry> load_env(new FSEntry(pkg_dir / "environment.bz2"));

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_uninstall());
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (can_skip_phase(id, *phase))
        {
            output_manager->stdout_stream() << "--- No need to do anything for " << phase->equal_option("skipname") << " phase" << std::endl;
            continue;
        }

        if (phase->option("unmerge"))
        {
            /* load CONFIG_PROTECT, CONFIG_PROTECT_MASK from vdb, supplement with env */
            std::string config_protect, config_protect_mask;

            try
            {
                SafeIFStream c(pkg_dir / "CONFIG_PROTECT");
                config_protect = std::string((std::istreambuf_iterator<char>(c)), std::istreambuf_iterator<char>()) +
                    " " + getenv_with_default("CONFIG_PROTECT", "");
            }
            catch (const SafeIFStreamError &)
            {
            }

            try
            {
                SafeIFStream c_m(pkg_dir / "CONFIG_PROTECT_MASK");
                config_protect_mask = std::string((std::istreambuf_iterator<char>(c_m)), std::istreambuf_iterator<char>()) +
                    " " + getenv_with_default("CONFIG_PROTECT_MASK", "");
            }
            catch (const SafeIFStreamError &)
            {
            }

            std::string final_config_protect(config_protect + " " + a.options.config_protect());

            /* unmerge */
            VDBUnmerger unmerger(
                    make_named_values<VDBUnmergerOptions>(
                        value_for<n::config_protect>(final_config_protect),
                        value_for<n::config_protect_mask>(config_protect_mask),
                        value_for<n::environment>(_imp->params.environment()),
                        value_for<n::ignore>(a.options.ignore_for_unmerge()),
                        value_for<n::output_manager>(output_manager),
                        value_for<n::package_id>(id),
                        value_for<n::root>(installed_root_key()->value())
                    ));

            unmerger.unmerge();
        }
        else
        {
            FSEntry package_builddir(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-uninstall"));
            EbuildCommandParams params(make_named_values<EbuildCommandParams>(
                    value_for<n::builddir>(_imp->params.builddir()),
                    value_for<n::clearenv>(phase->option("clearenv")),
                    value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                    value_for<n::distdir>(pkg_dir),
                    value_for<n::ebuild_dir>(pkg_dir),
                    value_for<n::ebuild_file>(pkg_dir / (stringify(id->name().package()) + "-" + stringify(id->version()) + ".ebuild")),
                    value_for<n::eclassdirs>(eclassdirs),
                    value_for<n::environment>(_imp->params.environment()),
                    value_for<n::exlibsdirs>(make_shared_ptr(new FSEntrySequence)),
                    value_for<n::files_dir>(pkg_dir),
                    value_for<n::maybe_output_manager>(output_manager),
                    value_for<n::package_builddir>(package_builddir),
                    value_for<n::package_id>(id),
                    value_for<n::portdir>(_imp->params.location()),
                    value_for<n::root>(stringify(_imp->params.root())),
                    value_for<n::sandbox>(phase->option("sandbox")),
                    value_for<n::sydbox>(phase->option("sydbox")),
                    value_for<n::userpriv>(phase->option("userpriv"))
                    ));

            EbuildUninstallCommandParams uninstall_params(make_named_values<EbuildUninstallCommandParams>(
                        value_for<n::load_environment>(load_env.get()),
                        value_for<n::loadsaveenv_dir>(package_builddir / "temp"),
                        value_for<n::replaced_by>(a.options.if_for_install_id()),
                        value_for<n::unmerge_only>(false)
                    ));

            EbuildUninstallCommand uninstall_cmd_pre(params, uninstall_params);
            uninstall_cmd_pre();
        }
    }

    /* remove vdb entry */
    for (DirIterator d(pkg_dir, DirIteratorOptions() + dio_include_dotfiles), d_end ; d != d_end ; ++d)
        FSEntry(*d).unlink();
    pkg_dir.rmdir();

    {
        CategoryMap::iterator it(_imp->categories.find(id->name().category()));
        if (_imp->categories.end() != it && it->second)
        {
            IDMap::iterator it2(_imp->ids.find(id->name()));
            if (_imp->ids.end() != it2)
            {
                std::tr1::shared_ptr<PackageIDSequence> ids(new PackageIDSequence);
                std::remove_copy(it2->second->begin(), it2->second->end(), ids->back_inserter(), id);
                it2->second = ids;
            }
        }
    }

    if (! a.options.is_overwrite())
    {
        std::tr1::shared_ptr<const PackageIDSequence> ids(package_ids(id->name()));
        bool only(true);
        for (PackageIDSequence::ConstIterator it(ids->begin()),
                 it_end(ids->end()); it_end != it; ++it)
            if (*std::tr1::static_pointer_cast<const PackageID>(id) != **it)
            {
                only = false;
                break;
            }
        if (only)
            _imp->names_cache->remove(id->name());
    }

    if (_imp->used_provides_cache || (! _imp->tried_provides_cache && load_provided_using_cache()))
    {
        _imp->provides_map->erase(std::make_pair(id->name(), id->version()));
        write_provides_cache();
        _imp->provides.reset();
    }
}

void
VDBRepository::invalidate()
{
    Lock l(*_imp->big_nasty_mutex);
    _imp.reset(new Implementation<VDBRepository>(this, _imp->params, _imp->big_nasty_mutex));
    _add_metadata_keys();
}

void
VDBRepository::invalidate_masks()
{
}

std::tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence>
VDBRepository::provided_packages() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->provides)
        return _imp->provides;

    Context context("When finding provided packages for '" + stringify(name()) + "':");

    if (! _imp->provides_map)
    {
        if (! load_provided_using_cache())
            load_provided_the_slow_way();
    }

    _imp->provides.reset(new RepositoryProvidesInterface::ProvidesSequence);
    for (ProvidesMap::const_iterator it(_imp->provides_map->begin()),
             it_end(_imp->provides_map->end()); it_end != it; ++it)
    {
        std::tr1::shared_ptr<const ERepositoryID> id(package_id_if_exists(it->first.first, it->first.second));
        if (! id)
        {
            Log::get_instance()->message("e.vdb.provides.no_package", ll_warning, lc_context) <<
                "No package available for '" << it->first.first <<  " " << it->first.second << "'";
            continue;
        }

        for (std::list<QualifiedPackageName>::const_iterator it2(it->second->begin()),
                 it2_end(it->second->end()); it2_end != it2; ++it2)
            _imp->provides->push_back(make_named_values<RepositoryProvidesEntry>(
                        value_for<n::provided_by>(id),
                        value_for<n::virtual_name>(*it2)
                    ));
    }

    return _imp->provides;
}

bool
VDBRepository::load_provided_using_cache() const
{
    Lock l(*_imp->big_nasty_mutex);
    _imp->tried_provides_cache = true;

    if (_imp->params.provides_cache() == FSEntry("/var/empty"))
        return false;

    Context context("When loading VDB PROVIDEs map using '" + stringify(_imp->params.provides_cache()) + "':");

    if (! _imp->params.provides_cache().is_regular_file())
    {
        Log::get_instance()->message("e.vdb.provides_cache.not_regular_file", ll_warning, lc_no_context)
            << "Provides cache at '" << _imp->params.provides_cache() << "' is not a regular file. Perhaps you need to regenerate "
            "the cache using 'paludis --regenerate-installed-cache'?";
        return false;
    }

    SafeIFStream provides_cache(_imp->params.provides_cache());

    std::string version;
    std::getline(provides_cache, version);

    if (version != "paludis-3")
    {
        Log::get_instance()->message("e.vdb.provides_cache.unsupported", ll_warning, lc_no_context) << "Can't use provides cache at '"
            << _imp->params.provides_cache() << "' because format '" << version << "' is not 'paludis-3'. Perhaps you need to regenerate "
            "the cache using 'paludis --regenerate-installed-cache'?";
        return false;
    }

    std::string for_name;
    std::getline(provides_cache, for_name);
    if (for_name != stringify(name()))
    {
        Log::get_instance()->message("e.vdb.provides_cache.unusable", ll_warning, lc_no_context)
            << "Can't use provides cache at '" << _imp->params.provides_cache() << "' because it was generated for repository '"
            << for_name << "'. You must not have multiple provides caches at the same location.";
        return false;
    }

    _imp->provides_map.reset(new ProvidesMap);

    std::string line;
    while (std::getline(provides_cache, line))
    {
        try
        {
            std::vector<std::string> tokens;
            tokenise_whitespace(line, std::back_inserter(tokens));
            if (tokens.size() < 3)
            {
                Log::get_instance()->message("e.vdb.provides_cache.malformed", ll_warning, lc_context)
                    << "Not using PROVIDES cache line '" << line << "' as it contains fewer than three tokens";
                continue;
            }

            QualifiedPackageName q(tokens.at(0));
            VersionSpec v(tokens.at(1), EAPIData::get_instance()->eapi_from_string(
                        _imp->params.eapi_when_unknown())->supported()->version_spec_options());

            std::tr1::shared_ptr<std::list<QualifiedPackageName> > qpns(new std::list<QualifiedPackageName>);
            std::copy(tokens.begin() + 2, tokens.end(), create_inserter<QualifiedPackageName>(
                        std::back_inserter(*qpns)));

            if (_imp->provides_map->end() != _imp->provides_map->find(std::make_pair(q, v)))
                Log::get_instance()->message("e.vdb.provides_cache.duplicate", ll_warning, lc_context)
                    << "Not using PROVIDES cache line '" << line << "' as it names a package that has already been specified";
            else
                _imp->provides_map->insert(std::make_pair(std::make_pair(q, v), qpns));
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.vdb.provides_cache.unusable", ll_warning, lc_context)
                << "Not using PROVIDES cache line '" << line << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        }
    }

    _imp->used_provides_cache = true;
    return true;
}

void
VDBRepository::provides_from_package_id(const PackageID & id) const
{
    Context context("When loading VDB PROVIDEs entry for '" + stringify(id) + "':");

    try
    {
        if (! id.provide_key())
            return;

        std::tr1::shared_ptr<const ProvideSpecTree> provide(id.provide_key()->value());
        DepSpecFlattener<ProvideSpecTree, PackageDepSpec> f(_imp->params.environment());
        provide->root()->accept(f);

        std::tr1::shared_ptr<std::list<QualifiedPackageName> > qpns(new std::list<QualifiedPackageName>);

        for (DepSpecFlattener<ProvideSpecTree, PackageDepSpec>::ConstIterator
                 p(f.begin()), p_end(f.end()) ; p != p_end ; ++p)
        {
            QualifiedPackageName pp((*p)->text());

            if (pp.category() != CategoryNamePart("virtual"))
                Log::get_instance()->message("e.vdb.provide.non_virtual", ll_warning, lc_no_context)
                    << "PROVIDE of non-virtual '" << pp << "' from '" << id << "' will not work as expected";

            qpns->push_back(pp);
        }

        ProvidesMap::iterator it(_imp->provides_map->find(std::make_pair(id.name(), id.version())));
        if (qpns->empty())
        {
            if (_imp->provides_map->end() != it)
                _imp->provides_map->erase(it);
        }
        else
        {
            if (_imp->provides_map->end() == it)
                _imp->provides_map->insert(std::make_pair(std::make_pair(id.name(), id.version()), qpns));
            else
                it->second = qpns;
        }
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & ee)
    {
        Log::get_instance()->message("e.vdb.provides.failure", ll_warning, lc_no_context) << "Skipping VDB PROVIDE entry for '"
            << id << "' due to exception '" << ee.message() << "' (" << ee.what() << ")";
    }
}

void
VDBRepository::load_provided_the_slow_way() const
{
    Lock l(*_imp->big_nasty_mutex);

    using namespace std::tr1::placeholders;

    Context context("When loading VDB PROVIDEs map the slow way:");

    Log::get_instance()->message("e.vdb.provides.starting", ll_debug, lc_no_context) << "Starting VDB PROVIDEs map creation";
    _imp->provides_map.reset(new ProvidesMap);

    need_category_names();
    std::for_each(_imp->categories.begin(), _imp->categories.end(),
            std::tr1::bind(std::tr1::mem_fn(&VDBRepository::need_package_ids), this,
                std::tr1::bind<CategoryNamePart>(std::tr1::mem_fn(
                        &std::pair<const CategoryNamePart, std::tr1::shared_ptr<QualifiedPackageNameSet> >::first), _1)));


    for (IDMap::const_iterator i(_imp->ids.begin()), i_end(_imp->ids.end()) ;
            i != i_end ; ++i)
        for (PackageIDSequence::ConstIterator e(i->second->begin()), e_end(i->second->end()) ;
                e != e_end ; ++e)
            provides_from_package_id(**e);

    Log::get_instance()->message("e.vdb.provides.done", ll_debug, lc_no_context) << "Done VDB PROVIDEs map creation";
}

void
VDBRepository::write_provides_cache() const
{
    Context context("When saving provides cache to '" + stringify(_imp->params.provides_cache()) + "':");

    try
    {
        SafeOFStream f(_imp->params.provides_cache());

        f << "paludis-3" << std::endl;
        f << name() << std::endl;

        for (ProvidesMap::const_iterator it(_imp->provides_map->begin()),
                 it_end(_imp->provides_map->end()); it_end != it; ++it)
        {
            f << it->first.first << " " << it->first.second;
            for (std::list<QualifiedPackageName>::const_iterator it2(it->second->begin()),
                     it2_end(it->second->end()); it2_end != it2; ++it2)
                f << " " << *it2;
            f << std::endl;
        }
    }
    catch (const SafeOFStreamError & e)
    {
        Log::get_instance()->message("e.vdb.provides.write_failed", ll_warning, lc_context) << "Cannot write to '" <<
                _imp->params.provides_cache() << "': '" << e.message() << "' (" << e.what() << ")";
        return;
    }
}

void
VDBRepository::regenerate_cache() const
{
    Lock l(*_imp->big_nasty_mutex);

    regenerate_provides_cache();
    _imp->names_cache->regenerate_cache();
}

void
VDBRepository::regenerate_provides_cache() const
{
    Lock l(*_imp->big_nasty_mutex);

    using namespace std::tr1::placeholders;

    if (_imp->params.provides_cache() == FSEntry("/var/empty"))
        return;

    Context context("When generating VDB repository provides cache at '"
            + stringify(_imp->params.provides_cache()) + "':");

    FSEntry(_imp->params.provides_cache()).unlink();
    _imp->params.provides_cache().dirname().mkdir();

    load_provided_the_slow_way();
    write_provides_cache();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
VDBRepository::category_names_containing_package(const PackageNamePart & p) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! _imp->names_cache->usable())
        return Repository::category_names_containing_package(p);

    std::tr1::shared_ptr<const CategoryNamePartSet> result(
            _imp->names_cache->category_names_containing_package(p));

    return result ? result : Repository::category_names_containing_package(p);
}

namespace
{
    bool slot_is_same(const std::tr1::shared_ptr<const PackageID> & a,
            const std::tr1::shared_ptr<const PackageID> & b)
    {
        if (a->slot_key())
            return b->slot_key() && a->slot_key()->value() == b->slot_key()->value();
        else
            return ! b->slot_key();
    }

    std::tr1::shared_ptr<OutputManager> this_output_manager(const std::tr1::shared_ptr<OutputManager> & o, const Action &)
    {
        return o;
    }
}

void
VDBRepository::merge(const MergeParams & m)
{
    Context context("When merging '" + stringify(*m.package_id()) + "' at '" + stringify(m.image_dir())
            + "' to VDB repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(*m.package_id()))
        throw ActionFailedError("Not a suitable destination for '" + stringify(*m.package_id()) + "'");

    std::tr1::shared_ptr<const ERepositoryID> is_replace(package_id_if_exists(m.package_id()->name(), m.package_id()->version()));

    FSEntry tmp_vdb_dir(_imp->params.location());
    if (! tmp_vdb_dir.exists())
        tmp_vdb_dir.mkdir();
    tmp_vdb_dir /= stringify(m.package_id()->name().category());
    if (! tmp_vdb_dir.exists())
        tmp_vdb_dir.mkdir();
    tmp_vdb_dir /= ("-checking-" + stringify(m.package_id()->name().package()) + "-" + stringify(m.package_id()->version()));
    tmp_vdb_dir.mkdir();

    WriteVDBEntryCommand write_vdb_entry_command(
            make_named_values<WriteVDBEntryParams>(
                value_for<n::environment>(_imp->params.environment()),
                value_for<n::environment_file>(m.environment_file()),
                value_for<n::maybe_output_manager>(m.output_manager()),
                value_for<n::output_directory>(tmp_vdb_dir),
                value_for<n::package_id>(std::tr1::static_pointer_cast<const ERepositoryID>(m.package_id()))
            ));

    write_vdb_entry_command();

    /* load CONFIG_PROTECT, CONFIG_PROTECT_MASK from vdb */
    std::string config_protect, config_protect_mask;
    try
    {
        SafeIFStream c(tmp_vdb_dir / "CONFIG_PROTECT");
        config_protect = std::string((std::istreambuf_iterator<char>(c)), std::istreambuf_iterator<char>());
    }
    catch (const SafeIFStreamError &)
    {
    }

    try
    {
        SafeIFStream c_m(tmp_vdb_dir / "CONFIG_PROTECT_MASK");
        config_protect_mask = std::string((std::istreambuf_iterator<char>(c_m)), std::istreambuf_iterator<char>());
    }
    catch (const SafeIFStreamError &)
    {
    }

    FSEntry vdb_dir(_imp->params.location());
    vdb_dir /= stringify(m.package_id()->name().category());
    vdb_dir /= (stringify(m.package_id()->name().package()) + "-" + stringify(m.package_id()->version()));


    VDBMerger merger(
            make_named_values<VDBMergerParams>(
                value_for<n::config_protect>(config_protect),
                value_for<n::config_protect_mask>(config_protect_mask),
                value_for<n::contents_file>(vdb_dir / "CONTENTS"),
                value_for<n::environment>(_imp->params.environment()),
                value_for<n::image>(m.image_dir()),
                value_for<n::merged_entries>(m.merged_entries()),
                value_for<n::options>(m.options()),
                value_for<n::output_manager>(m.output_manager()),
                value_for<n::package_id>(m.package_id()),
                value_for<n::root>(installed_root_key()->value())
            ));

    (m.used_this_for_config_protect())(config_protect);

    if (! merger.check())
    {
        for (DirIterator d(tmp_vdb_dir, DirIteratorOptions() + dio_include_dotfiles), d_end ; d != d_end ; ++d)
            FSEntry(*d).unlink();
        tmp_vdb_dir.rmdir();
        throw ActionFailedError("Not proceeding with install due to merge sanity check failing");
    }

    if (is_replace)
    {
        /* hack: before we nuke its vdb dir, preload CONTENTS */
        is_replace->contents_key()->value();

        FSEntry old_vdb_dir(_imp->params.location());
        old_vdb_dir /= stringify(is_replace->name().category());
        old_vdb_dir /= (stringify(is_replace->name().package()) + "-" + stringify(is_replace->version()));

        if ((old_vdb_dir.dirname() / ("-reinstalling-" + old_vdb_dir.basename())).exists())
            throw ActionFailedError("Directory '" + stringify(old_vdb_dir.dirname() /
                        ("-reinstalling-" + old_vdb_dir.basename())) + "' already exists, probably due to "
                    "a previous failed upgrade. If it is safe to do so, remove this directory and try "
                    "again.");
        old_vdb_dir.rename(old_vdb_dir.dirname() / ("-reinstalling-" + old_vdb_dir.basename()));
    }

    tmp_vdb_dir.rename(vdb_dir);

    std::tr1::shared_ptr<const PackageID> new_id;
    {
        CategoryMap::iterator it(_imp->categories.find(m.package_id()->name().category()));
        if (_imp->categories.end() != it && it->second)
        {
            it->second->insert(m.package_id()->name());
            IDMap::iterator it2(_imp->ids.find(m.package_id()->name()));
            if (_imp->ids.end() == it2)
            {
                std::tr1::shared_ptr<PackageIDSequence> ids(new PackageIDSequence);
                it2 = _imp->ids.insert(std::make_pair(m.package_id()->name(), ids)).first;
            }
            it2->second->push_back(new_id = make_id(m.package_id()->name(), m.package_id()->version(), vdb_dir));
        }
    }

    merger.merge();

    if (is_replace)
    {
        UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                    value_for<n::config_protect>(config_protect),
                    value_for<n::if_for_install_id>(m.package_id()),
                    value_for<n::ignore_for_unmerge>(std::tr1::bind(&ignore_merged, m.merged_entries(),
                            std::tr1::placeholders::_1)),
                    value_for<n::is_overwrite>(true),
                    value_for<n::make_output_manager>(std::tr1::bind(&this_output_manager, m.output_manager(), std::tr1::placeholders::_1))
                    ));
        m.perform_uninstall()(is_replace, uo);
    }

    if (std::tr1::static_pointer_cast<const ERepositoryID>(m.package_id())
            ->eapi()->supported()->ebuild_phases()->ebuild_new_upgrade_phase_order())
    {
        const std::tr1::shared_ptr<const PackageIDSequence> & replace_candidates(package_ids(m.package_id()->name()));
        for (PackageIDSequence::ConstIterator it(replace_candidates->begin()),
                 it_end(replace_candidates->end()); it_end != it; ++it)
        {
            std::tr1::shared_ptr<const ERepositoryID> candidate(std::tr1::static_pointer_cast<const ERepositoryID>(*it));
            if (candidate != is_replace && candidate != new_id && slot_is_same(candidate, m.package_id()))
            {
                UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                            value_for<n::config_protect>(config_protect),
                            value_for<n::if_for_install_id>(m.package_id()),
                            value_for<n::ignore_for_unmerge>(std::tr1::bind(&ignore_merged, m.merged_entries(),
                                    std::tr1::placeholders::_1)),
                            value_for<n::is_overwrite>(false),
                            value_for<n::make_output_manager>(std::tr1::bind(&this_output_manager, m.output_manager(), std::tr1::placeholders::_1))
                            ));
                m.perform_uninstall()(candidate, uo);
            }
        }
    }

    VDBPostMergeCommand post_merge_command(
            make_named_values<VDBPostMergeCommandParams>(
                value_for<n::root>(installed_root_key()->value())
            ));

    post_merge_command();
    _imp->names_cache->add(m.package_id()->name());

    if (_imp->used_provides_cache || (! _imp->tried_provides_cache && load_provided_using_cache()))
    {
        provides_from_package_id(*m.package_id());
        write_provides_cache();
        _imp->provides.reset();
    }
}

void
VDBRepository::need_category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_category_names)
        return;

    Context context("When loading category names from '" + stringify(_imp->params.location()) + "':");

    for (DirIterator d(_imp->params.location(), DirIteratorOptions() + dio_inode_sort), d_end ; d != d_end ; ++d)
        try
        {
            if (d->is_directory_or_symlink_to_directory())
                _imp->categories.insert(std::make_pair(CategoryNamePart(d->basename()),
                            std::tr1::shared_ptr<QualifiedPackageNameSet>()));
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.vdb.categories.failure", ll_warning, lc_context) << "Skipping VDB category dir '"
                << *d << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        }

    _imp->has_category_names = true;
}

void
VDBRepository::need_package_ids(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->categories[c])
        return;

    Context context("When loading package names from '" + stringify(_imp->params.location()) +
            "' in category '" + stringify(c) + "':");

    std::tr1::shared_ptr<QualifiedPackageNameSet> q(new QualifiedPackageNameSet);

    for (DirIterator d(_imp->params.location() / stringify(c), DirIteratorOptions() + dio_inode_sort), d_end ; d != d_end ; ++d)
        try
        {
            if (d->is_directory_or_symlink_to_directory())
            {
                std::string s(d->basename());
                if (std::string::npos == s.rfind('-'))
                    continue;

                PackageDepSpec p(parse_user_package_dep_spec("=" + stringify(c) + "/" + s,
                            _imp->params.environment(), UserPackageDepSpecOptions()));
                q->insert(*p.package_ptr());
                IDMap::iterator i(_imp->ids.find(*p.package_ptr()));
                if (_imp->ids.end() == i)
                    i = _imp->ids.insert(std::make_pair(*p.package_ptr(), make_shared_ptr(new PackageIDSequence))).first;
                i->second->push_back(make_id(*p.package_ptr(), p.version_requirements_ptr()->begin()->version_spec(), *d));
            }
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.vdb.packages.failure", ll_warning, lc_context) << "Skipping VDB package dir '"
                << *d << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        }

    _imp->categories[c] = q;
}

const std::tr1::shared_ptr<const ERepositoryID>
VDBRepository::make_id(const QualifiedPackageName & q, const VersionSpec & v, const FSEntry & f) const
{
    Lock l(*_imp->big_nasty_mutex);

    Context context("When creating ID for '" + stringify(q) + "-" + stringify(v) + "' from '" + stringify(f) + "':");

    std::tr1::shared_ptr<VDBID> result(new VDBID(q, v, _imp->params.environment(), shared_from_this(), f));
    return result;
}

const std::tr1::shared_ptr<const ERepositoryID>
VDBRepository::package_id_if_exists(const QualifiedPackageName & q, const VersionSpec & v) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_package_named(q))
        return std::tr1::shared_ptr<const ERepositoryID>();

    need_package_ids(q.category());

    using namespace std::tr1::placeholders;

    PackageIDSequence::ConstIterator i(_imp->ids[q]->begin()), i_end(_imp->ids[q]->end());
    for ( ; i != i_end ; ++i)
        if (v == (*i)->version())
            return std::tr1::static_pointer_cast<const ERepositoryID>(*i);
    return std::tr1::shared_ptr<const ERepositoryID>();
}

void
VDBRepository::need_keys_added() const
{
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
VDBRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
VDBRepository::location_key() const
{
    return _imp->location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
VDBRepository::installed_root_key() const
{
    return _imp->root_key;
}

namespace
{
    typedef std::map<QualifiedPackageName, QualifiedPackageName> DepRewrites;

    struct DepRewriter
    {
        const DepRewrites & rewrites;
        bool changed;

        std::stringstream str;
        StringifyFormatter f;

        DepRewriter(const DepRewrites & w) :
            rewrites(w),
            changed(false)
        {
        }

        void do_annotations(const DepSpec & p)
        {
            if (p.annotations_key() && (p.annotations_key()->begin_metadata() != p.annotations_key()->end_metadata()))
            {
                str << " [[ ";
                for (MetadataSectionKey::MetadataConstIterator k(p.annotations_key()->begin_metadata()),
                        k_end(p.annotations_key()->end_metadata()) ;
                        k != k_end ; ++k)
                {
                    const MetadataValueKey<std::string> * r(
                            simple_visitor_cast<const MetadataValueKey<std::string> >(**k));
                    if (! r)
                        throw InternalError(PALUDIS_HERE, "annotations must be string keys");
                    str << (*k)->raw_name() << " = [" << (r->value().empty() ? " " : " " + r->value() + " ") << "] ";
                }
                str << "]] ";
            }
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            str << "|| ( ";
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            str << " ) ";
            do_annotations(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            str << "( ";
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            str << " ) ";
            do_annotations(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node)
        {
            /* don't rewrite blocks. some people block the old package after
             * doing a move. */
            str << f.format(*node.spec(), format::Plain()) << " ";
            do_annotations(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
        {
            str << f.format(*node.spec(), format::Plain()) << " ";
            do_annotations(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            if (node.spec()->package_ptr() && rewrites.end() != rewrites.find(*node.spec()->package_ptr()))
            {
                changed = true;
                str << f.format(PartiallyMadePackageDepSpec(*node.spec())
                        .package(rewrites.find(*node.spec()->package_ptr())->second),
                        format::Plain()) << " ";
            }
            else
                str << f.format(*node.spec(), format::Plain()) << " ";

            do_annotations(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            str << f.format(*node.spec(), format::Plain()) << " ( ";
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            str << " ) ";
            do_annotations(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            str << f.format(*node.spec(), format::Plain()) << " ";
            do_annotations(*node.spec());
        }
    };

    bool rewrite_dependencies(
            const FSEntry & f,
            const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > & key,
            const DepRewrites & rewrites)
    {
        DepRewriter v(rewrites);
        key->value()->root()->accept(v);
        if (v.changed)
        {
            if ("yes" == getenv_with_default("PALUDIS_CARRY_OUT_UPDATES", ""))
            {
                std::cout << "    Rewriting " << f << std::endl;
                SafeOFStream ff(f);
                ff << v.str.str() << std::endl;
            }
            else
                std::cout << "    Would rewrite " << f << std::endl;
        }

        return v.changed;
    }
}

void
VDBRepository::perform_updates()
{
    Context context("When performing updates:");

    DepRewrites dep_rewrites;

    typedef std::list<std::pair<std::tr1::shared_ptr<const PackageID>, QualifiedPackageName> > Moves;
    Moves moves;

    typedef std::list<std::pair<std::tr1::shared_ptr<const PackageID>, SlotName> > SlotMoves;
    SlotMoves slot_moves;

    std::time_t ignore_updates_before(0);
    FSEntry cache_dir(_imp->params.location() / ".cache");
    FSEntry cache_file(cache_dir / "updates_time_cache");
    if (cache_file.is_regular_file_or_symlink_to_regular_file())
        ignore_updates_before = cache_file.mtim().seconds();

    std::cout << std::endl << "Checking for updates (package moves etc):" << std::endl;

    for (PackageDatabase::RepositoryConstIterator r(_imp->params.environment()->package_database()->begin_repositories()),
            r_end(_imp->params.environment()->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        Context context_2("When performing updates from '" + stringify((*r)->name()) + "':");

        Repository::MetadataConstIterator k_iter((*r)->find_metadata("e_updates_location"));
        if (k_iter == (*r)->end_metadata())
        {
            Log::get_instance()->message("e.vdb.updates.no_key", ll_debug, lc_context) <<
                "Repository " << (*r)->name() << " defines no e_updates_location key";
            continue;
        }

        const MetadataValueKey<FSEntry> * k(simple_visitor_cast<const MetadataValueKey<FSEntry> >(**k_iter));
        if (! k)
        {
            Log::get_instance()->message("e.vdb.udpates.bad_key", ll_warning, lc_context) <<
                "Repository " << (*r)->name() << " defines an e_updates_location key, but it is not an FSEntry key";
            continue;
        }

        FSEntry dir(k->value());
        if (! dir.is_directory_or_symlink_to_directory())
        {
            Log::get_instance()->message("e.vdb.updates.bad_key", ll_warning, lc_context) <<
                "Repository " << (*r)->name() << " has e_updates_location " << dir << ", but this is not a directory";
            continue;
        }

        try
        {
            for (DirIterator d(k->value(), DirIteratorOptions()), d_end ;
                    d != d_end ; ++d)
            {
                Context context_3("When performing updates from '" + stringify(*d) + "':");

                if (! d->is_regular_file_or_symlink_to_regular_file())
                    continue;

                if (d->mtim().seconds() <= ignore_updates_before)
                {
                    Log::get_instance()->message("e.vdb.updates.ignoring", ll_debug, lc_context) <<
                        "Ignoring " << *d << " because it hasn't changed";
                    continue;
                }

                std::cout << "    Considering " << *d << std::endl;

                LineConfigFile f(*d, LineConfigFileOptions());

                for (LineConfigFile::ConstIterator line(f.begin()), line_end(f.end()) ;
                        line != line_end ; ++line)
                {
                    std::vector<std::string> tokens;
                    tokenise_whitespace(*line, std::back_inserter(tokens));

                    if (tokens.empty())
                        continue;

                    if ("move" == tokens.at(0))
                    {
                        if (3 == tokens.size())
                        {
                            QualifiedPackageName old_q(tokens.at(1)), new_q(tokens.at(2));

                            /* we want to rewrite deps to avoid a mess. we do
                             * this even if we don't have an installed thing
                             * matching the dep, since a package might dep upon
                             * || ( a b ) where a is installed and b is being
                             * moved. */
                            dep_rewrites.insert(std::make_pair(old_q, new_q)).first->second = new_q;

                            const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->params.environment())[selection::AllVersionsSorted(
                                        generator::Package(old_q) & generator::InRepository(name())
                                        )]);
                            if (! ids->empty())
                            {
                                for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                                        i != i_end ; ++i)
                                    moves.push_back(std::make_pair(*i, new_q));
                            }
                        }
                        else
                            Log::get_instance()->message("e.vdb.updates.bad_line", ll_warning, lc_context) <<
                                "Don't know how to handle '" << *line << "' in " << *d << ": expected 3 tokens for a move";
                    }
                    else if ("slotmove" == tokens.at(0))
                    {
                        if (4 == tokens.size())
                        {
                            PackageDepSpec old_spec(parse_user_package_dep_spec(tokens.at(1), _imp->params.environment(),
                                        UserPackageDepSpecOptions()));
                            SlotName old_slot(tokens.at(2)), new_slot(tokens.at(3));

                            const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->params.environment())[selection::AllVersionsSorted(
                                        (generator::Matches(old_spec, MatchPackageOptions()) & generator::InRepository(name())) |
                                        filter::Slot(old_slot)
                                        )]);
                            if (! ids->empty())
                            {
                                for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                                        i != i_end ; ++i)
                                    slot_moves.push_back(std::make_pair(*i, new_slot));
                            }
                        }
                        else
                            Log::get_instance()->message("e.vdb.updates.bad_line", ll_warning, lc_context) <<
                                "Don't know how to handle '" << *line << "' in " << *d << ": expected 4 tokens for a slotmove";
                    }
                    else
                        Log::get_instance()->message("e.vdb.updates.bad_line", ll_warning, lc_context) <<
                            "Don't know how to handle '" << *line << "' in " << *d << ": unknown operation";
                }
            }
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.vdb.updates.exception", ll_warning, lc_context) <<
                "Caught exception '" << e.message() << "' (" << e.what() << ") when looking for updates. This is "
                "probably bad.";
        }
    }

    bool failed(false);
    try
    {
        std::cout << std::endl;

        if (! moves.empty())
        {
            if ("yes" == getenv_with_default("PALUDIS_CARRY_OUT_UPDATES", ""))
            {
                std::cout << std::endl << "Performing moves:" << std::endl;
                for (Moves::const_iterator m(moves.begin()), m_end(moves.end()) ;
                        m != m_end ; ++m)
                {
                    std::cout << "    " << *m->first << " to " << m->second << std::endl;

                    FSEntry target_cat_dir(_imp->params.location() / stringify(m->second.category()));
                    target_cat_dir.mkdir();

                    FSEntry from_dir(m->first->fs_location_key()->value());
                    FSEntry to_dir(target_cat_dir / ((stringify(m->second.package()) + "-" + stringify(m->first->version()))));

                    if (to_dir.exists())
                    {
                        /* Uh oh. It's possible to install both a package and its renamed version. */
                        Log::get_instance()->message("e.vdb.updates.collision", ll_warning, lc_context) <<
                            "I wanted to rename '" << from_dir << "' to '" << to_dir << "' for a package move, but the "
                            "latter already exists. Consult the Paludis FAQ for how to proceed.";
                        failed = true;
                    }
                    else
                    {
                        from_dir.rename(to_dir);
                        SafeOFStream pf(to_dir / "PF");
                        pf << m->second.package() << "-" << m->first->version() << std::endl;
                        SafeOFStream category(to_dir / "CATEGORY");
                        category << m->second.category() << std::endl;
                    }
                }
            }
            else
            {
                std::cout << std::endl << "The following package moves need to be performed:" << std::endl;
                for (Moves::const_iterator m(moves.begin()), m_end(moves.end()) ;
                        m != m_end ; ++m)
                    std::cout << "    " << *m->first << " to " << m->second << std::endl;
                std::cout << std::endl;
            }
        }

        if (! slot_moves.empty())
        {
            if ("yes" == getenv_with_default("PALUDIS_CARRY_OUT_UPDATES", ""))
            {
                std::cout << std::endl << "Performing slot moves:" << std::endl;
                for (SlotMoves::const_iterator m(slot_moves.begin()), m_end(slot_moves.end()) ;
                        m != m_end ; ++m)
                {
                    std::cout << "    " << *m->first << " to " << m->second << std::endl;

                    SafeOFStream f(m->first->fs_location_key()->value() / "SLOT");
                    f << m->second << std::endl;
                }
            }
            else
            {
                std::cout << std::endl << "The following slot moves need to be performed:" << std::endl;
                for (SlotMoves::const_iterator m(slot_moves.begin()), m_end(slot_moves.end()) ;
                        m != m_end ; ++m)
                    std::cout << "    " << *m->first << " to " << m->second << std::endl;
                std::cout << std::endl;
            }
        }

        if ((! moves.empty()) || (! slot_moves.empty()))
            invalidate();

        if ("yes" != getenv_with_default("PALUDIS_CARRY_OUT_UPDATES", ""))
        {
            if ((! moves.empty()) || (! slot_moves.empty()))
            {
                std::cout << "Profile updates support is currently considered experimental. See the Paludis" << std::endl;
                std::cout << "FAQ for how to proceed." << std::endl;
                std::cout << std::endl;
            }
        }
        else
        {
            if ((! moves.empty()) || (! slot_moves.empty()))
            {
                if (_imp->params.provides_cache() != FSEntry("/var/empty"))
                    if (_imp->params.provides_cache().is_regular_file_or_symlink_to_regular_file())
                    {
                        std::cout << std::endl << "Invalidating provides cache following updates" << std::endl;
                        FSEntry(_imp->params.provides_cache()).unlink();
                        regenerate_provides_cache();
                    }

                std::cout << std::endl << "Invalidating names cache following updates" << std::endl;
                _imp->names_cache->regenerate_cache();
            }
        }

        if (! dep_rewrites.empty())
        {
            std::cout << std::endl << "Updating installed package dependencies" << std::endl;

            bool rewrite_done(false);
            const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->params.environment())[selection::AllVersionsSorted(
                        generator::InRepository(name()))]);
            for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                    i != i_end ; ++i)
            {
                if ((*i)->build_dependencies_key())
                    rewrite_done |= rewrite_dependencies((*i)->fs_location_key()->value() / (*i)->build_dependencies_key()->raw_name(),
                            (*i)->build_dependencies_key(), dep_rewrites);
                if ((*i)->run_dependencies_key())
                    rewrite_done |= rewrite_dependencies((*i)->fs_location_key()->value() / (*i)->run_dependencies_key()->raw_name(),
                            (*i)->run_dependencies_key(), dep_rewrites);
                if ((*i)->post_dependencies_key())
                    rewrite_done |= rewrite_dependencies((*i)->fs_location_key()->value() / (*i)->post_dependencies_key()->raw_name(),
                            (*i)->post_dependencies_key(), dep_rewrites);
                if ((*i)->suggested_dependencies_key())
                    rewrite_done |= rewrite_dependencies((*i)->fs_location_key()->value() / (*i)->suggested_dependencies_key()->raw_name(),
                            (*i)->suggested_dependencies_key(), dep_rewrites);
            }

            if ((rewrite_done) && ("yes" != getenv_with_default("PALUDIS_CARRY_OUT_UPDATES", "")))
            {
                std::cout << "Some installed packages have dependencies that need rewriting for package" << std::endl;
                std::cout << "moves. See the Paludis FAQ for how to proceed." << std::endl;
                std::cout << std::endl;
            }
        }

        if ((! failed) && ("yes" == getenv_with_default("PALUDIS_CARRY_OUT_UPDATES", "")))
        {
            cache_dir.mkdir();
            SafeOFStream cache_file_f(cache_file);
            cache_file_f << std::endl;
        }
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message("e.vdb.updates.exception", ll_warning, lc_context) <<
            "Caught exception '" << e.message() << "' (" << e.what() << ") when performing updates. This is "
            "probably bad.";
    }
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
VDBRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

