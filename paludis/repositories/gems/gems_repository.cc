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

#include <paludis/repositories/gems/gems_repository.hh>
#include <paludis/repositories/gems/params.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/repositories/gems/gem_specification.hh>
#include <paludis/repositories/gems/gem_specifications.hh>
#include <paludis/repositories/gems/exceptions.hh>
#include <paludis/repositories/gems/extra_distribution_data.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <unordered_map>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<GemsRepository>
    {
        const gems::RepositoryParams params;

        const std::shared_ptr<Mutex> big_nasty_mutex;

        mutable std::shared_ptr<const CategoryNamePartSet> category_names;
        mutable std::unordered_map<CategoryNamePart, std::shared_ptr<const QualifiedPackageNameSet>, Hash<CategoryNamePart> > package_names;
        mutable std::unordered_map<QualifiedPackageName, std::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> > ids;

        mutable bool has_category_names;
        mutable bool has_ids;

        std::shared_ptr<const MetadataValueKey<FSEntry> > location_key;
        std::shared_ptr<const MetadataValueKey<FSEntry> > install_dir_key;
        std::shared_ptr<const MetadataValueKey<FSEntry> > builddir_key;
        std::shared_ptr<const MetadataValueKey<std::string> > sync_key;
        std::shared_ptr<const MetadataValueKey<std::string> > sync_options_key;
        std::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Implementation(const gems::RepositoryParams p, std::shared_ptr<Mutex> m = std::make_shared<Mutex>()) :
            params(p),
            big_nasty_mutex(m),
            has_category_names(false),
            has_ids(false),
            location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                        mkt_significant, params.location())),
            install_dir_key(new LiteralMetadataValueKey<FSEntry> ("install_dir", "install_dir",
                        mkt_normal, params.install_dir())),
            builddir_key(new LiteralMetadataValueKey<FSEntry> ("builddir", "builddir",
                        mkt_normal, params.builddir())),
            sync_key(new LiteralMetadataValueKey<std::string> ("sync", "sync",
                        mkt_normal, params.sync())),
            sync_options_key(new LiteralMetadataValueKey<std::string> (
                        "sync_options", "sync_options", mkt_normal, params.sync_options())),
            format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                        mkt_significant, "gems"))
        {
        }
    };
}

GemsRepository::GemsRepository(const gems::RepositoryParams & params) :
    Repository(params.environment(), RepositoryName("gems"),
            make_named_values<RepositoryCapabilities>(
                n::destination_interface() = static_cast<RepositoryDestinationInterface *>(0),
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(0),
                n::make_virtuals_interface() = static_cast<RepositoryMakeVirtualsInterface *>(0),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(0),
                n::provides_interface() = static_cast<RepositoryProvidesInterface *>(0),
                n::virtuals_interface() = static_cast<RepositoryVirtualsInterface *>(0)
            )),
    PrivateImplementationPattern<GemsRepository>(params),
    _imp(PrivateImplementationPattern<GemsRepository>::_imp)
{
    _add_metadata_keys();
}

GemsRepository::~GemsRepository()
{
}

void
GemsRepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->install_dir_key);
    add_metadata_key(_imp->builddir_key);
    add_metadata_key(_imp->sync_key);
    add_metadata_key(_imp->sync_options_key);
    add_metadata_key(_imp->format_key);
}

void
GemsRepository::invalidate()
{
    Lock l(*_imp->big_nasty_mutex);

    _imp.reset(new Implementation<GemsRepository>(_imp->params, _imp->big_nasty_mutex));
    _add_metadata_keys();
}

void
GemsRepository::invalidate_masks()
{
    Lock l(*_imp->big_nasty_mutex);

    for (std::unordered_map<QualifiedPackageName, std::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> >::iterator
            it(_imp->ids.begin()), it_end(_imp->ids.end());
            it_end != it; ++it)
        for (PackageIDSequence::ConstIterator it2(it->second->begin()), it2_end(it->second->end());
                it2_end != it2; ++it2)
            (*it2)->invalidate_masks();
}

bool
GemsRepository::has_category_named(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    need_category_names();
    return _imp->category_names->end() != _imp->category_names->find(c);
}

bool
GemsRepository::has_package_named(const QualifiedPackageName & q) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_category_named(q.category()))
        return false;

    need_ids();
    return _imp->package_names.find(q.category())->second->end() != _imp->package_names.find(q.category())->second->find(q);
}

std::shared_ptr<const CategoryNamePartSet>
GemsRepository::category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    need_category_names();
    return _imp->category_names;
}

std::shared_ptr<const QualifiedPackageNameSet>
GemsRepository::package_names(const CategoryNamePart & c) const
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
GemsRepository::package_ids(const QualifiedPackageName & q) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_package_named(q))
        return std::make_shared<PackageIDSequence>();

    need_ids();

    std::unordered_map<QualifiedPackageName, std::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> >::const_iterator i(
            _imp->ids.find(q));
    if (i == _imp->ids.end())
        return std::make_shared<PackageIDSequence>();

    return i->second;
}

void
GemsRepository::need_category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_category_names)
        return;

    std::shared_ptr<CategoryNamePartSet> cat(new CategoryNamePartSet);
    _imp->category_names = cat;

    cat->insert(CategoryNamePart("gems"));
    _imp->has_category_names = true;
}

void
GemsRepository::need_ids() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_ids)
        return;

    need_category_names();

    std::shared_ptr<QualifiedPackageNameSet> pkgs(new QualifiedPackageNameSet);
    _imp->package_names.insert(std::make_pair(CategoryNamePart("gems"), pkgs));

    Context context("When loading gems yaml file:");

    SafeIFStream yaml_file(_imp->params.location() / "yaml");

    std::string output((std::istreambuf_iterator<char>(yaml_file)), std::istreambuf_iterator<char>());
    yaml::Document master_doc(output);
    gems::GemSpecifications specs(_imp->params.environment(), shared_from_this(), *master_doc.top());

    for (gems::GemSpecifications::ConstIterator i(specs.begin()), i_end(specs.end()) ;
            i != i_end ; ++i)
    {
        pkgs->insert(i->first.first);

        std::unordered_map<QualifiedPackageName, std::shared_ptr<PackageIDSequence>, Hash<QualifiedPackageName> >::iterator
            v(_imp->ids.find(i->first.first));
        if (_imp->ids.end() == v)
            v = _imp->ids.insert(std::make_pair(i->first.first, std::make_shared<PackageIDSequence>())).first;

        v->second->push_back(i->second);
    }

    _imp->has_ids = true;
}

#if 0
void
GemsRepository::do_install(const std::shared_ptr<const PackageID> & id, const InstallOptions & o) const
{
    if (o.fetch_only)
        return;

    Command cmd(getenv_with_default("PALUDIS_GEMS_DIR", LIBEXECDIR "/paludis") +
            "/gems/gems.bash install '" + stringify(id->name().package()) + "' '" + stringify(id->version()) + "'");
    cmd.with_stderr_prefix(stringify(*id) + "> ");
    cmd.with_setenv("GEMCACHE", stringify(_imp->params.location / "yaml"));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Install of '" + stringify(*id) + "' failed");
}
#endif

namespace
{
    struct SupportsActionQuery
    {
        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
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

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return false;
        }
    };
}

bool
GemsRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

bool
GemsRepository::some_ids_might_not_be_masked() const
{
    return true;
}

const bool
GemsRepository::is_unimportant() const
{
    return false;
}

void
GemsRepository::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
GemsRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSEntry> >
GemsRepository::location_key() const
{
    return _imp->location_key;
}

const std::shared_ptr<const MetadataValueKey<FSEntry> >
GemsRepository::installed_root_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSEntry> >();
}

std::shared_ptr<Repository>
GemsRepository::repository_factory_create(
        Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    std::string location(f("location"));
    if (location.empty())
        throw gems::RepositoryConfigurationError("Key 'location' not specified or empty");

    std::string install_dir(f("install_dir"));
    if (install_dir.empty())
        throw gems::RepositoryConfigurationError("Key 'install_dir' not specified or empty");

    std::string sync(f("sync"));

    std::string sync_options(f("sync_options"));

    std::string builddir(f("builddir"));
    if (builddir.empty())
        builddir = gems::GemsExtraDistributionData::get_instance()->data_from_distribution(
                *DistributionData::get_instance()->distribution_from_string(env->distribution()))->default_buildroot();

    return std::make_shared<GemsRepository>(make_named_values<gems::RepositoryParams>(
                n::builddir() = builddir,
                n::environment() = env,
                n::install_dir() = install_dir,
                n::location() = location,
                n::sync() = sync,
                n::sync_options() = sync_options
                ));
}

RepositoryName
GemsRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return RepositoryName("gems");
}

std::shared_ptr<const RepositoryNameSet>
GemsRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

void
GemsRepository::populate_sets() const
{
}

HookResult
GemsRepository::perform_hook(const Hook &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

bool
GemsRepository::sync(const std::shared_ptr<OutputManager> &) const
{
    return false;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
GemsRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
GemsRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

