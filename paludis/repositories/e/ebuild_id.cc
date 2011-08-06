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

#include <paludis/repositories/e/ebuild_id.hh>
#include <paludis/repositories/e/ebuild_flat_metadata_cache.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/e_slot_key.hh>
#include <paludis/repositories/e/e_choices_key.hh>
#include <paludis/repositories/e/e_mask.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/manifest2_reader.hh>
#include <paludis/repositories/e/e_choice_value.hh>
#include <paludis/repositories/e/metadata_xml.hh>
#include <paludis/repositories/e/do_pretend_action.hh>
#include <paludis/repositories/e/do_pretend_fetch_action.hh>
#include <paludis/repositories/e/do_install_action.hh>
#include <paludis/repositories/e/do_info_action.hh>
#include <paludis/repositories/e/do_fetch_action.hh>
#include <paludis/repositories/e/e_string_set_key.hh>
#include <paludis/repositories/e/e_keywords_key.hh>

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
#include <paludis/always_enabled_dependency_label.hh>

#include <paludis/util/fs_error.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/save.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/join.hh>

#include <set>
#include <iterator>
#include <algorithm>
#include <ctime>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct EbuildIDData :
        Singleton<EbuildIDData>
    {
        std::shared_ptr<Set<std::string> > pbin_behaviours_value;
        std::shared_ptr<LiteralMetadataStringSetKey> pbin_behaviours_key;

        std::shared_ptr<DependenciesLabelSequence> raw_dependencies_labels;
        std::shared_ptr<DependenciesLabelSequence> build_dependencies_labels;
        std::shared_ptr<DependenciesLabelSequence> run_dependencies_labels;
        std::shared_ptr<DependenciesLabelSequence> post_dependencies_labels;

        EbuildIDData() :
            pbin_behaviours_value(std::make_shared<Set<std::string>>()),
            pbin_behaviours_key(std::make_shared<LiteralMetadataStringSetKey>("behaviours", "behaviours", mkt_internal, pbin_behaviours_value)),
            raw_dependencies_labels(std::make_shared<DependenciesLabelSequence>()),
            build_dependencies_labels(std::make_shared<DependenciesLabelSequence>()),
            run_dependencies_labels(std::make_shared<DependenciesLabelSequence>()),
            post_dependencies_labels(std::make_shared<DependenciesLabelSequence>())
        {
            pbin_behaviours_value->insert("unbinaryable");
            pbin_behaviours_value->insert("binary");

            raw_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag> >("build"));
            raw_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesRunLabelTag> >("run"));

            build_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag> >("DEPEND"));
            run_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesRunLabelTag> >("RDEPEND"));
            post_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesPostLabelTag> >("PDEPEND"));
        }
    };

    std::shared_ptr<LiteralMetadataValueKey<FSPath> > make_fs_location(const std::string & e, const FSPath & p)
    {
        auto eapi(EAPIData::get_instance()->eapi_from_string(e));
        if (eapi->supported())
            return std::make_shared<LiteralMetadataValueKey<FSPath> >(
                eapi->supported()->ebuild_options()->fs_location_name(),
                eapi->supported()->ebuild_options()->fs_location_description(),
                mkt_internal, p);
        else
            return std::make_shared<LiteralMetadataValueKey<FSPath> >(
                "FS_LOCATION",
                "FS Location",
                mkt_internal, p);
    }
}

namespace paludis
{
    template <>
    struct Imp<EbuildID>
    {
        mutable Mutex mutex;

        const QualifiedPackageName name;
        const VersionSpec version;
        const Environment * const environment;
        const RepositoryName repository_name;
        mutable std::shared_ptr<const EAPI> eapi;
        const std::string guessed_eapi;
        const time_t master_mtime;
        const std::shared_ptr<const EclassMtimes> eclass_mtimes;
        mutable bool has_non_xml_keys;
        mutable bool has_xml_keys;
        mutable bool has_masks;

        const std::shared_ptr<const LiteralMetadataValueKey<FSPath> > fs_location;

        mutable std::shared_ptr<const MetadataValueKey<SlotName> > slot;
        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > short_description;
        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > long_description;
        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > captured_stdout_key;
        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > captured_stderr_key;
        mutable std::shared_ptr<const EDependenciesKey> dependencies;
        mutable std::shared_ptr<const EDependenciesKey> build_dependencies;
        mutable std::shared_ptr<const EDependenciesKey> run_dependencies;
        mutable std::shared_ptr<const EDependenciesKey> post_dependencies;
        mutable std::shared_ptr<const EPlainTextSpecKey> restrictions;
        mutable std::shared_ptr<const EPlainTextSpecKey> properties;
        mutable std::shared_ptr<const EFetchableURIKey> src_uri;
        mutable std::shared_ptr<const ESimpleURIKey> homepage;
        mutable std::shared_ptr<const ELicenseKey> license;
        mutable std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> > keywords;
        mutable std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_iuse;
        mutable std::shared_ptr<const LiteralMetadataStringSetKey> raw_iuse_effective;
        mutable std::shared_ptr<const EMyOptionsKey> raw_myoptions;
        mutable std::shared_ptr<const ERequiredUseKey> required_use;
        mutable std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > inherited;
        mutable std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use;
        mutable std::shared_ptr<const LiteralMetadataStringSetKey> raw_use_expand;
        mutable std::shared_ptr<const LiteralMetadataStringSetKey> raw_use_expand_hidden;
        mutable std::shared_ptr<const EPlainTextSpecKey> remote_ids;
        mutable std::shared_ptr<const EPlainTextSpecKey> bugs_to;
        mutable std::shared_ptr<const ESimpleURIKey> upstream_changelog;
        mutable std::shared_ptr<const ESimpleURIKey> upstream_documentation;
        mutable std::shared_ptr<const ESimpleURIKey> upstream_release_notes;
        mutable std::shared_ptr<const EChoicesKey> choices;
        mutable std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > defined_phases;
        mutable std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > generated_from;
        mutable std::shared_ptr<const LiteralMetadataTimeKey> generated_time;
        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > generated_using;
        mutable std::shared_ptr<const LiteralMetadataStringSetKey> behaviours;
        mutable std::shared_ptr<const LiteralMetadataValueKey<std::string> > scm_revision;

        Imp(const QualifiedPackageName & q, const VersionSpec & v,
                const Environment * const e,
                const RepositoryName & r, const FSPath & f, const std::string & g,
                const time_t t, const std::shared_ptr<const EclassMtimes> & m) :
            name(q),
            version(v),
            environment(e),
            repository_name(r),
            guessed_eapi(g),
            master_mtime(t),
            eclass_mtimes(m),
            has_non_xml_keys(false),
            has_xml_keys(false),
            has_masks(false),
            fs_location(make_fs_location(guessed_eapi, f))
        {
        }
    };
}

namespace
{
    std::string guess_eapi(const std::string & g, const Environment * const env, const RepositoryName & repo_name)
    {
        if (! g.empty())
            return g;
        auto repo(env->fetch_repository(repo_name));
        auto e_repo(std::static_pointer_cast<const ERepository>(repo));
        return e_repo->params().eapi_when_unknown();
    }
}

EbuildID::EbuildID(const QualifiedPackageName & q, const VersionSpec & v,
        const Environment * const e,
        const RepositoryName & r,
        const FSPath & f,
        const std::string & g,
        const time_t t,
        const std::shared_ptr<const EclassMtimes> & m) :
    _imp(q, v, e, r, f, guess_eapi(g, e, r), t, m)
{
}

EbuildID::~EbuildID()
{
}

void
EbuildID::need_non_xml_keys_added() const
{
    Lock l(_imp->mutex);

    if (_imp->has_non_xml_keys)
        return;

    _imp->has_non_xml_keys = true;

    Context context("When generating metadata for ID '" + canonical_form(idcf_full) + "':");

    add_metadata_key(_imp->fs_location);

    auto repo(_imp->environment->fetch_repository(repository_name()));
    auto e_repo(std::static_pointer_cast<const ERepository>(repo));
    FSPath cache_file(e_repo->params().cache());
    cache_file /= stringify(name().category());
    cache_file /= stringify(name().package()) + "-" + stringify(version());

    FSPath write_cache_file(e_repo->params().write_cache());
    if (e_repo->params().append_repository_name_to_write_cache())
        write_cache_file /= stringify(repository_name());
    write_cache_file /= stringify(name().category());
    write_cache_file /= stringify(name().package()) + "-" + stringify(version());

    bool ok(false);
    if (e_repo->params().cache().basename() != "empty")
    {
        EbuildFlatMetadataCache metadata_cache(_imp->environment, cache_file, _imp->fs_location->parse_value(), _imp->master_mtime, _imp->eclass_mtimes, false);
        if (metadata_cache.load(shared_from_this(), false))
            ok = true;
    }

    if ((! ok) && e_repo->params().write_cache().basename() != "empty")
    {
        EbuildFlatMetadataCache write_metadata_cache(_imp->environment,
                write_cache_file, _imp->fs_location->parse_value(), _imp->master_mtime, _imp->eclass_mtimes, true);
        if (write_metadata_cache.load(shared_from_this(), false))
            ok = true;
        else if (write_cache_file.stat().exists())
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
        if (e_repo->params().cache().basename() != "empty")
            Log::get_instance()->message("e.ebuild.cache.no_usable", ll_qa, lc_no_context)
                << "No usable cache entry for '" + canonical_form(idcf_full);

        _imp->environment->trigger_notifier_callback(NotifierCallbackGeneratingMetadataEvent(repository_name()));

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
                    n::builddir() = e_repo->params().builddir(),
                    n::clearenv() = phases.begin_phases()->option("clearenv"),
                    n::commands() = join(phases.begin_phases()->begin_commands(), phases.begin_phases()->end_commands(), " "),
                    n::distdir() = e_repo->params().distdir(),
                    n::ebuild_dir() = e_repo->layout()->package_directory(name()),
                    n::ebuild_file() = _imp->fs_location->parse_value(),
                    n::eclassdirs() = e_repo->params().eclassdirs(),
                    n::environment() = _imp->environment,
                    n::exlibsdirs() = e_repo->layout()->exlibsdirs(name()),
                    n::files_dir() = e_repo->layout()->package_directory(name()) / "files",
                    n::maybe_output_manager() = make_null_shared_ptr(),
                    n::package_builddir() = e_repo->params().builddir() / (stringify(name().category()) + "-" + stringify(name().package()) + "-" + stringify(version()) + "-metadata"),
                    n::package_id() = shared_from_this(),
                    n::portdir() =
                        (e_repo->params().master_repositories() && ! e_repo->params().master_repositories()->empty()) ?
                        (*e_repo->params().master_repositories()->begin())->params().location() : e_repo->params().location(),
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

            if (e_repo->params().write_cache().basename() != "empty" && _imp->eapi->supported())
            {
                EbuildFlatMetadataCache metadata_cache(_imp->environment, write_cache_file, _imp->fs_location->parse_value(), _imp->master_mtime,
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

    add_metadata_key(std::make_shared<LiteralMetadataValueKey<std::string>>("EAPI", "EAPI", mkt_internal, _imp->eapi->name()));

    if (_imp->eapi->supported())
    {
        _imp->raw_use_expand = std::make_shared<LiteralMetadataStringSetKey>(
                    _imp->eapi->supported()->ebuild_metadata_variables()->use_expand()->name(),
                    _imp->eapi->supported()->ebuild_metadata_variables()->use_expand()->description(),
                    mkt_internal,
                    e_repo->profile()->use_expand());
        _imp->raw_use_expand_hidden = std::make_shared<LiteralMetadataStringSetKey>(
                    _imp->eapi->supported()->ebuild_metadata_variables()->use_expand_hidden()->name(),
                    _imp->eapi->supported()->ebuild_metadata_variables()->use_expand_hidden()->description(),
                    mkt_internal,
                    e_repo->profile()->use_expand_hidden());

        if (_imp->eapi->supported()->choices_options()->profile_iuse_injection())
        {
            std::shared_ptr<Set<std::string> > iuse_effective(std::make_shared<Set<std::string>>());
            if (! _imp->raw_iuse)
                throw InternalError(PALUDIS_HERE, "no raw_iuse?");

            auto iu(_imp->raw_iuse->parse_value());
            std::copy(iu->begin(), iu->end(), iuse_effective->inserter());
            std::copy(e_repo->profile()->iuse_implicit()->begin(), e_repo->profile()->iuse_implicit()->end(),
                    iuse_effective->inserter());

            const std::shared_ptr<const Set<std::string> > use_expand(e_repo->profile()->use_expand());
            const std::shared_ptr<const Set<std::string> > use_expand_unprefixed(e_repo->profile()->use_expand_unprefixed());
            const std::string separator(stringify(_imp->eapi->supported()->choices_options()->use_expand_separator()));

            for (Set<std::string>::ConstIterator x(e_repo->profile()->use_expand_implicit()->begin()),
                    x_end(e_repo->profile()->use_expand_implicit()->end()) ;
                    x != x_end ; ++x)
            {
                std::string lower_x;
                std::transform(x->begin(), x->end(), std::back_inserter(lower_x), &::tolower);

                bool prefixed(use_expand->end() != use_expand->find(*x));
                bool unprefixed(use_expand_unprefixed->end() != use_expand_unprefixed->find(*x));

                if ((! unprefixed) && (! prefixed))
                    Log::get_instance()->message("e.ebuild.iuse_effective.neither", ll_qa, lc_context)
                        << "USE_EXPAND_IMPLICIT value " << *x << " is not in either USE_EXPAND or USE_EXPAND_UNPREFIXED";

                const std::shared_ptr<const Set<std::string> > values(e_repo->profile()->use_expand_values(*x));
                for (Set<std::string>::ConstIterator v(values->begin()), v_end(values->end()) ;
                        v != v_end ; ++v)
                {
                    if (prefixed)
                        iuse_effective->insert(lower_x + separator + *v);
                    if (unprefixed)
                        iuse_effective->insert(*v);
                }
            }

            _imp->raw_iuse_effective = std::make_shared<LiteralMetadataStringSetKey>(
                    _imp->eapi->supported()->ebuild_metadata_variables()->iuse_effective()->name(),
                    _imp->eapi->supported()->ebuild_metadata_variables()->iuse_effective()->description(),
                    mkt_internal,
                    iuse_effective);
            add_metadata_key(_imp->raw_iuse_effective);
        }

        _imp->choices = std::make_shared<EChoicesKey>(_imp->environment, shared_from_this(), "PALUDIS_CHOICES",
                    _imp->eapi->supported()->ebuild_environment_variables()->description_choices(),
                    mkt_normal, e_repo,
                    std::bind(&EbuildID::choice_descriptions, this));

        if (_imp->eapi->supported()->is_pbin())
        {
            _imp->behaviours = EbuildIDData::get_instance()->pbin_behaviours_key;
            add_metadata_key(_imp->behaviours);
        }
    }
    else
        _imp->choices = std::make_shared<EChoicesKey>(_imp->environment, shared_from_this(), "PALUDIS_CHOICES", "Choices", mkt_normal,
                    e_repo, std::bind(return_literal_function(make_null_shared_ptr())));
    add_metadata_key(_imp->choices);
}

void
EbuildID::need_xml_keys_added() const
{
    Lock l(_imp->mutex);

    if (_imp->has_xml_keys)
        return;

    _imp->has_xml_keys = true;

    Context context("When generating XML-related metadata for ID '" + canonical_form(idcf_full) + "':");

    need_non_xml_keys_added();

    if (_imp->eapi->supported())
    {
        std::shared_ptr<const MetadataXML> m(MetadataXMLPool::get_instance()->metadata_if_exists(
                    _imp->fs_location->parse_value().dirname() / "metadata.xml"));
        if (m)
        {
            if (! m->long_description().empty())
                add_metadata_key(_imp->long_description = std::make_shared<LiteralMetadataValueKey<std::string>>("long_description",
                                "Long Description", mkt_normal, m->long_description()));
            if (! m->herds()->empty())
                add_metadata_key(std::make_shared<LiteralMetadataStringSequenceKey>("herds", "Herds", mkt_normal, m->herds()));
            if (! m->maintainers()->empty())
                add_metadata_key(std::make_shared<LiteralMetadataStringSequenceKey>("maintainers", "Maintainers", mkt_normal, m->maintainers()));
        }
    }
}

void
EbuildID::need_keys_added() const
{
    Lock l(_imp->mutex);

    need_non_xml_keys_added();
    need_xml_keys_added();
}

const std::shared_ptr<const Map<ChoiceNameWithPrefix, std::string> >
EbuildID::choice_descriptions() const
{
    std::shared_ptr<const MetadataXML> m(MetadataXMLPool::get_instance()->metadata_if_exists(
                _imp->fs_location->parse_value().dirname() / "metadata.xml"));
    if (m)
        return m->uses();
    else
        return make_null_shared_ptr();
}

namespace
{
    struct LicenceChecker
    {
        bool ok;
        const Environment * const env;
        bool (Environment::* const func) (const std::string &, const std::shared_ptr<const PackageID> &) const;
        const std::shared_ptr<const PackageID> id;

        LicenceChecker(const Environment * const e,
                bool (Environment::* const f) (const std::string &, const std::shared_ptr<const PackageID> &) const,
                const std::shared_ptr<const PackageID> & i) :
            ok(true),
            env(e),
            func(f),
            id(i)
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
            if (node.spec()->condition_met(env, id))
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const LicenseSpecTree::NodeType<LicenseDepSpec>::Type & node)
        {
            if (! (env->*func)(node.spec()->text(), id))
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
        add_mask(std::make_shared<EUnsupportedMask>('E', "eapi", eapi()->name()));
        return;
    }

    auto repo(_imp->environment->fetch_repository(_imp->repository_name));
    auto e_repo(std::static_pointer_cast<const ERepository>(repo));

    if (keywords_key())
    {
        auto keywords(keywords_key()->parse_value());
        if (! _imp->environment->accept_keywords(keywords, shared_from_this()))
        {
            add_mask(create_e_unaccepted_mask('K',
                        DistributionData::get_instance()->distribution_from_string(
                            _imp->environment->distribution())->concept_keyword(), keywords_key()->raw_name()));
        }
        else if (keywords->end() == std::find_if(keywords->begin(), keywords->end(), &is_stable_keyword))
        {
            add_overridden_mask(std::make_shared<OverriddenMask>(
                            make_named_values<OverriddenMask>(
                                n::mask() = create_e_unaccepted_mask('~',
                                            DistributionData::get_instance()->distribution_from_string(
                                                _imp->environment->distribution())->concept_keyword() + " (unstable accepted)", keywords_key()->raw_name()),
                                n::override_reason() = mro_accepted_unstable
                                )));
        }
    }

    if (license_key())
    {
        LicenceChecker c(_imp->environment, &Environment::accept_license, shared_from_this());
        license_key()->parse_value()->top()->accept(c);
        if (! c.ok)
            add_mask(create_e_unaccepted_mask('L',
                        DistributionData::get_instance()->distribution_from_string(
                            _imp->environment->distribution())->concept_license(), license_key()->raw_name()));
    }

    {
        auto repo_masks(e_repo->layout()->repository_masks(shared_from_this()));

        for (auto r(repo_masks->begin()), r_end(repo_masks->end()) ;
                r != r_end ; ++r)
            if (_imp->environment->unmasked_by_user(shared_from_this(), r->token()))
                add_overridden_mask(std::make_shared<OverriddenMask>(
                            make_named_values<OverriddenMask>(
                                n::mask() = std::make_shared<ERepositoryMask>('r', "repository (overridden)",
                                    r->comment(), r->token(), r->mask_file()),
                                n::override_reason() = mro_overridden_by_user
                                )));
            else
                add_mask(std::make_shared<ERepositoryMask>('R', "repository", r->comment(), r->token(), r->mask_file())); 
    }

    {
        auto profile_masks(e_repo->profile()->profile_masks(shared_from_this()));
        for (auto r(profile_masks->begin()), r_end(profile_masks->end()) ;
                r != r_end ; ++r)
            if (_imp->environment->unmasked_by_user(shared_from_this(), r->token()))
                add_overridden_mask(std::make_shared<OverriddenMask>(
                            make_named_values<OverriddenMask>(
                                n::mask() = std::make_shared<ERepositoryMask>('p', "profile (overridden)",
                                    r->comment(), r->token(), r->mask_file()),
                                n::override_reason() = mro_overridden_by_user
                                )));
            else
                add_mask(std::make_shared<ERepositoryMask>('P', "profile", r->comment(), r->token(), r->mask_file()));
    }

    {
        /* user */
        std::shared_ptr<const Mask> user_mask(_imp->environment->mask_for_user(shared_from_this(), false));
        if (user_mask)
        {
            if (_imp->environment->unmasked_by_user(shared_from_this(), get_mask_token(*user_mask)))
                add_overridden_mask(std::make_shared<OverriddenMask>(
                            make_named_values<OverriddenMask>(
                                n::mask() = user_mask,
                                n::override_reason() = mro_overridden_by_user
                                )));
            else
                add_mask(user_mask);
        }
    }

    /* break portage */
    std::shared_ptr<const Mask> breaks_mask(_imp->environment->mask_for_breakage(shared_from_this()));
    if (breaks_mask)
        add_mask(breaks_mask);
}

const std::string
EbuildID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            if (_imp->slot)
                return stringify(name()) + "-" + stringify(version()) + ":" + stringify(_imp->slot->parse_value()) +
                    "::" + stringify(repository_name());

            return stringify(name()) + "-" + stringify(version()) + "::" + stringify(repository_name());

        case idcf_no_version:
            if (_imp->slot)
                return stringify(name()) + ":" + stringify(_imp->slot->parse_value()) +
                    "::" + stringify(repository_name());

            return stringify(name()) + "::" + stringify(repository_name());

        case idcf_version:
            return stringify(version());

        case idcf_no_name:
            if (_imp->slot)
                return stringify(version()) + ":" + stringify(_imp->slot->parse_value()) + "::" +
                    stringify(repository_name());

            return stringify(version()) + "::" + stringify(repository_name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
EbuildID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            (slot_key() ? ":" + stringify(slot_key()->parse_value()) : "") + "::" + stringify(repository_name()),
            _imp->environment, { });
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

const RepositoryName
EbuildID::repository_name() const
{
    return _imp->repository_name;
}

const std::shared_ptr<const EAPI>
EbuildID::eapi() const
{
    if (_imp->eapi)
        return _imp->eapi;

    need_non_xml_keys_added();

    if (! _imp->eapi)
        throw InternalError(PALUDIS_HERE, "_imp->eapi still not set");

    return _imp->eapi;
}

const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
EbuildID::keywords_key() const
{
    need_non_xml_keys_added();
    return _imp->keywords;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::raw_iuse_key() const
{
    need_non_xml_keys_added();
    return _imp->raw_iuse;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::raw_iuse_effective_key() const
{
    need_non_xml_keys_added();
    return _imp->raw_iuse_effective;
}

const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EbuildID::raw_myoptions_key() const
{
    need_non_xml_keys_added();
    return _imp->raw_myoptions;
}

const std::shared_ptr<const MetadataSpecTreeKey<RequiredUseSpecTree> >
EbuildID::required_use_key() const
{
    need_non_xml_keys_added();
    return _imp->required_use;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::raw_use_key() const
{
    need_non_xml_keys_added();
    return _imp->raw_use;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::raw_use_expand_key() const
{
    need_non_xml_keys_added();
    return _imp->raw_use_expand;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::raw_use_expand_hidden_key() const
{
    need_non_xml_keys_added();
    return _imp->raw_use_expand_hidden;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::behaviours_key() const
{
    need_non_xml_keys_added();
    return _imp->behaviours;
}

const std::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >
EbuildID::license_key() const
{
    need_non_xml_keys_added();
    return _imp->license;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EbuildID::dependencies_key() const
{
    need_non_xml_keys_added();
    return _imp->dependencies;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EbuildID::build_dependencies_key() const
{
    need_non_xml_keys_added();
    return _imp->build_dependencies;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EbuildID::run_dependencies_key() const
{
    need_non_xml_keys_added();
    return _imp->run_dependencies;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EbuildID::post_dependencies_key() const
{
    need_non_xml_keys_added();
    return _imp->post_dependencies;
}

const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EbuildID::restrict_key() const
{
    need_non_xml_keys_added();
    return _imp->restrictions;
}

const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EbuildID::properties_key() const
{
    need_non_xml_keys_added();
    return _imp->properties;
}

const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
EbuildID::fetches_key() const
{
    need_non_xml_keys_added();
    return _imp->src_uri;
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
EbuildID::homepage_key() const
{
    need_non_xml_keys_added();
    return _imp->homepage;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
EbuildID::short_description_key() const
{
    need_non_xml_keys_added();
    return _imp->short_description;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
EbuildID::long_description_key() const
{
    need_xml_keys_added();
    return _imp->long_description;
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >
EbuildID::contents_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >();
}

const std::shared_ptr<const MetadataTimeKey>
EbuildID::installed_time_key() const
{
    return std::shared_ptr<const MetadataTimeKey>();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::from_repositories_key() const
{
    if (might_be_binary())
    {
        need_non_xml_keys_added();
        return _imp->generated_from;
    }
    else
        return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::inherited_key() const
{
    return _imp->inherited;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::defined_phases_key() const
{
    return _imp->defined_phases;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
EbuildID::fs_location_key() const
{
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

void
EbuildID::load_captured_stderr(const std::string & r, const std::string & h, const MetadataKeyType t, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->captured_stderr_key = std::make_shared<LiteralMetadataValueKey<std::string> >(r, h, t, v);
    add_metadata_key(_imp->captured_stderr_key);
}

void
EbuildID::load_captured_stdout(const std::string & r, const std::string & h, const MetadataKeyType t, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->captured_stdout_key = std::make_shared<LiteralMetadataValueKey<std::string> >(r, h, t, v);
    add_metadata_key(_imp->captured_stdout_key);
}

void
EbuildID::load_short_description(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->short_description = std::make_shared<LiteralMetadataValueKey<std::string> >(r, h, mkt_significant, v);
    add_metadata_key(_imp->short_description);
}

void
EbuildID::load_long_description(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->long_description = std::make_shared<LiteralMetadataValueKey<std::string> >(r, h, mkt_normal, v);
    add_metadata_key(_imp->long_description);
}

void
EbuildID::load_dependencies(const std::string & r, const std::string & h, const std::string & v) const
{
    if (! v.empty())
    {
        Lock l(_imp->mutex);
        _imp->dependencies = std::make_shared<EDependenciesKey>(_imp->environment, shared_from_this(), r, h, v,
                    EbuildIDData::get_instance()->raw_dependencies_labels, mkt_dependencies);
        add_metadata_key(_imp->dependencies);
    }
}

void
EbuildID::load_build_depend(const std::string & r, const std::string & h, const std::string & v,
        bool rewritten) const
{
    if (! v.empty())
    {
        Lock l(_imp->mutex);
        _imp->build_dependencies = std::make_shared<EDependenciesKey>(_imp->environment, shared_from_this(), r, h, v,
                    EbuildIDData::get_instance()->build_dependencies_labels, rewritten ? mkt_internal : mkt_dependencies);
        add_metadata_key(_imp->build_dependencies);
    }
}

void
EbuildID::load_run_depend(const std::string & r, const std::string & h, const std::string & v,
        bool rewritten) const
{
    if (! v.empty())
    {
        Lock l(_imp->mutex);
        _imp->run_dependencies = std::make_shared<EDependenciesKey>(_imp->environment, shared_from_this(), r, h, v,
                    EbuildIDData::get_instance()->run_dependencies_labels, rewritten ? mkt_internal : mkt_dependencies);
        add_metadata_key(_imp->run_dependencies);
    }
}

void
EbuildID::load_post_depend(const std::string & r, const std::string & h, const std::string & v,
        bool rewritten) const
{
    if (! v.empty())
    {
        Lock l(_imp->mutex);
        _imp->post_dependencies = std::make_shared<EDependenciesKey>(_imp->environment, shared_from_this(), r, h, v,
                    EbuildIDData::get_instance()->post_dependencies_labels, rewritten ? mkt_internal : mkt_dependencies);
        add_metadata_key(_imp->post_dependencies);
    }
}

void
EbuildID::load_src_uri(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->src_uri = std::make_shared<EFetchableURIKey>(_imp->environment, shared_from_this(), m, v, mkt_dependencies);
    add_metadata_key(_imp->src_uri);
}

void
EbuildID::load_homepage(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->homepage = std::make_shared<ESimpleURIKey>(_imp->environment, m, eapi(), v, mkt_significant, is_installed());
    add_metadata_key(_imp->homepage);
}

void
EbuildID::load_license(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->license = std::make_shared<ELicenseKey>(_imp->environment, m, eapi(), v, mkt_internal, is_installed());
    add_metadata_key(_imp->license);
}

void
EbuildID::load_restrict(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    if (! v.empty())
    {
        Lock l(_imp->mutex);
        _imp->restrictions = std::make_shared<EPlainTextSpecKey>(_imp->environment, m, eapi(), v, mkt_internal, is_installed());
        add_metadata_key(_imp->restrictions);
    }
}

void
EbuildID::load_properties(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    if (! v.empty())
    {
        Lock l(_imp->mutex);
        _imp->properties = std::make_shared<EPlainTextSpecKey>(_imp->environment, m, eapi(), v, mkt_internal, is_installed());
        add_metadata_key(_imp->properties);
    }
}

void
EbuildID::load_iuse(const std::shared_ptr<const EAPIMetadataVariable> & k, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->raw_iuse = EStringSetKeyStore::get_instance()->fetch(k, v, mkt_internal);
    add_metadata_key(_imp->raw_iuse);
}

void
EbuildID::load_myoptions(const std::shared_ptr<const EAPIMetadataVariable> & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->raw_myoptions = std::make_shared<EMyOptionsKey>(_imp->environment, h, eapi(), v, mkt_internal, is_installed());
    add_metadata_key(_imp->raw_myoptions);
}

void
EbuildID::load_required_use(const std::shared_ptr<const EAPIMetadataVariable> & k, const std::string & v) const
{
    if (! v.empty())
    {
        Lock l(_imp->mutex);
        _imp->required_use = std::make_shared<ERequiredUseKey>(_imp->environment, k, eapi(), v, mkt_internal, is_installed());
        add_metadata_key(_imp->required_use);
    }
}

void
EbuildID::load_use(const std::shared_ptr<const EAPIMetadataVariable> & r, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->raw_use = EStringSetKeyStore::get_instance()->fetch(r, v, mkt_internal);
    add_metadata_key(_imp->raw_use);
}

void
EbuildID::load_keywords(const std::shared_ptr<const EAPIMetadataVariable> & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->keywords = EKeywordsKeyStore::get_instance()->fetch(h, v, mkt_internal);
    add_metadata_key(_imp->keywords);
}

void
EbuildID::load_inherited(const std::shared_ptr<const EAPIMetadataVariable> & r, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->inherited = EStringSetKeyStore::get_instance()->fetch(r, v, mkt_internal);
    add_metadata_key(_imp->inherited);
}

void
EbuildID::load_defined_phases(const std::shared_ptr<const EAPIMetadataVariable> & h, const std::string & v) const
{
    if (v.empty())
        throw InternalError(PALUDIS_HERE, "v should not be empty");

    Lock l(_imp->mutex);
    _imp->defined_phases = EStringSetKeyStore::get_instance()->fetch(h, v, mkt_internal);
    add_metadata_key(_imp->defined_phases);
}

void
EbuildID::load_upstream_changelog(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->upstream_changelog = std::make_shared<ESimpleURIKey>(_imp->environment, m, eapi(), v, mkt_normal, is_installed());
    add_metadata_key(_imp->upstream_changelog);
}

void
EbuildID::load_upstream_documentation(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->upstream_documentation = std::make_shared<ESimpleURIKey>(_imp->environment, m, eapi(), v, mkt_normal, is_installed());
    add_metadata_key(_imp->upstream_documentation);
}

void
EbuildID::load_upstream_release_notes(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->upstream_release_notes = std::make_shared<ESimpleURIKey>(_imp->environment, m, eapi(), v, mkt_normal, is_installed());
    add_metadata_key(_imp->upstream_release_notes);
}

void
EbuildID::load_bugs_to(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->bugs_to = std::make_shared<EPlainTextSpecKey>(_imp->environment, m, eapi(), v, mkt_normal, is_installed());
    add_metadata_key(_imp->bugs_to);
}

void
EbuildID::load_remote_ids(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->remote_ids = std::make_shared<EPlainTextSpecKey>(_imp->environment, m, eapi(), v, mkt_internal, is_installed());
    add_metadata_key(_imp->remote_ids);
}

void
EbuildID::load_slot(const std::shared_ptr<const EAPIMetadataVariable> & m, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->slot = ESlotKeyStore::get_instance()->fetch(m, v, mkt_internal);
    add_metadata_key(_imp->slot);
}

void
EbuildID::load_generated_from(const std::shared_ptr<const EAPIMetadataVariable> & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->generated_from = EStringSetKeyStore::get_instance()->fetch(h, v, mkt_normal);
    add_metadata_key(_imp->generated_from);
}

void
EbuildID::load_generated_time(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->generated_time = std::make_shared<LiteralMetadataTimeKey>(r, h, mkt_normal, Timestamp(destringify<std::time_t>(v), 0));
    add_metadata_key(_imp->generated_time);
}

void
EbuildID::load_generated_using(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->generated_using = std::make_shared<LiteralMetadataValueKey<std::string> >(r, h, mkt_normal, v);
    add_metadata_key(_imp->generated_using);
}

void
EbuildID::load_scm_revision(const std::string & r, const std::string & h, const std::string & v) const
{
    Lock l(_imp->mutex);
    _imp->scm_revision = std::make_shared<LiteralMetadataValueKey<std::string> >(r, h, mkt_normal, v);
    add_metadata_key(_imp->scm_revision);
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
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;

        void visit(InstallAction & a)
        {
            auto repo(env->fetch_repository(id->repository_name()));
            auto e_repo(std::static_pointer_cast<const ERepository>(repo));
            do_install_action(
                    env,
                    e_repo.get(),
                    std::static_pointer_cast<const ERepositoryID>(id),
                    a);
        }

        void visit(FetchAction & a)
        {
            auto repo(env->fetch_repository(id->repository_name()));
            auto e_repo(std::static_pointer_cast<const ERepository>(repo));
            do_fetch_action(
                    env,
                    e_repo.get(),
                    std::static_pointer_cast<const ERepositoryID>(id),
                    a);
        }

        void visit(PretendFetchAction & a)
        {
            auto repo(env->fetch_repository(id->repository_name()));
            auto e_repo(std::static_pointer_cast<const ERepository>(repo));
            do_pretend_fetch_action(
                    env,
                    e_repo.get(),
                    std::static_pointer_cast<const ERepositoryID>(id),
                    a);
        }

        void visit(PretendAction & action)
        {
            auto repo(env->fetch_repository(id->repository_name()));
            auto e_repo(std::static_pointer_cast<const ERepository>(repo));
            if (! do_pretend_action(
                        env,
                        e_repo.get(),
                        std::static_pointer_cast<const ERepositoryID>(id),
                        action))
                action.set_failed();
        }

        void visit(InfoAction & action)
        {
            auto repo(env->fetch_repository(id->repository_name()));
            auto e_repo(std::static_pointer_cast<const ERepository>(repo));
            do_info_action(
                    env,
                    e_repo.get(),
                    std::static_pointer_cast<const ERepositoryID>(id),
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

    PerformAction b{_imp->environment, shared_from_this()};
    a.accept(b);
}

const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EbuildID::remote_ids_key() const
{
    need_non_xml_keys_added();
    return _imp->remote_ids;
}

const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EbuildID::bugs_to_key() const
{
    need_non_xml_keys_added();
    return _imp->bugs_to;
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
EbuildID::upstream_changelog_key() const
{
    need_non_xml_keys_added();
    return _imp->upstream_changelog;
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
EbuildID::upstream_documentation_key() const
{
    need_non_xml_keys_added();
    return _imp->upstream_documentation;
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
EbuildID::upstream_release_notes_key() const
{
    need_non_xml_keys_added();
    return _imp->upstream_release_notes;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EbuildID::generated_from_key() const
{
    need_non_xml_keys_added();
    return _imp->generated_from;
}

const std::shared_ptr<const MetadataTimeKey>
EbuildID::generated_time_key() const
{
    need_non_xml_keys_added();
    return _imp->generated_time;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
EbuildID::generated_using_key() const
{
    need_non_xml_keys_added();
    return _imp->generated_using;
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >
EbuildID::choices_key() const
{
    need_non_xml_keys_added();
    return _imp->choices;
}

const std::shared_ptr<const MetadataValueKey<SlotName> >
EbuildID::slot_key() const
{
    need_non_xml_keys_added();
    return _imp->slot;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
EbuildID::scm_revision_key() const
{
    need_non_xml_keys_added();
    return _imp->scm_revision;
}

namespace
{
    std::string get_description(const std::shared_ptr<const UseDesc> & use_desc, const std::string & override_description,
            const QualifiedPackageName & package, const ChoicePrefixName & prefix, const UnprefixedChoiceName & suffix)
    {
        if (! override_description.empty())
            return override_description;

        if (use_desc)
            return use_desc->describe(package, prefix, suffix);
        else
            return "";
    }
}


const std::shared_ptr<const ChoiceValue>
EbuildID::make_choice_value(
        const std::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_name,
        const Tribool iuse_default,
        const bool iuse_default_wins,
        const ChoiceOrigin origin,
        const std::string & override_description,
        const bool force_locked
        ) const
{
    if (! eapi()->supported())
        throw InternalError(PALUDIS_HERE, "Unsupported EAPI");

    auto repo(_imp->environment->fetch_repository(repository_name()));
    auto e_repo(std::static_pointer_cast<const ERepository>(repo));

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
        auto raw_use(raw_use_key()->parse_value());
        enabled = enabled_by_default = (raw_use->end() != raw_use->find(name_with_prefix_s));
    }
    else
    {
        if (e_repo->profile()->use_masked(shared_from_this(), choice, value_name, name_with_prefix))
        {
            locked = true;
            enabled = enabled_by_default = false;
        }
        else if (e_repo->profile()->use_forced(shared_from_this(), choice, value_name, name_with_prefix))
        {
            locked = true;
            enabled = enabled_by_default = true;
        }
        else if (iuse_default_wins && ! iuse_default.is_indeterminate())
        {
            if (iuse_default.is_true())
                enabled_by_default = true;
            else
                enabled_by_default = false;

            enabled = enabled_by_default;
        }
        else
        {
            Tribool profile_want(e_repo->profile()->use_state_ignoring_masks(shared_from_this(), choice, value_name, name_with_prefix));
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

    return create_e_choice_value(make_named_values<EChoiceValueParams>(
                n::choice_name_with_prefix() = ChoiceNameWithPrefix(name_with_prefix),
                n::choice_prefix_name() = choice->prefix(),
                n::description() = get_description(e_repo->use_desc(), override_description, name(), choice->prefix(), value_name),
                n::enabled() = enabled,
                n::enabled_by_default() = enabled_by_default,
                n::locked() = force_locked || locked,
                n::origin() = origin,
                n::unprefixed_choice_name() = value_name
                ));
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
EbuildID::add_build_options(const std::shared_ptr<Choices> & choices) const
{
    if (eapi()->supported())
    {
        std::shared_ptr<Choice> build_options(std::make_shared<Choice>(make_named_values<ChoiceParams>(
                        n::consider_added_or_changed() = false,
                        n::contains_every_value() = false,
                        n::hidden() = false,
                        n::human_name() = canonical_build_options_human_name(),
                        n::prefix() = canonical_build_options_prefix(),
                        n::raw_name() = canonical_build_options_raw_name(),
                        n::show_with_no_prefix() = false
                        )));
        choices->add(build_options);

        if (! eapi()->supported()->is_pbin())
        {
            bool may_be_unrestricted_test(true), may_be_unrestricted_strip(true);

            /* if we unconditionally restrict an action, don't add / force mask
             * a build option for it. but if we conditionally restrict it, do,
             * to avoid weirdness in cases like RESTRICT="test? ( test )." */
            if (restrict_key())
            {
                UnconditionalRestrictFinder f;
                restrict_key()->parse_value()->top()->accept(f);
                may_be_unrestricted_test = f.s.end() == f.s.find("test");
                may_be_unrestricted_strip = f.s.end() == f.s.find("strip");
            }

            /* optional_tests */
            if (eapi()->supported()->choices_options()->has_optional_tests())
                build_options->add(std::make_shared<ELikeOptionalTestsChoiceValue>(shared_from_this(), _imp->environment, build_options,
                            ! may_be_unrestricted_test));

            /* recommended_tests */
            if (eapi()->supported()->choices_options()->has_recommended_tests())
                build_options->add(std::make_shared<ELikeRecommendedTestsChoiceValue>(shared_from_this(), _imp->environment, build_options,
                            ! may_be_unrestricted_test));

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
                        auto defined_phases(_imp->defined_phases->parse_value());
                        if (defined_phases->end() != defined_phases->find(phase->equal_option("skipname")))
                        {
                            has_expensive_test_phase = true;
                            break;
                        }
                    }
                }

                if (has_expensive_test_phase)
                    build_options->add(std::make_shared<ELikeExpensiveTestsChoiceValue>(shared_from_this(), _imp->environment, build_options, false));
            }

            /* symbols */
            if (may_be_unrestricted_strip)
                build_options->add(std::make_shared<ELikeSymbolsChoiceValue>(shared_from_this(), _imp->environment, build_options, last_escvp));

            /* jobs */
            if (! eapi()->supported()->ebuild_environment_variables()->env_jobs().empty())
            {
                if (! _imp->defined_phases)
                    throw InternalError(PALUDIS_HERE, "bug! no defined_phases yet");

                build_options->add(std::make_shared<ELikeJobsChoiceValue>(
                            shared_from_this(), _imp->environment, build_options));
            }
        }

        /* trace */
        build_options->add(std::make_shared<ELikeTraceChoiceValue>(
                        shared_from_this(), _imp->environment, build_options));

        /* preserve_work */
        build_options->add(std::make_shared<ELikePreserveWorkChoiceValue>(
                        shared_from_this(), _imp->environment, build_options, indeterminate));
    }
}

void
EbuildID::purge_invalid_cache() const
{
    auto repo(_imp->environment->fetch_repository(repository_name()));
    auto e_repo(std::static_pointer_cast<const ERepository>(repo));

    FSPath write_cache_file(e_repo->params().write_cache());
    if (e_repo->params().append_repository_name_to_write_cache())
        write_cache_file /= stringify(repository_name());
    write_cache_file /= stringify(name().category());
    write_cache_file /= stringify(name().package()) + "-" + stringify(version());

    if (write_cache_file.stat().exists())
    {
        if (e_repo->params().write_cache().basename() != "empty")
        {
            EbuildFlatMetadataCache write_metadata_cache(_imp->environment,
                    write_cache_file, _imp->fs_location->parse_value(), _imp->master_mtime, _imp->eclass_mtimes, true);
            if (! write_metadata_cache.load(shared_from_this(), true))
                write_cache_file.unlink();
        }
    }
}

bool
EbuildID::might_be_binary() const
{
    auto path(stringify(_imp->fs_location->parse_value()));
    auto dot_pos(path.rfind('.'));

    if (std::string::npos != dot_pos)
    {
        auto extension(path.substr(dot_pos + 1));
        return 0 == extension.compare(0, 4, "pbin");
    }
    else
        return false;
}

bool
EbuildID::is_installed() const
{
    return false;
}

void
EbuildID::set_scm_revision(const std::string & s) const
{
    Lock l(_imp->mutex);

    if ((! eapi()->supported()) ||
            eapi()->supported()->ebuild_metadata_variables()->scm_revision()->name().empty() ||
            _imp->scm_revision)
        throw CannotChangeSCMRevision(stringify(*this), s);
    else
    {
        _imp->scm_revision = std::make_shared<LiteralMetadataValueKey<std::string> >(
                eapi()->supported()->ebuild_metadata_variables()->scm_revision()->name(),
                eapi()->supported()->ebuild_metadata_variables()->scm_revision()->description(),
                mkt_normal,
                s);

        add_metadata_key(_imp->scm_revision);
    }
}

