/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/cookie.hh>
#include <paludis/util/set.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/system.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/ndbam.hh>
#include <paludis/ndbam_merger.hh>
#include <paludis/ndbam_unmerger.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>
#include <tr1/functional>
#include <iostream>
#include <fstream>

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
    struct Implementation<ExndbamRepository>
    {
        ExndbamRepositoryParams params;
        mutable NDBAM ndbam;

        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > root_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > builddir_key;

        Implementation(const ExndbamRepositoryParams & p) :
            params(p),
            ndbam(params.location(), &supported_exndbam, "exndbam-1"),
            location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                        mkt_significant, params.location())),
            root_key(new LiteralMetadataValueKey<FSEntry> ("root", "root",
                        mkt_normal, params.root())),
            format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                        mkt_significant, "vdb")),
            builddir_key(new LiteralMetadataValueKey<FSEntry> ("builddir", "builddir",
                        mkt_normal, params.builddir()))
        {
        }
    };
}

ExndbamRepository::ExndbamRepository(const RepositoryName & n, const ExndbamRepositoryParams & p) :
    EInstalledRepository(
            make_named_values<EInstalledRepositoryParams>(
                value_for<n::builddir>(p.builddir()),
                value_for<n::environment>(p.environment()),
                value_for<n::root>(p.root())
                ),
            n,
            make_named_values<RepositoryCapabilities>(
                value_for<n::destination_interface>(this),
                value_for<n::e_interface>(static_cast<RepositoryEInterface *>(0)),
                value_for<n::environment_variable_interface>(this),
                value_for<n::hook_interface>(this),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(static_cast<RepositoryManifestInterface *>(0)),
                value_for<n::mirrors_interface>(static_cast<RepositoryMirrorsInterface *>(0)),
                value_for<n::provides_interface>(static_cast<RepositoryProvidesInterface *>(0)),
                value_for<n::qa_interface>(static_cast<RepositoryQAInterface *>(0)),
                value_for<n::sets_interface>(this),
                value_for<n::syncable_interface>(static_cast<RepositorySyncableInterface *>(0)),
                value_for<n::virtuals_interface>(static_cast<RepositoryVirtualsInterface *>(0))
            )),
    PrivateImplementationPattern<ExndbamRepository>(new Implementation<ExndbamRepository>(p)),
    _imp(PrivateImplementationPattern<ExndbamRepository>::_imp)
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
}

std::tr1::shared_ptr<Repository>
ExndbamRepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
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

    return std::tr1::shared_ptr<Repository>(new ExndbamRepository(
                RepositoryName(name),
                make_named_values<ExndbamRepositoryParams>(
                    value_for<n::builddir>(builddir),
                    value_for<n::environment>(env),
                    value_for<n::location>(location),
                    value_for<n::root>(root)
                    )
                ));
}

RepositoryName
ExndbamRepository::repository_factory_name(
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
ExndbamRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

void
ExndbamRepository::invalidate()
{
    _imp.reset(new Implementation<ExndbamRepository>(_imp->params));
    _add_metadata_keys();
}

void
ExndbamRepository::invalidate_masks()
{
}

std::tr1::shared_ptr<const PackageIDSequence>
ExndbamRepository::package_ids(const QualifiedPackageName & q) const
{
    std::tr1::shared_ptr<NDBAMEntrySequence> entries(_imp->ndbam.entries(q));
    std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

    for (IndirectIterator<NDBAMEntrySequence::ConstIterator> e(entries->begin()), e_end(entries->end()) ;
            e != e_end ; ++e)
    {
        Lock l(*(*e).mutex());
        if (! (*e).package_id())
            (*e).package_id().reset(new ExndbamID((*e).name(), (*e).version(), _imp->params.environment(),
                        shared_from_this(), (*e).fs_location(), &_imp->ndbam));
        result->push_back((*e).package_id());
    }

    return result;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
ExndbamRepository::package_names(const CategoryNamePart & c) const
{
    return _imp->ndbam.package_names(c);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
ExndbamRepository::category_names() const
{
    return _imp->ndbam.category_names();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
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

ExndbamRepositoryConfigurationError::ExndbamRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("Exndbam repository configuration error: " + msg)
{
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
ExndbamRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
ExndbamRepository::location_key() const
{
    return _imp->location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
ExndbamRepository::installed_root_key() const
{
    return _imp->root_key;
}

void
ExndbamRepository::need_keys_added() const
{
}

namespace
{
    std::pair<uid_t, gid_t>
    get_new_ids_or_minus_one(const Environment * const env, const FSEntry & f)
    {
        uid_t uid = (f.owner() == env->reduced_uid()) ? 0 : -1;
        gid_t gid = (f.group() == env->reduced_gid()) ? 0 : -1;

        return std::make_pair(uid, gid);
    }
}

void
ExndbamRepository::merge(const MergeParams & m)
{
    Context context("When merging '" + stringify(*m.package_id()) + "' at '" + stringify(m.image_dir())
            + "' to Exndbam repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(*m.package_id()))
        throw InstallActionError("Not a suitable destination for '" + stringify(*m.package_id()) + "'");

    std::tr1::shared_ptr<const PackageID> if_overwritten_id, if_same_name_id;
    {
        std::tr1::shared_ptr<const PackageIDSequence> ids(package_ids(m.package_id()->name()));
        for (PackageIDSequence::ConstIterator v(ids->begin()), v_end(ids->end()) ;
                v != v_end ; ++v)
        {
            if_same_name_id = *v;
            if ((*v)->version() == m.package_id()->version() && (*v)->slot() == m.package_id()->slot())
            {
                if_overwritten_id = *v;
                break;
            }
        }
    }

    FSEntry uid_dir(_imp->params.location());
    if (if_same_name_id)
        uid_dir = if_same_name_id->fs_location_key()->value().dirname();
    else
    {
        std::string uid(stringify(m.package_id()->name().category()) + "---" + stringify(m.package_id()->name().package()));
        uid_dir /= "data";
        uid_dir.mkdir();
        uid_dir /= uid;
        uid_dir.mkdir();
    }

    FSEntry target_ver_dir(uid_dir);
    target_ver_dir /= (stringify(m.package_id()->version()) + ":" + stringify(m.package_id()->slot()) + ":" + cookie());

    if (target_ver_dir.exists())
        throw InstallActionError("Temporary merge directory '" + stringify(target_ver_dir) + "' already exists, probably "
                "due to a previous failed install. If it is safe to do so, please remove this directory and try again.");
    target_ver_dir.mkdir();

    WriteVDBEntryCommand write_vdb_entry_command(
            make_named_values<WriteVDBEntryParams>(
            value_for<n::environment>(_imp->params.environment()),
            value_for<n::environment_file>(m.environment_file()),
            value_for<n::output_directory>(target_ver_dir),
            value_for<n::package_id>(std::tr1::static_pointer_cast<const ERepositoryID>(m.package_id()))
            ));

    write_vdb_entry_command();

    /* load CONFIG_PROTECT, CONFIG_PROTECT_MASK back */
    std::string config_protect, config_protect_mask;
    {
        std::ifstream c(stringify(target_ver_dir / "CONFIG_PROTECT").c_str());
        config_protect = std::string((std::istreambuf_iterator<char>(c)), std::istreambuf_iterator<char>());

        std::ifstream c_m(stringify(target_ver_dir / "CONFIG_PROTECT_MASK").c_str());
        config_protect_mask = std::string((std::istreambuf_iterator<char>(c_m)), std::istreambuf_iterator<char>());
    }

    NDBAMMerger merger(
            make_named_values<NDBAMMergerParams>(
            value_for<n::config_protect>(config_protect),
            value_for<n::config_protect_mask>(config_protect_mask),
            value_for<n::contents_file>(target_ver_dir / "contents"),
            value_for<n::environment>(_imp->params.environment()),
            value_for<n::get_new_ids_or_minus_one>(std::tr1::bind(&get_new_ids_or_minus_one, _imp->params.environment(), std::tr1::placeholders::_1)),
            value_for<n::image>(m.image_dir()),
            value_for<n::install_under>(FSEntry("/")),
            value_for<n::options>(m.options()),
            value_for<n::package_id>(m.package_id()),
            value_for<n::root>(installed_root_key()->value())
            ));

    (m.used_this_for_config_protect())(config_protect);

    if (! merger.check())
    {
        for (DirIterator d(target_ver_dir, DirIteratorOptions() + dio_include_dotfiles), d_end
                ; d != d_end ; ++d)
            FSEntry(*d).unlink();
        target_ver_dir.rmdir();
        throw InstallActionError("Not proceeding with install due to merge sanity check failing");
    }

    merger.merge();

    _imp->ndbam.index(m.package_id()->name(), uid_dir.basename());

    if (if_overwritten_id)
    {
        perform_uninstall(std::tr1::static_pointer_cast<const ERepositoryID>(if_overwritten_id), true, config_protect);
    }
    if (std::tr1::static_pointer_cast<const ERepositoryID>(m.package_id())
            ->eapi()->supported()->ebuild_phases()->ebuild_new_upgrade_phase_order())
    {
        const std::tr1::shared_ptr<const PackageIDSequence> & replace_candidates(package_ids(m.package_id()->name()));
        for (PackageIDSequence::ConstIterator it(replace_candidates->begin()),
                 it_end(replace_candidates->end()); it_end != it; ++it)
        {
            std::tr1::shared_ptr<const ERepositoryID> candidate(std::tr1::static_pointer_cast<const ERepositoryID>(*it));
            if (candidate != if_overwritten_id && candidate->slot() == m.package_id()->slot())
                perform_uninstall(candidate, false, "");
        }
    }

    VDBPostMergeCommand post_merge_command(
            make_named_values<VDBPostMergeCommandParams>(
                value_for<n::root>(installed_root_key()->value())
            ));

    post_merge_command();
}

void
ExndbamRepository::perform_uninstall(const std::tr1::shared_ptr<const ERepositoryID> & id,
        bool replace, const std::string & merge_config_protect) const
{
    Context context("When uninstalling '" + stringify(*id) + (replace ? "' for a reinstall:" : "':"));

    if (! _imp->params.root().is_directory())
        throw InstallActionError("Couldn't uninstall '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root()) + "') is not a directory");

    FSEntry ver_dir(id->fs_location_key()->value());
    std::tr1::shared_ptr<FSEntry> load_env(new FSEntry(ver_dir / "environment.bz2"));

    std::tr1::shared_ptr<FSEntrySequence> eclassdirs(new FSEntrySequence);
    eclassdirs->push_back(ver_dir);

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_uninstall());
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (can_skip_phase(id, *phase))
        {
            std::cout << "--- No need to do anything for " << phase->equal_option("skipname") << " phase" << std::endl;
            continue;
        }

        if (phase->option("unmerge"))
        {
            /* load CONFIG_PROTECT, CONFIG_PROTECT_MASK from vdb, supplement with env */
            std::string config_protect, config_protect_mask;
            {
                std::ifstream c(stringify(ver_dir / "CONFIG_PROTECT").c_str());
                config_protect = std::string((std::istreambuf_iterator<char>(c)), std::istreambuf_iterator<char>()) +
                    " " + getenv_with_default("CONFIG_PROTECT", "");

                std::ifstream c_m(stringify(ver_dir / "CONFIG_PROTECT_MASK").c_str());
                config_protect_mask = std::string((std::istreambuf_iterator<char>(c_m)), std::istreambuf_iterator<char>()) +
                    " " + getenv_with_default("CONFIG_PROTECT_MASK", "");
            }

            std::string final_config_protect(config_protect + " " + merge_config_protect);

            /* unmerge */
            NDBAMUnmerger unmerger(
                    make_named_values<NDBAMUnmergerOptions>(
                    value_for<n::config_protect>(final_config_protect),
                    value_for<n::config_protect_mask>(config_protect_mask),
                    value_for<n::contents_file>(ver_dir / "contents"),
                    value_for<n::environment>(_imp->params.environment()),
                    value_for<n::ndbam>(&_imp->ndbam),
                    value_for<n::package_id>(id),
                    value_for<n::root>(installed_root_key()->value())
                    ));

            unmerger.unmerge();
        }
        else
        {
            EbuildCommandParams params(
                    make_named_values<EbuildCommandParams>(
                        value_for<n::builddir>(_imp->params.builddir()),
                        value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                        value_for<n::distdir>(ver_dir),
                        value_for<n::ebuild_dir>(ver_dir),
                        value_for<n::ebuild_file>(ver_dir / (stringify(id->name().package()) + "-" + stringify(id->version()) + ".ebuild")),
                        value_for<n::eclassdirs>(eclassdirs),
                        value_for<n::environment>(_imp->params.environment()),
                        value_for<n::exlibsdirs>(make_shared_ptr(new FSEntrySequence)),
                        value_for<n::files_dir>(ver_dir),
                        value_for<n::package_id>(id),
                        value_for<n::portdir>(_imp->params.location()),
                        value_for<n::sandbox>(phase->option("sandbox")),
                        value_for<n::userpriv>(phase->option("userpriv"))
                    ));

            EbuildUninstallCommandParams uninstall_params(
                    make_named_values<EbuildUninstallCommandParams>(
                        value_for<n::load_environment>(load_env.get()),
                        value_for<n::loadsaveenv_dir>(ver_dir),
                        value_for<n::root>(stringify(_imp->params.root())),
                        value_for<n::unmerge_only>(false)
                    ));

            EbuildUninstallCommand uninstall_cmd_pre(params, uninstall_params);
            uninstall_cmd_pre();
        }
    }

    for (DirIterator d(ver_dir, DirIteratorOptions() + dio_include_dotfiles), d_end ; d != d_end ; ++d)
        FSEntry(*d).unlink();
    ver_dir.rmdir();

    FSEntry pkg_dir(ver_dir.dirname());
    if (DirIterator() == DirIterator(pkg_dir, DirIteratorOptions() + dio_include_dotfiles + dio_inode_sort + dio_first_only))
    {
        pkg_dir.rmdir();

        _imp->ndbam.deindex(id->name());
    }
}

void
ExndbamRepository::regenerate_cache() const
{
}

