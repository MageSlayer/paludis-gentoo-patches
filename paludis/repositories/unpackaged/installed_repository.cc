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

#include <paludis/repositories/unpackaged/installed_repository.hh>
#include <paludis/repositories/unpackaged/installed_id.hh>
#include <paludis/repositories/unpackaged/exceptions.hh>
#include <paludis/ndbam.hh>
#include <paludis/ndbam_merger.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/system.hh>
#include <paludis/util/cookie.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/indirect_iterator.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/action.hh>
#include <paludis/environment.hh>
#include <paludis/dep_tag.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/selection.hh>
#include <paludis/hook.hh>
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
    struct Implementation<InstalledUnpackagedRepository>
    {
        const InstalledUnpackagedRepositoryParams params;
        mutable NDBAM ndbam;

        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > root_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Implementation(const InstalledUnpackagedRepositoryParams & p) :
            params(p),
            ndbam(p.location(), &supported_installed_unpackaged, "installed_unpackaged-1", user_version_spec_options()),
            location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                        mkt_significant, params.location())),
            root_key(new LiteralMetadataValueKey<FSEntry> ("root", "root",
                        mkt_normal, params.root())),
            format_key(new LiteralMetadataValueKey<std::string> (
                        "format", "format", mkt_significant, "installed_unpackaged"))
        {
        }
    };
}

InstalledUnpackagedRepository::InstalledUnpackagedRepository(
        const RepositoryName & n, const InstalledUnpackagedRepositoryParams & p) :
    PrivateImplementationPattern<InstalledUnpackagedRepository>(new Implementation<InstalledUnpackagedRepository>(p)),
    Repository(p.environment(), n, make_named_values<RepositoryCapabilities>(
                value_for<n::destination_interface>(this),
                value_for<n::environment_variable_interface>(static_cast<RepositoryEnvironmentVariableInterface *>(0)),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(static_cast<RepositoryManifestInterface *>(0)),
                value_for<n::mirrors_interface>(static_cast<RepositoryMirrorsInterface *>(0)),
                value_for<n::provides_interface>(static_cast<RepositoryProvidesInterface *>(0)),
                value_for<n::virtuals_interface>(static_cast<RepositoryVirtualsInterface *>(0))
            )),
    _imp(PrivateImplementationPattern<InstalledUnpackagedRepository>::_imp)
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

std::tr1::shared_ptr<const PackageIDSequence>
InstalledUnpackagedRepository::package_ids(const QualifiedPackageName & q) const
{
    std::tr1::shared_ptr<NDBAMEntrySequence> entries(_imp->ndbam.entries(q));
    std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

    for (IndirectIterator<NDBAMEntrySequence::ConstIterator> e(entries->begin()), e_end(entries->end()) ;
            e != e_end ; ++e)
    {
        Lock l(*(*e).mutex());
        if (! (*e).package_id())
            (*e).package_id().reset(new InstalledUnpackagedID(_imp->params.environment(), (*e).name(), (*e).version(),
                        (*e).slot(), name(), (*e).fs_location(), (*e).magic(), installed_root_key()->value(), &_imp->ndbam));
        result->push_back((*e).package_id());
    }

    return result;
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
InstalledUnpackagedRepository::package_names(const CategoryNamePart & c) const
{
    return _imp->ndbam.package_names(c);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
InstalledUnpackagedRepository::category_names() const
{
    return _imp->ndbam.category_names();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
InstalledUnpackagedRepository::category_names_containing_package(
        const PackageNamePart & p) const
{
    return _imp->ndbam.category_names_containing_package(p);
}

bool
InstalledUnpackagedRepository::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->ndbam.has_package_named(q);
}

bool
InstalledUnpackagedRepository::has_category_named(const CategoryNamePart & c) const
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

namespace
{
    std::pair<uid_t, gid_t>
    get_new_ids_or_minus_one(const Environment * const env, const int rewrite_ids_over_to_root, const FSEntry & f)
    {
        uid_t uid;
        gid_t gid;

        if (f.owner() == env->reduced_uid() || (rewrite_ids_over_to_root != -1
                    && f.owner() > static_cast<unsigned int>(rewrite_ids_over_to_root)))
            uid = 0;
        else
            uid = -1;

        if (f.group() == env->reduced_gid() || (rewrite_ids_over_to_root != -1
                    && f.group() > static_cast<unsigned int>(rewrite_ids_over_to_root)))
            gid = 0;
        else
            gid = -1;

        return std::make_pair(uid, gid);
    }

    bool slot_is_same(const std::tr1::shared_ptr<const PackageID> & a,
            const std::tr1::shared_ptr<const PackageID> & b)
    {
        if (a->slot_key())
            return b->slot_key() && a->slot_key()->value() == b->slot_key()->value();
        else
            return ! b->slot_key();
    }
}

void
InstalledUnpackagedRepository::merge(const MergeParams & m)
{
    using namespace std::tr1::placeholders;

    Context context("When merging '" + stringify(*m.package_id()) + "' at '" + stringify(m.image_dir())
            + "' to InstalledUnpackagedRepository repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(*m.package_id()))
        throw ActionFailedError("Not a suitable destination for '" + stringify(*m.package_id()) + "'");

    FSEntry install_under("/");
    {
        Repository::MetadataConstIterator k(m.package_id()->repository()->find_metadata("install_under"));
        if (k == m.package_id()->repository()->end_metadata())
            throw ActionFailedError("Could not fetch install_under key from owning repository");
        const MetadataValueKey<FSEntry> * kk(simple_visitor_cast<const MetadataValueKey<FSEntry> >(**k));
        if (! kk)
            throw ActionFailedError("Fetched install_under key but did not get an FSEntry key from owning repository");
        install_under = kk->value();
    }

    int rewrite_ids_over_to_root(-1);
    {
        Repository::MetadataConstIterator k(m.package_id()->repository()->find_metadata("rewrite_ids_over_to_root"));
        if (k == m.package_id()->repository()->end_metadata())
            throw ActionFailedError("Could not fetch rewrite_ids_over_to_root key from owning repository");
        const MetadataValueKey<long> * kk(simple_visitor_cast<const MetadataValueKey<long> >(**k));
        if (! kk)
            throw ActionFailedError("Fetched rewrite_ids_over_to_root key but did not get a long key from owning repository");
        rewrite_ids_over_to_root = kk->value();
    }

    std::tr1::shared_ptr<const PackageID> if_overwritten_id, if_same_name_id;
    {
        std::tr1::shared_ptr<const PackageIDSequence> ids(package_ids(m.package_id()->name()));
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
    target_ver_dir /= (stringify(m.package_id()->version()) + ":" + stringify(m.package_id()->slot_key()->value()) + ":" + cookie());

    if (target_ver_dir.exists())
        throw ActionFailedError("Temporary merge directory '" + stringify(target_ver_dir) + "' already exists, probably "
                "due to a previous failed install. If it is safe to do so, please remove this directory and try again.");
    target_ver_dir.mkdir();

    {
        SafeOFStream source_repository_file(target_ver_dir / "source_repository");
        source_repository_file << m.package_id()->repository()->name() << std::endl;
    }

    if (m.package_id()->short_description_key())
    {
        SafeOFStream description_file(target_ver_dir / "description");
        description_file << m.package_id()->short_description_key()->value() << std::endl;
    }

    if (m.package_id()->build_dependencies_key())
    {
        SafeOFStream build_dependencies_file(target_ver_dir / "build_dependencies");
        StringifyFormatter f;
        build_dependencies_file << m.package_id()->build_dependencies_key()->pretty_print_flat(f) << std::endl;
    }

    if (m.package_id()->run_dependencies_key())
    {
        SafeOFStream run_dependencies_file(target_ver_dir / "run_dependencies");
        StringifyFormatter f;
        run_dependencies_file << m.package_id()->run_dependencies_key()->pretty_print_flat(f) << std::endl;
    }

    NDBAMMerger merger(
            make_named_values<NDBAMMergerParams>(
                value_for<n::config_protect>(getenv_with_default("CONFIG_PROTECT", "")),
                value_for<n::config_protect_mask>(getenv_with_default("CONFIG_PROTECT_MASK", "")),
                value_for<n::contents_file>(target_ver_dir / "contents"),
                value_for<n::environment>(_imp->params.environment()),
                value_for<n::get_new_ids_or_minus_one>(std::tr1::bind(&get_new_ids_or_minus_one,
                        _imp->params.environment(), rewrite_ids_over_to_root, _1)),
                value_for<n::image>(m.image_dir()),
                value_for<n::install_under>(install_under),
                value_for<n::merged_entries>(make_shared_ptr(new FSEntrySet)),
                value_for<n::options>(MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs),
                value_for<n::output_manager>(m.output_manager()),
                value_for<n::package_id>(m.package_id()),
                value_for<n::root>(installed_root_key()->value())
            ));

    if (! merger.check())
    {
        for (DirIterator d(target_ver_dir, DirIteratorOptions() + dio_include_dotfiles), d_end ; d != d_end ; ++d)
            FSEntry(*d).unlink();
        target_ver_dir.rmdir();
        throw ActionFailedError("Not proceeding with install due to merge sanity check failing");
    }

    merger.merge();

    _imp->ndbam.index(m.package_id()->name(), uid_dir.basename());

    if (if_overwritten_id)
    {
        std::tr1::static_pointer_cast<const InstalledUnpackagedID>(if_overwritten_id)->uninstall(true,
                if_overwritten_id, m.output_manager());
    }
}

bool
InstalledUnpackagedRepository::is_suitable_destination_for(const PackageID & e) const
{
    std::string f(e.repository()->format_key() ? e.repository()->format_key()->value() : "");
    return f == "unpackaged";
}

bool
InstalledUnpackagedRepository::is_default_destination() const
{
    return _imp->params.environment()->root() == installed_root_key()->value();
}

bool
InstalledUnpackagedRepository::want_pre_post_phases() const
{
    return true;
}

void
InstalledUnpackagedRepository::invalidate()
{
    _imp.reset(new Implementation<InstalledUnpackagedRepository>(_imp->params));
    _add_metadata_keys();
}

void
InstalledUnpackagedRepository::invalidate_masks()
{
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

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedRepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledUnpackagedRepository::location_key() const
{
    return _imp->location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledUnpackagedRepository::installed_root_key() const
{
    return _imp->root_key;
}

std::tr1::shared_ptr<Repository>
InstalledUnpackagedRepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    Context context("When creating InstalledUnpackagedRepository:");

    std::string location(f("location"));
    if (location.empty())
        throw unpackaged_repositories::RepositoryConfigurationError("Key 'location' not specified or empty");

    std::string root(f("root"));
    if (root.empty())
        throw unpackaged_repositories::RepositoryConfigurationError("Key 'root' not specified or empty");

    return make_shared_ptr(new InstalledUnpackagedRepository(RepositoryName("installed-unpackaged"),
                make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                    value_for<n::environment>(env),
                    value_for<n::location>(location),
                    value_for<n::root>(root)
                )));
}

RepositoryName
InstalledUnpackagedRepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return RepositoryName("installed-unpackaged");
}

std::tr1::shared_ptr<const RepositoryNameSet>
InstalledUnpackagedRepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> &)
{
    return make_shared_ptr(new RepositoryNameSet);
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
InstalledUnpackagedRepository::populate_sets() const
{
    _imp->params.environment()->add_set(
            SetName("everything"),
            SetName("everything::" + stringify(name())),
            std::tr1::bind(get_everything_set, _imp->params.environment(), this),
            true);
}

HookResult
InstalledUnpackagedRepository::perform_hook(const Hook &)
{
    return make_named_values<HookResult>(value_for<n::max_exit_status>(0), value_for<n::output>(""));
}

bool
InstalledUnpackagedRepository::sync(const std::tr1::shared_ptr<OutputManager> &) const
{
    return false;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedRepository::accept_keywords_key() const
{
    return make_null_shared_ptr();
}

