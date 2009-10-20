/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Danny van Dyk
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/util/config_file.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/repositories/cran/cran_package_id.hh>
#include <paludis/repositories/cran/cran_repository.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/output_manager.hh>
#include <paludis/syncer.hh>
#include <paludis/hook.hh>
#include <tr1/unordered_map>
#include <tr1/functional>
#include <functional>
#include <algorithm>

using namespace paludis;

typedef std::tr1::unordered_map<
        QualifiedPackageName,
        std::tr1::shared_ptr<const cranrepository::CRANPackageID>,
        Hash<QualifiedPackageName> >
    IDMap;

namespace paludis
{
    template <>
    struct Implementation<CRANRepository>
    {
        CRANRepositoryParams params;

        mutable std::tr1::shared_ptr<Mutex> big_nasty_mutex;
        mutable bool has_ids;
        mutable IDMap ids;

        Implementation(const CRANRepositoryParams &, const std::tr1::shared_ptr<Mutex> &);
        ~Implementation();

        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > distdir_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > builddir_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > library_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > sync_key;
    };
}

Implementation<CRANRepository>::Implementation(const CRANRepositoryParams & p, const std::tr1::shared_ptr<Mutex> & m) :
    params(p),
    big_nasty_mutex(m),
    has_ids(false),
    location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location", mkt_significant, params.location())),
    distdir_key(new LiteralMetadataValueKey<FSEntry> ("distdir", "distdir", mkt_normal, params.distdir())),
    format_key(new LiteralMetadataValueKey<std::string> ("format", "format", mkt_significant, "cran")),
    builddir_key(new LiteralMetadataValueKey<FSEntry> ("builddir", "builddir", mkt_normal, params.builddir())),
    library_key(new LiteralMetadataValueKey<FSEntry> ("library", "library", mkt_normal, params.library())),
    sync_key(new LiteralMetadataValueKey<std::string> ("sync", "sync", mkt_normal, params.sync()))
{
}

Implementation<CRANRepository>::~Implementation()
{
}


CRANRepository::CRANRepository(const CRANRepositoryParams & p) :
    Repository(
            p.environment(),
            CRANRepository::fetch_repo_name(stringify(p.location())),
            make_named_values<RepositoryCapabilities>(
                value_for<n::destination_interface>(static_cast<RepositoryDestinationInterface *>(0)),
                value_for<n::e_interface>(static_cast<RepositoryEInterface *>(0)),
                value_for<n::environment_variable_interface>(static_cast<RepositoryEnvironmentVariableInterface *>(0)),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(static_cast<RepositoryManifestInterface *>(0)),
                value_for<n::mirrors_interface>(static_cast<RepositoryMirrorsInterface *>(0)),
                value_for<n::provides_interface>(static_cast<RepositoryProvidesInterface *>(0)),
                value_for<n::syncable_interface>(this),
                value_for<n::virtuals_interface>(static_cast<RepositoryVirtualsInterface *>(0))
                )),
    PrivateImplementationPattern<CRANRepository>(new Implementation<CRANRepository>(p, make_shared_ptr(new Mutex))),
    _imp(PrivateImplementationPattern<CRANRepository>::_imp)
{
    _add_metadata_keys();
}

CRANRepository::~CRANRepository()
{
}

void
CRANRepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->distdir_key);
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->builddir_key);
    add_metadata_key(_imp->library_key);
    add_metadata_key(_imp->sync_key);
}

bool
CRANRepository::has_category_named(const CategoryNamePart & c) const
{
    return "cran" == stringify(c);
}

bool
CRANRepository::has_package_named(const QualifiedPackageName & q) const
{
    Context context("When checking for package '" + stringify(q) + "' in " + stringify(name()) + ":");
    Lock l(*_imp->big_nasty_mutex);

    if (! has_category_named(q.category()))
        return false;

    need_ids();
    return _imp->ids.end() != _imp->ids.find(q);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
CRANRepository::category_names() const
{
    Context context("When fetching category names in " + stringify(name()) + ":");
    Lock l(*_imp->big_nasty_mutex);

    std::tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
    result->insert(CategoryNamePart("cran"));

    return result;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
CRANRepository::package_names(const CategoryNamePart & c) const
{
    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");
    Lock l(*_imp->big_nasty_mutex);

    std::tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
    if (! has_category_named(c))
        return result;

    need_ids();

    std::transform(_imp->ids.begin(), _imp->ids.end(), result->inserter(),
            std::tr1::mem_fn(&std::pair<const QualifiedPackageName, std::tr1::shared_ptr<const cranrepository::CRANPackageID> >::first));

    return result;
}

std::tr1::shared_ptr<const PackageIDSequence>
CRANRepository::package_ids(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");
    Lock l(*_imp->big_nasty_mutex);

    std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);
    if (! has_package_named(n))
        return result;

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(n));
    if (i != _imp->ids.end())
        result->push_back(i->second);
    return result;
}

void
CRANRepository::need_ids() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_ids)
        return;

    Context context("When loading IDs for " + stringify(name()) + ":");

    for (DirIterator d(_imp->params.location()), d_end ; d != d_end ; ++d)
        if (is_file_with_extension(*d, ".DESCRIPTION", IsFileWithOptions()))
        {
            std::tr1::shared_ptr<cranrepository::CRANPackageID> id(new cranrepository::CRANPackageID(_imp->params.environment(),
                        shared_from_this(), *d));
            if (! _imp->ids.insert(std::make_pair(id->name(), id)).second)
                Log::get_instance()->message("cran.id.duplicate", ll_warning, lc_context)
                    << "Couldn't insert package '" << *id << "' due to name collision";

            if (id->contains_key())
                for (PackageIDSequence::ConstIterator i(id->contains_key()->value()->begin()),
                        i_end(id->contains_key()->value()->end()) ; i != i_end ; ++i)
                    if (! _imp->ids.insert(std::make_pair((*i)->name(),
                                    std::tr1::static_pointer_cast<const cranrepository::CRANPackageID>(*i))).second)
                        Log::get_instance()->message("cran.id.duplicate", ll_warning, lc_context) << "Couldn't insert package '" << **i
                            << "', which is contained in '" << *id << "', due to name collision";
        }

    _imp->has_ids = true;
}

RepositoryName
CRANRepository::fetch_repo_name(const std::string & location)
{
    std::string modified_location(FSEntry(location).basename());
    std::replace(modified_location.begin(), modified_location.end(), '/', '-');
    if (modified_location == "cran")
        return RepositoryName("cran");
    else
        return RepositoryName("cran-" + modified_location);
}

#if 0
void
CRANRepository::do_install(const std::tr1::shared_ptr<const PackageID> & id_uncasted, const InstallOptions & o) const
{
    if (id_uncasted->repository().get() != this)
        throw PackageInstallActionError("Couldn't install '" + stringify(*id_uncasted) + "' using repository '" +
                stringify(name()) + "'");

    const std::tr1::shared_ptr<const CRANPackageID> id(std::tr1::static_pointer_cast<const CRANPackageID>(id_uncasted));
    if (id->bundle_member_key())
        return;

    std::tr1::shared_ptr<const FSEntrySequence> bashrc_files(_imp->params.environment->bashrc_files());

    Command cmd(Command(LIBEXECDIR "/paludis/cran.bash fetch")
            .with_setenv("CATEGORY", "cran")
            .with_setenv("DISTDIR", stringify(_imp->params.distdir))
            .with_setenv("DISTFILE", id->native_package() + "_" + id->native_version() + ".tar.gz")
            .with_setenv("PN", id->native_package())
            .with_setenv("PV", id->native_version())
            .with_setenv("PALUDIS_CRAN_MIRRORS", _imp->params.mirror)
            .with_setenv("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"))
            .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
            .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " ")));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't fetch sources for '" + stringify(*id) + "'");

    if (o.fetch_only)
        return;

    FSEntry image(_imp->params.buildroot / stringify(id->native_package()) / "image");
    FSEntry workdir(_imp->params.buildroot / stringify(id->native_package()) / "work");

    if (! o.destination)
        throw PackageInstallActionError("Can't merge '" + stringify(*id) + "' because no destination was provided.");

    cmd = Command(LIBEXECDIR "/paludis/cran.bash clean install")
        .with_sandbox()
        .with_setenv("CATEGORY", "cran")
        .with_setenv("DISTDIR", stringify(_imp->params.distdir))
        .with_setenv("DISTFILE", id->native_package() + "_" + id->native_version() + ".tar.gz")
        .with_setenv("IMAGE", stringify(image))
        .with_setenv("IS_BUNDLE", (id->bundle_key() ? "yes" : ""))
        .with_setenv("LOCATION", stringify(_imp->params.location))
        .with_setenv("PN", id->native_package())
        .with_setenv("PV", id->native_version())
        .with_setenv("PALUDIS_CRAN_LIBRARY", stringify(_imp->params.library))
        .with_setenv("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"))
        .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
        .with_setenv("ROOT", stringify(o.destination->installed_interface->root()))
        .with_setenv("WORKDIR", stringify(workdir));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't install '" + stringify(*id) + "' to '" +
                stringify(image) + "'");

    MergeOptions m(id, image, FSEntry("/dev/null"));

    if (! o.destination->destination_interface)
        throw PackageInstallActionError("Couldn't install '" + stringify(*id) + "' to '" +
                stringify(o.destination->name()) + "' because it does not provide destination_interface");

    if (! o.destination->installed_interface)
        throw PackageInstallActionError("Couldn't install '" + stringify(*id) + "' to '" +
                stringify(o.destination->name()) + "' because it does not provide installed_interface");

    o.destination->destination_interface->merge(m);

    cmd = Command(LIBEXECDIR "/paludis/cran.bash clean")
        .with_setenv("IMAGE", stringify(image))
        .with_setenv("PN", id->native_package())
        .with_setenv("PV", id->native_version())
        .with_setenv("PALUDIS_CRAN_LIBRARY", stringify(_imp->params.library))
        .with_setenv("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"))
        .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
        .with_setenv("ROOT", stringify(o.destination->installed_interface->root()))
        .with_setenv("WORKDIR", stringify(workdir))
        .with_setenv("REPOSITORY", stringify(name()));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't clean '" + stringify(*id) + "'");

    return;
}
#endif

bool
CRANRepository::sync(const std::tr1::shared_ptr<OutputManager> & output_manager) const
{
    Context context("When syncing repository '" + stringify(name()) + "':");
    Lock l(*_imp->big_nasty_mutex);

    std::string cmd("rsync --delete --recursive --progress --exclude \"*.html\" --exclude \"*.INDEX\" '" +
                    _imp->params.sync() + "/src/contrib/Descriptions/' ./");

    Command command1(Command(cmd).with_chdir(_imp->params.location()));

    command1
        .with_captured_stdout_stream(&output_manager->stdout_stream())
        .with_captured_stderr_stream(&output_manager->stderr_stream())
        .with_ptys()
        ;

    if (0 != run_command(command1))
        throw SyncFailedError(stringify(_imp->params.location()), _imp->params.sync());

    cmd = "rsync --progress '" + _imp->params.sync() + "/src/contrib/PACKAGES' ./";

    Command command2(Command(cmd).with_chdir(_imp->params.location()));

    command2
        .with_captured_stdout_stream(&output_manager->stdout_stream())
        .with_captured_stderr_stream(&output_manager->stderr_stream())
        .with_ptys()
        ;

    if (0 != run_command(command2))
        throw SyncFailedError(stringify(_imp->params.location()), _imp->params.sync());

    cmd = "rsync --progress '" + _imp->params.sync() + "/CRAN_mirrors.csv' ./";

    Command command3(Command(cmd).with_chdir(_imp->params.location()));

    command3
        .with_captured_stdout_stream(&output_manager->stdout_stream())
        .with_captured_stderr_stream(&output_manager->stderr_stream())
        .with_ptys()
        ;

    if (0 != run_command(command3))
        throw SyncFailedError(stringify(_imp->params.location()), _imp->params.sync());

    return true;
}

std::tr1::shared_ptr<Repository>
CRANRepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    Context context("When making CRAN repository from repo_file '" + f("repo_file") + "':");

    std::string location(f("location"));
    if (location.empty())
        throw CRANRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string library(f("library"));
    if (library.empty())
        throw CRANRepositoryConfigurationError("Key 'library' not specified or empty");

    std::string distdir(f("distdir"));
    if (distdir.empty())
        distdir = location + "/distfiles";

    std::string mirror(f("mirror"));
    if (mirror.empty())
        mirror = "http://cran.r-project.org/";

    std::string sync(f("sync"));
    if (sync.empty())
        sync = "rsync://cran.r-project.org/CRAN";

    std::string builddir(f("builddir"));
    if (builddir.empty())
        builddir = "/var/tmp/paludis";

    return std::tr1::shared_ptr<Repository>(new CRANRepository(make_named_values<CRANRepositoryParams>(
                    value_for<n::builddir>(builddir),
                    value_for<n::distdir>(distdir),
                    value_for<n::environment>(env),
                    value_for<n::library>(library),
                    value_for<n::location>(location),
                    value_for<n::mirror>(mirror),
                    value_for<n::sync>(sync)
                )));
}

RepositoryName
CRANRepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    if (f("location").empty())
        throw CRANRepositoryConfigurationError("Key 'location' not specified or empty");
    return fetch_repo_name(f("location"));
}

std::tr1::shared_ptr<const RepositoryNameSet>
CRANRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

CRANRepositoryConfigurationError::CRANRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("CRAN repository configuration error: " + msg)
{
}


void
CRANRepository::invalidate()
{
    _imp.reset(new Implementation<CRANRepository>(_imp->params, _imp->big_nasty_mutex));
    _add_metadata_keys();
}

void
CRANRepository::invalidate_masks()
{
}

namespace
{
    struct SupportsActionQuery
    {
        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<ConfigAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return false;
        }
    };
}

bool
CRANRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

void
CRANRepository::need_keys_added() const
{
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
CRANRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
CRANRepository::location_key() const
{
    return _imp->location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
CRANRepository::installed_root_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

void
CRANRepository::populate_sets() const
{
}

HookResult
CRANRepository::perform_hook(const Hook &)
{
    return make_named_values<HookResult>(value_for<n::max_exit_status>(0), value_for<n::output>(""));
}

