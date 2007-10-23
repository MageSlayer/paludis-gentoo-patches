/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/repositories/e/vdb_id.hh>
#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>

#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/mutex.hh>
#include <iterator>
#include <fstream>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    std::string file_contents(const FSEntry & f)
    {
        Context c("When reading '" + stringify(f) + "':");
        std::ifstream i(stringify(f).c_str());
        if (! i)
            throw ConfigurationError("Cannot open '" + stringify(f) + "' for read");

        return strip_trailing(std::string((std::istreambuf_iterator<char>(i)), std::istreambuf_iterator<char>()), "\r\n");
    }
}

namespace paludis
{
    template <>
    struct Implementation<VDBID>
    {
        mutable Mutex mutex;

        const QualifiedPackageName name;
        const VersionSpec version;
        const Environment * const environment;
        const tr1::shared_ptr<const Repository> repository;
        const FSEntry dir;
        mutable bool has_keys;

        tr1::shared_ptr<const SlotName> slot;
        tr1::shared_ptr<const EAPI> eapi;

        tr1::shared_ptr<const MetadataFSEntryKey> fs_location;
        tr1::shared_ptr<const MetadataSetKey<UseFlagNameSet> > use;
        tr1::shared_ptr<const MetadataSetKey<Set<std::string> > > inherited;
        tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> > iuse;
        tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license;
        tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > provide;
        tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies;
        tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies;
        tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies;
        tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> > restrictions;
        tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > src_uri;
        tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage;
        tr1::shared_ptr<const MetadataStringKey> short_description;
        tr1::shared_ptr<const MetadataContentsKey> contents;
        tr1::shared_ptr<const MetadataTimeKey> installed_time;
        tr1::shared_ptr<const MetadataStringKey> source_origin;
        tr1::shared_ptr<const MetadataStringKey> binary_origin;

        tr1::shared_ptr<const MetadataStringKey> asflags;
        tr1::shared_ptr<const MetadataStringKey> cbuild;
        tr1::shared_ptr<const MetadataStringKey> cflags;
        tr1::shared_ptr<const MetadataStringKey> chost;
        tr1::shared_ptr<const MetadataStringKey> ctarget;
        tr1::shared_ptr<const MetadataStringKey> cxxflags;
        tr1::shared_ptr<const MetadataStringKey> ldflags;
        tr1::shared_ptr<const MetadataStringKey> pkgmanager;
        tr1::shared_ptr<const MetadataStringKey> vdb_format;

        Implementation(const QualifiedPackageName & q, const VersionSpec & v,
                const Environment * const e,
                const tr1::shared_ptr<const Repository> r, const FSEntry & f) :
            name(q),
            version(v),
            environment(e),
            repository(r),
            dir(f),
            has_keys(false)
        {
        }
    };
}

VDBID::VDBID(const QualifiedPackageName & q, const VersionSpec & v,
        const Environment * const e,
        const tr1::shared_ptr<const Repository> & r,
        const FSEntry & f) :
    PrivateImplementationPattern<VDBID>(new Implementation<VDBID>(q, v, e, r, f)),
    _imp(PrivateImplementationPattern<VDBID>::_imp.get())
{
}

VDBID::~VDBID()
{
}

void
VDBID::need_keys_added() const
{
    Lock l(_imp->mutex);

    if (_imp->has_keys)
        return;
    _imp->has_keys = true;


    // fs_location key could have been loaded by the ::fs_location_key() already.
    if (! _imp->fs_location)
    {
        _imp->fs_location.reset(new EFSLocationKey(shared_from_this(), "VDBDIR", "VDB Location",
                    _imp->dir, mkt_internal));
        add_metadata_key(_imp->fs_location);
    }

    Context context("When loading VDB ID keys from '" + stringify(_imp->dir) + "':");

    if (! eapi()->supported)
    {
        Log::get_instance()->message(ll_debug, lc_context) << "Not loading further keys for '" << *this << "' because EAPI '"
            << eapi()->name << "' is not supported";
        return;
    }

    const tr1::shared_ptr<const EAPIEbuildMetadataVariables> vars(eapi()->supported->ebuild_metadata_variables);
    if (! vars)
        throw InternalError(PALUDIS_HERE, "EAPI '" + eapi()->name + "' supported but has no ebuild_metadata_variables");

    const tr1::shared_ptr<const EAPIEbuildEnvironmentVariables> env(eapi()->supported->ebuild_environment_variables);
    if (! env)
        throw InternalError(PALUDIS_HERE, "EAPI '" + eapi()->name + "' supported but has no ebuild_environment_variables");

    if (! env->env_use.empty())
        if ((_imp->dir / env->env_use).exists())
        {
            _imp->use.reset(new EUseKey(_imp->environment, shared_from_this(), env->env_use, env->description_use,
                        file_contents(_imp->dir / env->env_use), mkt_internal));
            add_metadata_key(_imp->use);
        }

    if (! vars->metadata_inherited.empty())
        if ((_imp->dir / vars->metadata_inherited).exists())
        {
            _imp->inherited.reset(new EInheritedKey(shared_from_this(), vars->metadata_inherited, vars->description_inherited,
                        file_contents(_imp->dir / vars->metadata_inherited), mkt_internal));
            add_metadata_key(_imp->inherited);
        }

    if (! vars->metadata_iuse.empty())
        if ((_imp->dir / vars->metadata_iuse).exists())
        {
            _imp->iuse.reset(new EIUseKey(_imp->environment, shared_from_this(), vars->metadata_iuse, vars->description_iuse,
                        file_contents(_imp->dir / vars->metadata_iuse), mkt_normal));
            add_metadata_key(_imp->iuse);
        }

    if (! vars->metadata_license.empty())
        if ((_imp->dir / vars->metadata_license).exists())
        {
            _imp->license.reset(new ELicenseKey(_imp->environment, shared_from_this(), vars->metadata_license, vars->description_license,
                        file_contents(_imp->dir / vars->metadata_license),  mkt_normal));
            add_metadata_key(_imp->license);
        }

    if (! vars->metadata_provide.empty())
        if ((_imp->dir / vars->metadata_provide).exists())
        {
            _imp->provide.reset(new EProvideKey(_imp->environment, shared_from_this(), vars->metadata_provide, vars->description_provide,
                        file_contents(_imp->dir / vars->metadata_provide), mkt_internal));
            add_metadata_key(_imp->provide);
        }

    if (! vars->metadata_build_depend.empty())
        if ((_imp->dir / vars->metadata_build_depend).exists())
        {
            _imp->build_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), vars->metadata_build_depend,
                        vars->description_build_depend, file_contents(_imp->dir / vars->metadata_build_depend), mkt_dependencies));
            add_metadata_key(_imp->build_dependencies);
        }

    if (! vars->metadata_run_depend.empty())
        if ((_imp->dir / vars->metadata_run_depend).exists())
        {
            _imp->run_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), vars->metadata_run_depend,
                        vars->description_run_depend, file_contents(_imp->dir / vars->metadata_run_depend), mkt_dependencies));
            add_metadata_key(_imp->run_dependencies);
        }

    if (! vars->metadata_pdepend.empty())
        if ((_imp->dir / vars->metadata_pdepend).exists())
        {
            _imp->post_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), vars->metadata_pdepend,
                        vars->description_pdepend, file_contents(_imp->dir / vars->metadata_pdepend), mkt_dependencies));
            add_metadata_key(_imp->post_dependencies);
        }

    if (! vars->metadata_restrict.empty())
        if ((_imp->dir / vars->metadata_restrict).exists())
        {
            _imp->restrictions.reset(new ERestrictKey(_imp->environment, shared_from_this(), vars->metadata_restrict, vars->description_restrict,
                        file_contents(_imp->dir / vars->metadata_restrict), mkt_internal));
            add_metadata_key(_imp->restrictions);
        }

    if (! vars->metadata_src_uri.empty())
        if ((_imp->dir / vars->metadata_src_uri).exists())
        {
            _imp->src_uri.reset(new EFetchableURIKey(_imp->environment, shared_from_this(), vars->metadata_src_uri, vars->description_src_uri,
                        file_contents(_imp->dir / vars->metadata_src_uri), mkt_dependencies));
            add_metadata_key(_imp->src_uri);
        }

    if (! vars->metadata_description.empty())
        if ((_imp->dir / vars->metadata_description).exists())
        {
            _imp->short_description.reset(new EStringKey(shared_from_this(), vars->metadata_description,
                        vars->description_description, file_contents(_imp->dir / vars->metadata_description), mkt_significant));
            add_metadata_key(_imp->short_description);
        }

    if (! vars->metadata_homepage.empty())
        if ((_imp->dir / vars->metadata_homepage).exists())
        {
            _imp->homepage.reset(new ESimpleURIKey(_imp->environment, shared_from_this(), vars->metadata_homepage, vars->description_homepage,
                        file_contents(_imp->dir / vars->metadata_homepage), mkt_significant));
            add_metadata_key(_imp->homepage);
        }

    _imp->contents.reset(new EContentsKey(shared_from_this(), "CONTENTS", "Contents",
                _imp->dir / "CONTENTS", mkt_internal));
    add_metadata_key(_imp->contents);

    _imp->installed_time.reset(new ECTimeKey(shared_from_this(), "INSTALLED_TIME", "Installed time",
                _imp->dir / "CONTENTS", mkt_normal));
    add_metadata_key(_imp->installed_time);

    if ((_imp->dir / "REPOSITORY").exists())
    {
        _imp->source_origin.reset(new EStringKey(shared_from_this(), "REPOSITORY", "Source repository",
                    file_contents(_imp->dir / "REPOSITORY"), mkt_normal));
        add_metadata_key(_imp->source_origin);
    }

    if ((_imp->dir / "BINARY_REPOSITORY").exists())
    {
        _imp->binary_origin.reset(new EStringKey(shared_from_this(), "BINARY_REPOSITORY", "Binary repository",
                    file_contents(_imp->dir / "BINARY_REPOSITORY"), mkt_normal));
        add_metadata_key(_imp->binary_origin);
    }

    if ((_imp->dir / "ASFLAGS").exists())
    {
        _imp->asflags.reset(new EStringKey(shared_from_this(), "ASFLAGS", "ASFLAGS",
                    file_contents(_imp->dir / "ASFLAGS"), mkt_internal));
        add_metadata_key(_imp->asflags);
    }

    if ((_imp->dir / "CBUILD").exists())
    {
        _imp->cbuild.reset(new EStringKey(shared_from_this(), "CBUILD", "CBUILD",
                    file_contents(_imp->dir / "CBUILD"), mkt_internal));
        add_metadata_key(_imp->cbuild);
    }

    if ((_imp->dir / "CFLAGS").exists())
    {
        _imp->cflags.reset(new EStringKey(shared_from_this(), "CFLAGS", "CFLAGS",
                    file_contents(_imp->dir / "CFLAGS"), mkt_internal));
        add_metadata_key(_imp->cflags);
    }

    if ((_imp->dir / "CHOST").exists())
    {
        _imp->chost.reset(new EStringKey(shared_from_this(), "CHOST", "CHOST",
                    file_contents(_imp->dir / "CHOST"), mkt_internal));
        add_metadata_key(_imp->chost);
    }

    if ((_imp->dir / "CXXFLAGS").exists())
    {
        _imp->cxxflags.reset(new EStringKey(shared_from_this(), "CXXFLAGS", "CXXFLAGS",
                    file_contents(_imp->dir / "CXXFLAGS"), mkt_internal));
        add_metadata_key(_imp->cxxflags);
    }

    if ((_imp->dir / "LDFLAGS").exists())
    {
        _imp->ldflags.reset(new EStringKey(shared_from_this(), "LDFLAGS", "LDFLAGS",
                    file_contents(_imp->dir / "LDFLAGS"), mkt_internal));
        add_metadata_key(_imp->ldflags);
    }

    if ((_imp->dir / "PKGMANAGER").exists())
    {
        _imp->pkgmanager.reset(new EStringKey(shared_from_this(), "PKGMANAGER", "Installed using",
                    file_contents(_imp->dir / "PKGMANAGER"), mkt_normal));
        add_metadata_key(_imp->pkgmanager);
    }

    if ((_imp->dir / "VDB_FORMAT").exists())
    {
        _imp->vdb_format.reset(new EStringKey(shared_from_this(), "VDB_FORMAT", "VDB Format",
                    file_contents(_imp->dir / "VDB_FORMAT"), mkt_internal));
        add_metadata_key(_imp->vdb_format);
    }
}

void
VDBID::need_masks_added() const
{
}

const std::string
VDBID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            if (_imp->slot)
                return stringify(name()) + "-" + stringify(version()) + ":" + stringify(slot()) + "::" +
                    stringify(repository()->name());

            return stringify(name()) + "-" + stringify(version()) + "::" + stringify(repository()->name());

        case idcf_no_version:
            if (_imp->slot)
                return stringify(name()) + ":" + stringify(slot()) + "::" +
                    stringify(repository()->name());

            return stringify(name()) + "::" + stringify(repository()->name());

        case idcf_version:
            return stringify(version());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

const QualifiedPackageName
VDBID::name() const
{
    return _imp->name;
}

const VersionSpec
VDBID::version() const
{
    return _imp->version;
}

const SlotName
VDBID::slot() const
{
    Lock l(_imp->mutex);

    if (_imp->slot)
        return *_imp->slot;

    Context context("When finding SLOT for '" + stringify(name()) + "-" + stringify(version()) + "::"
            + stringify(repository()->name()) + "':");

    if ((_imp->dir / "SLOT").exists())
        _imp->slot.reset(new SlotName(file_contents(_imp->dir / "SLOT")));
    else
    {
        Log::get_instance()->message(ll_warning, lc_context) << "No SLOT entry in '" << _imp->dir << "', pretending '0'";
        _imp->slot.reset(new SlotName("0"));
    }

    return *_imp->slot;
}

const tr1::shared_ptr<const Repository>
VDBID::repository() const
{
    return _imp->repository;
}

const tr1::shared_ptr<const EAPI>
VDBID::eapi() const
{
    Lock l(_imp->mutex);

    if (_imp->eapi)
        return _imp->eapi;

    Context context("When finding EAPI for '" + canonical_form(idcf_full) + "':");

    if ((_imp->dir / "EAPI").exists())
        _imp->eapi = EAPIData::get_instance()->eapi_from_string(file_contents(_imp->dir / "EAPI"));
    else
    {
        Log::get_instance()->message(ll_debug, lc_context) << "No EAPI entry in '" << _imp->dir << "', pretending '"
            << _imp->environment->default_distribution() << "'";
        _imp->eapi = EAPIData::get_instance()->eapi_from_string(
                DistributionData::get_instance()->distribution_from_string(
                    _imp->environment->default_distribution())->default_ebuild_eapi_when_unspecified);
    }

    return _imp->eapi;
}

const tr1::shared_ptr<const MetadataPackageIDKey>
VDBID::virtual_for_key() const
{
    return tr1::shared_ptr<const MetadataPackageIDKey>();
}

const tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >
VDBID::keywords_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >();
}

const tr1::shared_ptr<const MetadataSetKey<UseFlagNameSet> >
VDBID::use_key() const
{
    need_keys_added();
    return _imp->use;
}

const tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> >
VDBID::iuse_key() const
{
    need_keys_added();
    return _imp->iuse;
}

const tr1::shared_ptr<const MetadataSetKey<Set<std::string> > >
VDBID::inherited_key() const
{
    need_keys_added();
    return _imp->inherited;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
VDBID::provide_key() const
{
    need_keys_added();
    return _imp->provide;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VDBID::build_dependencies_key() const
{
    need_keys_added();
    return _imp->build_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VDBID::run_dependencies_key() const
{
    need_keys_added();
    return _imp->run_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VDBID::post_dependencies_key() const
{
    need_keys_added();
    return _imp->post_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VDBID::suggested_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> >
VDBID::restrict_key() const
{
    need_keys_added();
    return _imp->restrictions;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
VDBID::fetches_key() const
{
    need_keys_added();
    return _imp->src_uri;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
VDBID::homepage_key() const
{
    need_keys_added();
    return _imp->homepage;
}

const tr1::shared_ptr<const MetadataStringKey>
VDBID::short_description_key() const
{
    need_keys_added();
    return _imp->short_description;
}

const tr1::shared_ptr<const MetadataStringKey>
VDBID::long_description_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataContentsKey>
VDBID::contents_key() const
{
    need_keys_added();
    return _imp->contents;
}

const tr1::shared_ptr<const MetadataTimeKey>
VDBID::installed_time_key() const
{
    need_keys_added();
    return _imp->installed_time;
}

const tr1::shared_ptr<const MetadataStringKey>
VDBID::source_origin_key() const
{
    need_keys_added();
    return _imp->source_origin;
}

const tr1::shared_ptr<const MetadataStringKey>
VDBID::binary_origin_key() const
{
    need_keys_added();
    return _imp->binary_origin;
}

const tr1::shared_ptr<const MetadataFSEntryKey>
VDBID::fs_location_key() const
{
    // Avoid loading whole metadata
    if (! _imp->fs_location)
    {
        Lock l(_imp->mutex);

        _imp->fs_location.reset(new EFSLocationKey(shared_from_this(), "VDBDIR", "VDB Location",
                    _imp->dir, mkt_internal));
        add_metadata_key(_imp->fs_location);
    }

    return _imp->fs_location;
}


bool
VDBID::arbitrary_less_than_comparison(const PackageID &) const
{
    return false;
}

std::size_t
VDBID::extra_hash_value() const
{
    return 0;
}

namespace
{
    struct SupportsActionQuery :
        ConstVisitor<SupportsActionTestVisitorTypes>
    {
        bool result;

        SupportsActionQuery() :
            result(false)
        {
        }

        void visit(const SupportsActionTest<InstalledAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
        }

        void visit(const SupportsActionTest<InfoAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<UninstallAction> &)
        {
            result = true;
        }
    };
}

bool
VDBID::supports_action(const SupportsActionTestBase & b) const
{
    SupportsActionQuery q;
    b.accept(q);
    return q.result;
}

namespace
{
    struct PerformAction :
        ConstVisitor<ActionVisitorTypes>
    {
        const tr1::shared_ptr<const erepository::ERepositoryID> id;

        PerformAction(const tr1::shared_ptr<const erepository::ERepositoryID> i) :
            id(i)
        {
        }

        void visit(const UninstallAction & a)
        {
            tr1::static_pointer_cast<const VDBRepository>(id->repository())->perform_uninstall(id,
                    a.options, false);
        }

        void visit(const InstalledAction &)
        {
        }

        void visit(const ConfigAction &)
        {
            tr1::static_pointer_cast<const VDBRepository>(id->repository())->perform_config(id);
        }

        void visit(const InfoAction &)
        {
            tr1::static_pointer_cast<const VDBRepository>(id->repository())->perform_info(id);
        }

        void visit(const InstallAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const PretendAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const FetchAction & a) PALUDIS_ATTRIBUTE((noreturn));
    };

    void PerformAction::visit(const InstallAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }

    void PerformAction::visit(const PretendAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }

    void PerformAction::visit(const FetchAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }
}

void
VDBID::perform_action(Action & a) const
{
    PerformAction b(shared_from_this());
    a.accept(b);
}

const tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >
VDBID::contains_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >();
}

const tr1::shared_ptr<const MetadataPackageIDKey>
VDBID::contained_in_key() const
{
    return tr1::shared_ptr<const MetadataPackageIDKey>();
}

