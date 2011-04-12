/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/unpackaged/installed_repository.hh>
#include <paludis/repositories/unpackaged/installed_id.hh>
#include <paludis/repositories/unpackaged/exceptions.hh>
#include <paludis/ndbam.hh>
#include <paludis/ndbam_merger.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/system.hh>
#include <paludis/util/cookie.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/action.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/selection.hh>
#include <paludis/hook.hh>
#include <paludis/common_sets.hh>
#include <paludis/unformatted_pretty_printer.hh>
#include <sstream>
#include <sys/time.h>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace
{
    bool supported_installed_unpackaged(const std::string & s)
    {
        return s == "installed_unpackaged-1";
    }
}

namespace paludis
{
    template <>
    struct Imp<InstalledUnpackagedRepository>
    {
        const InstalledUnpackagedRepositoryParams params;
        mutable NDBAM ndbam;

        std::shared_ptr<const MetadataValueKey<FSPath> > location_key;
        std::shared_ptr<const MetadataValueKey<FSPath> > root_key;
        std::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Imp(const InstalledUnpackagedRepositoryParams & p) :
            params(p),
            ndbam(p.location(), &supported_installed_unpackaged, "installed_unpackaged-1", user_version_spec_options()),
            location_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("location", "location",
                        mkt_significant, params.location())),
            root_key(std::make_shared<LiteralMetadataValueKey<FSPath> >("root", "root",
                        mkt_normal, params.root())),
            format_key(std::make_shared<LiteralMetadataValueKey<std::string> >(
                        "format", "format", mkt_significant, "installed_unpackaged"))
        {
        }
    };
}

InstalledUnpackagedRepository::InstalledUnpackagedRepository(
        const RepositoryName & n, const InstalledUnpackagedRepositoryParams & p) :
    Repository(p.environment(), n, make_named_values<RepositoryCapabilities>(
                n::destination_interface() = this,
                n::environment_variable_interface() = static_cast<RepositoryEnvironmentVariableInterface *>(0),
                n::make_virtuals_interface() = static_cast<RepositoryMakeVirtualsInterface *>(0),
                n::manifest_interface() = static_cast<RepositoryManifestInterface *>(0),
                n::provides_interface() = static_cast<RepositoryProvidesInterface *>(0),
                n::virtuals_interface() = static_cast<RepositoryVirtualsInterface *>(0)
            )),
    _imp(p)
{
    _add_metadata_keys();
}

InstalledUnpackagedRepository::~InstalledUnpackagedRepository()
{
}

void
InstalledUnpackagedRepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->root_key);
    add_metadata_key(_imp->format_key);
}

std::shared_ptr<const PackageIDSequence>
InstalledUnpackagedRepository::package_ids(const QualifiedPackageName & q, const RepositoryContentMayExcludes &) const
{
    std::shared_ptr<NDBAMEntrySequence> entries(_imp->ndbam.entries(q));
    std::shared_ptr<PackageIDSequence> result(std::make_shared<PackageIDSequence>());

    for (IndirectIterator<NDBAMEntrySequence::ConstIterator> e(entries->begin()), e_end(entries->end()) ;
            e != e_end ; ++e)
    {
        Lock l(*(*e).mutex());
        if (! (*e).package_id())
            (*e).package_id() = std::make_shared<InstalledUnpackagedID>(_imp->params.environment(), (*e).name(), (*e).version(),
                        (*e).slot(), name(), (*e).fs_location(), (*e).magic(), installed_root_key()->parse_value(), &_imp->ndbam);
        result->push_back((*e).package_id());
    }

    return result;
}

std::shared_ptr<const QualifiedPackageNameSet>
InstalledUnpackagedRepository::package_names(const CategoryNamePart & c, const RepositoryContentMayExcludes &) const
{
    return _imp->ndbam.package_names(c);
}

std::shared_ptr<const CategoryNamePartSet>
InstalledUnpackagedRepository::category_names(const RepositoryContentMayExcludes &) const
{
    return _imp->ndbam.category_names();
}

std::shared_ptr<const CategoryNamePartSet>
InstalledUnpackagedRepository::category_names_containing_package(
        const PackageNamePart & p, const RepositoryContentMayExcludes &) const
{
    return _imp->ndbam.category_names_containing_package(p);
}

bool
InstalledUnpackagedRepository::has_package_named(const QualifiedPackageName & q, const RepositoryContentMayExcludes &) const
{
    return _imp->ndbam.has_package_named(q);
}

bool
InstalledUnpackagedRepository::has_category_named(const CategoryNamePart & c, const RepositoryContentMayExcludes &) const
{
    return _imp->ndbam.has_category_named(c);
}

namespace
{
    struct SomeIDsMightSupportVisitor
    {
        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<ConfigAction> &) const
        {
           return false;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
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

        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return false;
        }
    };
}

bool
InstalledUnpackagedRepository::some_ids_might_support_action(const SupportsActionTestBase & test) const
{
    SomeIDsMightSupportVisitor v;
    return test.accept_returning<bool>(v);
}

bool
InstalledUnpackagedRepository::some_ids_might_not_be_masked() const
{
    return true;
}

const bool
InstalledUnpackagedRepository::is_unimportant() const
{
    return false;
}

namespace
{
    std::pair<uid_t, gid_t>
    get_new_ids_or_minus_one(const Environment * const env, const int rewrite_ids_over_to_root, const FSPath & f)
    {
        uid_t uid;
        gid_t gid;

        FSStat f_stat(f);

        if (f_stat.owner() == env->reduced_uid() || (rewrite_ids_over_to_root != -1
                    && f_stat.owner() > static_cast<unsigned int>(rewrite_ids_over_to_root)))
            uid = 0;
        else
            uid = -1;

        if (f_stat.group() == env->reduced_gid() || (rewrite_ids_over_to_root != -1
                    && f_stat.group() > static_cast<unsigned int>(rewrite_ids_over_to_root)))
            gid = 0;
        else
            gid = -1;

        return std::make_pair(uid, gid);
    }

    bool slot_is_same(const std::shared_ptr<const PackageID> & a,
            const std::shared_ptr<const PackageID> & b)
    {
        if (a->slot_key())
            return b->slot_key() && a->slot_key()->parse_value() == b->slot_key()->parse_value();
        else
            return ! b->slot_key();
    }
}

void
InstalledUnpackagedRepository::merge(const MergeParams & m)
{
    using namespace std::placeholders;

    Context context("When merging '" + stringify(*m.package_id()) + "' at '" + stringify(m.image_dir())
            + "' to InstalledUnpackagedRepository repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(m.package_id()))
        throw ActionFailedError("Not a suitable destination for '" + stringify(*m.package_id()) + "'");

    auto repo(_imp->params.environment()->fetch_repository(m.package_id()->repository_name()));
    FSPath install_under("/");
    {
        Repository::MetadataConstIterator k(repo->find_metadata("install_under"));
        if (k == repo->end_metadata())
            throw ActionFailedError("Could not fetch install_under key from owning repository");
        const MetadataValueKey<FSPath> * kk(visitor_cast<const MetadataValueKey<FSPath> >(**k));
        if (! kk)
            throw ActionFailedError("Fetched install_under key but did not get an FSPath key from owning repository");
        install_under = kk->parse_value();
    }

    int rewrite_ids_over_to_root(-1);
    {
        Repository::MetadataConstIterator k(repo->find_metadata("rewrite_ids_over_to_root"));
        if (k == repo->end_metadata())
            throw ActionFailedError("Could not fetch rewrite_ids_over_to_root key from owning repository");
        const MetadataValueKey<long> * kk(visitor_cast<const MetadataValueKey<long> >(**k));
        if (! kk)
            throw ActionFailedError("Fetched rewrite_ids_over_to_root key but did not get a long key from owning repository");
        rewrite_ids_over_to_root = kk->parse_value();
    }

    std::shared_ptr<const PackageID> if_overwritten_id, if_same_name_id;
    {
        std::shared_ptr<const PackageIDSequence> ids(package_ids(m.package_id()->name(), { }));
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
        uid_dir = if_same_name_id->fs_location_key()->parse_value().dirname();
    else
    {
        std::string uid(stringify(m.package_id()->name().category()) + "---" + stringify(m.package_id()->name().package()));

        uid_dir /= "data";
        if (! m.check())
            uid_dir.mkdir(0755, { fspmkdo_ok_if_exists });

        uid_dir /= uid;
        if (! m.check())
            uid_dir.mkdir(0755, { fspmkdo_ok_if_exists });
    }

    FSPath target_ver_dir(uid_dir);
    target_ver_dir /= (stringify(m.package_id()->version()) + ":" + stringify(m.package_id()->slot_key()->parse_value()) + ":" + cookie());

    if (target_ver_dir.stat().exists())
        throw ActionFailedError("Temporary merge directory '" + stringify(target_ver_dir) + "' already exists, probably "
                "due to a previous failed install. If it is safe to do so, please remove this directory and try again.");

    if (! m.check())
        target_ver_dir.mkdir(0755, { });

    if (! m.check())
    {
        {
            SafeOFStream source_repository_file(target_ver_dir / "source_repository", -1, true);
            source_repository_file << m.package_id()->repository_name() << std::endl;
        }

        if (m.package_id()->short_description_key())
        {
            SafeOFStream description_file(target_ver_dir / "description", -1, true);
            description_file << m.package_id()->short_description_key()->parse_value() << std::endl;
        }

        if (m.package_id()->build_dependencies_key())
        {
            SafeOFStream build_dependencies_file(target_ver_dir / "build_dependencies", -1, true);
            build_dependencies_file << m.package_id()->build_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        if (m.package_id()->run_dependencies_key())
        {
            SafeOFStream run_dependencies_file(target_ver_dir / "run_dependencies", -1, true);
            run_dependencies_file << m.package_id()->run_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }
    }

    NDBAMMerger merger(
            make_named_values<NDBAMMergerParams>(
                n::config_protect() = getenv_with_default("CONFIG_PROTECT", ""),
                n::config_protect_mask() = getenv_with_default("CONFIG_PROTECT_MASK", ""),
                n::contents_file() = target_ver_dir / "contents",
                n::environment() = _imp->params.environment(),
                n::fix_mtimes_before() = m.build_start_time(),
                n::get_new_ids_or_minus_one() = std::bind(&get_new_ids_or_minus_one,
                        _imp->params.environment(), rewrite_ids_over_to_root, _1),
                n::image() = m.image_dir(),
                n::install_under() = install_under,
                n::merged_entries() = std::make_shared<FSPathSet>(),
                n::options() = m.options(),
                n::output_manager() = m.output_manager(),
                n::package_id() = m.package_id(),
                n::root() = installed_root_key()->parse_value()
            ));

    if (m.check())
    {
        if (! merger.check())
            throw ActionFailedError("Not proceeding with install due to merge sanity check failing");
        return;
    }

    merger.merge();

    _imp->ndbam.index(m.package_id()->name(), uid_dir.basename());

    if (if_overwritten_id)
    {
        std::static_pointer_cast<const InstalledUnpackagedID>(if_overwritten_id)->uninstall(true,
                if_overwritten_id, m.output_manager());
    }
}

bool
InstalledUnpackagedRepository::is_suitable_destination_for(const std::shared_ptr<const PackageID> & id) const
{
    auto repo(_imp->params.environment()->fetch_repository(id->repository_name()));
    std::string f(repo->format_key() ? repo->format_key()->parse_value() : "");
    return f == "unpackaged";
}

bool
InstalledUnpackagedRepository::want_pre_post_phases() const
{
    return true;
}

void
InstalledUnpackagedRepository::invalidate()
{
    _imp.reset(new Imp<InstalledUnpackagedRepository>(_imp->params));
    _add_metadata_keys();
}

void
InstalledUnpackagedRepository::deindex(const QualifiedPackageName & q) const
{
    _imp->ndbam.deindex(q);
}

void
InstalledUnpackagedRepository::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedRepository::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
InstalledUnpackagedRepository::location_key() const
{
    return _imp->location_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
InstalledUnpackagedRepository::installed_root_key() const
{
    return _imp->root_key;
}

std::shared_ptr<Repository>
InstalledUnpackagedRepository::repository_factory_create(
        Environment * const env,
        const std::function<std::string (const std::string &)> & f)
{
    Context context("When creating InstalledUnpackagedRepository:");

    std::string location(f("location"));
    if (location.empty())
        throw unpackaged_repositories::RepositoryConfigurationError("Key 'location' not specified or empty");

    std::string root(f("root"));
    if (root.empty())
        throw unpackaged_repositories::RepositoryConfigurationError("Key 'root' not specified or empty");

    return std::make_shared<InstalledUnpackagedRepository>(RepositoryName("installed-unpackaged"),
                make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                    n::environment() = env,
                    n::location() = location,
                    n::root() = root
                ));
}

RepositoryName
InstalledUnpackagedRepository::repository_factory_name(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return RepositoryName("installed-unpackaged");
}

std::shared_ptr<const RepositoryNameSet>
InstalledUnpackagedRepository::repository_factory_dependencies(
        const Environment * const,
        const std::function<std::string (const std::string &)> &)
{
    return std::make_shared<RepositoryNameSet>();
}

void
InstalledUnpackagedRepository::populate_sets() const
{
    add_common_sets_for_installed_repo(_imp->params.environment(), *this);
}

HookResult
InstalledUnpackagedRepository::perform_hook(const Hook &, const std::shared_ptr<OutputManager> &)
{
    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

bool
InstalledUnpackagedRepository::sync(
        const std::string &,
        const std::string &,
        const std::shared_ptr<OutputManager> &) const
{
    return false;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > >
InstalledUnpackagedRepository::sync_host_key() const
{
    return make_null_shared_ptr();
}

