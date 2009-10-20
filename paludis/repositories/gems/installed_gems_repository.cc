/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
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

#include <paludis/repositories/gems/installed_gems_repository.hh>
#include <paludis/repositories/gems/params.hh>
#include <paludis/repositories/gems/gem_specification.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/repositories/gems/exceptions.hh>
#include <paludis/repositories/gems/extra_distribution_data.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/system.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/distribution.hh>
#include <paludis/action.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/hook.hh>
#include <tr1/unordered_map>

using namespace paludis;

typedef std::tr1::unordered_map<QualifiedPackageName, std::tr1::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> > IDMap;

namespace paludis
{
    template <>
    struct Implementation<InstalledGemsRepository>
    {
        const std::tr1::shared_ptr<Mutex> big_nasty_mutex;

        const gems::InstalledRepositoryParams params;

        mutable std::tr1::shared_ptr<const CategoryNamePartSet> category_names;
        mutable std::tr1::unordered_map<CategoryNamePart, std::tr1::shared_ptr<const QualifiedPackageNameSet>, Hash<CategoryNamePart> > package_names;
        mutable IDMap ids;

        mutable bool has_category_names;
        mutable bool has_ids;

        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > install_dir_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > builddir_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > root_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Implementation(const gems::InstalledRepositoryParams p,
                       std::tr1::shared_ptr<Mutex> m = make_shared_ptr(new Mutex)) :
            big_nasty_mutex(m),
            params(p),
            has_category_names(false),
            has_ids(false),
            install_dir_key(new LiteralMetadataValueKey<FSEntry> ("install_dir", "install_dir",
                        mkt_normal, params.install_dir())),
            builddir_key(new LiteralMetadataValueKey<FSEntry> ("builddir", "builddir",
                        mkt_normal, params.builddir())),
            root_key(new LiteralMetadataValueKey<FSEntry> (
                        "root", "root", mkt_normal, params.root())),
            format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                        mkt_significant, "gems"))
        {
        }
    };
}

InstalledGemsRepository::InstalledGemsRepository(const gems::InstalledRepositoryParams & params) :
    Repository(params.environment(),
            RepositoryName("installed-gems"),
            make_named_values<RepositoryCapabilities>(
                value_for<n::destination_interface>(this),
                value_for<n::e_interface>(static_cast<RepositoryEInterface *>(0)),
                value_for<n::environment_variable_interface>(static_cast<RepositoryEnvironmentVariableInterface *>(0)),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(static_cast<RepositoryManifestInterface *>(0)),
                value_for<n::mirrors_interface>(static_cast<RepositoryMirrorsInterface *>(0)),
                value_for<n::provides_interface>(static_cast<RepositoryProvidesInterface *>(0)),
                value_for<n::syncable_interface>(static_cast<RepositorySyncableInterface *>(0)),
                value_for<n::virtuals_interface>(static_cast<RepositoryVirtualsInterface *>(0))
            )),
    PrivateImplementationPattern<InstalledGemsRepository>(new Implementation<InstalledGemsRepository>(params)),
    _imp(PrivateImplementationPattern<InstalledGemsRepository>::_imp)
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
    _imp.reset(new Implementation<InstalledGemsRepository>(_imp->params, _imp->big_nasty_mutex));
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

std::tr1::shared_ptr<const CategoryNamePartSet>
InstalledGemsRepository::category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    need_category_names();
    return _imp->category_names;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
InstalledGemsRepository::package_names(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_category_named(c))
        return make_shared_ptr(new QualifiedPackageNameSet);

    need_ids();

    std::tr1::unordered_map<CategoryNamePart, std::tr1::shared_ptr<const QualifiedPackageNameSet>, Hash<CategoryNamePart> >::const_iterator i(
            _imp->package_names.find(c));
    if (i == _imp->package_names.end())
        return make_shared_ptr(new QualifiedPackageNameSet);
    return i->second;
}

std::tr1::shared_ptr<const PackageIDSequence>
InstalledGemsRepository::package_ids(const QualifiedPackageName & q) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_package_named(q))
        return make_shared_ptr(new PackageIDSequence);

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(q));
    if (i == _imp->ids.end())
        return make_shared_ptr(new PackageIDSequence);

    return i->second;
}

void
InstalledGemsRepository::need_category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_category_names)
        return;

    std::tr1::shared_ptr<CategoryNamePartSet> cat(new CategoryNamePartSet);
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

    std::tr1::shared_ptr<QualifiedPackageNameSet> pkgs(new QualifiedPackageNameSet);
    _imp->package_names.insert(std::make_pair(gems, pkgs));

    for (DirIterator d(_imp->params.install_dir() / "specifications"), d_end ; d != d_end ; ++d)
    {
        if (! is_file_with_extension(*d, ".gemspec", IsFileWithOptions()))
            continue;

        std::string s(strip_trailing_string(d->basename(), ".gemspec"));
        std::string::size_type h(s.rfind('-'));
        if (std::string::npos == h)
        {
            Log::get_instance()->message("gems.id.unrecognised", ll_qa, lc_context) << "Unrecognised file name format '"
                << *d << "' (no hyphen)";
            continue;
        }

        VersionSpec v(s.substr(h + 1), VersionSpecOptions());
        PackageNamePart p(s.substr(0, h));
        pkgs->insert(gems + p);

        if (_imp->ids.end() == _imp->ids.find(gems + p))
            _imp->ids.insert(std::make_pair(gems + p, make_shared_ptr(new PackageIDSequence)));
        _imp->ids.find(gems + p)->second->push_back(make_shared_ptr(new gems::GemSpecification(
                        _imp->params.environment(), shared_from_this(), p, v, *d)));
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
InstalledGemsRepository::do_uninstall(const std::tr1::shared_ptr<const PackageID> & id,
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

void
InstalledGemsRepository::need_keys_added() const
{
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledGemsRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledGemsRepository::location_key() const
{
    return _imp->install_dir_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledGemsRepository::installed_root_key() const
{
    return _imp->root_key;
}

RepositoryName
InstalledGemsRepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return RepositoryName("installed-gems");
}

std::tr1::shared_ptr<const RepositoryNameSet>
InstalledGemsRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

std::tr1::shared_ptr<Repository>
InstalledGemsRepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
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

    return make_shared_ptr(new InstalledGemsRepository(make_named_values<gems::InstalledRepositoryParams>(
                value_for<n::builddir>(builddir),
                value_for<n::environment>(env),
                value_for<n::install_dir>(install_dir),
                value_for<n::root>(root)
                )));
}

namespace
{
    std::tr1::shared_ptr<SetSpecTree> get_everything_set(
            const Environment * const env,
            const Repository * const repo)
    {
        Context context("When making 'everything' set from '" + stringify(repo->name()) + "':");

        std::tr1::shared_ptr<SetSpecTree> result(new SetSpecTree(make_shared_ptr(new AllDepSpec)));

        std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(
                    generator::InRepository(repo->name()))]);
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
            result->root()->append(make_shared_ptr(new PackageDepSpec(
                            make_package_dep_spec(PartiallyMadePackageDepSpecOptions())
                            .package((*i)->name())
                            )));

        return result;
    }
}

void
InstalledGemsRepository::populate_sets() const
{
    _imp->params.environment()->add_set(
            SetName("everything"),
            SetName("everything::" + stringify(name())),
            std::tr1::bind(get_everything_set, _imp->params.environment(), this),
            true);
}

HookResult
InstalledGemsRepository::perform_hook(const Hook &)
{
    return make_named_values<HookResult>(value_for<n::max_exit_status>(0), value_for<n::output>(""));
}


