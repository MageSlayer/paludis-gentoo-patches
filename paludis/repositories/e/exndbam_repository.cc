/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/exndbam_repository.hh>
#include <paludis/repositories/e/exndbam_id.hh>
#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/extra_distribution_data.hh>
#include <paludis/repositories/e/can_skip_phase.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/cookie.hh>
#include <paludis/util/set.hh>
#include <paludis/util/system.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/output_manager.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/ndbam.hh>
#include <paludis/ndbam_merger.hh>
#include <paludis/ndbam_unmerger.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>
#include <functional>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    bool supported_exndbam(const std::string & s)
    {
        return s == "exndbam-1";
    }
}

namespace paludis
{
    template <>
    struct Imp<ExndbamRepository>
    {
        ExndbamRepositoryParams params;
        mutable NDBAM ndbam;

        std::shared_ptr<const MetadataValueKey<FSPath> > location_key;
        std::shared_ptr<const MetadataValueKey<FSPath> > root_key;
        std::shared_ptr<const MetadataValueKey<std::string> > format_key;
        std::shared_ptr<const MetadataValueKey<FSPath> > builddir_key;
        std::shared_ptr<const MetadataValueKey<std::string> > eapi_when_unknown_key;

        Imp(const ExndbamRepositoryParams & p) :
            params(p),
            ndbam(params.location(), &supported_exndbam, "exndbam-1",
                    EAPIData::get_instance()->eapi_from_string(
                        params.eapi_when_unknown())->supported()->version_spec_options()),
            location_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("location", "location",
                        mkt_significant, params.location())),
            root_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("root", "root",
                        mkt_normal, params.root())),
            format_key(std::make_shared<LiteralMetadataValueKey<std::string> >("format", "format",
                        mkt_significant, "vdb")),
            builddir_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("builddir", "builddir",
                        mkt_normal, params.builddir())),
            eapi_when_unknown_key(std::make_shared<LiteralMetadataValueKey<std::string> >(
                        "eapi_when_unknown", "eapi_when_unknown", mkt_normal, params.eapi_when_unknown()))
        {
        }
    };
}

ExndbamRepository::ExndbamRepository(const RepositoryName & n, const ExndbamRepositoryParams & p) :
    EInstalledRepository(
            make_named_values<EInstalledRepositoryParams>(
                n::builddir() = p.builddir(),
                n::environment() = p.environment(),
                n::root() = p.root()
                ),
            n,
            make_named_values<RepositoryCapabilities>(
                n::destination_interface() = this,
                n::environment_variable_interface() = this,
                n::make_virtuals_interface() = static_cast<RepositoryMakeVirtualsInterface *>(0),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(0),
                n::provides_interface() = static_cast<RepositoryProvidesInterface *>(0),
                n::virtuals_interface() = static_cast<RepositoryVirtualsInterface *>(0)
            )),
    _imp(p)
{
    _add_metadata_keys();
}

ExndbamRepository::~ExndbamRepository()
{
}

void
ExndbamRepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->root_key);
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->builddir_key);
    add_metadata_key(_imp->eapi_when_unknown_key);
}

std::shared_ptr<Repository>
ExndbamRepository::repository_factory_create(
        Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    Context context("When making Exndbam repository from repo_file '" + f("repo_file") + "':");

    std::string location(f("location"));
    if (location.empty())
        throw ExndbamRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string root(f("root"));
    if (root.empty())
        root = "/";

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

    return std::make_shared<ExndbamRepository>(
            RepositoryName(name),
            make_named_values<ExndbamRepositoryParams>(
                n::builddir() = builddir,
                n::eapi_when_unknown() = eapi_when_unknown,
                n::environment() = env,
                n::location() = location,
                n::root() = root
                )
            );
}

RepositoryName
ExndbamRepository::repository_factory_name(
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
ExndbamRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

void
ExndbamRepository::invalidate()
{
    _imp.reset(new Imp<ExndbamRepository>(_imp->params));
    _add_metadata_keys();
}

void
ExndbamRepository::invalidate_masks()
{
}

std::shared_ptr<const PackageIDSequence>
ExndbamRepository::package_ids(const QualifiedPackageName & q) const
{
    std::shared_ptr<NDBAMEntrySequence> entries(_imp->ndbam.entries(q));
    std::shared_ptr<PackageIDSequence> result(std::make_shared<PackageIDSequence>());

    for (IndirectIterator<NDBAMEntrySequence::ConstIterator> e(entries->begin()), e_end(entries->end()) ;
            e != e_end ; ++e)
    {
        Lock l(*(*e).mutex());
        if (! (*e).package_id())
            (*e).package_id() = std::make_shared<ExndbamID>((*e).name(), (*e).version(), _imp->params.environment(),
                        name(), (*e).fs_location(), &_imp->ndbam);
        result->push_back((*e).package_id());
    }

    return result;
}

std::shared_ptr<const QualifiedPackageNameSet>
ExndbamRepository::package_names(const CategoryNamePart & c) const
{
    return _imp->ndbam.package_names(c);
}

std::shared_ptr<const CategoryNamePartSet>
ExndbamRepository::category_names() const
{
    return _imp->ndbam.category_names();
}

std::shared_ptr<const CategoryNamePartSet>
ExndbamRepository::category_names_containing_package(
        const PackageNamePart & p) const
{
    return _imp->ndbam.category_names_containing_package(p);
}

bool
ExndbamRepository::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->ndbam.has_package_named(q);
}

bool
ExndbamRepository::has_category_named(const CategoryNamePart & c) const
{
    return _imp->ndbam.has_category_named(c);
}

const bool
ExndbamRepository::is_unimportant() const
{
    return false;
}

ExndbamRepositoryConfigurationError::ExndbamRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("Exndbam repository configuration error: " + msg)
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
ExndbamRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
ExndbamRepository::location_key() const
{
    return _imp->location_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
ExndbamRepository::installed_root_key() const
{
    return _imp->root_key;
}

const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > >
ExndbamRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

void
ExndbamRepository::need_keys_added() const
{
}

namespace
{
    std::pair<uid_t, gid_t>
    get_new_ids_or_minus_one(const Environment * const env, const FSPath & f)
    {
        FSStat f_stat(f);
        uid_t uid = (f_stat.owner() == env->reduced_uid()) ? 0 : -1;
        gid_t gid = (f_stat.group() == env->reduced_gid()) ? 0 : -1;

        return std::make_pair(uid, gid);
    }

    bool slot_is_same(const std::shared_ptr<const PackageID> & a,
            const std::shared_ptr<const PackageID> & b)
    {
        if (a->slot_key())
            return b->slot_key() && a->slot_key()->value() == b->slot_key()->value();
        else
            return ! b->slot_key();
    }

    std::shared_ptr<OutputManager> this_output_manager(const std::shared_ptr<OutputManager> & o, const Action &)
    {
        return o;
    }

    bool ignore_merged(const std::shared_ptr<const FSPathSet> & s,
            const FSPath & f)
    {
        return s->end() != s->find(f);
    }
}

void
ExndbamRepository::merge(const MergeParams & m)
{
    Context context("When merging '" + stringify(*m.package_id()) + "' at '" + stringify(m.image_dir())
            + "' to Exndbam repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(m.package_id()))
        throw ActionFailedError("Not a suitable destination for '" + stringify(*m.package_id()) + "'");

    std::shared_ptr<const PackageID> if_overwritten_id, if_same_name_id;
    {
        std::shared_ptr<const PackageIDSequence> ids(package_ids(m.package_id()->name()));
        for (PackageIDSequence::ConstIterator v(ids->begin()), v_end(ids->end()) ;
                v != v_end ; ++v)
        {
            if_same_name_id = *v;
            if ((*v)->version() == m.package_id()->version() && slot_is_same(*v, m.package_id()))
            {
                if_overwritten_id = *v;
                break;
            }
        }
    }

    FSPath uid_dir(_imp->params.location());
    if (if_same_name_id)
        uid_dir = if_same_name_id->fs_location_key()->value().dirname();
    else
    {
        std::string uid(stringify(m.package_id()->name().category()) + "---" + stringify(m.package_id()->name().package()));
        uid_dir /= "data";
        uid_dir.mkdir(0755, { fspmkdo_ok_if_exists });
        uid_dir /= uid;
        uid_dir.mkdir(0755, { fspmkdo_ok_if_exists });
    }

    FSPath target_ver_dir(uid_dir);
    target_ver_dir /= (stringify(m.package_id()->version()) + ":" + stringify(m.package_id()->slot_key()->value()) + ":" + cookie());

    if (target_ver_dir.stat().exists())
        throw ActionFailedError("Temporary merge directory '" + stringify(target_ver_dir) + "' already exists, probably "
                "due to a previous failed install. If it is safe to do so, please remove this directory and try again.");
    target_ver_dir.mkdir(0755, { });

    WriteVDBEntryCommand write_vdb_entry_command(
            make_named_values<WriteVDBEntryParams>(
                n::environment() = _imp->params.environment(),
                n::environment_file() = m.environment_file(),
                n::maybe_output_manager() = m.output_manager(),
                n::output_directory() = target_ver_dir,
                n::package_id() = std::static_pointer_cast<const ERepositoryID>(m.package_id())
            ));

    write_vdb_entry_command();

    _imp->ndbam.add_entry(m.package_id()->name(), target_ver_dir);

    /* load CONFIG_PROTECT, CONFIG_PROTECT_MASK back */
    std::string config_protect, config_protect_mask;
    try
    {
        SafeIFStream c(target_ver_dir / "CONFIG_PROTECT");
        config_protect = std::string((std::istreambuf_iterator<char>(c)), std::istreambuf_iterator<char>());
    }
    catch (const SafeIFStreamError &)
    {
    }

    try
    {
        SafeIFStream c_m(target_ver_dir / "CONFIG_PROTECT_MASK");
        config_protect_mask = std::string((std::istreambuf_iterator<char>(c_m)), std::istreambuf_iterator<char>());
    }
    catch (const SafeIFStreamError &)
    {
    }

    bool fix_mtimes(std::static_pointer_cast<const ERepositoryID>(
                m.package_id())->eapi()->supported()->ebuild_options()->fix_mtimes());

    NDBAMMerger merger(
            make_named_values<NDBAMMergerParams>(
                n::config_protect() = config_protect,
                n::config_protect_mask() = config_protect_mask,
                n::contents_file() = target_ver_dir / "contents",
                n::environment() = _imp->params.environment(),
                n::fix_mtimes_before() = fix_mtimes ?  m.build_start_time() : Timestamp(0, 0),
                n::get_new_ids_or_minus_one() = std::bind(&get_new_ids_or_minus_one, _imp->params.environment(), std::placeholders::_1),
                n::image() = m.image_dir(),
                n::install_under() = FSPath("/"),
                n::merged_entries() = m.merged_entries(),
                n::options() = m.options(),
                n::output_manager() = m.output_manager(),
                n::package_id() = m.package_id(),
                n::root() = installed_root_key()->value()
            ));

    (m.used_this_for_config_protect())(config_protect);

    if (! merger.check())
    {
        for (FSIterator d(target_ver_dir, { fsio_inode_sort, fsio_include_dotfiles }), d_end ; d != d_end ; ++d)
            d->unlink();
        target_ver_dir.rmdir();
        throw ActionFailedError("Not proceeding with install due to merge sanity check failing");
    }

    merger.merge();

    _imp->ndbam.index(m.package_id()->name(), uid_dir.basename());

    if (if_overwritten_id)
    {
        UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                    n::config_protect() = config_protect,
                    n::if_for_install_id() = m.package_id(),
                    n::ignore_for_unmerge() = std::bind(&ignore_merged, m.merged_entries(),
                            std::placeholders::_1),
                    n::is_overwrite() = true,
                    n::make_output_manager() = std::bind(&this_output_manager, m.output_manager(), std::placeholders::_1)
                    ));
        m.perform_uninstall()(if_overwritten_id, uo);
    }

    if (std::static_pointer_cast<const ERepositoryID>(m.package_id())
            ->eapi()->supported()->ebuild_phases()->ebuild_new_upgrade_phase_order())
    {
        const std::shared_ptr<const PackageIDSequence> & replace_candidates(package_ids(m.package_id()->name()));
        for (PackageIDSequence::ConstIterator it(replace_candidates->begin()),
                 it_end(replace_candidates->end()); it_end != it; ++it)
        {
            std::shared_ptr<const ERepositoryID> candidate(std::static_pointer_cast<const ERepositoryID>(*it));
            if (candidate != if_overwritten_id && candidate->fs_location_key()->value() != target_ver_dir && slot_is_same(candidate, m.package_id()))
            {
                UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                            n::config_protect() = config_protect,
                            n::if_for_install_id() = m.package_id(),
                            n::ignore_for_unmerge() = std::bind(&ignore_merged, m.merged_entries(),
                                    std::placeholders::_1),
                            n::is_overwrite() = false,
                            n::make_output_manager() = std::bind(&this_output_manager, m.output_manager(), std::placeholders::_1)
                            ));
                m.perform_uninstall()(candidate, uo);
            }
        }
    }

    VDBPostMergeUnmergeCommand post_merge_command(
            make_named_values<VDBPostMergeUnmergeCommandParams>(
                n::root() = installed_root_key()->value()
            ));
    post_merge_command();
}

void
ExndbamRepository::perform_uninstall(
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

    FSPath ver_dir(id->fs_location_key()->value().realpath());
    std::shared_ptr<FSPath> load_env(std::make_shared<FSPath>(ver_dir / "environment.bz2"));

    auto eclassdirs(std::make_shared<FSPathSequence>());
    eclassdirs->push_back(ver_dir);

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_uninstall());
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (can_skip_phase(_imp->params.environment(), id, *phase))
        {
            output_manager->stdout_stream() << "--- No need to do anything for " <<
                phase->equal_option("skipname") << " phase" << std::endl;
            continue;
        }

        if (phase->option("unmerge"))
        {
            /* load CONFIG_PROTECT, CONFIG_PROTECT_MASK from vdb, supplement with env */
            std::string config_protect, config_protect_mask;

            try
            {
                SafeIFStream c(ver_dir / "CONFIG_PROTECT");
                config_protect = std::string((std::istreambuf_iterator<char>(c)), std::istreambuf_iterator<char>()) +
                        " " + getenv_with_default("CONFIG_PROTECT", "");
            }
            catch (const SafeIFStreamError &)
            {
            }

            try
            {
                SafeIFStream c_m(ver_dir / "CONFIG_PROTECT_MASK");
                config_protect_mask = std::string((std::istreambuf_iterator<char>(c_m)), std::istreambuf_iterator<char>()) +
                    " " + getenv_with_default("CONFIG_PROTECT_MASK", "");
            }
            catch (const SafeIFStreamError &)
            {
            }

            std::string final_config_protect(config_protect + " " + a.options.config_protect());

            /* unmerge */
            NDBAMUnmerger unmerger(
                    make_named_values<NDBAMUnmergerOptions>(
                    n::config_protect() = final_config_protect,
                    n::config_protect_mask() = config_protect_mask,
                    n::contents_file() = ver_dir / "contents",
                    n::environment() = _imp->params.environment(),
                    n::ignore() = a.options.ignore_for_unmerge(),
                    n::ndbam() = &_imp->ndbam,
                    n::output_manager() = output_manager,
                    n::package_id() = id,
                    n::root() = installed_root_key()->value()
                    ));

            unmerger.unmerge();

            VDBPostMergeUnmergeCommand post_unmerge_command(
                    make_named_values<VDBPostMergeUnmergeCommandParams>(
                        n::root() = installed_root_key()->value()
                    ));
            post_unmerge_command();
        }
        else
        {
            FSPath package_builddir(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-uninstall"));
            EbuildCommandParams params(
                    make_named_values<EbuildCommandParams>(
                        n::builddir() = _imp->params.builddir(),
                        n::clearenv() = phase->option("clearenv"),
                        n::commands() = join(phase->begin_commands(), phase->end_commands(), " "),
                        n::distdir() = ver_dir,
                        n::ebuild_dir() = ver_dir,
                        n::ebuild_file() = ver_dir / (stringify(id->name().package()) + "-" + stringify(id->version()) + ".ebuild"),
                        n::eclassdirs() = eclassdirs,
                        n::environment() = _imp->params.environment(),
                        n::exlibsdirs() = std::make_shared<FSPathSequence>(),
                        n::files_dir() = ver_dir,
                        n::maybe_output_manager() = output_manager,
                        n::package_builddir() = package_builddir,
                        n::package_id() = id,
                        n::portdir() = _imp->params.location(),
                        n::root() = stringify(_imp->params.root()),
                        n::sandbox() = phase->option("sandbox"),
                        n::sydbox() = phase->option("sydbox"),
                        n::userpriv() = phase->option("userpriv")
                    ));

            EbuildUninstallCommandParams uninstall_params(
                    make_named_values<EbuildUninstallCommandParams>(
                        n::load_environment() = load_env.get(),
                        n::loadsaveenv_dir() = package_builddir / "temp",
                        n::replaced_by() = a.options.if_for_install_id(),
                        n::unmerge_only() = false
                    ));

            EbuildUninstallCommand uninstall_cmd_pre(params, uninstall_params);
            uninstall_cmd_pre();
        }
    }

    for (FSIterator d(ver_dir, { fsio_inode_sort, fsio_include_dotfiles }), d_end ; d != d_end ; ++d)
        d->unlink();
    ver_dir.rmdir();

    _imp->ndbam.remove_entry(id->name(), ver_dir);

    FSPath pkg_dir(ver_dir.dirname());
    if (FSIterator() == FSIterator(pkg_dir, { fsio_include_dotfiles, fsio_inode_sort, fsio_first_only }))
    {
        pkg_dir.rmdir();

        _imp->ndbam.deindex(id->name());
    }
}

void
ExndbamRepository::regenerate_cache() const
{
}

void
ExndbamRepository::perform_updates()
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
ExndbamRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

