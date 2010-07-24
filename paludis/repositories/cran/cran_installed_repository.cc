/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Danny van Dyk
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/environment.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_tag.hh>
#include <paludis/util/config_file.hh>
#include <paludis/set_file.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/selection.hh>
#include <paludis/hook.hh>
#include <paludis/common_sets.hh>
#include <paludis/repositories/cran/cran_package_id.hh>
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/repositories/cran/cran_installed_repository.hh>
#include <paludis/repositories/cran/package_dep_spec.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/map.hh>
#include <paludis/util/set.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <unordered_map>
#include <functional>
#include <algorithm>

using namespace paludis;

typedef std::unordered_map<
    QualifiedPackageName,
    std::shared_ptr<const cranrepository::CRANPackageID>,
    Hash<QualifiedPackageName> > IDMap;

namespace paludis
{
    template <>
    struct Imp<CRANInstalledRepository>
    {
        CRANInstalledRepositoryParams params;

        mutable bool has_ids;
        mutable IDMap ids;

        Imp(const CRANInstalledRepositoryParams &);
        ~Imp();

        std::shared_ptr<const MetadataValueKey<FSEntry> > location_key;
        std::shared_ptr<const MetadataValueKey<FSEntry> > installed_root_key;
        std::shared_ptr<const MetadataValueKey<std::string> > format_key;
    };
}

Imp<CRANInstalledRepository>::Imp(const CRANInstalledRepositoryParams & p) :
    params(p),
    has_ids(false),
    location_key(std::make_shared<LiteralMetadataValueKey<FSEntry> >("location", "location", mkt_significant, params.location())),
    installed_root_key(std::make_shared<LiteralMetadataValueKey<FSEntry> >("root", "root", mkt_normal, params.root())),
    format_key(std::make_shared<LiteralMetadataValueKey<std::string> >("format", "format", mkt_significant, "installed_cran"))
{
}

Imp<CRANInstalledRepository>::~Imp()
{
}

#if 0
void
Imp<CRANInstalledRepository>::need_ids() const
{
    Context context("When loading CRANInstalledRepository IDs from '" + stringify(location) + "':");

    entries_valid = true;
    try
    {
        for (DirIterator d(location), d_end ; d != d_end ; ++d)
        {
            Context local_context("When parsing directoryy '" + stringify(*d) + "':");
            if (! d->is_directory())
                continue;

            FSEntry f(FSEntry(stringify(*d)) / "DESCRIPTION");
            if (! f.is_regular_file())
                continue;

            std::string n(d->basename());
            CRANDescription::normalise_name(n);

            CRANDescription desc(n, f, true);
            entries.push_back(desc);

            QualifiedPackageName q("cran/" + n);
            metadata.insert(std::make_pair(std::make_pair(q, VersionSpec(desc.version)), desc.metadata));
        }

        if (! FSEntry(location / "paludis" / "bundles").is_directory())
            return;

        // Load Bundle Metadata
        for (DirIterator f(location / "paludis" / "bundles"), f_end ; f != f_end ; ++f)
        {
            Context local_context("When parsing file '" + stringify(*f) + "'.");

            if (! f->is_regular_file())
            {
                continue;
            }

            std::string n(f->basename());
            std::string::size_type pos(n.find_last_of("."));
            if (std::string::npos != pos)
                n.erase(pos);
            CRANDescription::normalise_name(n);

            CRANDescription desc(n, *f, true);
            entries.push_back(desc);

            QualifiedPackageName q("cran/" + n);
            metadata.insert(std::make_pair(std::make_pair(q, VersionSpec(desc.version)), desc.metadata));
        }

    }
    catch (...)
    {
        entries_valid = false;
        throw;
    }
}
#endif

CRANInstalledRepository::CRANInstalledRepository(const CRANInstalledRepositoryParams & p) :
    Repository(
            p.environment(),
            RepositoryName("installed-cran"),
            make_named_values<RepositoryCapabilities>(
                n::destination_interface() = this,
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(0),
                n::make_virtuals_interface() = static_cast<RepositoryMakeVirtualsInterface *>(0),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(0),
                n::provides_interface() = static_cast<RepositoryProvidesInterface *>(0),
                n::virtuals_interface() = static_cast<RepositoryVirtualsInterface *>(0)
                )),
    Pimp<CRANInstalledRepository>(p),
    _imp(Pimp<CRANInstalledRepository>::_imp)
{
    _add_metadata_keys();
}

CRANInstalledRepository::~CRANInstalledRepository()
{
}

void
CRANInstalledRepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->installed_root_key);
    add_metadata_key(_imp->format_key);
}

bool
CRANInstalledRepository::has_category_named(const CategoryNamePart & c) const
{
    return (CategoryNamePart("cran") == c);
}

const bool
CRANInstalledRepository::is_unimportant() const
{
    return false;
}

bool
CRANInstalledRepository::has_package_named(const QualifiedPackageName & q) const
{
    Context context("When checking for package '" + stringify(q) + "' in " + stringify(name()) + ":");

    if (! has_category_named(q.category()))
        return false;

    need_ids();

    return _imp->ids.end() != _imp->ids.find(q);
}

std::shared_ptr<const CategoryNamePartSet>
CRANInstalledRepository::category_names() const
{
    std::shared_ptr<CategoryNamePartSet> result(std::make_shared<CategoryNamePartSet>());
    result->insert(CategoryNamePart("cran"));
    return result;
}

std::shared_ptr<const QualifiedPackageNameSet>
CRANInstalledRepository::package_names(const CategoryNamePart & c) const
{
    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    std::shared_ptr<QualifiedPackageNameSet> result(std::make_shared<QualifiedPackageNameSet>());
    if (! has_category_named(c))
        return result;

    need_ids();

    std::transform(_imp->ids.begin(), _imp->ids.end(), result->inserter(),
            std::mem_fn(&std::pair<const QualifiedPackageName, std::shared_ptr<const cranrepository::CRANPackageID> >::first));

    return result;
}

std::shared_ptr<const PackageIDSequence>
CRANInstalledRepository::package_ids(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    std::shared_ptr<PackageIDSequence> result(std::make_shared<PackageIDSequence>());
    if (! has_package_named(n))
        return result;

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(n));
    if (i != _imp->ids.end())
        result->push_back(i->second);
    return result;
}

#if 0
std::shared_ptr<const Contents>
CRANInstalledRepository::do_contents(const Package ID & id) const
{
    Context context("When fetching contents for " + stringify(id) + ":");

    std::shared_ptr<Contents> result(std::make_shared<Contents>());

    if (! _imp->entries_valid)
        _imp->load_entries();

    if (! has_version(q, v))
        return result;

    std::string pn = stringify(q.package());
    CRANDescription::normalise_name(pn);
    FSEntry f(_imp->location / "paludis" / pn / "CONTENTS");
    if (! f.is_regular_file())
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "CONTENTS lookup failed for request for' " +
                stringify(q) + "-" + stringify(v) + "' in CRANInstalled '" +
                stringify(_imp->location) + "'");
        return result;
    }

    SafeIFStream ff(f);
    if (! ff)
        throw ConfigurationError("Could not read '" + stringify(f) + "'");

    std::string line;
    unsigned line_number(0);
    while (std::getline(ff, line))
    {
        ++line_number;

        std::vector<std::string> tokens;
        WhitespaceTokeniser::tokenise(line, std::back_inserter(tokens));
        if (tokens.empty())
            continue;

        if (tokens.size() < 2)
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "CONTENTS for '" +
                    stringify(q) + "-" + stringify(v) + "' in vdb '" +
                    stringify(_imp->location) + "' has broken line " +
                    stringify(line_number) + ", skipping");
            continue;
        }

        if ("obj" == tokens.at(0))
            result->add(std::shared_ptr<ContentsEntry>(std::make_shared<ContentsFileEntry>(tokens.at(1))));
        else if ("dir" == tokens.at(0))
            result->add(std::shared_ptr<ContentsEntry>(std::make_shared<ContentsDirEntry>(tokens.at(1))));
        else if ("misc" == tokens.at(0))
            result->add(std::shared_ptr<ContentsEntry>(std::make_shared<ContentsMiscEntry>(tokens.at(1))));
        else if ("sym" == tokens.at(0))
        {
            if (tokens.size() < 4)
            {
                Log::get_instance()->message(ll_warning, lc_no_context, "CONTENTS for '" +
                        stringify(q) + "-" + stringify(v) + "' in vdb '" +
                        stringify(_imp->location) + "' has broken sym line " +
                        stringify(line_number) + ", skipping");
                continue;
            }

            result->add(std::shared_ptr<ContentsEntry>(std::make_shared<ContentsSymEntry>(
                            tokens.at(1), tokens.at(3))));
        }
    }

    return result;
}

time_t
CRANInstalledRepository::do_installed_time(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    Context context("When finding installed time for '" + stringify(q) +
            "-" + stringify(v) + "':");

    if (! _imp->entries_valid)
        _imp->load_entries();

    std::vector<CRANDescription>::iterator r(_imp->entries.begin()), r_end(_imp->entries.end());
    for ( ; r != r_end ; ++r)
    {
        if (q == r->name)
            break;
    }

    if (r == r_end)
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));
    else
    {
        if (0 == r->installed_time)
        {
            std::string pn(stringify(q.package()));
            CRANDescription::normalise_name(pn);
            FSEntry f(_imp->location / "paludis" / pn / "CONTENTS");
            try
            {
                r->installed_time = f.mtime();
            }
            catch (const FSError & e)
            {
                Log::get_instance()->message(ll_warning, lc_no_context, "Can't get mtime of '"
                        + stringify(f) + "' due to exception '" + e.message() + "' (" + e.what()
                        + ")");
                r->installed_time = 1;
            }
        }
        return r->installed_time;
    }
}
#endif

std::shared_ptr<Repository>
CRANInstalledRepository::repository_factory_create(
        Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    Context context("When making CRAN installed repository from repo_file '" + f("repo_file") + "':");

    std::string location(f("location"));
    if (location.empty())
        throw CRANInstalledRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string root(f("root"));
    if (root.empty())
        root = stringify(env->root());

    if (! f("world").empty())
        throw CRANInstalledRepositoryConfigurationError("Key 'world' is no longer supported.");

    return std::make_shared<CRANInstalledRepository>(make_named_values<CRANInstalledRepositoryParams>(
                n::environment() = env,
                n::location() = location,
                n::root() = root
                ));
}

RepositoryName
CRANInstalledRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return RepositoryName("installed-cran");
}

std::shared_ptr<const RepositoryNameSet>
CRANInstalledRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

CRANInstalledRepositoryConfigurationError::CRANInstalledRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("CRAN installed repository configuration error: " + msg)
{
}

#if 0
void
CRANInstalledRepository::do_uninstall(const QualifiedPackageName & q, const VersionSpec & v,
        const UninstallOptions &) const
{
    Context context("When uninstalling '" + stringify(q) + "-" + stringify(v) +
            "' from '" + stringify(name()) + "':");

    if (! _imp->location.is_directory())
        throw PackageInstallActionError("Couldn't uninstall '" + stringify(q) + "-" +
                stringify(v) + "' because its location ('" + stringify(_imp->location) + "') is not a directory");

    std::shared_ptr<const VersionMetadata> vm(do_version_metadata(q, v));
    std::shared_ptr<const FSEntryCollection> bashrc_files(_imp->env->bashrc_files());

    Command cmd(Command(LIBEXECDIR "/paludis/cran.bash unmerge")
            .with_setenv("PN", vm->cran_interface->package())
            .with_setenv("PV", stringify(v))
            .with_setenv("PALUDIS_CRAN_LIBRARY", stringify(_imp->location))
            .with_setenv("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"))
            .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " ")));

    if (0 != run_command(cmd))
        throw PackageUninstallActionError("Couldn't unmerge '" + stringify(q) + "-" + stringify(v) + "'");
}
#endif

void
CRANInstalledRepository::invalidate()
{
    _imp.reset(new Imp<CRANInstalledRepository>(_imp->params));
    _add_metadata_keys();
}

void
CRANInstalledRepository::invalidate_masks()
{
}

bool
CRANInstalledRepository::is_suitable_destination_for(const PackageID & e) const
{
    std::string f(e.repository()->format_key() ? e.repository()->format_key()->value() : "");
    return f == "cran";
}
bool
CRANInstalledRepository::is_default_destination() const
{
    return _imp->params.environment()->root() == installed_root_key()->value();
}

bool
CRANInstalledRepository::want_pre_post_phases() const
{
    return true;
}

void
CRANInstalledRepository::merge(const MergeParams & m)
{
    Context context("When merging '" + stringify(*m.package_id()) + "' at '" + stringify(m.image_dir())
            + "' to repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(*m.package_id()))
        throw ActionFailedError("Not a suitable destination for '" + stringify(*m.package_id()) + "'");

}

namespace
{
    struct SupportsActionQuery
    {
        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return false;
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
            return false;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return false;
        }
    };
}

bool
CRANInstalledRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

bool
CRANInstalledRepository::some_ids_might_not_be_masked() const
{
    return true;
}

void
CRANInstalledRepository::need_ids() const
{
}

void
CRANInstalledRepository::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
CRANInstalledRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSEntry> >
CRANInstalledRepository::location_key() const
{
    return _imp->location_key;
}

const std::shared_ptr<const MetadataValueKey<FSEntry> >
CRANInstalledRepository::installed_root_key() const
{
    return _imp->installed_root_key;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
CRANInstalledRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

void
CRANInstalledRepository::populate_sets() const
{
    add_common_sets_for_installed_repo(_imp->params.environment(), *this);
}

HookResult
CRANInstalledRepository::perform_hook(const Hook &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

bool
CRANInstalledRepository::sync(const std::shared_ptr<OutputManager> &) const
{
    return false;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
CRANInstalledRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

