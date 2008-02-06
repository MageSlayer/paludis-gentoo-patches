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
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/cookie.hh>
#include <paludis/util/set.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/kc.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/system.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/ndbam.hh>
#include <paludis/ndbam_merger.hh>
#include <paludis/ndbam_unmerger.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>
#include <fstream>

using namespace paludis;
using namespace paludis::erepository;

#include <paludis/repositories/e/exndbam_repository-sr.cc>

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

        tr1::shared_ptr<const MetadataFSEntryKey> location_key;
        tr1::shared_ptr<const MetadataFSEntryKey> root_key;
        tr1::shared_ptr<const MetadataStringKey> format_key;
        tr1::shared_ptr<const MetadataFSEntryKey> world_key;
        tr1::shared_ptr<const MetadataFSEntryKey> builddir_key;

        Implementation(const ExndbamRepositoryParams & p) :
            params(p),
            ndbam(params.location, &supported_exndbam, "exndbam-1"),
            location_key(new LiteralMetadataFSEntryKey("location", "location",
                        mkt_significant, params.location)),
            root_key(new LiteralMetadataFSEntryKey("root", "root",
                        mkt_normal, params.root)),
            format_key(new LiteralMetadataStringKey("format", "format",
                        mkt_significant, "vdb")),
            world_key(new LiteralMetadataFSEntryKey("world", "world",
                        mkt_normal, params.world)),
            builddir_key(new LiteralMetadataFSEntryKey("builddir", "builddir",
                        mkt_normal, params.builddir))
        {
        }
    };
}

ExndbamRepository::ExndbamRepository(const RepositoryName & n, const ExndbamRepositoryParams & p) :
    EInstalledRepository(
            EInstalledRepositoryParams::create()
            .environment(p.environment)
            .world(p.world)
            .builddir(p.builddir)
            .root(p.root),
            n, RepositoryCapabilities::create()
            .sets_interface(this)
            .syncable_interface(0)
            .use_interface(this)
            .world_interface(this)
            .environment_variable_interface(this)
            .mirrors_interface(0)
            .provides_interface(0)
            .virtuals_interface(0)
            .destination_interface(this)
            .e_interface(0)
            .make_virtuals_interface(0)
            .qa_interface(0)
            .hook_interface(this)
            .manifest_interface(0)),
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
    add_metadata_key(_imp->world_key);
    add_metadata_key(_imp->builddir_key);
}


tr1::shared_ptr<Repository>
ExndbamRepository::make_exndbam_repository(
        Environment * const env,
        tr1::shared_ptr<const Map<std::string, std::string> > m)
{
    std::string repo_file(m->end() == m->find("repo_file") ? std::string("?") : m->find("repo_file")->second);
    Context context("When making Exndbam repository from repo_file '" + repo_file + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw ExndbamRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string root;
    if (m->end() == m->find("root") || ((root = m->find("root")->second)).empty())
        root = "/";

    std::string world;
    if (m->end() == m->find("world") || ((world = m->find("world")->second)).empty())
        world = location + "/world";

    std::string builddir;
    if (m->end() == m->find("builddir") || ((builddir = m->find("builddir")->second)).empty())
    {
        if (m->end() == m->find("buildroot") || ((builddir = m->find("buildroot")->second)).empty())
            builddir = (*DistributionData::get_instance()->distribution_from_string(
                    env->default_distribution()))[k::default_ebuild_builddir()];
        else
            Log::get_instance()->message(ll_warning, lc_context) << "Key 'buildroot' is deprecated, use 'builddir' instead";
    }

    std::string name;
    if (m->end() == m->find("name") || ((name = m->find("name")->second)).empty())
        name = "installed";

    return tr1::shared_ptr<Repository>(new ExndbamRepository(
                RepositoryName(name),
                ExndbamRepositoryParams::create()
                .environment(env)
                .location(location)
                .root(root)
                .world(world)
                .builddir(builddir)));
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

tr1::shared_ptr<const PackageIDSequence>
ExndbamRepository::package_ids(const QualifiedPackageName & q) const
{
    tr1::shared_ptr<NDBAMEntrySequence> entries(_imp->ndbam.entries(q));
    tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

    for (IndirectIterator<NDBAMEntrySequence::ConstIterator> e(entries->begin()), e_end(entries->end()) ;
            e != e_end ; ++e)
    {
        Lock l(*e->mutex);
        if (! e->package_id)
            e->package_id.reset(new ExndbamID(e->name, e->version, _imp->params.environment,
                        shared_from_this(), e->fs_location, &_imp->ndbam));
        result->push_back(e->package_id);
    }

    return result;
}

tr1::shared_ptr<const QualifiedPackageNameSet>
ExndbamRepository::package_names(const CategoryNamePart & c) const
{
    return _imp->ndbam.package_names(c);
}

tr1::shared_ptr<const CategoryNamePartSet>
ExndbamRepository::category_names() const
{
    return _imp->ndbam.category_names();
}

tr1::shared_ptr<const CategoryNamePartSet>
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

const tr1::shared_ptr<const MetadataStringKey>
ExndbamRepository::format_key() const
{
    return _imp->format_key;
}

const tr1::shared_ptr<const MetadataFSEntryKey>
ExndbamRepository::installed_root_key() const
{
    return _imp->root_key;
}

void
ExndbamRepository::need_keys_added() const
{
}

void
ExndbamRepository::merge(const MergeOptions & m)
{
    Context context("When merging '" + stringify(*m.package_id) + "' at '" + stringify(m.image_dir)
            + "' to Exndbam repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(*m.package_id))
        throw InstallActionError("Not a suitable destination for '" + stringify(*m.package_id) + "'");

    tr1::shared_ptr<const PackageID> if_overwritten_id, if_same_name_id;
    {
        tr1::shared_ptr<const PackageIDSequence> ids(package_ids(m.package_id->name()));
        for (PackageIDSequence::ConstIterator v(ids->begin()), v_end(ids->end()) ;
                v != v_end ; ++v)
        {
            if_same_name_id = *v;
            if ((*v)->version() == m.package_id->version() && (*v)->slot() == m.package_id->slot())
            {
                if_overwritten_id = *v;
                break;
            }
        }
    }

    FSEntry uid_dir(_imp->params.location);
    if (if_same_name_id)
        uid_dir = if_same_name_id->fs_location_key()->value().dirname();
    else
    {
        std::string uid(stringify(m.package_id->name().category) + "---" + stringify(m.package_id->name().package));
        uid_dir /= "data";
        uid_dir.mkdir();
        uid_dir /= uid;
        uid_dir.mkdir();
    }

    FSEntry target_ver_dir(uid_dir);
    target_ver_dir /= (stringify(m.package_id->version()) + ":" + stringify(m.package_id->slot()) + ":" + cookie());

    if (target_ver_dir.exists())
        throw InstallActionError("Temporary merge directory '" + stringify(target_ver_dir) + "' already exists, probably "
                "due to a previous failed install. If it is safe to do so, please remove this directory and try again.");
    target_ver_dir.mkdir();

    WriteVDBEntryCommand write_vdb_entry_command(
            WriteVDBEntryParams::create()
            .environment(_imp->params.environment)
            .package_id(tr1::static_pointer_cast<const ERepositoryID>(m.package_id))
            .output_directory(target_ver_dir)
            .environment_file(m.environment_file));

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
            NDBAMMergerOptions::create()
            .environment(_imp->params.environment)
            .image(m.image_dir)
            .root(installed_root_key()->value())
            .contents_file(target_ver_dir / "contents")
            .config_protect(config_protect)
            .config_protect_mask(config_protect_mask)
            .package_id(m.package_id));

    if (! merger.check())
    {
        for (DirIterator d(target_ver_dir, DirIteratorOptions() + dio_include_dotfiles), d_end
                ; d != d_end ; ++d)
            FSEntry(*d).unlink();
        target_ver_dir.rmdir();
        throw InstallActionError("Not proceeding with install due to merge sanity check failing");
    }

    merger.merge();

    _imp->ndbam.index(m.package_id->name(), uid_dir.basename());

    if (if_overwritten_id)
    {
        UninstallActionOptions uninstall_options(false);
        perform_uninstall(tr1::static_pointer_cast<const ERepositoryID>(if_overwritten_id), uninstall_options, true);
    }

    VDBPostMergeCommand post_merge_command(
            VDBPostMergeCommandParams::create()
            .root(installed_root_key()->value()));

    post_merge_command();
}

void
ExndbamRepository::perform_uninstall(const tr1::shared_ptr<const ERepositoryID> & id,
        const UninstallActionOptions & o, bool replace) const
{
    Context context("When uninstalling '" + stringify(*id) + (replace ? "' for a reinstall:" : "':"));

    bool last(! replace);
    if (last)
    {
        tr1::shared_ptr<const PackageIDSequence> ids(package_ids(id->name()));
        for (PackageIDSequence::ConstIterator v(ids->begin()), v_end(ids->end()) ;
                v != v_end ; ++v)
            if (**v != *id)
            {
                last = false;
                break;
            }
    }

    if (! _imp->params.root.is_directory())
        throw InstallActionError("Couldn't uninstall '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root) + "') is not a directory");

    FSEntry ver_dir(id->fs_location_key()->value());
    tr1::shared_ptr<FSEntry> load_env(new FSEntry(ver_dir / "environment.bz2"));

    tr1::shared_ptr<FSEntrySequence> eclassdirs(new FSEntrySequence);
    eclassdirs->push_back(ver_dir);

    EAPIPhases phases(id->eapi()->supported->ebuild_phases->ebuild_uninstall);
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
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

            /* unmerge */
            NDBAMUnmerger unmerger(
                    NDBAMUnmergerOptions::create()
                    .environment(_imp->params.environment)
                    .root(installed_root_key()->value())
                    .contents_file(ver_dir / "contents")
                    .config_protect(config_protect)
                    .config_protect_mask(config_protect_mask)
                    .ndbam(&_imp->ndbam)
                    .package_id(id));

            unmerger.unmerge();
        }
        else
        {
            EbuildCommandParams params(EbuildCommandParams::create()
                    .environment(_imp->params.environment)
                    .package_id(id)
                    .ebuild_dir(ver_dir)
                    .ebuild_file(ver_dir / (stringify(id->name().package) + "-" + stringify(id->version()) + ".ebuild"))
                    .files_dir(ver_dir)
                    .eclassdirs(eclassdirs)
                    .exlibsdirs(make_shared_ptr(new FSEntrySequence))
                    .portdir(_imp->params.location)
                    .distdir(ver_dir)
                    .sandbox(phase->option("sandbox"))
                    .userpriv(phase->option("userpriv"))
                    .commands(join(phase->begin_commands(), phase->end_commands(), " "))
                    .builddir(_imp->params.builddir));

            EbuildUninstallCommandParams uninstall_params(EbuildUninstallCommandParams::create()
                    .root(stringify(_imp->params.root) + "/")
                    .disable_cfgpro(o[k::no_config_protect()])
                    .unmerge_only(false)
                    .loadsaveenv_dir(ver_dir)
                    .load_environment(load_env.get()));

            EbuildUninstallCommand uninstall_cmd_pre(params, uninstall_params);
            uninstall_cmd_pre();
        }
    }

    for (DirIterator d(ver_dir, DirIteratorOptions() + dio_include_dotfiles), d_end ; d != d_end ; ++d)
        FSEntry(*d).unlink();
    ver_dir.rmdir();

    if (last)
    {
        FSEntry pkg_dir(ver_dir.dirname());
        pkg_dir.rmdir();

        _imp->ndbam.deindex(id->name());
    }
}

void
ExndbamRepository::regenerate_cache() const
{
}

