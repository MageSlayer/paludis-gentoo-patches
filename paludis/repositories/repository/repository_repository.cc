/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/active_object_ptr.hh>
#include <paludis/util/deferred_construction_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/simple_parser.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/safe_ofstream.hh>
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
    std::tr1::shared_ptr<RepositoryRepositoryStore>
    make_store(const RepositoryRepository * const repo, const RepositoryRepositoryParams & p)
    {
        return make_shared_ptr(new RepositoryRepositoryStore(p.environment(), repo));
    }
}

namespace paludis
{
    template <>
    struct Implementation<RepositoryRepository>
    {
        const RepositoryRepositoryParams params;

        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > config_filename_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > config_template_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<FSEntry> > installed_root_key;

        const ActiveObjectPtr<DeferredConstructionPtr<
            std::tr1::shared_ptr<RepositoryRepositoryStore> > > store;

        Implementation(const RepositoryRepository * const repo, const RepositoryRepositoryParams & p) :
            params(p),
            format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                        mkt_significant, "repository")),
            config_filename_key(new LiteralMetadataValueKey<std::string> (
                        "config_filename", "config_filename", mkt_normal, params.config_filename())),
            config_template_key(new LiteralMetadataValueKey<std::string> (
                        "config_template", "config_template", mkt_normal, params.config_template())),
            installed_root_key(new LiteralMetadataValueKey<FSEntry>("root", "root", mkt_normal, p.root())),
            store(DeferredConstructionPtr<std::tr1::shared_ptr<RepositoryRepositoryStore> > (
                        std::tr1::bind(&make_store, repo, std::tr1::cref(params))))
        {
        }
    };
}

RepositoryRepositoryConfigurationError::RepositoryRepositoryConfigurationError(const std::string & s) throw () :
    ConfigurationError("RepositoryRepository configuration error: " + s)
{
}

RepositoryRepository::RepositoryRepository(const RepositoryRepositoryParams & p) :
    PrivateImplementationPattern<RepositoryRepository>(new Implementation<RepositoryRepository>(this, p)),
    Repository(
            p.environment(),
            p.name(),
            make_named_values<RepositoryCapabilities>(
                n::destination_interface() = static_cast<RepositoryDestinationInterface *>(this),
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(0),
                n::make_virtuals_interface() = static_cast<RepositoryMakeVirtualsInterface *>(0),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(0),
                n::provides_interface() = static_cast<RepositoryProvidesInterface *>(0),
                n::virtuals_interface() = static_cast<RepositoryVirtualsInterface *>(0)
                )),
    _imp(PrivateImplementationPattern<RepositoryRepository>::_imp)
{
    _add_metadata_keys();
}

RepositoryRepository::~RepositoryRepository()
{
}

bool
RepositoryRepository::can_be_favourite_repository() const
{
    return false;
}

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

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
RepositoryRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
RepositoryRepository::location_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
RepositoryRepository::installed_root_key() const
{
    return _imp->installed_root_key;
}

void
RepositoryRepository::invalidate()
{
    _imp.reset(new Implementation<RepositoryRepository>(this, _imp->params));
    _add_metadata_keys();
}

void
RepositoryRepository::invalidate_masks()
{
}

bool
RepositoryRepository::has_category_named(const CategoryNamePart & c) const
{
    return _imp->store->has_category_named(c);
}

bool
RepositoryRepository::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->store->has_package_named(q);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
RepositoryRepository::category_names() const
{
    return _imp->store->category_names();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
RepositoryRepository::unimportant_category_names() const
{
    return _imp->store->unimportant_category_names();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
RepositoryRepository::category_names_containing_package(const PackageNamePart & p) const
{
    return Repository::category_names_containing_package(p);
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
RepositoryRepository::package_names(const CategoryNamePart & c) const
{
    return _imp->store->package_names(c);
}

std::tr1::shared_ptr<const PackageIDSequence>
RepositoryRepository::package_ids(const QualifiedPackageName & p) const
{
    return _imp->store->package_ids(p);
}

namespace
{
    struct SupportsActionQuery
    {
        bool visit(const SupportsActionTest<InstallAction> &) const
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
RepositoryRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

bool
RepositoryRepository::sync(const std::tr1::shared_ptr<OutputManager> &) const
{
    return false;
}

std::tr1::shared_ptr<Repository>
RepositoryRepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
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

    return std::tr1::shared_ptr<RepositoryRepository>(new RepositoryRepository(
                make_named_values<RepositoryRepositoryParams>(
                    n::config_filename() = config_filename,
                    n::config_template() = config_template,
                    n::environment() = env,
                    n::name() = RepositoryName(name_str),
                    n::root() = root_str
                )));
}

RepositoryName
RepositoryRepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    if (f("name").empty())
        return RepositoryName("repository");
    else
        return RepositoryName(f("name"));
}

std::tr1::shared_ptr<const RepositoryNameSet>
RepositoryRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
}

void
RepositoryRepository::populate_sets() const
{
}

HookResult
RepositoryRepository::perform_hook(const Hook &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
RepositoryRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
RepositoryRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

bool
RepositoryRepository::is_suitable_destination_for(const PackageID & e) const
{
    std::string f(e.repository()->format_key() ? e.repository()->format_key()->value() : "");
    return f == "unavailable";
}

bool
RepositoryRepository::is_default_destination() const
{
    return false;
}

bool
RepositoryRepository::want_pre_post_phases() const
{
    return true;
}

namespace
{
    std::string get_string_key(const std::tr1::shared_ptr<const MetadataKeyHolder> & id, const std::string & k)
    {
        PackageID::MetadataConstIterator i(id->find_metadata(k));
        if (id->end_metadata() == i)
            return "";

        const MetadataValueKey<std::string> * const ii(simple_visitor_cast<const MetadataValueKey<std::string> >(**i));
        if (! ii)
            return "";

        return ii->value();
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
    using namespace std::tr1::placeholders;

    Context context("When merging '" + stringify(*m.package_id())
            + "' to RepositoryRepository repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(*m.package_id()))
        throw ActionFailedError("Not a suitable destination for '" + stringify(*m.package_id()) + "'");

    std::string repo_sync(get_string_key(m.package_id(), "REPOSITORY_SYNC"));
    std::string repo_format(get_string_key(m.package_id(), "REPOSITORY_FORMAT"));
    std::string repo_name(stringify(m.package_id()->name().package()));

    if (repo_sync.empty())
        throw InternalError(PALUDIS_HERE, "no REPOSITORY_SYNC in " + stringify(*m.package_id()));
    if (repo_format.empty())
        throw InternalError(PALUDIS_HERE, "no REPOSITORY_FORMAT in " + stringify(*m.package_id()));

    std::string config_template(_imp->config_template_key->value());
    std::string config_filename(_imp->config_filename_key->value());

    config_template = replace_vars(config_template, repo_sync, repo_format, repo_name);
    config_filename = replace_vars(config_filename, repo_sync, repo_format, repo_name);

    FSEntry config_template_file(config_template);
    if (! config_template_file.is_regular_file_or_symlink_to_regular_file())
        throw ConfigurationError("config_template '" + stringify(config_template_file) + "' is not a regular file");

    FSEntry config_filename_file(config_filename);
    if (config_filename_file.exists())
        throw ConfigurationError("config_filename '" + stringify(config_filename_file) + "' already exists");

    try
    {
        m.output_manager()->stdout_stream() << "Creating " << config_filename_file << "..." << std::endl;

        {
            SafeIFStream config_template_input(config_template_file);
            std::string data((std::istreambuf_iterator<char>(config_template_input)), std::istreambuf_iterator<char>());
            data = replace_vars(data, repo_sync, repo_format, repo_name);

            SafeOFStream config_filename_output(config_filename_file);
            config_filename_output << data;
        }

        m.output_manager()->stdout_stream() << "Syncing..." << std::endl;
        _imp->params.environment()->repository_from_new_config_file(config_filename_file)->sync(m.output_manager());

        /* the repo we'd get before syncing is mostly unusable */
        const std::tr1::shared_ptr<Repository> newly_created_repo(
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

template class PrivateImplementationPattern<repository_repository::RepositoryRepository>;

