/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
#include <paludis/ndbam.hh>
#include <paludis/ndbam_merger.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/system.hh>
#include <paludis/util/cookie.hh>
#include <paludis/util/kc.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/action.hh>
#include <paludis/environment.hh>
#include <paludis/dep_tag.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <fstream>
#include <sstream>
#include <sys/time.h>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

#include <paludis/repositories/unpackaged/installed_repository-sr.cc>

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

        tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > root_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;

        Implementation(const InstalledUnpackagedRepositoryParams & p) :
            params(p),
            ndbam(p.location, &supported_installed_unpackaged, "installed_unpackaged-1"),
            location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                        mkt_significant, params.location)),
            root_key(new LiteralMetadataValueKey<FSEntry> ("root", "root",
                        mkt_normal, params.root)),
            format_key(new LiteralMetadataValueKey<std::string> (
                        "format", "format", mkt_significant, "installed_unpackaged"))
        {
        }
    };
}

InstalledUnpackagedRepository::InstalledUnpackagedRepository(
        const RepositoryName & n, const InstalledUnpackagedRepositoryParams & p) :
    PrivateImplementationPattern<InstalledUnpackagedRepository>(new Implementation<InstalledUnpackagedRepository>(p)),
    Repository(n, RepositoryCapabilities::named_create()
            (k::sets_interface(), this)
            (k::syncable_interface(), static_cast<RepositorySyncableInterface *>(0))
            (k::use_interface(), static_cast<RepositoryUseInterface *>(0))
            (k::mirrors_interface(), static_cast<RepositoryMirrorsInterface *>(0))
            (k::environment_variable_interface(), static_cast<RepositoryEnvironmentVariableInterface *>(0))
            (k::provides_interface(), static_cast<RepositoryProvidesInterface *>(0))
            (k::virtuals_interface(), static_cast<RepositoryVirtualsInterface *>(0))
            (k::make_virtuals_interface(), static_cast<RepositoryMakeVirtualsInterface *>(0))
            (k::destination_interface(), this)
            (k::e_interface(), static_cast<RepositoryEInterface *>(0))
            (k::hook_interface(), static_cast<RepositoryHookInterface *>(0))
            (k::qa_interface(), static_cast<RepositoryQAInterface *>(0))
            (k::manifest_interface(), static_cast<RepositoryManifestInterface *>(0))),
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

tr1::shared_ptr<const PackageIDSequence>
InstalledUnpackagedRepository::package_ids(const QualifiedPackageName & q) const
{
    tr1::shared_ptr<NDBAMEntrySequence> entries(_imp->ndbam.entries(q));
    tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

    for (IndirectIterator<NDBAMEntrySequence::ConstIterator> e(entries->begin()), e_end(entries->end()) ;
            e != e_end ; ++e)
    {
        Lock l(*(*e)[k::mutex()]);
        if (! (*e)[k::package_id()])
            (*e)[k::package_id()].reset(new InstalledUnpackagedID(_imp->params.environment, (*e)[k::name()], (*e)[k::version()],
                        (*e)[k::slot()], name(), (*e)[k::fs_location()], (*e)[k::magic()], installed_root_key()->value(), &_imp->ndbam));
        result->push_back((*e)[k::package_id()]);
    }

    return result;
}

tr1::shared_ptr<const QualifiedPackageNameSet>
InstalledUnpackagedRepository::package_names(const CategoryNamePart & c) const
{
    return _imp->ndbam.package_names(c);
}

tr1::shared_ptr<const CategoryNamePartSet>
InstalledUnpackagedRepository::category_names() const
{
    return _imp->ndbam.category_names();
}

tr1::shared_ptr<const CategoryNamePartSet>
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
    struct SomeIDsMightSupportVisitor :
        ConstVisitor<SupportsActionTestVisitorTypes>
    {
        bool result;

        void visit(const SupportsActionTest<UninstallAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<InstalledAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
           result = false;
        }

        void visit(const SupportsActionTest<InfoAction> &)
        {
            result = false;
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
            result = false;
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
            result = false;
        }

        void visit(const SupportsActionTest<PretendFetchAction> &)
        {
            result = false;
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
            result = false;
        }
    };
}

bool
InstalledUnpackagedRepository::some_ids_might_support_action(const SupportsActionTestBase & test) const
{
    SomeIDsMightSupportVisitor v;
    test.accept(v);
    return v.result;
}

void
InstalledUnpackagedRepository::merge(const MergeParams & m)
{
    Context context("When merging '" + stringify(*m[k::package_id()]) + "' at '" + stringify(m[k::image_dir()])
            + "' to InstalledUnpackagedRepository repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(*m[k::package_id()]))
        throw InstallActionError("Not a suitable destination for '" + stringify(*m[k::package_id()]) + "'");

    FSEntry install_under("/");
    {
        Repository::MetadataConstIterator k(m[k::package_id()]->repository()->find_metadata("install_under"));
        if (k == m[k::package_id()]->repository()->end_metadata())
            throw InstallActionError("Could not fetch install_under key from owning repository");
        const MetadataValueKey<FSEntry> * kk(visitor_cast<const MetadataValueKey<FSEntry> >(**k));
        if (! kk)
            throw InstallActionError("Fetched install_under key but did not get an FSEntry key from owning repository");
        install_under = kk->value();
    }

    tr1::shared_ptr<const PackageID> if_overwritten_id, if_same_name_id;
    {
        tr1::shared_ptr<const PackageIDSequence> ids(package_ids(m[k::package_id()]->name()));
        for (PackageIDSequence::ConstIterator v(ids->begin()), v_end(ids->end()) ;
                v != v_end ; ++v)
        {
            if_same_name_id = *v;
            if ((*v)->version() == m[k::package_id()]->version() && (*v)->slot() == m[k::package_id()]->slot())
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
        std::string uid(stringify(m[k::package_id()]->name().category) + "---" + stringify(m[k::package_id()]->name().package));
        uid_dir /= "data";
        uid_dir.mkdir();
        uid_dir /= uid;
        uid_dir.mkdir();
    }

    FSEntry target_ver_dir(uid_dir);
    target_ver_dir /= (stringify(m[k::package_id()]->version()) + ":" + stringify(m[k::package_id()]->slot()) + ":" + cookie());

    if (target_ver_dir.exists())
        throw InstallActionError("Temporary merge directory '" + stringify(target_ver_dir) + "' already exists, probably "
                "due to a previous failed install. If it is safe to do so, please remove this directory and try again.");
    target_ver_dir.mkdir();

    {
        std::ofstream source_repository_file(stringify(target_ver_dir / "source_repository").c_str());
        source_repository_file << m[k::package_id()]->repository()->name() << std::endl;
        if (! source_repository_file)
            throw InstallActionError("Could not write to '" + stringify(target_ver_dir / "source_repository") + "'");
    }

    if (m[k::package_id()]->short_description_key())
    {
        std::ofstream description_file(stringify(target_ver_dir / "description").c_str());
        description_file << m[k::package_id()]->short_description_key()->value() << std::endl;
        if (! description_file)
            throw InstallActionError("Could not write to '" + stringify(target_ver_dir / "description") + "'");
    }

    if (m[k::package_id()]->build_dependencies_key())
    {
        std::ofstream build_dependencies_file(stringify(target_ver_dir / "build_dependencies").c_str());
        StringifyFormatter f;
        build_dependencies_file << m[k::package_id()]->build_dependencies_key()->pretty_print_flat(f) << std::endl;
        if (! build_dependencies_file)
            throw InstallActionError("Could not write to '" + stringify(target_ver_dir / "build_dependencies") + "'");
    }

    if (m[k::package_id()]->run_dependencies_key())
    {
        std::ofstream run_dependencies_file(stringify(target_ver_dir / "run_dependencies").c_str());
        StringifyFormatter f;
        run_dependencies_file << m[k::package_id()]->run_dependencies_key()->pretty_print_flat(f) << std::endl;
        if (! run_dependencies_file)
            throw InstallActionError("Could not write to '" + stringify(target_ver_dir / "run_dependencies") + "'");
    }

    NDBAMMerger merger(
            NDBAMMergerParams::named_create()
            (k::environment(), _imp->params.environment)
            (k::image(), m[k::image_dir()])
            (k::install_under(), install_under)
            (k::root(), installed_root_key()->value())
            (k::contents_file(), target_ver_dir / "contents")
            (k::config_protect(), getenv_with_default("CONFIG_PROTECT", ""))
            (k::config_protect_mask(), getenv_with_default("CONFIG_PROTECT_MASK", ""))
            (k::package_id(), m[k::package_id()])
            (k::options(), MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs));

    if (! merger.check())
    {
        for (DirIterator d(target_ver_dir, DirIteratorOptions() + dio_include_dotfiles), d_end ; d != d_end ; ++d)
            FSEntry(*d).unlink();
        target_ver_dir.rmdir();
        throw InstallActionError("Not proceeding with install due to merge sanity check failing");
    }

    merger.merge();

    _imp->ndbam.index(m[k::package_id()]->name(), uid_dir.basename());

    if (if_overwritten_id)
    {
        tr1::static_pointer_cast<const InstalledUnpackagedID>(if_overwritten_id)->uninstall(UninstallActionOptions::named_create()
                (k::no_config_protect(), false),
                true);
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
    return _imp->params.environment->root() == installed_root_key()->value();
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

tr1::shared_ptr<SetSpecTree::ConstItem>
InstalledUnpackagedRepository::package_set(const SetName & s) const
{
    using namespace tr1::placeholders;

    Context context("When fetching package set '" + stringify(s) + "' from '" +
            stringify(name()) + "':");

    if ("ununused" == s.data())
    {
        tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > result(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                    make_shared_ptr(new AllDepSpec)));
        tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(s, stringify(name())));

        tr1::shared_ptr<const CategoryNamePartSet> cats(category_names());
        for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
                c != c_end ; ++c)
        {
            tr1::shared_ptr<const QualifiedPackageNameSet> pkgs(package_names(*c));
            for (QualifiedPackageNameSet::ConstIterator e(pkgs->begin()), e_end(pkgs->end()) ;
                    e != e_end ; ++e)
            {
                tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(make_package_dep_spec().package(QualifiedPackageName(*e))));
                spec->set_tag(tag);
                result->add(make_shared_ptr(new TreeLeaf<SetSpecTree, PackageDepSpec>(spec)));
            }
        }

        return result;
    }
    else
        return tr1::shared_ptr<SetSpecTree::ConstItem>();
}

tr1::shared_ptr<const SetNameSet>
InstalledUnpackagedRepository::sets_list() const
{
    Context context("While generating the list of sets:");

    tr1::shared_ptr<SetNameSet> result(new SetNameSet);
    result->insert(SetName("everything"));
    result->insert(SetName("ununused"));
    return result;
}

void
InstalledUnpackagedRepository::need_keys_added() const
{
}

const tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedRepository::format_key() const
{
    return _imp->format_key;
}

const tr1::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledUnpackagedRepository::installed_root_key() const
{
    return _imp->root_key;
}

