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

#include <paludis/repositories/e/ebuild_id.hh>
#include <paludis/repositories/e/ebuild_flat_metadata_cache.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/repositories/e/e_repository_entries.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/e_mask.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/manifest2_reader.hh>

#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>

#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/save.hh>
#include <paludis/util/make_named_values.hh>

#include <iterator>
#include <fstream>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Implementation<EbuildID>
    {
        Mutex mutex;

        const QualifiedPackageName name;
        const VersionSpec version;
        const Environment * const environment;
        const std::tr1::shared_ptr<const ERepository> repository;
        const FSEntry ebuild;
        std::tr1::shared_ptr<const SlotName> slot;
        mutable std::tr1::shared_ptr<const EAPI> eapi;
        const std::string guessed_eapi;
        const time_t master_mtime;
        const std::tr1::shared_ptr<const EclassMtimes> eclass_mtimes;
        mutable bool has_keys;
        mutable bool has_masks;

        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<FSEntry> > fs_location;
        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<std::string> > short_description;
        mutable std::tr1::shared_ptr<const EDependenciesKey> build_dependencies;
        mutable std::tr1::shared_ptr<const EDependenciesKey> run_dependencies;
        mutable std::tr1::shared_ptr<const EDependenciesKey> post_dependencies;
        mutable std::tr1::shared_ptr<const EProvideKey> provide;
        mutable std::tr1::shared_ptr<const EPlainTextSpecKey> restrictions;
        mutable std::tr1::shared_ptr<const EFetchableURIKey> src_uri;
        mutable std::tr1::shared_ptr<const ESimpleURIKey> homepage;
        mutable std::tr1::shared_ptr<const ELicenseKey> license;
        mutable std::tr1::shared_ptr<const EKeywordsKey> keywords;
        mutable std::tr1::shared_ptr<const EIUseKey> iuse;
        mutable std::tr1::shared_ptr<const EInheritedKey> inherited;
        mutable std::tr1::shared_ptr<const EUseKey> use;
        mutable std::tr1::shared_ptr<EMutableRepositoryMaskInfoKey> repository_mask;
        mutable std::tr1::shared_ptr<EMutableRepositoryMaskInfoKey> profile_mask;

        std::tr1::shared_ptr<DependencyLabelSequence> build_dependencies_labels;
        std::tr1::shared_ptr<DependencyLabelSequence> run_dependencies_labels;
        std::tr1::shared_ptr<DependencyLabelSequence> post_dependencies_labels;

        Implementation(const QualifiedPackageName & q, const VersionSpec & v,
                const Environment * const e,
                const std::tr1::shared_ptr<const ERepository> r, const FSEntry & f, const std::string & g,
                const time_t t, const std::tr1::shared_ptr<const EclassMtimes> & m) :
            name(q),
            version(v),
            environment(e),
            repository(r),
            ebuild(f),
            guessed_eapi(g),
            master_mtime(t),
            eclass_mtimes(m),
            has_keys(false),
            has_masks(false),
            build_dependencies_labels(new DependencyLabelSequence),
            run_dependencies_labels(new DependencyLabelSequence),
            post_dependencies_labels(new DependencyLabelSequence)
        {
            build_dependencies_labels->push_back(make_shared_ptr(new DependencyBuildLabel("DEPEND")));
            run_dependencies_labels->push_back(make_shared_ptr(new DependencyRunLabel("RDEPEND")));
            post_dependencies_labels->push_back(make_shared_ptr(new DependencyPostLabel("PDEPEND")));
        }
    };
}

EbuildID::EbuildID(const QualifiedPackageName & q, const VersionSpec & v,
        const Environment * const e,
        const std::tr1::shared_ptr<const ERepository> & r,
        const FSEntry & f,
        const std::string & g,
        const time_t t,
        const std::tr1::shared_ptr<const EclassMtimes> & m) :
    PrivateImplementationPattern<EbuildID>(new Implementation<EbuildID>(q, v, e, r, f, g, t, m)),
    _imp(PrivateImplementationPattern<EbuildID>::_imp)
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
        _imp->fs_location.reset(new LiteralMetadataValueKey<FSEntry> ("EBUILD", "Ebuild Location",
                    mkt_internal, _imp->ebuild));
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
        EbuildFlatMetadataCache metadata_cache(_imp->environment, cache_file, _imp->ebuild, _imp->master_mtime, _imp->eclass_mtimes, false);
        if (metadata_cache.load(shared_from_this()))
            ok = true;
    }

    if ((! ok) && _imp->repository->params().write_cache.basename() != "empty")
    {
        EbuildFlatMetadataCache write_metadata_cache(_imp->environment,
                write_cache_file, _imp->ebuild, _imp->master_mtime, _imp->eclass_mtimes, true);
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
            Log::get_instance()->message("e.ebuild.cache.no_usable", ll_qa, lc_no_context)
                << "No usable cache entry for '" + canonical_form(idcf_full);

        std::string eapi_str(_imp->guessed_eapi);
        if (eapi_str.empty())
            eapi_str = _imp->repository->params().eapi_when_unknown;
        _imp->eapi = EAPIData::get_instance()->eapi_from_string(eapi_str);

        if (_imp->eapi->supported())
        {
            Log::get_instance()->message("e.ebuild.metadata.using_eapi", ll_debug, lc_context) << "Generating metadata command for '"
                << canonical_form(idcf_full) << "' using EAPI '" << _imp->eapi->name() << "'";

            EAPIPhases phases(_imp->eapi->supported()->ebuild_phases()->ebuild_metadata());

            int count(std::distance(phases.begin_phases(), phases.end_phases()));
            if (1 != count)
                throw EAPIConfigurationError("EAPI '" + _imp->eapi->name() + "' defines "
                        + (count == 0 ? "no" : stringify(count)) + " ebuild variable phases but expected exactly one");

            EbuildMetadataCommand cmd(make_named_values<EbuildCommandParams>(
                    value_for<n::builddir>(_imp->repository->params().builddir),
                    value_for<n::commands>(join(phases.begin_phases()->begin_commands(), phases.begin_phases()->end_commands(), " ")),
                    value_for<n::distdir>(_imp->repository->params().distdir),
                    value_for<n::ebuild_dir>(_imp->repository->layout()->package_directory(name())),
                    value_for<n::ebuild_file>(_imp->ebuild),
                    value_for<n::eclassdirs>(_imp->repository->params().eclassdirs),
                    value_for<n::environment>(_imp->environment),
                    value_for<n::exlibsdirs>(_imp->repository->layout()->exlibsdirs(name())),
                    value_for<n::files_dir>(_imp->repository->layout()->package_directory(name()) / "files"),
                    value_for<n::package_id>(shared_from_this()),
                    value_for<n::portdir>(_imp->repository->params().master_repository ?
                        _imp->repository->params().master_repository->params().location : _imp->repository->params().location),
                    value_for<n::sandbox>(phases.begin_phases()->option("sandbox")),
                    value_for<n::userpriv>(phases.begin_phases()->option("userpriv"))
                    ));

            if (! cmd())
                Log::get_instance()->message("e.ebuild.metadata.unusable", ll_warning, lc_no_context) << "No usable metadata for '" +
                    stringify(canonical_form(idcf_full)) << "'";

            cmd.load(shared_from_this());

            Log::get_instance()->message("e.ebuild.metadata.generated_eapi", ll_debug, lc_context) << "Generated metadata for '"
                << canonical_form(idcf_full) << "' has EAPI '" << _imp->eapi->name() << "'";

            if (_imp->repository->params().write_cache.basename() != "empty" && _imp->eapi->supported())
            {
                EbuildFlatMetadataCache metadata_cache(_imp->environment, write_cache_file, _imp->ebuild, _imp->master_mtime,
                        _imp->eclass_mtimes, false);
                metadata_cache.save(shared_from_this());
            }
        }
        else
        {
            Log::get_instance()->message("e.ebuild.metadata.unknown_eapi", ll_debug, lc_context) << "Can't run metadata command for '"
                << canonical_form(idcf_full) << "' because EAPI '" << _imp->eapi->name() << "' is unknown";
        }
    }

    add_metadata_key(make_shared_ptr(new LiteralMetadataValueKey<std::string>("EAPI", "EAPI", mkt_internal, _imp->eapi->name())));

    _imp->repository_mask = make_shared_ptr(new EMutableRepositoryMaskInfoKey(shared_from_this(), "repository_mask", "Repository masked",
        std::tr1::static_pointer_cast<const ERepository>(repository())->repository_masked(*this), mkt_internal));
    add_metadata_key(_imp->repository_mask);
    _imp->profile_mask = make_shared_ptr(new EMutableRepositoryMaskInfoKey(shared_from_this(), "profile_mask", "Profile masked",
        std::tr1::static_pointer_cast<const ERepository>(repository())->profile()->profile_masked(*this), mkt_internal));
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

        void visit_sequence(const ConditionalDepSpec & spec,
                LicenseSpecTree::ConstSequenceIterator begin,
                LicenseSpecTree::ConstSequenceIterator end)
        {
            if (spec.condition_met())
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

    if (! eapi()->supported())
    {
        add_mask(make_shared_ptr(new EUnsupportedMask('E', "eapi", eapi()->name())));
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
        std::tr1::shared_ptr<const Mask> user_mask(_imp->environment->mask_for_user(*this));
        if (user_mask)
            add_mask(user_mask);
    }

    /* break portage */
    std::tr1::shared_ptr<const Mask> breaks_mask(_imp->environment->mask_for_breakage(*this));
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
    _imp->repository_mask->set_value(std::tr1::static_pointer_cast<const ERepository>(repository())->repository_masked(*this));
    _imp->profile_mask->set_value(std::tr1::static_pointer_cast<const ERepository>(repository())->profile()->profile_masked(*this));
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

const std::tr1::shared_ptr<const Repository>
EbuildID::repository() const
{
    return _imp->repository;
}

const std::tr1::shared_ptr<const EAPI>
EbuildID::eapi() const
{
    if (_imp->eapi)
        return _imp->eapi;

    need_keys_added();

    if (! _imp->eapi)
        throw InternalError(PALUDIS_HERE, "_imp->eapi still not set");

    return _imp->eapi;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
EbuildID::virtual_for_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
EbuildID::keywords_key() const
{
    need_keys_added();
    return _imp->keywords;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<IUseFlagSet> >
EbuildID::iuse_key() const
{
    need_keys_added();
    return _imp->iuse;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<UseFlagNameSet> >
EbuildID::use_key() const
{
    need_keys_added();
    return _imp->use;
}

const std::tr1::shared_ptr<const MetadataValueKey<bool> >
EbuildID::transient_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<bool> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >
EbuildID::license_key() const
{
    need_keys_added();
    return _imp->license;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
EbuildID::provide_key() const
{
    need_keys_added();
    return _imp->provide;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EbuildID::build_dependencies_key() const
{
    need_keys_added();
    return _imp->build_dependencies;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EbuildID::run_dependencies_key() const
{
    need_keys_added();
    return _imp->run_dependencies;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EbuildID::post_dependencies_key() const
{
    need_keys_added();
    return _imp->post_dependencies;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EbuildID::suggested_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EbuildID::restrict_key() const
{
    need_keys_added();
    return _imp->restrictions;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
EbuildID::fetches_key() const
{
    need_keys_added();
    return _imp->src_uri;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
EbuildID::homepage_key() const
{
    need_keys_added();
    return _imp->homepage;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
EbuildID::short_description_key() const
{
    need_keys_added();
    return _imp->short_description;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
EbuildID::long_description_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
EbuildID::contents_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >();
}

const std::tr1::shared_ptr<const MetadataTimeKey>
EbuildID::installed_time_key() const
{
    return std::tr1::shared_ptr<const MetadataTimeKey>();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::from_repositories_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::inherited_key() const
{
    return _imp->inherited;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
EbuildID::fs_location_key() const
{
    // Avoid loading whole metadata
    if (! _imp->fs_location)
    {
        Lock l(_imp->mutex);

        _imp->fs_location.reset(new LiteralMetadataValueKey<FSEntry> ("EBUILD", "Ebuild Location", mkt_internal, _imp->ebuild));
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

std::tr1::shared_ptr<const ERepository>
EbuildID::e_repository() const
{
    return _imp->repository;
}

void
EbuildID::load_short_description(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->short_description.reset(new LiteralMetadataValueKey<std::string> (r, h, mkt_significant, v));
    add_metadata_key(_imp->short_description);
}

void
EbuildID::load_build_depend(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->build_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), r, h, v,
                _imp->build_dependencies_labels, mkt_dependencies));
    add_metadata_key(_imp->build_dependencies);
}

void
EbuildID::load_run_depend(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->run_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), r, h, v,
                _imp->run_dependencies_labels, mkt_dependencies));
    add_metadata_key(_imp->run_dependencies);
}

void
EbuildID::load_post_depend(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->post_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), r, h, v,
                _imp->post_dependencies_labels, mkt_dependencies));
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
    _imp->restrictions.reset(new EPlainTextSpecKey(_imp->environment, shared_from_this(), r, h, v, mkt_internal));
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
EbuildID::load_use(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->use.reset(new EUseKey(_imp->environment, shared_from_this(), r, h, v, mkt_internal));
    add_metadata_key(_imp->use);
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

        void visit(const SupportsActionTest<PretendFetchAction> &)
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
    SupportsActionQuery q;
    b.accept(q);

    return q.result && eapi()->supported();
}

namespace
{
    struct PerformAction :
        Visitor<ActionVisitorTypes>
    {
        const std::tr1::shared_ptr<const PackageID> id;

        PerformAction(const std::tr1::shared_ptr<const PackageID> i) :
            id(i)
        {
        }

        void visit(InstallAction & a)
        {
            std::tr1::static_pointer_cast<const ERepository>(id->repository())->entries()->install(
                    std::tr1::static_pointer_cast<const ERepositoryID>(id),
                    a.options,
                    std::tr1::static_pointer_cast<const ERepository>(id->repository())->profile());
        }

        void visit(FetchAction & a)
        {
            std::tr1::static_pointer_cast<const ERepository>(id->repository())->entries()->fetch(
                    std::tr1::static_pointer_cast<const ERepositoryID>(id),
                    a.options,
                    std::tr1::static_pointer_cast<const ERepository>(id->repository())->profile());
        }

        void visit(PretendFetchAction & a)
        {
            std::tr1::static_pointer_cast<const ERepository>(id->repository())->entries()->pretend_fetch(
                    std::tr1::static_pointer_cast<const ERepositoryID>(id),
                    a,
                    std::tr1::static_pointer_cast<const ERepository>(id->repository())->profile());
        }

        void visit(PretendAction & action)
        {
            if (! std::tr1::static_pointer_cast<const ERepository>(id->repository())->entries()->pretend(
                        std::tr1::static_pointer_cast<const ERepositoryID>(id),
                        std::tr1::static_pointer_cast<const ERepository>(id->repository())->profile()))
                action.set_failed();
        }

        void visit(InfoAction &)
        {
            std::tr1::static_pointer_cast<const ERepository>(id->repository())->entries()->info(
                    std::tr1::static_pointer_cast<const ERepositoryID>(id),
                    std::tr1::static_pointer_cast<const ERepository>(id->repository())->profile());
        }

        void visit(InstalledAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(UninstallAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(ConfigAction & a) PALUDIS_ATTRIBUTE((noreturn));
    };

    void PerformAction::visit(InstalledAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }

    void PerformAction::visit(UninstallAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }

    void PerformAction::visit(ConfigAction & a)
    {
        throw UnsupportedActionError(*id, a);
    }
}

void
EbuildID::perform_action(Action & a) const
{
    if (! eapi()->supported())
        throw UnsupportedActionError(*this, a);

    PerformAction b(shared_from_this());
    a.accept(b);
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
EbuildID::contains_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
EbuildID::contained_in_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

