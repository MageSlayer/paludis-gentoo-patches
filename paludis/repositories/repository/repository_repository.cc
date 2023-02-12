/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/repositories/repository/repository_repository.hh>
#include <paludis/repositories/repository/repository_repository_store.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/active_object_ptr.hh>
#include <paludis/util/deferred_construction_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/simple_parser.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/syncer.hh>
#include <paludis/hook.hh>
#include <paludis/package_id.hh>
#include <paludis/output_manager.hh>
#include <paludis/environment.hh>
#include <list>

using namespace paludis;
using namespace paludis::repository_repository;

namespace
{
    std::shared_ptr<RepositoryRepositoryStore>
    make_store(const RepositoryRepository * const repo, const RepositoryRepositoryParams & p)
    {
        return std::make_shared<RepositoryRepositoryStore>(p.environment(), repo);
    }
}

namespace paludis
{
    template <>
    struct Imp<RepositoryRepository>
    {
        const RepositoryRepositoryParams params;

        const std::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        const std::shared_ptr<LiteralMetadataValueKey<std::string> > config_filename_key;
        const std::shared_ptr<LiteralMetadataValueKey<std::string> > config_template_key;
        const std::shared_ptr<LiteralMetadataValueKey<FSPath> > installed_root_key;

        const ActiveObjectPtr<DeferredConstructionPtr<
            std::shared_ptr<RepositoryRepositoryStore> > > store;

        Imp(const RepositoryRepository * const repo, const RepositoryRepositoryParams & p) :
            params(p),
            format_key(std::make_shared<LiteralMetadataValueKey<std::string> >("format", "format",
                        mkt_significant, "repository")),
            config_filename_key(std::make_shared<LiteralMetadataValueKey<std::string> >(
                        "config_filename", "config_filename", mkt_normal, params.config_filename())),
            config_template_key(std::make_shared<LiteralMetadataValueKey<std::string> >(
                        "config_template", "config_template", mkt_normal, params.config_template())),
            installed_root_key(std::make_shared<LiteralMetadataValueKey<FSPath>>("root", "root", mkt_normal, p.root())),
            store(DeferredConstructionPtr<std::shared_ptr<RepositoryRepositoryStore> > (
                        std::bind(&make_store, repo, std::cref(params))))
        {
        }
    };
}

RepositoryRepositoryConfigurationError::RepositoryRepositoryConfigurationError(const std::string & s) noexcept :
    ConfigurationError("RepositoryRepository configuration error: " + s)
{
}

RepositoryRepository::RepositoryRepository(const RepositoryRepositoryParams & p) :
    Repository(
            p.environment(),
            p.name(),
            make_named_values<RepositoryCapabilities>(
                n::destination_interface() = static_cast<RepositoryDestinationInterface *>(this),
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(nullptr),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(nullptr)
                )),
    _imp(this, p)
{
    _add_metadata_keys();
}

RepositoryRepository::~RepositoryRepository() = default;

const bool
RepositoryRepository::is_unimportant() const
{
    return true;
}

void
RepositoryRepository::_add_metadata_keys()
{
    clear_metadata_keys();
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->config_filename_key);
    add_metadata_key(_imp->config_template_key);
    add_metadata_key(_imp->installed_root_key);
}

void
RepositoryRepository::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<std::string>>
RepositoryRepository::cross_compile_host_key() const
{
    return nullptr;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
RepositoryRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
RepositoryRepository::location_key() const
{
    return nullptr;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
RepositoryRepository::installed_root_key() const
{
    return _imp->installed_root_key;
}

void
RepositoryRepository::invalidate()
{
    _imp.reset(new Imp<RepositoryRepository>(this, _imp->params));
    _add_metadata_keys();
}

bool
RepositoryRepository::has_category_named(const CategoryNamePart & c, const RepositoryContentMayExcludes &) const
{
    return _imp->store->has_category_named(c);
}

bool
RepositoryRepository::has_package_named(const QualifiedPackageName & q, const RepositoryContentMayExcludes &) const
{
    return _imp->store->has_package_named(q);
}

std::shared_ptr<const CategoryNamePartSet>
RepositoryRepository::category_names(const RepositoryContentMayExcludes &) const
{
    return _imp->store->category_names();
}

std::shared_ptr<const CategoryNamePartSet>
RepositoryRepository::unimportant_category_names(const RepositoryContentMayExcludes &) const
{
    return _imp->store->unimportant_category_names();
}

std::shared_ptr<const CategoryNamePartSet>
RepositoryRepository::category_names_containing_package(const PackageNamePart & p, const RepositoryContentMayExcludes & x) const
{
    return Repository::category_names_containing_package(p, x);
}

std::shared_ptr<const QualifiedPackageNameSet>
RepositoryRepository::package_names(const CategoryNamePart & c, const RepositoryContentMayExcludes &) const
{
    return _imp->store->package_names(c);
}

std::shared_ptr<const PackageIDSequence>
RepositoryRepository::package_ids(const QualifiedPackageName & p, const RepositoryContentMayExcludes &) const
{
    return _imp->store->package_ids(p);
}

bool
RepositoryRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    return a.make_accept_returning(
            [&] (const SupportsActionTest<InstallAction> &)      { return false; },
            [&] (const SupportsActionTest<FetchAction> &)        { return false; },
            [&] (const SupportsActionTest<PretendFetchAction> &) { return false; },
            [&] (const SupportsActionTest<ConfigAction> &)       { return false; },
            [&] (const SupportsActionTest<PretendAction> &)      { return false; },
            [&] (const SupportsActionTest<InfoAction> &)         { return false; },
            [&] (const SupportsActionTest<UninstallAction> &)    { return false; }
            );
}

bool
RepositoryRepository::some_ids_might_not_be_masked() const
{
    return true;
}

bool
RepositoryRepository::sync(const std::string &, const std::string &, const std::shared_ptr<OutputManager> &) const
{
    return false;
}

std::shared_ptr<Repository>
RepositoryRepository::repository_factory_create(
        Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    Context context("When making repository repository from repo_file '" + f("repo_file") + "':");

    std::string name_str(f("name"));
    if (name_str.empty())
        name_str = "repository";

    std::string config_filename(f("config_filename"));
    if (config_filename.empty())
        throw RepositoryRepositoryConfigurationError("Key 'config_filename' not specified or empty");

    std::string config_template(f("config_template"));
    if (config_template.empty())
        throw RepositoryRepositoryConfigurationError("Key 'config_template' not specified or empty");

    std::string root_str(f("root"));
    if (root_str.empty())
        root_str = "/";

    return std::make_shared<RepositoryRepository>(
            make_named_values<RepositoryRepositoryParams>(
                n::config_filename() = config_filename,
                n::config_template() = config_template,
                n::environment() = env,
                n::name() = RepositoryName(name_str),
                n::root() = root_str
                ));
}

RepositoryName
RepositoryRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> & f)
{
    if (f("name").empty())
        return RepositoryName("repository");
    else
        return RepositoryName(f("name"));
}

std::shared_ptr<const RepositoryNameSet>
RepositoryRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

void
RepositoryRepository::populate_sets() const
{
}

HookResult
RepositoryRepository::perform_hook(const Hook &, const std::shared_ptr<OutputManager> &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > >
RepositoryRepository::sync_host_key() const
{
    return nullptr;
}

const std::shared_ptr<const MetadataValueKey<std::string>>
RepositoryRepository::tool_prefix_key() const
{
    return nullptr;
}

bool
RepositoryRepository::is_suitable_destination_for(const std::shared_ptr<const PackageID> & id) const
{
    auto repo(_imp->params.environment()->fetch_repository(id->repository_name()));
    std::string f(repo->format_key() ? repo->format_key()->parse_value() : "");
    return f == "unavailable";
}

bool
RepositoryRepository::want_pre_post_phases() const
{
    return true;
}

std::string
RepositoryRepository::split_debug_location() const
{
    return std::string();
}

namespace
{
    std::string get_string_key(const std::shared_ptr<const MetadataKeyHolder> & id, const std::string & k)
    {
        PackageID::MetadataConstIterator i(id->find_metadata(k));
        if (id->end_metadata() == i)
            return "";

        const MetadataValueKey<std::string> * const ii(visitor_cast<const MetadataValueKey<std::string> >(**i));
        if (! ii)
            return "";

        return ii->parse_value();
    }

    std::string replace_vars(
            const std::string & s,
            const std::string & sync,
            const std::string & format,
            const std::string & name)
    {
        std::string result;
        SimpleParser parser(s);

        while (! parser.eof())
        {
            std::string read;

            if (parser.consume(simple_parser::exact("%%")))
                result.append("%");
            else if (parser.consume(simple_parser::exact("%")))
            {
                if (parser.consume(simple_parser::exact("{")))
                {
                    if (! parser.consume((+simple_parser::any_except("}") >> read) &
                                simple_parser::exact("}")))
                        throw ConfigurationError("Bad %{variable} in '" + s + "'");
                }
                else if (parser.consume(+simple_parser::any_of(
                                "abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                "0123456789_"
                                ) >> read))
                {
                }
                else
                    throw ConfigurationError("Bad %variable in '" + s + "'");

                if (read == "repository_template_name")
                    result.append(name);
                else if (read == "repository_template_format")
                    result.append(format);
                else if (read == "repository_template_sync")
                    result.append(sync);
                else
                    throw ConfigurationError("Unknown %variable '" + read + "' in '" + s + "'");
            }
            else if (parser.consume(+simple_parser::any_except("%") >> read))
                result.append(read);
            else
                throw InternalError(PALUDIS_HERE, "failed to consume anything");
        }

        return result;
    }
}

void
RepositoryRepository::merge(const MergeParams & m)
{
    using namespace std::placeholders;

    Context context("When merging '" + stringify(*m.package_id())
            + "' to RepositoryRepository repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(m.package_id()))
        throw ActionFailedError("Not a suitable destination for '" + stringify(*m.package_id()) + "'");

    if (m.check())
        return;

    std::string repo_sync(get_string_key(m.package_id(), "REPOSITORY_SYNC"));
    std::string repo_format(get_string_key(m.package_id(), "REPOSITORY_FORMAT"));
    std::string repo_name(stringify(m.package_id()->name().package()));

    if (repo_sync.empty())
        throw InternalError(PALUDIS_HERE, "no REPOSITORY_SYNC in " + stringify(*m.package_id()));
    if (repo_format.empty())
        throw InternalError(PALUDIS_HERE, "no REPOSITORY_FORMAT in " + stringify(*m.package_id()));

    std::string config_template(_imp->config_template_key->parse_value());
    std::string config_filename(_imp->config_filename_key->parse_value());

    config_template = replace_vars(config_template, repo_sync, repo_format, repo_name);
    config_filename = replace_vars(config_filename, repo_sync, repo_format, repo_name);

    FSPath config_template_file(config_template);
    if (! config_template_file.stat().is_regular_file_or_symlink_to_regular_file())
        throw ConfigurationError("config_template '" + stringify(config_template_file) + "' is not a regular file");

    FSPath config_filename_file(config_filename);
    if (config_filename_file.stat().exists())
        throw ConfigurationError("config_filename '" + stringify(config_filename_file) + "' already exists");

    try
    {
        m.output_manager()->stdout_stream() << "Creating " << config_filename_file << "..." << std::endl;

        {
            SafeIFStream config_template_input(config_template_file);
            std::string data((std::istreambuf_iterator<char>(config_template_input)), std::istreambuf_iterator<char>());
            data = replace_vars(data, repo_sync, repo_format, repo_name);

            SafeOFStream config_filename_output(config_filename_file, -1, true);
            config_filename_output << data;
        }

        m.output_manager()->stdout_stream() << "Syncing..." << std::endl;
        _imp->params.environment()->repository_from_new_config_file(config_filename_file)->sync("", "", m.output_manager());

        /* the repo we'd get before syncing is mostly unusable */
        const std::shared_ptr<Repository> newly_created_repo(
                _imp->params.environment()->repository_from_new_config_file(config_filename_file));

        m.output_manager()->stdout_stream() << "Fixing cache..." << std::endl;
        newly_created_repo->regenerate_cache();
    }
    catch (...)
    {
        m.output_manager()->stdout_stream() << "Removing " << config_filename_file << "..." << std::endl;
        config_filename_file.unlink();
        throw;
    }
}

const std::shared_ptr<const Set<std::string> >
RepositoryRepository::maybe_expand_licence_nonrecursively(const std::string &) const
{
    return nullptr;
}

namespace paludis
{
    template class Pimp<repository_repository::RepositoryRepository>;
}

