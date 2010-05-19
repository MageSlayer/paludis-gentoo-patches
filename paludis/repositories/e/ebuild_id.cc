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

#include <paludis/repositories/e/ebuild_id.hh>
#include <paludis/repositories/e/ebuild_flat_metadata_cache.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/e_choices_key.hh>
#include <paludis/repositories/e/e_mask.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/manifest2_reader.hh>
#include <paludis/repositories/e/e_choice_value.hh>
#include <paludis/repositories/e/metadata_xml.hh>

#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/elike_choices.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/notifier_callback.hh>

#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/save.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/indirect_iterator-impl.hh>

#include <set>
#include <iterator>

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
        mutable std::tr1::shared_ptr<const EAPI> eapi;
        const std::string guessed_eapi;
        const time_t master_mtime;
        const std::tr1::shared_ptr<const EclassMtimes> eclass_mtimes;
        mutable bool has_keys;
        mutable bool has_masks;

        mutable std::tr1::shared_ptr<const ESlotKey> slot;
        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<FSEntry> > fs_location;
        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<std::string> > short_description;
        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<std::string> > long_description;
        mutable std::tr1::shared_ptr<const LiteralMetadataValueKey<std::string> > captured_stderr_key;
        mutable std::tr1::shared_ptr<const EDependenciesKey> raw_dependencies;
        mutable std::tr1::shared_ptr<const EDependenciesKey> build_dependencies;
        mutable std::tr1::shared_ptr<const EDependenciesKey> run_dependencies;
        mutable std::tr1::shared_ptr<const EDependenciesKey> post_dependencies;
        mutable std::tr1::shared_ptr<const EProvideKey> provide;
        mutable std::tr1::shared_ptr<const EPlainTextSpecKey> restrictions;
        mutable std::tr1::shared_ptr<const EPlainTextSpecKey> properties;
        mutable std::tr1::shared_ptr<const EFetchableURIKey> src_uri;
        mutable std::tr1::shared_ptr<const ESimpleURIKey> homepage;
        mutable std::tr1::shared_ptr<const ELicenseKey> license;
        mutable std::tr1::shared_ptr<const EKeywordsKey> keywords;
        mutable std::tr1::shared_ptr<const EStringSetKey> raw_iuse;
        mutable std::tr1::shared_ptr<const LiteralMetadataStringSetKey> raw_iuse_effective;
        mutable std::tr1::shared_ptr<const EMyOptionsKey> raw_myoptions;
        mutable std::tr1::shared_ptr<const EStringSetKey> inherited;
        mutable std::tr1::shared_ptr<const EStringSetKey> raw_use;
        mutable std::tr1::shared_ptr<const LiteralMetadataStringSetKey> raw_use_expand;
        mutable std::tr1::shared_ptr<const LiteralMetadataStringSetKey> raw_use_expand_hidden;
        mutable std::tr1::shared_ptr<EMutableRepositoryMaskInfoKey> repository_mask;
        mutable std::tr1::shared_ptr<EMutableRepositoryMaskInfoKey> profile_mask;
        mutable std::tr1::shared_ptr<const EPlainTextSpecKey> remote_ids;
        mutable std::tr1::shared_ptr<const EPlainTextSpecKey> bugs_to;
        mutable std::tr1::shared_ptr<const ESimpleURIKey> upstream_changelog;
        mutable std::tr1::shared_ptr<const ESimpleURIKey> upstream_documentation;
        mutable std::tr1::shared_ptr<const ESimpleURIKey> upstream_release_notes;
        mutable std::tr1::shared_ptr<const EChoicesKey> choices;
        mutable std::tr1::shared_ptr<const EStringSetKey> defined_phases;

        std::tr1::shared_ptr<DependenciesLabelSequence> raw_dependencies_labels;
        std::tr1::shared_ptr<DependenciesLabelSequence> build_dependencies_labels;
        std::tr1::shared_ptr<DependenciesLabelSequence> run_dependencies_labels;
        std::tr1::shared_ptr<DependenciesLabelSequence> post_dependencies_labels;

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
            raw_dependencies_labels(new DependenciesLabelSequence),
            build_dependencies_labels(new DependenciesLabelSequence),
            run_dependencies_labels(new DependenciesLabelSequence),
            post_dependencies_labels(new DependenciesLabelSequence)
        {
            raw_dependencies_labels->push_back(make_shared_ptr(new DependenciesBuildLabel("build",
                            return_literal_function(true))));
            raw_dependencies_labels->push_back(make_shared_ptr(new DependenciesRunLabel("run",
                            return_literal_function(true))));

            build_dependencies_labels->push_back(make_shared_ptr(new DependenciesBuildLabel("DEPEND",
                            return_literal_function(true))));
            run_dependencies_labels->push_back(make_shared_ptr(new DependenciesRunLabel("RDEPEND",
                            return_literal_function(true))));
            post_dependencies_labels->push_back(make_shared_ptr(new DependenciesPostLabel("PDEPEND",
                            return_literal_function(true))));
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
    PrivateImplementationPattern<EbuildID>(new Implementation<EbuildID>(q, v, e, r, f, g.empty() ? r->params().eapi_when_unknown() : g, t, m)),
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

    FSEntry cache_file(_imp->repository->params().cache());
    cache_file /= stringify(name().category());
    cache_file /= stringify(name().package()) + "-" + stringify(version());

    FSEntry write_cache_file(_imp->repository->params().write_cache());
    if (_imp->repository->params().append_repository_name_to_write_cache())
        write_cache_file /= stringify(repository()->name());
    write_cache_file /= stringify(name().category());
    write_cache_file /= stringify(name().package()) + "-" + stringify(version());

    bool ok(false);
    if (_imp->repository->params().cache().basename() != "empty")
    {
        EbuildFlatMetadataCache metadata_cache(_imp->environment, cache_file, _imp->ebuild, _imp->master_mtime, _imp->eclass_mtimes, false);
        if (metadata_cache.load(shared_from_this(), false))
            ok = true;
    }

    if ((! ok) && _imp->repository->params().write_cache().basename() != "empty")
    {
        EbuildFlatMetadataCache write_metadata_cache(_imp->environment,
                write_cache_file, _imp->ebuild, _imp->master_mtime, _imp->eclass_mtimes, true);
        if (write_metadata_cache.load(shared_from_this(), false))
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
        if (_imp->repository->params().cache().basename() != "empty")
            Log::get_instance()->message("e.ebuild.cache.no_usable", ll_qa, lc_no_context)
                << "No usable cache entry for '" + canonical_form(idcf_full);

        _imp->environment->trigger_notifier_callback(NotifierCallbackGeneratingMetadataEvent(
                    _imp->repository->name()));

        _imp->eapi = EAPIData::get_instance()->eapi_from_string(_imp->guessed_eapi);

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
                    n::builddir() = _imp->repository->params().builddir(),
                    n::clearenv() = phases.begin_phases()->option("clearenv"),
                    n::commands() = join(phases.begin_phases()->begin_commands(), phases.begin_phases()->end_commands(), " "),
                    n::distdir() = _imp->repository->params().distdir(),
                    n::ebuild_dir() = _imp->repository->layout()->package_directory(name()),
                    n::ebuild_file() = _imp->ebuild,
                    n::eclassdirs() = _imp->repository->params().eclassdirs(),
                    n::environment() = _imp->environment,
                    n::exlibsdirs() = _imp->repository->layout()->exlibsdirs(name()),
                    n::files_dir() = _imp->repository->layout()->package_directory(name()) / "files",
                    n::maybe_output_manager() = make_null_shared_ptr(),
                    n::package_builddir() = _imp->repository->params().builddir() / (stringify(name().category()) + "-" + stringify(name().package()) + "-" + stringify(version()) + "-metadata"),
                    n::package_id() = shared_from_this(),
                    n::portdir() = 
                        (_imp->repository->params().master_repositories() && ! _imp->repository->params().master_repositories()->empty()) ?
                        (*_imp->repository->params().master_repositories()->begin())->params().location() : _imp->repository->params().location(),
                    n::root() = "/",
                    n::sandbox() = phases.begin_phases()->option("sandbox"),
                    n::sydbox() = phases.begin_phases()->option("sydbox"),
                    n::userpriv() = phases.begin_phases()->option("userpriv")
                    ));

            if (! cmd())
                Log::get_instance()->message("e.ebuild.metadata.unusable", ll_warning, lc_no_context) << "No usable metadata for '" +
                    stringify(canonical_form(idcf_full)) << "'";

            cmd.load(shared_from_this());

            Log::get_instance()->message("e.ebuild.metadata.generated_eapi", ll_debug, lc_context) << "Generated metadata for '"
                << canonical_form(idcf_full) << "' has EAPI '" << _imp->eapi->name() << "'";

            if (_imp->repository->params().write_cache().basename() != "empty" && _imp->eapi->supported())
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

    std::tr1::shared_ptr<const Map<ChoiceNameWithPrefix, std::string> > maybe_use_descriptions;
    if (_imp->eapi->supported())
    {
        _imp->raw_use_expand = make_shared_ptr(new LiteralMetadataStringSetKey(
                    _imp->eapi->supported()->ebuild_metadata_variables()->use_expand()->name(),
                    _imp->eapi->supported()->ebuild_metadata_variables()->use_expand()->description(),
                    mkt_internal,
                    e_repository()->profile()->use_expand()));
        _imp->raw_use_expand_hidden = make_shared_ptr(new LiteralMetadataStringSetKey(
                    _imp->eapi->supported()->ebuild_metadata_variables()->use_expand_hidden()->name(),
                    _imp->eapi->supported()->ebuild_metadata_variables()->use_expand_hidden()->description(),
                    mkt_internal,
                    e_repository()->profile()->use_expand_hidden()));

        std::tr1::shared_ptr<const MetadataXML> m(MetadataXMLPool::get_instance()->metadata_if_exists(
                    _imp->fs_location->value().dirname() / "metadata.xml"));
        if (m)
        {
            if (! m->long_description().empty())
                add_metadata_key(_imp->long_description = make_shared_ptr(new LiteralMetadataValueKey<std::string>("long_description",
                                "Long Description", mkt_normal, m->long_description())));
            if (! m->herds()->empty())
                add_metadata_key(make_shared_ptr(new LiteralMetadataStringSequenceKey("herds", "Herds", mkt_normal, m->herds())));
            if (! m->maintainers()->empty())
                add_metadata_key(make_shared_ptr(new LiteralMetadataStringSequenceKey("maintainers", "Maintainers", mkt_normal, m->maintainers())));
            maybe_use_descriptions = m->uses();
        }

        if (_imp->eapi->supported()->choices_options()->profile_iuse_injection())
        {
            std::tr1::shared_ptr<Set<std::string> > iuse_effective(new Set<std::string>);
            if (! _imp->raw_iuse)
                throw InternalError(PALUDIS_HERE, "no raw_iuse?");

            std::copy(_imp->raw_iuse->value()->begin(), _imp->raw_iuse->value()->end(), iuse_effective->inserter());
            std::copy(e_repository()->profile()->iuse_implicit()->begin(), e_repository()->profile()->iuse_implicit()->end(),
                    iuse_effective->inserter());

            const std::tr1::shared_ptr<const Set<std::string> > use_expand(e_repository()->profile()->use_expand());
            const std::tr1::shared_ptr<const Set<std::string> > use_expand_unprefixed(e_repository()->profile()->use_expand_unprefixed());
            const std::string separator(stringify(_imp->eapi->supported()->choices_options()->use_expand_separator()));

            for (Set<std::string>::ConstIterator x(e_repository()->profile()->use_expand_implicit()->begin()),
                    x_end(e_repository()->profile()->use_expand_implicit()->end()) ;
                    x != x_end ; ++x)
            {
                std::string lower_x;
                std::transform(x->begin(), x->end(), std::back_inserter(lower_x), &::tolower);

                bool prefixed(use_expand->end() != use_expand->find(*x));
                bool unprefixed(use_expand_unprefixed->end() != use_expand_unprefixed->find(*x));

                if ((! unprefixed) && (! prefixed))
                    Log::get_instance()->message("e.ebuild.iuse_effective.neither", ll_qa, lc_context)
                        << "USE_EXPAND_IMPLICIT value " << *x << " is not in either USE_EXPAND or USE_EXPAND_UNPREFIXED";

                const std::tr1::shared_ptr<const Set<std::string> > values(e_repository()->profile()->use_expand_values(*x));
                for (Set<std::string>::ConstIterator v(values->begin()), v_end(values->end()) ;
                        v != v_end ; ++v)
                {
                    if (prefixed)
                        iuse_effective->insert(lower_x + separator + *v);
                    if (unprefixed)
                        iuse_effective->insert(*v);
                }
            }

            _imp->raw_iuse_effective.reset(new LiteralMetadataStringSetKey(
                    _imp->eapi->supported()->ebuild_metadata_variables()->iuse_effective()->name(),
                    _imp->eapi->supported()->ebuild_metadata_variables()->iuse_effective()->description(),
                    mkt_internal,
                    iuse_effective));
            add_metadata_key(_imp->raw_iuse_effective);
        }

        _imp->choices.reset(new EChoicesKey(_imp->environment, shared_from_this(), "PALUDIS_CHOICES",
                    _imp->eapi->supported()->ebuild_environment_variables()->description_choices(),
                    mkt_normal, e_repository(),
                    maybe_use_descriptions));
    }
    else
        _imp->choices.reset(new EChoicesKey(_imp->environment, shared_from_this(), "PALUDIS_CHOICES", "Choices", mkt_normal,
                    e_repository(), maybe_use_descriptions));
    add_metadata_key(_imp->choices);
}

namespace
{
    struct LicenceChecker
    {
        bool ok;
        const Environment * const env;
        bool (Environment::* const func) (const std::string &, const PackageID &) const;
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

        void visit(const LicenseSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const LicenseSpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            bool local_ok(false);

            if (node.begin() == node.end())
                local_ok = true;
            else
            {
                for (LicenseSpecTree::NodeType<AnyDepSpec>::Type::ConstIterator c(node.begin()), c_end(node.end()) ;
                        c != c_end ; ++c)
                {
                    Save<bool> save_ok(&ok, true);
                    (*c)->accept(*this);
                    local_ok |= ok;
                }
            }

            ok &= local_ok;
        }

        void visit(const LicenseSpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (node.spec()->condition_met())
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const LicenseSpecTree::NodeType<LicenseDepSpec>::Type & node)
        {
            if (! (env->*func)(node.spec()->text(), *id))
                ok = false;
        }
    };

    bool is_stable_keyword(const KeywordName & k)
    {
        char c(*stringify(k).c_str());
        return (c != '~') && (c != '-');
    }
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
    {
        if (! _imp->environment->accept_keywords(keywords_key()->value(), *this))
        {
            add_mask(make_shared_ptr(new EUnacceptedMask('K',
                            DistributionData::get_instance()->distribution_from_string(
                                _imp->environment->distribution())->concept_keyword(), keywords_key())));
        }
        else if (keywords_key()->value()->end() == std::find_if(keywords_key()->value()->begin(),
                    keywords_key()->value()->end(), &is_stable_keyword))
        {
            add_overridden_mask(make_shared_ptr(new OverriddenMask(
                            make_named_values<OverriddenMask>(
                                n::mask() = make_shared_ptr(new EUnacceptedMask('~',
                                            DistributionData::get_instance()->distribution_from_string(
                                                _imp->environment->distribution())->concept_keyword() + " (unstable accepted)", keywords_key())),
                                n::override_reason() = mro_accepted_unstable
                                ))));
        }
    }

    if (license_key())
    {
        LicenceChecker c(_imp->environment, &Environment::accept_license, this);
        license_key()->value()->root()->accept(c);
        if (! c.ok)
            add_mask(make_shared_ptr(new EUnacceptedMask('L',
                            DistributionData::get_instance()->distribution_from_string(
                                _imp->environment->distribution())->concept_license(), license_key())));
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
        std::tr1::shared_ptr<const Mask> user_mask(_imp->environment->mask_for_user(*this, false));
        if (user_mask)
            add_mask(user_mask);
    }
    else
    {
        /* repo overridden by user */
        if (_imp->repository_mask->value())
            add_overridden_mask(make_shared_ptr(new OverriddenMask(
                            make_named_values<OverriddenMask>(
                                n::mask() = make_shared_ptr(new ERepositoryMask('r', "repository (overridden)", _imp->repository_mask)),
                                n::override_reason() = mro_overridden_by_user
                                ))));

        /* profile unless user */
        if (_imp->profile_mask->value())
            add_overridden_mask(make_shared_ptr(new OverriddenMask(
                            make_named_values<OverriddenMask>(
                                n::mask() = make_shared_ptr(new ERepositoryMask('p', "profile (overridden)", _imp->profile_mask)),
                                n::override_reason() = mro_overridden_by_user
                                ))));

        /* user */
        std::tr1::shared_ptr<const Mask> user_mask(_imp->environment->mask_for_user(*this, true));
        if (user_mask)
            add_overridden_mask(make_shared_ptr(new OverriddenMask(
                            make_named_values<OverriddenMask>(
                                n::mask() = user_mask,
                                n::override_reason() = mro_overridden_by_user
                                ))));

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
                return stringify(name()) + "-" + stringify(version()) + ":" + stringify(_imp->slot->value()) +
                    "::" + stringify(repository()->name());

            return stringify(name()) + "-" + stringify(version()) + "::" + stringify(repository()->name());

        case idcf_no_version:
            if (_imp->slot)
                return stringify(name()) + ":" + stringify(_imp->slot->value()) +
                    "::" + stringify(repository()->name());

            return stringify(name()) + "::" + stringify(repository()->name());

        case idcf_version:
            return stringify(version());

        case idcf_no_name:
            if (_imp->slot)
                return stringify(version()) + ":" + stringify(_imp->slot->value()) + "::" +
                    stringify(repository()->name());

            return stringify(version()) + "::" + stringify(repository()->name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
EbuildID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            (slot_key() ? ":" + stringify(slot_key()->value()) : "") + "::" + stringify(repository()->name()),
            _imp->environment, UserPackageDepSpecOptions());
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

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::raw_iuse_key() const
{
    need_keys_added();
    return _imp->raw_iuse;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::raw_iuse_effective_key() const
{
    need_keys_added();
    return _imp->raw_iuse_effective;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EbuildID::raw_myoptions_key() const
{
    need_keys_added();
    return _imp->raw_myoptions;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::raw_use_key() const
{
    need_keys_added();
    return _imp->raw_use;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::raw_use_expand_key() const
{
    need_keys_added();
    return _imp->raw_use_expand;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::raw_use_expand_hidden_key() const
{
    need_keys_added();
    return _imp->raw_use_expand_hidden;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::behaviours_key() const
{
    return make_null_shared_ptr();
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
EbuildID::dependencies_key() const
{
    need_keys_added();
    return _imp->raw_dependencies;
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

const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EbuildID::properties_key() const
{
    need_keys_added();
    return _imp->properties;
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
    need_keys_added();
    return _imp->long_description;
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

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::defined_phases_key() const
{
    return _imp->defined_phases;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
EbuildID::fs_location_key() const
{
    Lock l(_imp->mutex);

    // Avoid loading whole metadata
    if (! _imp->fs_location)
    {
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

std::string
EbuildID::guessed_eapi_name() const
{
    return _imp->guessed_eapi;
}

std::tr1::shared_ptr<const ERepository>
EbuildID::e_repository() const
{
    return _imp->repository;
}

void
EbuildID::load_captured_stderr(const std::string & r, const std::string & h, const MetadataKeyType t, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->captured_stderr_key.reset(new LiteralMetadataValueKey<std::string> (r, h, t, v));
    add_metadata_key(_imp->captured_stderr_key);
}

void
EbuildID::load_short_description(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->short_description.reset(new LiteralMetadataValueKey<std::string> (r, h, mkt_significant, v));
    add_metadata_key(_imp->short_description);
}

void
EbuildID::load_long_description(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->long_description.reset(new LiteralMetadataValueKey<std::string> (r, h, mkt_normal, v));
    add_metadata_key(_imp->long_description);
}

void
EbuildID::load_raw_depend(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->raw_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), r, h, v,
                _imp->raw_dependencies_labels, mkt_dependencies));
    add_metadata_key(_imp->raw_dependencies);
}

void
EbuildID::load_build_depend(const std::string & r, const std::string & h, const std::string & v,
        bool rewritten) const
{
    Lock l(_imp->mutex);
    _imp->build_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), r, h, v,
                _imp->build_dependencies_labels, rewritten ? mkt_internal : mkt_dependencies));
    add_metadata_key(_imp->build_dependencies);
}

void
EbuildID::load_run_depend(const std::string & r, const std::string & h, const std::string & v,
        bool rewritten) const
{
    Lock l(_imp->mutex);
    _imp->run_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), r, h, v,
                _imp->run_dependencies_labels, rewritten ? mkt_internal : mkt_dependencies));
    add_metadata_key(_imp->run_dependencies);
}

void
EbuildID::load_post_depend(const std::string & r, const std::string & h, const std::string & v,
        bool rewritten) const
{
    Lock l(_imp->mutex);
    _imp->post_dependencies.reset(new EDependenciesKey(_imp->environment, shared_from_this(), r, h, v,
                _imp->post_dependencies_labels, rewritten ? mkt_internal : mkt_dependencies));
    add_metadata_key(_imp->post_dependencies);
}

void
EbuildID::load_src_uri(const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->src_uri.reset(new EFetchableURIKey(_imp->environment, shared_from_this(), m, v, mkt_dependencies));
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
EbuildID::load_license(const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->license.reset(new ELicenseKey(_imp->environment, shared_from_this(), m, v, mkt_internal));
    add_metadata_key(_imp->license);
}

void
EbuildID::load_restrict(const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->restrictions.reset(new EPlainTextSpecKey(_imp->environment, shared_from_this(), m, v, mkt_internal));
    add_metadata_key(_imp->restrictions);
}

void
EbuildID::load_properties(const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->properties.reset(new EPlainTextSpecKey(_imp->environment, shared_from_this(), m, v, mkt_internal));
    add_metadata_key(_imp->properties);
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
    _imp->raw_iuse.reset(new EStringSetKey(shared_from_this(), r, h, v, mkt_internal));
    add_metadata_key(_imp->raw_iuse);
}

void
EbuildID::load_myoptions(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->raw_myoptions.reset(new EMyOptionsKey(_imp->environment, shared_from_this(), r, h, v, mkt_internal));
    add_metadata_key(_imp->raw_myoptions);
}

void
EbuildID::load_use(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->raw_use.reset(new EStringSetKey(shared_from_this(), r, h, v, mkt_internal));
    add_metadata_key(_imp->raw_use);
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
    _imp->inherited.reset(new EStringSetKey(shared_from_this(), r, h, v, mkt_internal));
    add_metadata_key(_imp->inherited);
}

void
EbuildID::load_defined_phases(const std::string & r, const std::string & h, const std::string & v) const
{
    if (v.empty())
        throw InternalError(PALUDIS_HERE, "v should not be empty");

    Lock l(_imp->mutex);
    _imp->defined_phases.reset(new EStringSetKey(shared_from_this(), r, h, v, mkt_internal));
    add_metadata_key(_imp->defined_phases);
}

void
EbuildID::load_upstream_changelog(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->upstream_changelog.reset(new ESimpleURIKey(_imp->environment, shared_from_this(), r, h, v, mkt_normal));
    add_metadata_key(_imp->upstream_changelog);
}

void
EbuildID::load_upstream_documentation(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->upstream_documentation.reset(new ESimpleURIKey(_imp->environment, shared_from_this(), r, h, v, mkt_normal));
    add_metadata_key(_imp->upstream_documentation);
}

void
EbuildID::load_upstream_release_notes(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->upstream_release_notes.reset(new ESimpleURIKey(_imp->environment, shared_from_this(), r, h, v, mkt_normal));
    add_metadata_key(_imp->upstream_release_notes);
}

void
EbuildID::load_bugs_to(const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->bugs_to.reset(new EPlainTextSpecKey(_imp->environment, shared_from_this(), m, v, mkt_normal));
    add_metadata_key(_imp->bugs_to);
}

void
EbuildID::load_remote_ids(const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->remote_ids.reset(new EPlainTextSpecKey(_imp->environment, shared_from_this(), m, v, mkt_internal));
    add_metadata_key(_imp->remote_ids);
}

void
EbuildID::load_slot(const std::tr1::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->slot.reset(new ESlotKey(m, v, mkt_internal));
    add_metadata_key(_imp->slot);
}

namespace
{
    struct SupportsActionQuery
    {
        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<ConfigAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return false;
        }
    };
}

bool
EbuildID::supports_action(const SupportsActionTestBase & b) const
{
    SupportsActionQuery q;
    return b.accept_returning<bool>(q) && eapi()->supported();
}

namespace
{
    struct PerformAction
    {
        const std::tr1::shared_ptr<const PackageID> id;

        PerformAction(const std::tr1::shared_ptr<const PackageID> i) :
            id(i)
        {
        }

        void visit(InstallAction & a)
        {
            std::tr1::static_pointer_cast<const ERepository>(id->repository())->install(
                    std::tr1::static_pointer_cast<const ERepositoryID>(id),
                    a.options);
        }

        void visit(FetchAction & a)
        {
            std::tr1::static_pointer_cast<const ERepository>(id->repository())->fetch(
                    std::tr1::static_pointer_cast<const ERepositoryID>(id),
                    a.options);
        }

        void visit(PretendFetchAction & a)
        {
            std::tr1::static_pointer_cast<const ERepository>(id->repository())->pretend_fetch(
                    std::tr1::static_pointer_cast<const ERepositoryID>(id),
                    a);
        }

        void visit(PretendAction & action)
        {
            if (! std::tr1::static_pointer_cast<const ERepository>(id->repository())->pretend(
                        std::tr1::static_pointer_cast<const ERepositoryID>(id),
                        action))
                action.set_failed();
        }

        void visit(InfoAction & action)
        {
            std::tr1::static_pointer_cast<const ERepository>(id->repository())->info(
                    std::tr1::static_pointer_cast<const ERepositoryID>(id),
                    action);
        }

        void visit(UninstallAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(ConfigAction & a) PALUDIS_ATTRIBUTE((noreturn));
    };

    void PerformAction::visit(UninstallAction & a)
    {
        throw ActionFailedError("Unsupported action: " + a.simple_name());
    }

    void PerformAction::visit(ConfigAction & a)
    {
        throw ActionFailedError("Unsupported action: " + a.simple_name());
    }
}

void
EbuildID::perform_action(Action & a) const
{
    if (! eapi()->supported())
        throw ActionFailedError("Unsupported EAPI");

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

const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EbuildID::remote_ids_key() const
{
    need_keys_added();
    return _imp->remote_ids;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EbuildID::bugs_to_key() const
{
    need_keys_added();
    return _imp->bugs_to;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
EbuildID::upstream_changelog_key() const
{
    need_keys_added();
    return _imp->upstream_changelog;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
EbuildID::upstream_documentation_key() const
{
    need_keys_added();
    return _imp->upstream_documentation;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
EbuildID::upstream_release_notes_key() const
{
    need_keys_added();
    return _imp->upstream_release_notes;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
EbuildID::choices_key() const
{
    need_keys_added();
    return _imp->choices;
}

const std::tr1::shared_ptr<const MetadataValueKey<SlotName> >
EbuildID::slot_key() const
{
    need_keys_added();
    return _imp->slot;
}

std::tr1::shared_ptr<ChoiceValue>
EbuildID::make_choice_value(
        const std::tr1::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_name,
        const Tribool iuse_default,
        const bool explicitly_listed,
        const std::string & override_description,
        const bool force_locked
        ) const
{
    if (! eapi()->supported())
        throw InternalError(PALUDIS_HERE, "Unsupported EAPI");

    std::string name_with_prefix_s;
    if (stringify(choice->prefix()).empty())
        name_with_prefix_s = stringify(value_name);
    else
    {
        char use_expand_separator(eapi()->supported()->choices_options()->use_expand_separator());
        if (! use_expand_separator)
            throw InternalError(PALUDIS_HERE, "No use_expand_separator defined");
        name_with_prefix_s = stringify(choice->prefix()) + std::string(1, use_expand_separator) + stringify(value_name);
    }
    ChoiceNameWithPrefix name_with_prefix(name_with_prefix_s);

    bool locked(false), enabled(false), enabled_by_default(false);
    if (raw_use_key())
    {
        locked = true;
        enabled = enabled_by_default = (raw_use_key()->value()->end() != raw_use_key()->value()->find(name_with_prefix_s));
    }
    else
    {
        if (_imp->repository->profile()->use_masked(shared_from_this(), choice, value_name, name_with_prefix))
        {
            locked = true;
            enabled = enabled_by_default = false;
        }
        else if (_imp->repository->profile()->use_forced(shared_from_this(), choice, value_name, name_with_prefix))
        {
            locked = true;
            enabled = enabled_by_default = true;
        }
        else
        {
            Tribool profile_want(_imp->repository->profile()->use_state_ignoring_masks(shared_from_this(), choice, value_name, name_with_prefix));
            if (profile_want.is_true())
                enabled_by_default = true;
            else if (profile_want.is_false())
                enabled_by_default = false;
            else if (iuse_default.is_true())
                enabled_by_default = true;
            else if (iuse_default.is_false())
                enabled_by_default = false;
            else
                enabled_by_default = false;
            Tribool env_want(_imp->environment->want_choice_enabled(shared_from_this(), choice, value_name));
            if (env_want.is_true())
                enabled = true;
            else if (env_want.is_false())
                enabled = false;
            else
                enabled = enabled_by_default;
        }
    }

    return make_shared_ptr(new EChoiceValue(choice->prefix(), value_name, ChoiceNameWithPrefix(name_with_prefix), name(),
                _imp->repository->use_desc(),
                enabled, enabled_by_default, force_locked || locked, explicitly_listed, override_description, ""));
}

namespace
{
    struct UnconditionalRestrictFinder
    {
        std::set<std::string> s;

        void visit(const PlainTextSpecTree::NodeType<PlainTextDepSpec>::Type & node)
        {
            s.insert(node.spec()->text());
        }

        void visit(const PlainTextSpecTree::NodeType<PlainTextLabelDepSpec>::Type &)
        {
        }

        void visit(const PlainTextSpecTree::NodeType<ConditionalDepSpec>::Type &)
        {
        }

        void visit(const PlainTextSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }
    };
}

void
EbuildID::add_build_options(const std::tr1::shared_ptr<Choices> & choices) const
{
    if (eapi()->supported())
    {
        std::tr1::shared_ptr<Choice> build_options(new Choice(make_named_values<ChoiceParams>(
                        n::consider_added_or_changed() = false,
                        n::contains_every_value() = false,
                        n::hidden() = false,
                        n::human_name() = canonical_build_options_human_name(),
                        n::prefix() = canonical_build_options_prefix(),
                        n::raw_name() = canonical_build_options_raw_name(),
                        n::show_with_no_prefix() = false
                        )));
        choices->add(build_options);

        bool may_be_unrestricted_test(true), may_be_unrestricted_strip(true);

        /* if we unconditionally restrict an action, don't add a build option
         * for it. but if we conditionally restrict it, do, to avoid weirdness
         * in cases like RESTRICT="test? ( test )." */
        if (restrict_key())
        {
            UnconditionalRestrictFinder f;
            restrict_key()->value()->root()->accept(f);
            may_be_unrestricted_test = f.s.end() == f.s.find("test");
            may_be_unrestricted_strip = f.s.end() == f.s.find("strip");
        }

        /* optional_tests */
        if (eapi()->supported()->choices_options()->has_optional_tests())
            if (may_be_unrestricted_test)
                build_options->add(make_shared_ptr(new ELikeOptionalTestsChoiceValue(shared_from_this(), _imp->environment, build_options)));

        /* recommended_tests */
        if (eapi()->supported()->choices_options()->has_recommended_tests())
            if (may_be_unrestricted_test)
                build_options->add(make_shared_ptr(new ELikeRecommendedTestsChoiceValue(shared_from_this(), _imp->environment, build_options)));

        /* expensive_tests */
        if (eapi()->supported()->choices_options()->has_expensive_tests())
        {
            if (! _imp->defined_phases)
                throw InternalError(PALUDIS_HERE, "bug! no defined_phases yet");

            bool has_expensive_test_phase(false);
            EAPIPhases phases(_imp->eapi->supported()->ebuild_phases()->ebuild_install());
            for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
                    phase != phase_end ; ++phase)
            {
                if (phase->option("expensive_tests"))
                {
                    if (_imp->defined_phases->value()->end() != _imp->defined_phases->value()->find(phase->equal_option("skipname")))
                    {
                        has_expensive_test_phase = true;
                        break;
                    }
                }
            }

            if (has_expensive_test_phase)
                build_options->add(make_shared_ptr(new ELikeExpensiveTestsChoiceValue(shared_from_this(), _imp->environment, build_options)));
        }

        /* split, strip */
        if (may_be_unrestricted_strip)
        {
            build_options->add(make_shared_ptr(new ELikeSplitChoiceValue(shared_from_this(), _imp->environment, build_options)));
            build_options->add(make_shared_ptr(new ELikeStripChoiceValue(shared_from_this(), _imp->environment, build_options)));
        }

        /* trace */
        build_options->add(make_shared_ptr(new ELikeTraceChoiceValue(
                        shared_from_this(), _imp->environment, build_options)));

        /* preserve_work */
        build_options->add(make_shared_ptr(new ELikePreserveWorkChoiceValue(
                        shared_from_this(), _imp->environment, build_options)));

        /* jobs */
        if (! eapi()->supported()->ebuild_environment_variables()->env_jobs().empty())
        {
            if (! _imp->defined_phases)
                throw InternalError(PALUDIS_HERE, "bug! no defined_phases yet");

            build_options->add(make_shared_ptr(new ELikeJobsChoiceValue(
                            shared_from_this(), _imp->environment, build_options)));
        }
    }
}

void
EbuildID::purge_invalid_cache() const
{
    FSEntry write_cache_file(_imp->repository->params().write_cache());
    if (_imp->repository->params().append_repository_name_to_write_cache())
        write_cache_file /= stringify(repository()->name());
    write_cache_file /= stringify(name().category());
    write_cache_file /= stringify(name().package()) + "-" + stringify(version());

    if (write_cache_file.exists())
    {
        if (_imp->repository->params().write_cache().basename() != "empty")
        {
            EbuildFlatMetadataCache write_metadata_cache(_imp->environment,
                    write_cache_file, _imp->ebuild, _imp->master_mtime, _imp->eclass_mtimes, true);
            if (! write_metadata_cache.load(shared_from_this(), true))
                write_cache_file.unlink();
        }
    }
}

