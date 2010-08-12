/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
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

#include <paludis/repositories/gems/installed_gems_repository.hh>
#include <paludis/repositories/gems/params.hh>
#include <paludis/repositories/gems/gem_specification.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/repositories/gems/exceptions.hh>
#include <paludis/repositories/gems/extra_distribution_data.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/system.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/distribution.hh>
#include <paludis/action.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/hook.hh>
#include <paludis/common_sets.hh>
#include <unordered_map>

using namespace paludis;

typedef std::unordered_map<QualifiedPackageName, std::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> > IDMap;

namespace paludis
{
    template <>
    struct Imp<InstalledGemsRepository>
    {
        const std::shared_ptr<Mutex> big_nasty_mutex;

        const gems::InstalledRepositoryParams params;

        mutable std::shared_ptr<const CategoryNamePartSet> category_names;
        mutable std::unordered_map<CategoryNamePart, std::shared_ptr<const QualifiedPackageNameSet>, Hash<CategoryNamePart> > package_names;
        mutable IDMap ids;

        mutable bool has_category_names;
        mutable bool has_ids;

        std::shared_ptr<const MetadataValueKey<FSEntry> > install_dir_key;
        std::shared_ptr<const MetadataValueKey<FSEntry> > builddir_key;
        std::shared_ptr<const MetadataValueKey<FSEntry> > root_key;
        std::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Imp(const gems::InstalledRepositoryParams p,
                       std::shared_ptr<Mutex> m = std::make_shared<Mutex>()) :
            big_nasty_mutex(m),
            params(p),
            has_category_names(false),
            has_ids(false),
            install_dir_key(std::make_shared<LiteralMetadataValueKey<FSEntry> >("install_dir", "install_dir",
                        mkt_normal, params.install_dir())),
            builddir_key(std::make_shared<LiteralMetadataValueKey<FSEntry> >("builddir", "builddir",
                        mkt_normal, params.builddir())),
            root_key(std::make_shared<LiteralMetadataValueKey<FSEntry> >(
                        "root", "root", mkt_normal, params.root())),
            format_key(std::make_shared<LiteralMetadataValueKey<std::string> >("format", "format",
                        mkt_significant, "gems"))
        {
        }
    };
}

InstalledGemsRepository::InstalledGemsRepository(const gems::InstalledRepositoryParams & params) :
    Repository(params.environment(),
            RepositoryName("installed-gems"),
            make_named_values<RepositoryCapabilities>(
                n::destination_interface() = this,
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(0),
                n::make_virtuals_interface() = static_cast<RepositoryMakeVirtualsInterface *>(0),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(0),
                n::provides_interface() = static_cast<RepositoryProvidesInterface *>(0),
                n::virtuals_interface() = static_cast<RepositoryVirtualsInterface *>(0)
            )),
    Pimp<InstalledGemsRepository>(params),
    _imp(Pimp<InstalledGemsRepository>::_imp)
{
    _add_metadata_keys();
}

InstalledGemsRepository::~InstalledGemsRepository()
{
}

void
InstalledGemsRepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->install_dir_key);
    add_metadata_key(_imp->builddir_key);
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->root_key);
}

void
InstalledGemsRepository::invalidate()
{
    Lock l(*_imp->big_nasty_mutex);
    _imp.reset(new Imp<InstalledGemsRepository>(_imp->params, _imp->big_nasty_mutex));
    _add_metadata_keys();
}

void
InstalledGemsRepository::invalidate_masks()
{
}

bool
InstalledGemsRepository::has_category_named(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    need_category_names();
    return _imp->category_names->end() != _imp->category_names->find(c);
}

bool
InstalledGemsRepository::has_package_named(const QualifiedPackageName & q) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_category_named(q.category()))
        return false;

    need_ids();
    return _imp->package_names.find(q.category())->second->end() != _imp->package_names.find(q.category())->second->find(q);
}

std::shared_ptr<const CategoryNamePartSet>
InstalledGemsRepository::category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    need_category_names();
    return _imp->category_names;
}

std::shared_ptr<const QualifiedPackageNameSet>
InstalledGemsRepository::package_names(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_category_named(c))
        return std::make_shared<QualifiedPackageNameSet>();

    need_ids();

    std::unordered_map<CategoryNamePart, std::shared_ptr<const QualifiedPackageNameSet>, Hash<CategoryNamePart> >::const_iterator i(
            _imp->package_names.find(c));
    if (i == _imp->package_names.end())
        return std::make_shared<QualifiedPackageNameSet>();
    return i->second;
}

std::shared_ptr<const PackageIDSequence>
InstalledGemsRepository::package_ids(const QualifiedPackageName & q) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_package_named(q))
        return std::make_shared<PackageIDSequence>();

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(q));
    if (i == _imp->ids.end())
        return std::make_shared<PackageIDSequence>();

    return i->second;
}

void
InstalledGemsRepository::need_category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_category_names)
        return;

    std::shared_ptr<CategoryNamePartSet> cat(std::make_shared<CategoryNamePartSet>());
    _imp->category_names = cat;

    cat->insert(CategoryNamePart("gems"));
    _imp->has_category_names = true;
}

void
InstalledGemsRepository::need_ids() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_ids)
        return;

    static CategoryNamePart gems("gems");

    Context c("When loading entries for repository '" + stringify(name()) + "':");

    need_category_names();

    std::shared_ptr<QualifiedPackageNameSet> pkgs(std::make_shared<QualifiedPackageNameSet>());
    _imp->package_names.insert(std::make_pair(gems, pkgs));

    for (DirIterator d(_imp->params.install_dir() / "specifications"), d_end ; d != d_end ; ++d)
    {
        if (! is_file_with_extension(*d, ".gemspec", { }))
            continue;

        std::string s(strip_trailing_string(d->basename(), ".gemspec"));
        std::string::size_type h(s.rfind('-'));
        if (std::string::npos == h)
        {
            Log::get_instance()->message("gems.id.unrecognised", ll_qa, lc_context) << "Unrecognised file name format '"
                << *d << "' (no hyphen)";
            continue;
        }

        VersionSpec v(s.substr(h + 1), { });
        PackageNamePart p(s.substr(0, h));
        pkgs->insert(gems + p);

        if (_imp->ids.end() == _imp->ids.find(gems + p))
            _imp->ids.insert(std::make_pair(gems + p, std::make_shared<PackageIDSequence>()));
        _imp->ids.find(gems + p)->second->push_back(std::make_shared<gems::GemSpecification>(
                        _imp->params.environment(), shared_from_this(), p, v, *d));
    }
}

bool
InstalledGemsRepository::is_suitable_destination_for(const PackageID & e) const
{
    Lock l(*_imp->big_nasty_mutex);

    std::string f(e.repository()->format_key() ? e.repository()->format_key()->value() : "");
    return f == "gems";
}

bool
InstalledGemsRepository::is_default_destination() const
{
    return true;
}

bool
InstalledGemsRepository::want_pre_post_phases() const
{
    return true;
}

void
InstalledGemsRepository::merge(const MergeParams &)
{
    throw InternalError(PALUDIS_HERE, "Invalid target for merge");
}

#if 0
void
InstalledGemsRepository::do_uninstall(const std::shared_ptr<const PackageID> & id,
        const UninstallOptions &) const
{
    Command cmd(getenv_with_default("PALUDIS_GEMS_DIR", LIBEXECDIR "/paludis") +
            "/gems/gems.bash uninstall '" + stringify(id->name().package()) + "' '" + stringify(id->version()) + "'");
    cmd.with_stderr_prefix(stringify(*id) + "> ");
    cmd.with_setenv("GEM_HOME", stringify(_imp->params.install_dir));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Uninstall of '" + stringify(*id) + "' failed");
}
#endif

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

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return true;
        }
    };
}

bool
InstalledGemsRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

bool
InstalledGemsRepository::some_ids_might_not_be_masked() const
{
    return true;
}

const bool
InstalledGemsRepository::is_unimportant() const
{
    return false;
}

void
InstalledGemsRepository::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
InstalledGemsRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledGemsRepository::location_key() const
{
    return _imp->install_dir_key;
}

const std::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledGemsRepository::installed_root_key() const
{
    return _imp->root_key;
}

RepositoryName
InstalledGemsRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return RepositoryName("installed-gems");
}

std::shared_ptr<const RepositoryNameSet>
InstalledGemsRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

std::shared_ptr<Repository>
InstalledGemsRepository::repository_factory_create(
        Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    std::string install_dir(f("install_dir"));
    if (install_dir.empty())
        throw gems::RepositoryConfigurationError("Key 'install_dir' not specified or empty");

    std::string builddir(f("builddir"));
    if (builddir.empty())
        builddir = gems::GemsExtraDistributionData::get_instance()->data_from_distribution(
                *DistributionData::get_instance()->distribution_from_string(env->distribution()))->default_buildroot();

    std::string root(f("root"));
    if (root.empty())
        root = "/";

    return std::make_shared<InstalledGemsRepository>(make_named_values<gems::InstalledRepositoryParams>(
                n::builddir() = builddir,
                n::environment() = env,
                n::install_dir() = install_dir,
                n::root() = root
                ));
}

void
InstalledGemsRepository::populate_sets() const
{
    add_common_sets_for_installed_repo(_imp->params.environment(), *this);
}

HookResult
InstalledGemsRepository::perform_hook(const Hook &, const std::shared_ptr<OutputManager> &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

bool
InstalledGemsRepository::sync(const std::shared_ptr<OutputManager> &) const
{
    return false;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
InstalledGemsRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
InstalledGemsRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

