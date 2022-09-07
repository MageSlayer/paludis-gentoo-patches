/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/match_package.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repository_name_cache.hh>
#include <paludis/set_file.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_requirements.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/output_manager.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/dep_spec_annotations.hh>
#include <paludis/unformatted_pretty_printer.hh>
#include <paludis/slot.hh>

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
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/return_literal_function.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>

#include <unordered_map>
#include <functional>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <iostream>
#include <mutex>
#include <cstring>
#include <cerrno>
#include <ctime>

using namespace paludis;
using namespace paludis::erepository;

typedef std::unordered_map<CategoryNamePart, std::shared_ptr<QualifiedPackageNameSet>, Hash<CategoryNamePart> > CategoryMap;
typedef std::unordered_map<QualifiedPackageName, std::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> > IDMap;
typedef std::map<std::pair<QualifiedPackageName, VersionSpec>, std::shared_ptr<std::list<QualifiedPackageName> > > ProvidesMap;

namespace paludis
{
    template <>
    struct Imp<VDBRepository>
    {
        VDBRepositoryParams params;

        const std::shared_ptr<std::recursive_mutex> big_nasty_mutex;

        mutable CategoryMap categories;
        mutable bool has_category_names;
        mutable IDMap ids;

        std::shared_ptr<RepositoryNameCache> names_cache;

        Imp(const VDBRepository * const, const VDBRepositoryParams &, std::shared_ptr<std::recursive_mutex> = std::make_shared<std::recursive_mutex>());
        ~Imp();

        std::shared_ptr<const MetadataValueKey<FSPath> > location_key;
        std::shared_ptr<const MetadataValueKey<FSPath> > root_key;
        std::shared_ptr<const MetadataValueKey<std::string> > format_key;
        std::shared_ptr<const MetadataValueKey<FSPath> > names_cache_key;
        std::shared_ptr<const MetadataValueKey<FSPath> > builddir_key;
        std::shared_ptr<const MetadataValueKey<std::string> > eapi_when_unknown_key;
    };

    Imp<VDBRepository>::Imp(const VDBRepository * const r,
            const VDBRepositoryParams & p, std::shared_ptr<std::recursive_mutex> m) :
        params(p),
        big_nasty_mutex(m),
        has_category_names(false),
        names_cache(std::make_shared<RepositoryNameCache>(p.names_cache(), r)),
        location_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("location", "location",
                    mkt_significant, params.location())),
        root_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("root", "root",
                    mkt_normal, params.root())),
        format_key(std::make_shared<LiteralMetadataValueKey<std::string> >("format", "format",
                    mkt_significant, "vdb")),
        names_cache_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("names_cache", "names_cache",
                    mkt_normal, params.names_cache())),
        builddir_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("builddir", "builddir",
                    mkt_normal, params.builddir())),
        eapi_when_unknown_key(std::make_shared<LiteralMetadataValueKey<std::string> >(
                    "eapi_when_unknown", "eapi_when_unknown", mkt_normal, params.eapi_when_unknown()))
    {
    }

    Imp<VDBRepository>::~Imp() = default;
}

VDBRepository::VDBRepository(const VDBRepositoryParams & p) :
    EInstalledRepository(
            make_named_values<EInstalledRepositoryParams>(
                n::builddir() = p.builddir(),
                n::environment() = p.environment(),
                n::root() = p.root()
                ),
            p.name(),
            make_named_values<RepositoryCapabilities>(
                n::destination_interface() = this,
                n::environment_variable_interface() = this,
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(nullptr)
            )),
    _imp(this, p)
{
    _add_metadata_keys();
}

VDBRepository::~VDBRepository() = default;

void
VDBRepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->root_key);
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->names_cache_key);
    add_metadata_key(_imp->builddir_key);
    add_metadata_key(_imp->eapi_when_unknown_key);
}

bool
VDBRepository::has_category_named(const CategoryNamePart & c, const RepositoryContentMayExcludes &) const
{
    std::unique_lock<std::recursive_mutex> lock(*_imp->big_nasty_mutex);

    Context context("When checking for category '" + stringify(c) +
            "' in " + stringify(name()) + ":");

    need_category_names();
    return _imp->categories.end() != _imp->categories.find(c);
}

bool
VDBRepository::has_package_named(const QualifiedPackageName & q, const RepositoryContentMayExcludes &) const
{
    std::unique_lock<std::recursive_mutex> lock(*_imp->big_nasty_mutex);

    Context context("When checking for package '" + stringify(q) +
            "' in " + stringify(name()) + ":");

    need_category_names();

    CategoryMap::iterator cat_iter(_imp->categories.find(q.category()));
    if (_imp->categories.end() == cat_iter)
        return false;

    need_package_ids(q.category());

    return cat_iter->second->end() != cat_iter->second->find(q);
}

const bool
VDBRepository::is_unimportant() const
{
    return false;
}

std::shared_ptr<const CategoryNamePartSet>
VDBRepository::category_names(const RepositoryContentMayExcludes &) const
{
    std::unique_lock<std::recursive_mutex> lock(*_imp->big_nasty_mutex);

    Context context("When fetching category names in " + stringify(name()) + ":");

    need_category_names();

    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());

    std::transform(_imp->categories.begin(), _imp->categories.end(), result->inserter(),
            std::mem_fn(&std::pair<const CategoryNamePart, std::shared_ptr<QualifiedPackageNameSet> >::first));

    return result;
}

std::shared_ptr<const QualifiedPackageNameSet>
VDBRepository::package_names(const CategoryNamePart & c, const RepositoryContentMayExcludes & x) const
{
    std::unique_lock<std::recursive_mutex> lock(*_imp->big_nasty_mutex);

    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    std::shared_ptr<QualifiedPackageNameSet> result(std::make_shared<QualifiedPackageNameSet>());

    need_category_names();
    if (! has_category_named(c, x))
        return std::make_shared<QualifiedPackageNameSet>();

    need_package_ids(c);

    return _imp->categories.find(c)->second;
}

std::shared_ptr<const PackageIDSequence>
VDBRepository::package_ids(const QualifiedPackageName & n, const RepositoryContentMayExcludes & x) const
{
    std::unique_lock<std::recursive_mutex> lock(*_imp->big_nasty_mutex);

    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");


    need_category_names();
    if (! has_category_named(n.category(), x))
        return std::make_shared<PackageIDSequence>();

    need_package_ids(n.category());
    if (! has_package_named(n, x))
        return std::make_shared<PackageIDSequence>();

    return _imp->ids.find(n)->second;
}

std::shared_ptr<Repository>
VDBRepository::repository_factory_create(
        Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    Context context("When making VDB repository from repo_file '" + f("repo_file") + "':");

    std::string location(f("location"));
    if (location.empty())
        throw VDBRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string root(f("root"));
    if (root.empty())
        root = "/";

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

    return std::make_shared<VDBRepository>(make_named_values<VDBRepositoryParams>(
                n::builddir() = builddir,
                n::eapi_when_unknown() = eapi_when_unknown,
                n::environment() = env,
                n::location() = location,
                n::name() = RepositoryName(name),
                n::names_cache() = names_cache,
                n::root() = root
                ));
}

RepositoryName
VDBRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> & f)
{
    std::string name(f("name"));
    if (name.empty())
        return RepositoryName("installed");
    else
        return RepositoryName(name);
}

std::shared_ptr<const RepositoryNameSet>
VDBRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

VDBRepositoryConfigurationError::VDBRepositoryConfigurationError(
        const std::string & msg) noexcept :
    ConfigurationError("VDB repository configuration error: " + msg)
{
}

VDBRepositoryKeyReadError::VDBRepositoryKeyReadError(
        const std::string & msg) noexcept :
    ConfigurationError("VDB repository key read error: " + msg)
{
}

namespace
{
    bool ignore_merged(const std::shared_ptr<const FSPathSet> & s, const FSPath & f)
    {
        return s->end() != s->find(f);
    }
}

void
VDBRepository::perform_uninstall(
        const std::shared_ptr<const ERepositoryID> & id,
        const UninstallAction & a) const
{
    Context context("When uninstalling '" + stringify(*id) + (a.options.is_overwrite() ? "' for an overwrite:" : "':"));

    if (! _imp->params.root().stat().is_directory())
        throw ActionFailedError("Couldn't uninstall '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root()) + "') is not a directory");

    if (! id->eapi()->supported())
        throw ActionFailedError("Couldn't uninstall '" + stringify(*id) +
                "' because EAPI is unsupported");

    std::shared_ptr<OutputManager> output_manager(a.options.make_output_manager()(a));

    std::string reinstalling_str(a.options.is_overwrite() ? "-reinstalling-" : "");

    std::shared_ptr<FSPathSequence> eclassdirs(std::make_shared<FSPathSequence>());
    eclassdirs->push_back(FSPath(_imp->params.location() / stringify(id->name().category()) /
                (reinstalling_str + stringify(id->name().package()) + "-" + stringify(id->version()))));

    FSPath pkg_dir(_imp->params.location() / stringify(id->name().category()) / (reinstalling_str +
                stringify(id->name().package()) + "-" + stringify(id->version())));

    std::shared_ptr<FSPath> load_env(std::make_shared<FSPath>(pkg_dir / "environment.bz2"));

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_uninstall());
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        bool skip(false);
        do
        {
            switch (a.options.want_phase()(phase->equal_option("skipname")))
            {
                case wp_yes:
                    continue;

                case wp_skip:
                    skip = true;
                    continue;

                case wp_abort:
                    throw ActionAbortedError("Told to abort install");

                case last_wp:
                    break;
            }

            throw InternalError(PALUDIS_HERE, "bad want_phase");
        } while (false);

        if (skip)
            continue;

        if (can_skip_phase(_imp->params.environment(), id, *phase))
        {
            output_manager->stdout_stream() << "--- No need to do anything for " << phase->equal_option("skipname") << " phase" << std::endl;
            continue;
        }

        if (phase->option("unmerge"))
        {
            /* load CONFIG_PROTECT, CONFIG_PROTECT_MASK from vdb, supplement with env */
            std::string config_protect;
            std::string config_protect_mask;

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

            std::shared_ptr<const Contents> contents(a.options.override_contents());
            if (! contents)
                contents = id->contents();

            /* unmerge */
            VDBUnmerger unmerger(
                    make_named_values<VDBUnmergerOptions>(
                        n::config_protect() = final_config_protect,
                        n::config_protect_mask() = config_protect_mask,
                        n::contents() = contents,
                        n::environment() = _imp->params.environment(),
                        n::ignore() = a.options.ignore_for_unmerge(),
                        n::output_manager() = output_manager,
                        n::package_id() = id,
                        n::root() = installed_root_key()->parse_value()
                    ));
            unmerger.unmerge();

            VDBPostMergeUnmergeCommand post_unmerge_command(
                    make_named_values<VDBPostMergeUnmergeCommandParams>(
                        n::root() = installed_root_key()->parse_value()
                    ));
            post_unmerge_command();
        }
        else
        {
            FSPath package_builddir(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-uninstall"));
            EbuildCommandParams params(make_named_values<EbuildCommandParams>(
                    n::builddir() = _imp->params.builddir(),
                    n::clearenv() = phase->option("clearenv"),
                    n::commands() = join(phase->begin_commands(), phase->end_commands(), " "),
                    n::distdir() = pkg_dir,
                    n::ebuild_dir() = pkg_dir,
                    n::ebuild_file() = pkg_dir / (stringify(id->name().package()) + "-" + stringify(id->version()) + ".ebuild"),
                    n::eclassdirs() = eclassdirs,
                    n::environment() = _imp->params.environment(),
                    n::exlibsdirs() = std::make_shared<FSPathSequence>(),
                    n::files_dir() = pkg_dir,
                    n::maybe_output_manager() = output_manager,
                    n::package_builddir() = package_builddir,
                    n::package_id() = id,
                    n::parts() = nullptr,
                    n::permitted_directories() = nullptr,
                    n::portdir() = _imp->params.location(),
                    n::root() = stringify(_imp->params.root()),
                    n::sandbox() = phase->option("sandbox"),
                    n::sydbox() = phase->option("sydbox"),
                    n::userpriv() = phase->option("userpriv"),
                    n::volatile_files() = nullptr
                    ));

            EbuildUninstallCommandParams uninstall_params(make_named_values<EbuildUninstallCommandParams>(
                        n::load_environment() = load_env.get(),
                        n::loadsaveenv_dir() = package_builddir / "temp",
                        n::replaced_by() = a.options.if_for_install_id(),
                        n::unmerge_only() = false
                    ));

            EbuildUninstallCommand uninstall_cmd_pre(params, uninstall_params);
            uninstall_cmd_pre();
        }
    }

    /* remove vdb entry */
    for (FSIterator d(pkg_dir, { fsio_include_dotfiles, fsio_inode_sort }), d_end ; d != d_end ; ++d)
        d->unlink();
    pkg_dir.rmdir();

    {
        CategoryMap::iterator it(_imp->categories.find(id->name().category()));
        if (_imp->categories.end() != it && it->second)
        {
            IDMap::iterator it2(_imp->ids.find(id->name()));
            if (_imp->ids.end() != it2)
            {
                std::shared_ptr<PackageIDSequence> ids(std::make_shared<PackageIDSequence>());
                std::remove_copy(it2->second->begin(), it2->second->end(), ids->back_inserter(), id);
                it2->second = ids;
            }
        }
    }

    if (! a.options.is_overwrite())
    {
        std::shared_ptr<const PackageIDSequence> ids(package_ids(id->name(), { }));
        bool only(true);
        for (const auto & it : *ids)
            if (*std::static_pointer_cast<const PackageID>(id) != *it)
            {
                only = false;
                break;
            }
        if (only)
            _imp->names_cache->remove(id->name());
    }
}

void
VDBRepository::invalidate()
{
    std::unique_lock<std::recursive_mutex> lock(*_imp->big_nasty_mutex);
    _imp.reset(new Imp<VDBRepository>(this, _imp->params, _imp->big_nasty_mutex));
    _add_metadata_keys();
}

void
VDBRepository::regenerate_cache() const
{
    std::unique_lock<std::recursive_mutex> lock(*_imp->big_nasty_mutex);

    _imp->names_cache->regenerate_cache();
}

std::shared_ptr<const CategoryNamePartSet>
VDBRepository::category_names_containing_package(const PackageNamePart & p, const RepositoryContentMayExcludes & x) const
{
    std::unique_lock<std::recursive_mutex> lock(*_imp->big_nasty_mutex);

    if (! _imp->names_cache->usable())
        return Repository::category_names_containing_package(p, x);

    std::shared_ptr<const CategoryNamePartSet> result(
            _imp->names_cache->category_names_containing_package(p));

    return result ? result : Repository::category_names_containing_package(p, x);
}

namespace
{
    bool parallel_slot_is_same(const std::shared_ptr<const PackageID> & a,
            const std::shared_ptr<const PackageID> & b)
    {
        if (a->slot_key())
            return b->slot_key() && a->slot_key()->parse_value().parallel_value() == b->slot_key()->parse_value().parallel_value();
        else
            return ! b->slot_key();
    }

    std::shared_ptr<OutputManager> this_output_manager(const std::shared_ptr<OutputManager> & o, const Action &)
    {
        return o;
    }
}

void
VDBRepository::merge(const MergeParams & m)
{
    Context context("When merging '" + stringify(*m.package_id()) + "' at '" + stringify(m.image_dir())
            + "' to VDB repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(m.package_id()))
        throw ActionFailedError("Not a suitable destination for '" + stringify(*m.package_id()) + "'");

    std::shared_ptr<const ERepositoryID> is_replace(package_id_if_exists(m.package_id()->name(), m.package_id()->version()));

    std::string config_protect;
    std::string config_protect_mask;

    FSPath tmp_vdb_dir(_imp->params.location());
    tmp_vdb_dir /= stringify(m.package_id()->name().category());
    tmp_vdb_dir /= ("-checking-" + stringify(m.package_id()->name().package()) + "-" + stringify(m.package_id()->version()));

    if (! m.check())
    {
        if (! tmp_vdb_dir.dirname().dirname().stat().exists())
            tmp_vdb_dir.dirname().dirname().mkdir(0755, { });
        if (! tmp_vdb_dir.dirname().stat().exists())
            tmp_vdb_dir.dirname().mkdir(0755, { });
        tmp_vdb_dir.mkdir(0755, { });

        WriteVDBEntryCommand write_vdb_entry_command(
                make_named_values<WriteVDBEntryParams>(
                    n::environment() = _imp->params.environment(),
                    n::environment_file() = m.environment_file(),
                    n::maybe_output_manager() = m.output_manager(),
                    n::output_directory() = tmp_vdb_dir,
                    n::package_id() = std::static_pointer_cast<const ERepositoryID>(m.package_id())
                ));

        write_vdb_entry_command();

        /* load CONFIG_PROTECT, CONFIG_PROTECT_MASK from vdb */
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
    }
    else
    {
        /* nasty: make CONFIG_PROTECT etc available for hooks */
        try
        {
            config_protect = snoop_variable_from_environment_file(m.environment_file(), "CONFIG_PROTECT");
            config_protect_mask = snoop_variable_from_environment_file(m.environment_file(), "CONFIG_PROTECT_MASK");
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.exndbam_repository.config_protect_unfetchable", ll_warning, lc_context)
                << "Could not load CONFIG_PROTECT for merge checks due to exception '" + e.message() + "' (" + e.what() + ")";
        }
    }

    FSPath vdb_dir(_imp->params.location());
    vdb_dir /= stringify(m.package_id()->name().category());
    vdb_dir /= (stringify(m.package_id()->name().package()) + "-" + stringify(m.package_id()->version()));

    auto eapi(std::static_pointer_cast<const ERepositoryID>(m.package_id())->eapi()->supported());
    auto fix_mtimes(eapi->ebuild_options()->fix_mtimes());

    VDBMerger merger(
            make_named_values<VDBMergerParams>(
                n::config_protect() = config_protect,
                n::config_protect_mask() = config_protect_mask,
                n::contents_file() = vdb_dir / "CONTENTS",
                n::environment() = _imp->params.environment(),
                n::fix_mtimes_before() = fix_mtimes ?  m.build_start_time() : Timestamp(0, 0),
                n::fs_merger_options() = eapi->fs_merger_options(),
                n::image() = m.image_dir(),
                n::merged_entries() = m.merged_entries(),
                n::options() = m.options(),
                n::output_manager() = m.output_manager(),
                n::package_id() = m.package_id(),
                n::permit_destination() = m.permit_destination(),
                n::root() = installed_root_key()->parse_value()
            ));

    (m.used_this_for_config_protect())(config_protect);

    if (m.check())
    {
        if (! merger.check())
            throw ActionFailedError("Not proceeding with install due to merge sanity check failing");
        return;
    }

    std::shared_ptr<const Contents> is_replace_contents;
    if (is_replace)
    {
        is_replace_contents = is_replace->contents();

        FSPath old_vdb_dir(_imp->params.location());
        old_vdb_dir /= stringify(is_replace->name().category());
        old_vdb_dir /= (stringify(is_replace->name().package()) + "-" + stringify(is_replace->version()));

        if ((old_vdb_dir.dirname() / ("-reinstalling-" + old_vdb_dir.basename())).stat().exists())
            throw ActionFailedError("Directory '" + stringify(old_vdb_dir.dirname() /
                        ("-reinstalling-" + old_vdb_dir.basename())) + "' already exists, probably due to "
                    "a previous failed upgrade. If it is safe to do so, remove this directory and try "
                    "again.");
        old_vdb_dir.rename(old_vdb_dir.dirname() / ("-reinstalling-" + old_vdb_dir.basename()));
    }

    tmp_vdb_dir.rename(vdb_dir);

    std::shared_ptr<const PackageID> new_id;
    {
        CategoryMap::iterator it(_imp->categories.find(m.package_id()->name().category()));
        if (_imp->categories.end() != it && it->second)
        {
            it->second->insert(m.package_id()->name());
            IDMap::iterator it2(_imp->ids.find(m.package_id()->name()));
            if (_imp->ids.end() == it2)
            {
                std::shared_ptr<PackageIDSequence> ids(std::make_shared<PackageIDSequence>());
                it2 = _imp->ids.insert(std::make_pair(m.package_id()->name(), ids)).first;
            }
            it2->second->push_back(new_id = make_id(m.package_id()->name(), m.package_id()->version(), vdb_dir));
        }
    }

    merger.merge();

    if (is_replace)
    {
        UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                    n::config_protect() = config_protect,
                    n::if_for_install_id() = m.package_id(),
                    n::ignore_for_unmerge() = std::bind(&ignore_merged, m.merged_entries(),
                            std::placeholders::_1),
                    n::is_overwrite() = true,
                    n::make_output_manager() = std::bind(&this_output_manager, m.output_manager(), std::placeholders::_1),
                    n::override_contents() = is_replace_contents,
                    n::want_phase() = m.want_phase()
                    ));
        m.perform_uninstall()(is_replace, uo);
    }

    if (std::static_pointer_cast<const ERepositoryID>(m.package_id())
            ->eapi()->supported()->ebuild_phases()->ebuild_new_upgrade_phase_order())
    {
        const std::shared_ptr<const PackageIDSequence> & replace_candidates(package_ids(m.package_id()->name(), { }));
        for (const auto & it : *replace_candidates)
        {
            std::shared_ptr<const ERepositoryID> candidate(std::static_pointer_cast<const ERepositoryID>(it));
            if (candidate != is_replace && candidate != new_id && parallel_slot_is_same(candidate, m.package_id()))
            {
                UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                            n::config_protect() = config_protect,
                            n::if_for_install_id() = m.package_id(),
                            n::ignore_for_unmerge() = std::bind(&ignore_merged, m.merged_entries(),
                                    std::placeholders::_1),
                            n::is_overwrite() = false,
                            n::make_output_manager() = std::bind(&this_output_manager, m.output_manager(), std::placeholders::_1),
                            n::override_contents() = nullptr,
                            n::want_phase() = m.want_phase()
                            ));
                m.perform_uninstall()(candidate, uo);
            }
        }
    }

    VDBPostMergeUnmergeCommand post_merge_command(
            make_named_values<VDBPostMergeUnmergeCommandParams>(
                n::root() = installed_root_key()->parse_value()
            ));
    post_merge_command();

    _imp->names_cache->add(m.package_id()->name());
}

void
VDBRepository::need_category_names() const
{
    std::unique_lock<std::recursive_mutex> lock(*_imp->big_nasty_mutex);

    if (_imp->has_category_names)
        return;

    Context context("When loading category names from '" + stringify(_imp->params.location()) + "':");

    for (FSIterator d(_imp->params.location(), { fsio_inode_sort, fsio_want_directories, fsio_deref_symlinks_for_wants }), d_end ; d != d_end ; ++d)
        try
        {
            _imp->categories.insert(std::make_pair(CategoryNamePart(d->basename()), nullptr));
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
    std::unique_lock<std::recursive_mutex> lock(*_imp->big_nasty_mutex);

    if (_imp->categories[c])
        return;

    Context context("When loading package names from '" + stringify(_imp->params.location()) +
            "' in category '" + stringify(c) + "':");

    std::shared_ptr<QualifiedPackageNameSet> q(std::make_shared<QualifiedPackageNameSet>());

    for (FSIterator d(_imp->params.location() / stringify(c), { fsio_inode_sort, fsio_want_directories, fsio_deref_symlinks_for_wants }), d_end ;
            d != d_end ; ++d)
        try
        {
            std::string s(d->basename());
            if (std::string::npos == s.rfind('-'))
                continue;

            PackageDepSpec p(parse_user_package_dep_spec("=" + stringify(c) + "/" + s,
                        _imp->params.environment(), { }));
            q->insert(*p.package_ptr());
            IDMap::iterator i(_imp->ids.find(*p.package_ptr()));
            if (_imp->ids.end() == i)
                i = _imp->ids.insert(std::make_pair(*p.package_ptr(), std::make_shared<PackageIDSequence>())).first;
            i->second->push_back(make_id(*p.package_ptr(), p.version_requirements_ptr()->begin()->version_spec(), *d));
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

const std::shared_ptr<const ERepositoryID>
VDBRepository::make_id(const QualifiedPackageName & q, const VersionSpec & v, const FSPath & f) const
{
    std::unique_lock<std::recursive_mutex> lock(*_imp->big_nasty_mutex);

    Context context("When creating ID for '" + stringify(q) + "-" + stringify(v) + "' from '" + stringify(f) + "':");

    std::shared_ptr<VDBID> result(std::make_shared<VDBID>(q, v, _imp->params.environment(), name(), f));
    return result;
}

const std::shared_ptr<const ERepositoryID>
VDBRepository::package_id_if_exists(const QualifiedPackageName & q, const VersionSpec & v) const
{
    std::unique_lock<std::recursive_mutex> lock(*_imp->big_nasty_mutex);

    if (! has_package_named(q, { }))
        return nullptr;

    need_package_ids(q.category());

    for (const auto & id : *_imp->ids[q])
        if (v == id->version())
            return std::static_pointer_cast<const ERepositoryID>(id);

    return nullptr;
}

void
VDBRepository::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
VDBRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
VDBRepository::location_key() const
{
    return _imp->location_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
VDBRepository::installed_root_key() const
{
    return _imp->root_key;
}

const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > >
VDBRepository::sync_host_key() const
{
    return nullptr;
}

namespace
{
    typedef std::map<QualifiedPackageName, QualifiedPackageName> DepRewrites;

    struct DepRewriter
    {
        const DepRewrites & rewrites;
        bool changed;

        std::stringstream str;
        UnformattedPrettyPrinter f;

        DepRewriter(const DepRewrites & w) :
            rewrites(w),
            changed(false)
        {
        }

        void do_annotations(const DepSpec & p)
        {
            if (p.maybe_annotations())
            {
                bool done_open(false);
                for (const auto & annotation : *p.maybe_annotations())
                {
                    switch (annotation.kind())
                    {
                        case dsak_literal:
                        case dsak_expandable:
                            break;

                        case dsak_expanded:
                        case dsak_synthetic:
                            continue;

                        case last_dsak:
                            throw InternalError(PALUDIS_HERE, "bad dsak. huh?");
                    }

                    if (! done_open)
                        str << " [[ ";
                    done_open = true;

                    str << annotation.key() << " = [" << (annotation.value().empty() ? " " : " " + annotation.value() + " ") << "] ";
                }

                if (done_open)
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
            str << f.prettify(*node.spec()) << " ";
            do_annotations(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
        {
            str << f.prettify(*node.spec()) << " ";
            do_annotations(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            if (node.spec()->package_ptr() && rewrites.end() != rewrites.find(*node.spec()->package_ptr()))
            {
                changed = true;
                str << f.prettify(PartiallyMadePackageDepSpec(*node.spec())
                        .package(rewrites.find(*node.spec()->package_ptr())->second)) << " ";
            }
            else
                str << f.prettify(*node.spec()) << " ";

            do_annotations(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            str << f.prettify(*node.spec()) << " ( ";
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            str << " ) ";
            do_annotations(*node.spec());
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            str << f.prettify(*node.spec()) << " ";
            do_annotations(*node.spec());
        }
    };

    void rewrite_dependencies(
            const FSPath & f,
            const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > & key,
            const DepRewrites & rewrites)
    {
        DepRewriter v(rewrites);
        key->parse_value()->top()->accept(v);
        if (v.changed)
        {
            std::cout << "    Rewriting " << f << std::endl;
            SafeOFStream ff(f, -1, true);
            ff << v.str.str() << std::endl;
        }
    }
}

void
VDBRepository::perform_updates()
{
    Context context("When performing updates:");

    DepRewrites dep_rewrites;

    typedef std::list<std::pair<std::shared_ptr<const PackageID>, QualifiedPackageName> > Moves;
    Moves moves;

    typedef std::list<std::pair<std::shared_ptr<const PackageID>, SlotName> > SlotMoves;
    SlotMoves slot_moves;

    std::map<FSPath, std::time_t, FSPathComparator> cache_contents;
    FSPath cache_dir(_imp->params.location() / ".cache");
    FSPath cache_file(cache_dir / "updates_time_cache");
    if (cache_file.stat().is_regular_file_or_symlink_to_regular_file())
    {
        Context ctx2("When reading update file timestamps from '" + stringify(cache_file) + "':");
        LineConfigFile f(cache_file, { lcfo_preserve_whitespace });

        for (const auto & line : f)
        {
            std::string::size_type tab(line.find('\t'));
            if (std::string::npos == tab)
            {
                Log::get_instance()->message("e.vdb.updates.bad_timestamp_line", ll_warning, lc_context)
                    << "Line '" << line << "' does not contain a tab character";
                continue;
            }
            try
            {
                cache_contents.insert(std::make_pair(FSPath(line.substr(tab + 1)), destringify<std::time_t>(line.substr(0, tab))));
            }
            catch (const DestringifyError &)
            {
                Log::get_instance()->message("e.vdb.updates.bad_timestamp_line", ll_warning, lc_context)
                    << "Line '" << line << "' contains an invalid timestamp";
            }
        }
    }

    std::cout << std::endl << "Checking for updates (package moves etc):" << std::endl;

    std::map<FSPath, std::time_t, FSPathComparator> update_timestamps;
    for (const auto & repository : _imp->params.environment()->repositories())
    {
        Context context_2("When performing updates from '" + stringify(repository->name()) + "':");

        try
        {
            if (0 == stringify(repository->name()).compare(0, 2, "x-"))
            {
                /* ticket:897. not really the best solution, but it'll do for now. */
                continue;
            }

            Repository::MetadataConstIterator k_iter(repository->find_metadata("e_updates_location"));
            if (k_iter == repository->end_metadata())
            {
                Log::get_instance()->message("e.vdb.updates.no_key", ll_debug, lc_context) << "Repository " << repository->name() << " defines no e_updates_location key";
                continue;
            }

            const MetadataValueKey<FSPath> * k(visitor_cast<const MetadataValueKey<FSPath> >(**k_iter));
            if (! k)
            {
                Log::get_instance()->message("e.vdb.udpates.bad_key", ll_warning, lc_context)
                    << "Repository " << repository->name() << " defines an e_updates_location key, but it is not an FSPath key";
                continue;
            }

            FSPath dir(k->parse_value());
            if (! dir.stat().is_directory_or_symlink_to_directory())
            {
                Log::get_instance()->message("e.vdb.updates.bad_key", ll_warning, lc_context)
                    << "Repository " << repository->name() << " has e_updates_location " << dir << ", but this is not a directory";
                continue;
            }

            for (FSIterator d(k->parse_value(), { fsio_want_regular_files, fsio_deref_symlinks_for_wants }), d_end ; d != d_end ; ++d)
            {
                Context context_3("When performing updates from '" + stringify(*d) + "':");

                FSStat d_stat(*d);
                update_timestamps.insert(std::make_pair(*d, d_stat.mtim().seconds()));
                std::map<FSPath, std::time_t, FSPathComparator>::const_iterator last_checked(cache_contents.find(*d));
                if (cache_contents.end() != last_checked && d_stat.mtim().seconds() <= last_checked->second)
                {
                    Log::get_instance()->message("e.vdb.updates.ignoring", ll_debug, lc_context) <<
                        "Ignoring " << *d << " because it hasn't changed";
                    continue;
                }

                std::cout << "    Considering " << *d << std::endl;

                LineConfigFile f(*d, { });

                for (const auto & line : f)
                {
                    std::vector<std::string> tokens;
                    tokenise_whitespace(line, std::back_inserter(tokens));

                    if (tokens.empty())
                        continue;

                    if ("move" == tokens.at(0))
                    {
                        if (3 == tokens.size())
                        {
                            QualifiedPackageName old_q(tokens.at(1));
                            QualifiedPackageName new_q(tokens.at(2));

                            /* we want to rewrite deps to avoid a mess. we do
                             * this even if we don't have an installed thing
                             * matching the dep, since a package might dep upon
                             * || ( a b ) where a is installed and b is being
                             * moved. */
                            dep_rewrites.insert(std::make_pair(old_q, new_q)).first->second = new_q;

                            const std::shared_ptr<const PackageIDSequence> ids((*_imp->params.environment())[selection::AllVersionsSorted(
                                        generator::Package(old_q) & generator::InRepository(name())
                                        )]);
                            if (! ids->empty())
                            {
                                for (const auto & id : *ids)
                                    moves.push_back(std::make_pair(id, new_q));
                            }
                        }
                        else
                            Log::get_instance()->message("e.vdb.updates.bad_line", ll_warning, lc_context) <<
                                "Don't know how to handle '" << line << "' in " << *d << ": expected 3 tokens for a move";
                    }
                    else if ("slotmove" == tokens.at(0))
                    {
                        if (4 == tokens.size())
                        {
                            PackageDepSpec old_spec(parse_user_package_dep_spec(tokens.at(1), _imp->params.environment(),
                                        { }));
                            SlotName old_slot(tokens.at(2));
                            SlotName new_slot(tokens.at(3));

                            const std::shared_ptr<const PackageIDSequence> ids((*_imp->params.environment())[selection::AllVersionsSorted(
                                        (generator::Matches(old_spec, nullptr, { }) & generator::InRepository(name())) |
                                        filter::Slot(old_slot)
                                        )]);
                            if (! ids->empty())
                            {
                                for (const auto & id : *ids)
                                    slot_moves.push_back(std::make_pair(id, new_slot));
                            }
                        }
                        else
                            Log::get_instance()->message("e.vdb.updates.bad_line", ll_warning, lc_context) <<
                                "Don't know how to handle '" << line << "' in " << *d << ": expected 4 tokens for a slotmove";
                    }
                    else
                        Log::get_instance()->message("e.vdb.updates.bad_line", ll_warning, lc_context) <<
                            "Don't know how to handle '" << line << "' in " << *d << ": unknown operation";
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
            std::cout << std::endl << "Performing moves:" << std::endl;
            for (const auto & move : moves)
            {
                std::cout << "    " << *move.first << " to " << move.second << std::endl;

                FSPath target_cat_dir(_imp->params.location() / stringify(move.second.category()));
                target_cat_dir.mkdir(0755, { fspmkdo_ok_if_exists });

                FSPath from_dir(move.first->fs_location_key()->parse_value());
                FSPath to_dir(target_cat_dir / ((stringify(move.second.package()) + "-" + stringify(move.first->version()))));

                if (to_dir.stat().exists())
                {
                    /* Uh oh. It's possible to install both a package and its renamed version. */
                    Log::get_instance()->message("e.vdb.updates.collision", ll_warning, lc_context) <<
                        "I wanted to rename '" << from_dir << "' to '" << to_dir << "' for a package move, but the "
                        "latter already exists. Consult the Paludis FAQ for how to proceed.";
                    failed = true;
                }
                else
                {
                    std::string oldpf(stringify(move.first->name().package()) + "-" + stringify(move.first->version()));
                    std::string newpf(stringify(move.second.package()) + "-" + stringify(move.first->version()));

                    from_dir.rename(to_dir);

                    std::shared_ptr<const EAPI> eapi(std::static_pointer_cast<const VDBID>(move.first)->eapi());
                    if (eapi->supported())
                    {
                        SafeOFStream pf(to_dir / eapi->supported()->ebuild_environment_variables()->env_pf(), -1, true);
                        pf << newpf << std::endl;
                    }
                    else
                    {
                        Log::get_instance()->message("e.vdb.updates.eapi", ll_warning, lc_context)
                            << "Unsupported EAPI '" << eapi->name() << "' for '" << *move.first
                            << "', cannot update PF-equivalent VDB key for move";
                    }

                    SafeOFStream category(to_dir / "CATEGORY", -1, true);
                    category << move.second.category() << std::endl;

                    if (newpf != oldpf)
                    {
                        for (FSIterator it(to_dir, { fsio_inode_sort }), it_end; it_end != it; ++it)
                        {
                            std::string::size_type lastdot(it->basename().rfind('.'));
                            if (std::string::npos != lastdot && 0 == it->basename().compare(0, lastdot, oldpf, 0, oldpf.length()))
                                it->rename(to_dir / (newpf + it->basename().substr(lastdot)));
                        }
                    }
                }
            }
        }

        if (! slot_moves.empty())
        {
            std::cout << std::endl << "Performing slot moves:" << std::endl;
            for (const auto & slot_move : slot_moves)
            {
                std::cout << "    " << *slot_move.first << " to " << slot_move.second << std::endl;

                SafeOFStream f(slot_move.first->fs_location_key()->parse_value() / "SLOT", -1, true);
                f << slot_move.second << std::endl;
            }
        }

        if ((! moves.empty()) || (! slot_moves.empty()))
        {
            invalidate();

            std::cout << std::endl << "Invalidating names cache following updates" << std::endl;
            _imp->names_cache->regenerate_cache();
        }

        if (! dep_rewrites.empty())
        {
            std::cout << std::endl << "Updating installed package dependencies" << std::endl;

            const std::shared_ptr<const PackageIDSequence> ids((*_imp->params.environment())[selection::AllVersionsSorted(
                        generator::InRepository(name()))]);
            for (const auto & id : *ids)
            {
                if (id->build_dependencies_key())
                    rewrite_dependencies(id->fs_location_key()->parse_value() / id->build_dependencies_key()->raw_name(),
                            id->build_dependencies_key(), dep_rewrites);
                if (id->run_dependencies_key())
                    rewrite_dependencies(id->fs_location_key()->parse_value() / id->run_dependencies_key()->raw_name(),
                            id->run_dependencies_key(), dep_rewrites);
                if (id->post_dependencies_key())
                    rewrite_dependencies(id->fs_location_key()->parse_value() / id->post_dependencies_key()->raw_name(),
                            id->post_dependencies_key(), dep_rewrites);
            }

            std::cout << std::endl << "Updating configuration files" << std::endl;

            for (const auto & dep_rewrite : dep_rewrites)
                _imp->params.environment()->update_config_files_for_package_move(
                        make_package_dep_spec({ }).package(dep_rewrite.first),
                        dep_rewrite.second
                        );
        }

        if (! failed)
        {
            cache_dir.mkdir(0755, { fspmkdo_ok_if_exists });
            SafeOFStream cache_file_f(cache_file, -1, true);
            for (const auto & update_timestamp : update_timestamps)
                cache_file_f << update_timestamp.second << '\t' << update_timestamp.first << std::endl;
        }
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message("e.vdb.updates.exception", ll_warning, lc_context) <<
            "Caught exception '" << e.message() << "' (" << e.what() << ") when performing updates. This is "
            "probably bad.";
    }
}

