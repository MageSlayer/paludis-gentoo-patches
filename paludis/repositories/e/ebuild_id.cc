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

#include <paludis/repositories/e/ebuild_id.hh>
#include <paludis/repositories/e/ebuild_flat_metadata_cache.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/repositories/e/e_repository_entries.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/e_mask.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>

#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/save.hh>

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

        return std::string((std::istreambuf_iterator<char>(i)), std::istreambuf_iterator<char>());
    }
}

namespace paludis
{
    template <>
    struct Implementation<EbuildID>
    {
        Mutex mutex;

        const QualifiedPackageName name;
        const VersionSpec version;
        const Environment * const environment;
        const tr1::shared_ptr<const ERepository> repository;
        const FSEntry ebuild;
        tr1::shared_ptr<const SlotName> slot;
        mutable tr1::shared_ptr<const EAPI> eapi;
        const std::string guessed_eapi;
        const time_t master_mtime;
        const tr1::shared_ptr<const EclassMtimes> eclass_mtimes;
        mutable bool has_keys;
        mutable bool has_masks;

        mutable tr1::shared_ptr<const EFSLocationKey> fs_location;
        mutable tr1::shared_ptr<const EStringKey> short_description;
        mutable tr1::shared_ptr<const EDependenciesKey> build_dependencies;
        mutable tr1::shared_ptr<const EDependenciesKey> run_dependencies;
        mutable tr1::shared_ptr<const EDependenciesKey> post_dependencies;
        mutable tr1::shared_ptr<const EProvideKey> provide;
        mutable tr1::shared_ptr<const ERestrictKey> restrictions;
        mutable tr1::shared_ptr<const EFetchableURIKey> src_uri;
        mutable tr1::shared_ptr<const ESimpleURIKey> homepage;
        mutable tr1::shared_ptr<const ELicenseKey> license;
        mutable tr1::shared_ptr<const EKeywordsKey> keywords;
        mutable tr1::shared_ptr<const EIUseKey> iuse;
        mutable tr1::shared_ptr<const EInheritedKey> inherited;
        mutable tr1::shared_ptr<EMutableRepositoryMaskInfoKey> repository_mask;
        mutable tr1::shared_ptr<EMutableRepositoryMaskInfoKey> profile_mask;

        Implementation(const QualifiedPackageName & q, const VersionSpec & v,
                const Environment * const e,
                const tr1::shared_ptr<const ERepository> r, const FSEntry & f, const std::string & g,
                const time_t t, const tr1::shared_ptr<const EclassMtimes> & m) :
            name(q),
            version(v),
            environment(e),
            repository(r),
            ebuild(f),
            guessed_eapi(g),
            master_mtime(t),
            eclass_mtimes(m),
            has_keys(false),
            has_masks(false)
        {
        }
    };
}

EbuildID::EbuildID(const QualifiedPackageName & q, const VersionSpec & v,
        const Environment * const e,
        const tr1::shared_ptr<const ERepository> & r,
        const FSEntry & f,
        const std::string & g,
        const time_t t,
        const tr1::shared_ptr<const EclassMtimes> & m) :
    PrivateImplementationPattern<EbuildID>(new Implementation<EbuildID>(q, v, e, r, f, g, t, m)),
    _imp(PrivateImplementationPattern<EbuildID>::_imp.get())
{
}

EbuildID::~EbuildID()
{
}

void
EbuildID::need_keys_added() const
{
    Lock l(_imp->mutex);

    if (_imp->has_keys)
        return;

    _imp->has_keys = true;

    // fs_location key could have been loaded by the ::fs_location_key() already.
    if (! _imp->fs_location)
    {
        _imp->fs_location.reset(new EFSLocationKey(shared_from_this(), "EBUILD", "Ebuild Location",
                    _imp->ebuild, mkt_internal));
        add_metadata_key(_imp->fs_location);
    }

    Context context("When generating metadata for ID '" + canonical_form(idcf_full) + "':");

    FSEntry cache_file(_imp->repository->params().cache);
    cache_file /= stringify(name().category);
    cache_file /= stringify(name().package) + "-" + stringify(version());

    FSEntry write_cache_file(_imp->repository->params().write_cache);
    if (_imp->repository->params().append_repository_name_to_write_cache)
        write_cache_file /= stringify(repository()->name());
    write_cache_file /= stringify(name().category);
    write_cache_file /= stringify(name().package) + "-" + stringify(version());

    bool ok(false);
    if (_imp->repository->params().cache.basename() != "empty")
    {
        EbuildFlatMetadataCache metadata_cache(cache_file, _imp->ebuild, _imp->master_mtime, _imp->eclass_mtimes, false);
        if (metadata_cache.load(shared_from_this()))
            ok = true;
    }

    if ((! ok) && _imp->repository->params().write_cache.basename() != "empty")
    {
        EbuildFlatMetadataCache write_metadata_cache(write_cache_file, _imp->ebuild, _imp->master_mtime, _imp->eclass_mtimes, true);
        if (write_metadata_cache.load(shared_from_this()))
            ok = true;
        else if (write_cache_file.exists())
        {
            try
            {
                write_cache_file.unlink();
            }
            catch (const FSError &)
            {
                // the attempt to write a fresh file will produce a
                // warning, no need to be too noisy
            }
        }
    }

    if (! ok)
    {
        if (_imp->repository->params().cache.basename() != "empty")
            Log::get_instance()->message(ll_qa, lc_no_context) << "No usable cache entry for '" + canonical_form(idcf_full);

        std::string eapi_str(_imp->guessed_eapi);
        if (eapi_str.empty())
            eapi_str = _imp->repository->params().eapi_when_unknown;
        _imp->eapi = EAPIData::get_instance()->eapi_from_string(eapi_str);

        if (_imp->eapi->supported)
        {
            Log::get_instance()->message(ll_debug, lc_context) << "Generating metadata command for '"
                << canonical_form(idcf_full) << "' using EAPI '" << _imp->eapi->name << "'";

            EAPIPhases phases(_imp->eapi->supported->ebuild_phases->ebuild_metadata);

            int count(std::distance(phases.begin_phases(), phases.end_phases()));
            if (1 != count)
                throw EAPIConfigurationError("EAPI '" + _imp->eapi->name + "' defines "
                        + (count == 0 ? "no" : stringify(count)) + " ebuild variable phases but expected exactly one");

            EbuildMetadataCommand cmd(EbuildCommandParams::create()
                    .environment(_imp->environment)
                    .package_id(shared_from_this())
                    .ebuild_dir(_imp->repository->layout()->package_directory(name()))
                    .ebuild_file(_imp->ebuild)
                    .files_dir(_imp->repository->layout()->package_directory(name()) / "files")
                    .eclassdirs(_imp->repository->params().eclassdirs)
                    .exlibsdirs(_imp->repository->layout()->exlibsdirs(name()))
                    .portdir(_imp->repository->params().master_repository ? _imp->repository->params().master_repository->params().location :
                        _imp->repository->params().location)
                    .distdir(_imp->repository->params().distdir)
                    .builddir(_imp->repository->params().builddir)
                    .commands(join(phases.begin_phases()->begin_commands(), phases.begin_phases()->end_commands(), " "))
                    .sandbox(phases.begin_phases()->option("sandbox"))
                    .userpriv(phases.begin_phases()->option("userpriv")));

            if (! cmd())
                Log::get_instance()->message(ll_warning, lc_no_context) << "No usable metadata for '" +
                    stringify(canonical_form(idcf_full)) << "'";

            cmd.load(shared_from_this());

            Log::get_instance()->message(ll_debug, lc_context) << "Generated metadata for '"
                << canonical_form(idcf_full) << "' has EAPI '" << _imp->eapi->name << "'";

            if (_imp->repository->params().write_cache.basename() != "empty" && _imp->eapi->supported)
            {
                EbuildFlatMetadataCache metadata_cache(write_cache_file, _imp->ebuild, _imp->master_mtime,
                        _imp->eclass_mtimes, false);
                metadata_cache.save(shared_from_this());
            }
        }
        else
        {
            Log::get_instance()->message(ll_debug, lc_context) << "Can't run metadata command for '"
                << canonical_form(idcf_full) << "' because EAPI '" << _imp->eapi->name << "' is unknown";
        }
    }

    add_metadata_key(make_shared_ptr(new EStringKey(shared_from_this(), "EAPI", "EAPI", _imp->eapi->name, mkt_internal)));

    _imp->repository_mask = make_shared_ptr(new EMutableRepositoryMaskInfoKey(shared_from_this(), "repository_mask", "Repository masked",
        tr1::static_pointer_cast<const ERepository>(repository())->repository_masked(*this), mkt_internal));
    add_metadata_key(_imp->repository_mask);
    _imp->profile_mask = make_shared_ptr(new EMutableRepositoryMaskInfoKey(shared_from_this(), "profile_mask", "Profile masked",
        tr1::static_pointer_cast<const ERepository>(repository())->profile()->profile_masked(*this), mkt_internal));
    add_metadata_key(_imp->profile_mask);
}

namespace
{
    struct LicenceChecker :
        ConstVisitor<LicenseSpecTree>,
        ConstVisitor<LicenseSpecTree>::VisitConstSequence<LicenceChecker, AllDepSpec>
    {
        using ConstVisitor<LicenseSpecTree>::VisitConstSequence<LicenceChecker, AllDepSpec>::visit_sequence;

        bool ok;
        const Environment * const env;
        bool  (Environment::* const func) (const std::string &, const PackageID &) const;
        const PackageID * const id;

        LicenceChecker(const Environment * const e,
                bool (Environment::* const f) (const std::string &, const PackageID &) const,
                const PackageID * const d) :
            ok(true),
            env(e),
            func(f),
            id(d)
        {
        }

        void visit_sequence(const AnyDepSpec &,
                LicenseSpecTree::ConstSequenceIterator begin,
                LicenseSpecTree::ConstSequenceIterator end)
        {
            bool local_ok(false);

            if (begin == end)
                local_ok = true;
            else
            {
                for ( ; begin != end ; ++begin)
                {
                    Save<bool> save_ok(&ok, true);
                    begin->accept(*this);
                    local_ok |= ok;
                }
            }

            ok &= local_ok;
        }

        void visit_sequence(const UseDepSpec & spec,
                LicenseSpecTree::ConstSequenceIterator begin,
                LicenseSpecTree::ConstSequenceIterator end)
        {
            if (env->query_use(spec.flag(), *id))
                std::for_each(begin, end, accept_visitor(*this));
        }

        void visit_leaf(const LicenseDepSpec & spec)
        {
            if (! (env->*func)(spec.text(), *id))
                ok = false;
        }
    };
}

void
EbuildID::need_masks_added() const
{
    Lock l(_imp->mutex);

    if (_imp->has_masks)
        return;

    _imp->has_masks = true;

    Context context("When generating masks for ID '" + canonical_form(idcf_full) + "':");

    if (! eapi()->supported)
    {
        add_mask(make_shared_ptr(new EUnsupportedMask('E', "eapi", eapi()->name)));
        return;
    }

    if (keywords_key())
        if (! _imp->environment->accept_keywords(keywords_key()->value(), *this))
            add_mask(make_shared_ptr(new EUnacceptedMask('K', "keywords", keywords_key())));

    if (license_key())
    {
        LicenceChecker c(_imp->environment, &Environment::accept_license, this);
        license_key()->value()->accept(c);
        if (! c.ok)
            add_mask(make_shared_ptr(new EUnacceptedMask('L', "license", license_key())));
    }

    if (! _imp->environment->unmasked_by_user(*this))
    {
        /* repo unless user */
        if (_imp->repository_mask->value())
            add_mask(make_shared_ptr(new ERepositoryMask('R', "repository", _imp->repository_mask)));

        /* profile unless user */
        if (_imp->profile_mask->value())
            add_mask(make_shared_ptr(new ERepositoryMask('P', "profile", _imp->profile_mask)));

        /* user */
        tr1::shared_ptr<const Mask> user_mask(_imp->environment->mask_for_user(*this));
        if (user_mask)
            add_mask(user_mask);
    }

    /* break portage */
    tr1::shared_ptr<const Mask> breaks_mask(_imp->environment->mask_for_breakage(*this));
    if (breaks_mask)
        add_mask(breaks_mask);
}

void
EbuildID::invalidate_masks() const
{
    Lock l(_imp->mutex);

    if (! _imp->has_masks)
        return;

    _imp->has_masks = false;
    PackageID::invalidate_masks();
    _imp->repository_mask->set_value(tr1::static_pointer_cast<const ERepository>(repository())->repository_masked(*this));
    _imp->profile_mask->set_value(tr1::static_pointer_cast<const ERepository>(repository())->profile()->profile_masked(*this));
}

const std::string
EbuildID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            if (_imp->slot)
                return stringify(name()) + "-" + stringify(version()) + ":" + stringify(*_imp->slot) +
                    "::" + stringify(repository()->name());

            return stringify(name()) + "-" + stringify(version()) + "::" + stringify(repository()->name());

        case idcf_no_version:
            if (_imp->slot)
                return stringify(name()) + ":" + stringify(*_imp->slot) +
                    "::" + stringify(repository()->name());

            return stringify(name()) + "::" + stringify(repository()->name());

        case idcf_version:
            return stringify(version());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

const QualifiedPackageName
EbuildID::name() const
{
    return _imp->name;
}

const VersionSpec
EbuildID::version() const
{
    return _imp->version;
}

const SlotName
EbuildID::slot() const
{
    if (_imp->slot)
        return *_imp->slot;

    need_keys_added();

    if (! _imp->slot)
        throw InternalError(PALUDIS_HERE, "_imp->slot still not set");

    return *_imp->slot;
}

const tr1::shared_ptr<const Repository>
EbuildID::repository() const
{
    return _imp->repository;
}

const tr1::shared_ptr<const EAPI>
EbuildID::eapi() const
{
    if (_imp->eapi)
        return _imp->eapi;

    need_keys_added();

    if (! _imp->eapi)
        throw InternalError(PALUDIS_HERE, "_imp->eapi still not set");

    return _imp->eapi;
}

const tr1::shared_ptr<const MetadataPackageIDKey>
EbuildID::virtual_for_key() const
{
    return tr1::shared_ptr<const MetadataPackageIDKey>();
}

const tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >
EbuildID::keywords_key() const
{
    need_keys_added();
    return _imp->keywords;
}

const tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> >
EbuildID::iuse_key() const
{
    need_keys_added();
    return _imp->iuse;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >
EbuildID::license_key() const
{
    need_keys_added();
    return _imp->license;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
EbuildID::provide_key() const
{
    need_keys_added();
    return _imp->provide;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EbuildID::build_dependencies_key() const
{
    need_keys_added();
    return _imp->build_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EbuildID::run_dependencies_key() const
{
    need_keys_added();
    return _imp->run_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EbuildID::post_dependencies_key() const
{
    need_keys_added();
    return _imp->post_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EbuildID::suggested_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> >
EbuildID::restrict_key() const
{
    need_keys_added();
    return _imp->restrictions;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
EbuildID::fetches_key() const
{
    need_keys_added();
    return _imp->src_uri;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
EbuildID::homepage_key() const
{
    need_keys_added();
    return _imp->homepage;
}

const tr1::shared_ptr<const MetadataStringKey>
EbuildID::short_description_key() const
{
    need_keys_added();
    return _imp->short_description;
}

const tr1::shared_ptr<const MetadataStringKey>
EbuildID::long_description_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataContentsKey>
EbuildID::contents_key() const
{
    return tr1::shared_ptr<const MetadataContentsKey>();
}

const tr1::shared_ptr<const MetadataTimeKey>
EbuildID::installed_time_key() const
{
    return tr1::shared_ptr<const MetadataTimeKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
EbuildID::source_origin_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
EbuildID::binary_origin_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataSetKey<Set<std::string> > >
EbuildID::inherited_key() const
{
    return _imp->inherited;
}

const tr1::shared_ptr<const MetadataFSEntryKey>
EbuildID::fs_location_key() const
{
    // Avoid loading whole metadata
    if (! _imp->fs_location)
    {
        Lock l(_imp->mutex);

        _imp->fs_location.reset(new EFSLocationKey(shared_from_this(), "EBUILD", "Ebuild Location",
                    _imp->ebuild, mkt_internal));
        add_metadata_key(_imp->fs_location);
    }

    return _imp->fs_location;
}

bool
EbuildID::arbitrary_less_than_comparison(const PackageID &) const
{
    return false;
}

std::size_t
EbuildID::extra_hash_value() const
{
    return 0;
}

void
EbuildID::set_eapi(const std::string & s) const
{
    Lock l(_imp->mutex);
    _imp->eapi = EAPIData::get_instance()->eapi_from_string(s);
}

void
EbuildID::set_slot(const SlotName & s) const
{
    Lock l(_imp->mutex);
    _imp->slot.reset(new SlotName(s));
}

tr1::shared_ptr<const ERepository>
EbuildID::e_repository() const
{
    return _imp->repository;
}

void
EbuildID::load_short_description(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->short_description.reset(new EStringKey(shared_from_this(), r, h, v, mkt_significant));
    add_metadata_key(_imp->short_description);
}

void
EbuildID::load_build_depend(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->build_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), r, h, v, mkt_dependencies));
    add_metadata_key(_imp->build_dependencies);
}

void
EbuildID::load_run_depend(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->run_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), r, h, v, mkt_dependencies));
    add_metadata_key(_imp->run_dependencies);
}

void
EbuildID::load_post_depend(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->post_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), r, h, v, mkt_dependencies));
    add_metadata_key(_imp->post_dependencies);
}

void
EbuildID::load_src_uri(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->src_uri.reset(new EFetchableURIKey(_imp->environment, shared_from_this(), r, h, v, mkt_dependencies));
    add_metadata_key(_imp->src_uri);
}

void
EbuildID::load_homepage(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->homepage.reset(new ESimpleURIKey(_imp->environment, shared_from_this(), r, h, v, mkt_significant));
    add_metadata_key(_imp->homepage);
}

void
EbuildID::load_license(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->license.reset(new ELicenseKey(_imp->environment, shared_from_this(), r, h, v, mkt_internal));
    add_metadata_key(_imp->license);
}

void
EbuildID::load_restrict(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->restrictions.reset(new ERestrictKey(_imp->environment, shared_from_this(), r, h, v, mkt_internal));
    add_metadata_key(_imp->restrictions);
}

void
EbuildID::load_provide(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->provide.reset(new EProvideKey(_imp->environment, shared_from_this(), r, h, v, mkt_dependencies));
    add_metadata_key(_imp->provide);
}

void
EbuildID::load_iuse(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->iuse.reset(new EIUseKey(_imp->environment, shared_from_this(), r, h, v, mkt_normal));
    add_metadata_key(_imp->iuse);
}

void
EbuildID::load_keywords(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->keywords.reset(new EKeywordsKey(_imp->environment, shared_from_this(), r, h, v, mkt_internal));
    add_metadata_key(_imp->keywords);
}

void
EbuildID::load_inherited(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->inherited.reset(new EInheritedKey(shared_from_this(), r, h, v, mkt_internal));
    add_metadata_key(_imp->inherited);
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
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<InfoAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<UninstallAction> &)
        {
        }
    };
}

bool
EbuildID::supports_action(const SupportsActionTestBase & b) const
{
    if (! eapi()->supported)
        return false;

    SupportsActionQuery q;
    b.accept(q);
    return q.result;
}

namespace
{
    struct PerformAction :
        ConstVisitor<ActionVisitorTypes>
    {
        const tr1::shared_ptr<const PackageID> id;

        PerformAction(const tr1::shared_ptr<const PackageID> i) :
            id(i)
        {
        }

        void visit(const InstallAction & a)
        {
            tr1::static_pointer_cast<const ERepository>(id->repository())->entries()->install(
                    tr1::static_pointer_cast<const ERepositoryID>(id),
                    a.options,
                    tr1::static_pointer_cast<const ERepository>(id->repository())->profile());
        }

        void visit(const FetchAction & a)
        {
            tr1::static_pointer_cast<const ERepository>(id->repository())->entries()->fetch(
                    tr1::static_pointer_cast<const ERepositoryID>(id),
                    a.options,
                    tr1::static_pointer_cast<const ERepository>(id->repository())->profile());
        }

        void visit(const PretendAction &)
        {
            tr1::static_pointer_cast<const ERepository>(id->repository())->entries()->pretend(
                    tr1::static_pointer_cast<const ERepositoryID>(id),
                    tr1::static_pointer_cast<const ERepository>(id->repository())->profile());
        }

        void visit(const InfoAction &)
        {
            tr1::static_pointer_cast<const ERepository>(id->repository())->entries()->info(
                    tr1::static_pointer_cast<const ERepositoryID>(id),
                    tr1::static_pointer_cast<const ERepository>(id->repository())->profile());
        }

        void visit(const InstalledAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const UninstallAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const ConfigAction & a) PALUDIS_ATTRIBUTE((noreturn));
    };

    void PerformAction::visit(const InstalledAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }

    void PerformAction::visit(const UninstallAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }

    void PerformAction::visit(const ConfigAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }
}

void
EbuildID::perform_action(Action & a) const
{
    if (! eapi()->supported)
        throw UnsupportedActionError(*this, a);

    PerformAction b(shared_from_this());
    a.accept(b);
}

const tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >
EbuildID::contains_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >();
}

const tr1::shared_ptr<const MetadataPackageIDKey>
EbuildID::contained_in_key() const
{
    return tr1::shared_ptr<const MetadataPackageIDKey>();
}

