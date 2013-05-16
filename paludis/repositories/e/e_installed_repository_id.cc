/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/repositories/e/e_installed_repository_id.hh>
#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/e_choices_key.hh>
#include <paludis/repositories/e/extra_distribution_data.hh>
#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/e_choice_value.hh>
#include <paludis/repositories/e/e_string_set_key.hh>
#include <paludis/repositories/e/e_slot_key.hh>

#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/strip.hh>

#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>
#include <paludis/output_manager.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/elike_choices.hh>
#include <paludis/always_enabled_dependency_label.hh>
#include <paludis/slot.hh>

#include <iterator>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    std::string file_contents(const FSPath & f)
    {
        Context c("When reading '" + stringify(f) + "':");
        SafeIFStream i(f);
        return strip_trailing(std::string((std::istreambuf_iterator<char>(i)), std::istreambuf_iterator<char>()), "\r\n");
    }

    struct EInstalledRepositoryIDKeys
    {
        std::shared_ptr<const MetadataValueKey<Slot> > slot;
        std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use;
        std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > inherited;
        std::shared_ptr<const MetadataCollectionKey<Set<std::string > > > raw_iuse;
        std::shared_ptr<const MetadataCollectionKey<Set<std::string > > > raw_iuse_effective;
        std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > raw_myoptions;
        std::shared_ptr<const MetadataSpecTreeKey<RequiredUseSpecTree> > required_use;
        std::shared_ptr<const MetadataCollectionKey<Set<std::string > > > raw_use_expand;
        std::shared_ptr<const MetadataCollectionKey<Set<std::string > > > raw_use_expand_hidden;
        std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > > choices;
        std::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license;
        std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > dependencies;
        std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies;
        std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies;
        std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies;
        std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > restrictions;
        std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > properties;
        std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > src_uri;
        std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > homepage;
        std::shared_ptr<const MetadataValueKey<std::string> > short_description;
        std::shared_ptr<const MetadataValueKey<std::string> > long_description;
        std::shared_ptr<const MetadataTimeKey> installed_time;
        std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > from_repositories;
        std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > upstream_changelog;
        std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > upstream_documentation;
        std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> > upstream_release_notes;
        std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > remote_ids;
        std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > bugs_to;
        std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > defined_phases;
        std::shared_ptr<const MetadataValueKey<std::string> > scm_revision;

        std::shared_ptr<const MetadataValueKey<std::string> > asflags;
        std::shared_ptr<const MetadataValueKey<std::string> > cbuild;
        std::shared_ptr<const MetadataValueKey<std::string> > cflags;
        std::shared_ptr<const MetadataValueKey<std::string> > chost;
        std::shared_ptr<const MetadataValueKey<std::string> > config_protect;
        std::shared_ptr<const MetadataValueKey<std::string> > config_protect_mask;
        std::shared_ptr<const MetadataValueKey<std::string> > ctarget;
        std::shared_ptr<const MetadataValueKey<std::string> > cxxflags;
        std::shared_ptr<const MetadataValueKey<std::string> > ldflags;
        std::shared_ptr<const MetadataValueKey<std::string> > pkgmanager;
        std::shared_ptr<const MetadataValueKey<std::string> > vdb_format;
    };


    struct EInstalledRepositoryIDData :
        Singleton<EInstalledRepositoryIDData>
    {
        std::shared_ptr<DependenciesLabelSequence> raw_dependencies_labels;
        std::shared_ptr<DependenciesLabelSequence> build_dependencies_labels;
        std::shared_ptr<DependenciesLabelSequence> run_dependencies_labels;
        std::shared_ptr<DependenciesLabelSequence> post_dependencies_labels;

        EInstalledRepositoryIDData() :
            raw_dependencies_labels(std::make_shared<DependenciesLabelSequence>()),
            build_dependencies_labels(std::make_shared<DependenciesLabelSequence>()),
            run_dependencies_labels(std::make_shared<DependenciesLabelSequence>()),
            post_dependencies_labels(std::make_shared<DependenciesLabelSequence>())
        {
            raw_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag> >("build"));
            raw_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesRunLabelTag> >("run"));

            build_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag> >("DEPEND"));
            run_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesRunLabelTag> >("RDEPEND"));
            post_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesPostLabelTag> >("PDEPEND"));
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<EInstalledRepositoryID>
    {
        mutable std::recursive_mutex mutex;

        const QualifiedPackageName name;
        const VersionSpec version;
        const Environment * const environment;
        const RepositoryName repository_name;
        const FSPath dir;

        mutable std::shared_ptr<EInstalledRepositoryIDKeys> keys;

        /* fs location and eapi are special */
        mutable std::shared_ptr<const MetadataValueKey<FSPath> > fs_location;
        mutable std::shared_ptr<const EAPI> eapi;

        Imp(const QualifiedPackageName & q, const VersionSpec & v,
                const Environment * const e,
                const RepositoryName & r, const FSPath & f) :
            name(q),
            version(v),
            environment(e),
            repository_name(r),
            dir(f)
        {
        }
    };
}

EInstalledRepositoryID::EInstalledRepositoryID(const QualifiedPackageName & q, const VersionSpec & v,
        const Environment * const e,
        const RepositoryName & r,
        const FSPath & f) :
    _imp(q, v, e, r, f)
{
}

EInstalledRepositoryID::~EInstalledRepositoryID()
{
}

void
EInstalledRepositoryID::need_keys_added() const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->mutex);

    if (_imp->keys)
        return;
    _imp->keys = std::make_shared<EInstalledRepositoryIDKeys>();

    // fs_location key could have been loaded by the ::fs_location_key() already. keep this
    // at the top, other keys use it.
    if (! _imp->fs_location)
    {
        _imp->fs_location = std::make_shared<LiteralMetadataValueKey<FSPath> >(fs_location_raw_name(), fs_location_human_name(),
                    mkt_internal, _imp->dir);
        add_metadata_key(_imp->fs_location);
    }

    Context context("When loading ID keys from '" + stringify(_imp->dir) + "':");

    add_metadata_key(std::make_shared<LiteralMetadataValueKey<std::string>>("EAPI", "EAPI", mkt_internal, eapi()->name()));

    if (! eapi()->supported())
    {
        Log::get_instance()->message("e.eapi.unsupported", ll_debug, lc_context)
            << "Not loading further keys for '" << *this << "' because EAPI '"
            << eapi()->name() << "' is not supported";
        return;
    }

    std::shared_ptr<const EAPIEbuildMetadataVariables> vars(eapi()->supported()->ebuild_metadata_variables());
    std::shared_ptr<const EAPIEbuildEnvironmentVariables> env(eapi()->supported()->ebuild_environment_variables());

    if (! env->env_use().empty())
        if ((_imp->dir / env->env_use()).stat().exists())
        {
            _imp->keys->raw_use = EStringSetKeyStore::get_instance()->fetch(vars->use(), file_contents(_imp->dir / env->env_use()), mkt_internal);
            add_metadata_key(_imp->keys->raw_use);
        }

    if (! vars->slot()->name().empty())
        if ((_imp->dir / vars->slot()->name()).stat().exists())
        {
            _imp->keys->slot = ESlotKeyStore::get_instance()->fetch(*eapi(), vars->slot(), file_contents(_imp->dir / vars->slot()->name()), mkt_internal);
            add_metadata_key(_imp->keys->slot);
        }

    if (! vars->inherited()->name().empty())
        if ((_imp->dir / vars->inherited()->name()).stat().exists())
        {
            _imp->keys->inherited = EStringSetKeyStore::get_instance()->fetch(vars->inherited(),
                    file_contents(_imp->dir / vars->inherited()->name()), mkt_internal);
            add_metadata_key(_imp->keys->inherited);
        }

    if (! vars->defined_phases()->name().empty())
        if ((_imp->dir / vars->defined_phases()->name()).stat().exists())
        {
            std::string d(file_contents(_imp->dir / vars->defined_phases()->name()));
            if (! strip_leading(d, " \t\r\n").empty())
            {
                _imp->keys->defined_phases = EStringSetKeyStore::get_instance()->fetch(vars->defined_phases(),
                        d, mkt_internal);
                add_metadata_key(_imp->keys->defined_phases);
            }
        }

    if (! vars->scm_revision()->name().empty())
        if ((_imp->dir / vars->scm_revision()->name()).stat().exists())
        {
            std::string d(file_contents(_imp->dir / vars->scm_revision()->name()));
            if (! d.empty())
            {
                _imp->keys->scm_revision = std::make_shared<LiteralMetadataValueKey<std::string> >(vars->scm_revision()->name(),
                        vars->scm_revision()->description(), mkt_normal, d);
                add_metadata_key(_imp->keys->scm_revision);
            }
        }

    if (! vars->iuse()->name().empty())
    {
        if ((_imp->dir / vars->iuse()->name()).stat().exists())
            _imp->keys->raw_iuse = EStringSetKeyStore::get_instance()->fetch(vars->iuse(),
                    file_contents(_imp->dir / vars->iuse()->name()), mkt_internal);
        else
        {
            /* hack: if IUSE doesn't exist, we still need an iuse_key to make the choices
             * code behave sanely. */
            _imp->keys->raw_iuse = EStringSetKeyStore::get_instance()->fetch(vars->iuse(),
                    "", mkt_internal);
        }
        add_metadata_key(_imp->keys->raw_iuse);
    }

    if (! vars->iuse_effective()->name().empty())
    {
        if ((_imp->dir / vars->iuse_effective()->name()).stat().exists())
        {
            _imp->keys->raw_iuse_effective = EStringSetKeyStore::get_instance()->fetch(vars->iuse_effective(),
                    file_contents(_imp->dir / vars->iuse_effective()->name()), mkt_internal);
            add_metadata_key(_imp->keys->raw_iuse_effective);
        }
    }

    if (! vars->myoptions()->name().empty())
        if ((_imp->dir / vars->myoptions()->name()).stat().exists())
        {
            _imp->keys->raw_myoptions = std::make_shared<EMyOptionsKey>(_imp->environment, vars->myoptions(),
                        eapi(), file_contents(_imp->dir / vars->myoptions()->name()), mkt_internal, is_installed());
            add_metadata_key(_imp->keys->raw_myoptions);
        }

    if (! vars->required_use()->name().empty())
        if ((_imp->dir / vars->required_use()->name()).stat().exists())
        {
            std::string v(file_contents(_imp->dir / vars->required_use()->name()));
            if (! strip_leading(v, " \t\r\n").empty())
            {
                _imp->keys->required_use = std::make_shared<ERequiredUseKey>(_imp->environment, vars->required_use(),
                        eapi(), v, mkt_internal, is_installed());
                add_metadata_key(_imp->keys->required_use);
            }
        }

    if (! vars->use_expand()->name().empty())
        if ((_imp->dir / vars->use_expand()->name()).stat().exists())
        {
            _imp->keys->raw_use_expand = EStringSetKeyStore::get_instance()->fetch(vars->use_expand(),
                    file_contents(_imp->dir / vars->use_expand()->name()), mkt_internal);
            add_metadata_key(_imp->keys->raw_use_expand);
        }

    if (! vars->use_expand_hidden()->name().empty())
        if ((_imp->dir / vars->use_expand_hidden()->name()).stat().exists())
        {
            _imp->keys->raw_use_expand_hidden = EStringSetKeyStore::get_instance()->fetch(vars->use_expand_hidden(),
                    file_contents(_imp->dir / vars->use_expand_hidden()->name()), mkt_internal);
            add_metadata_key(_imp->keys->raw_use_expand_hidden);
        }

    if (! vars->license()->name().empty())
        if ((_imp->dir / vars->license()->name()).stat().exists())
        {
            _imp->keys->license = std::make_shared<ELicenseKey>(_imp->environment, vars->license(), eapi(),
                        file_contents(_imp->dir / vars->license()->name()), mkt_normal, is_installed());
            add_metadata_key(_imp->keys->license);
        }

    if (! vars->dependencies()->name().empty())
    {
        if ((_imp->dir / vars->dependencies()->name()).stat().exists())
        {
            std::string v(file_contents(_imp->dir / vars->dependencies()->name()));
            if (! strip_leading(v, " \t\r\n").empty())
            {
                _imp->keys->dependencies = std::make_shared<EDependenciesKey>(_imp->environment, shared_from_this(), vars->dependencies()->name(),
                            vars->dependencies()->description(), v,
                            EInstalledRepositoryIDData::get_instance()->build_dependencies_labels, mkt_dependencies);
                add_metadata_key(_imp->keys->dependencies);
            }
        }
    }
    else
    {
        if (! vars->build_depend()->name().empty())
            if ((_imp->dir / vars->build_depend()->name()).stat().exists())
            {
                std::string v(file_contents(_imp->dir / vars->build_depend()->name()));
                if (! strip_leading(v, " \t\r\n").empty())
                {
                    _imp->keys->build_dependencies = std::make_shared<EDependenciesKey>(_imp->environment, shared_from_this(), vars->build_depend()->name(),
                                vars->build_depend()->description(), v,
                                EInstalledRepositoryIDData::get_instance()->build_dependencies_labels, mkt_dependencies);
                    add_metadata_key(_imp->keys->build_dependencies);
                }
            }

        if (! vars->run_depend()->name().empty())
            if ((_imp->dir / vars->run_depend()->name()).stat().exists())
            {
                std::string v(file_contents(_imp->dir / vars->run_depend()->name()));
                if (! strip_leading(v, " \t\r\n").empty())
                {
                    _imp->keys->run_dependencies = std::make_shared<EDependenciesKey>(_imp->environment, shared_from_this(), vars->run_depend()->name(),
                                vars->run_depend()->description(), v,
                                EInstalledRepositoryIDData::get_instance()->run_dependencies_labels, mkt_dependencies);
                    add_metadata_key(_imp->keys->run_dependencies);
                }
            }

        if (! vars->pdepend()->name().empty())
        {
            if ((_imp->dir / vars->pdepend()->name()).stat().exists())
            {
                std::string v(file_contents(_imp->dir / vars->pdepend()->name()));
                if (! strip_leading(v, " \t\r\n").empty())
                {
                    _imp->keys->post_dependencies = std::make_shared<EDependenciesKey>(_imp->environment, shared_from_this(), vars->pdepend()->name(),
                                vars->pdepend()->description(), v,
                                EInstalledRepositoryIDData::get_instance()->post_dependencies_labels, mkt_dependencies);
                    add_metadata_key(_imp->keys->post_dependencies);
                }
            }
        }
    }

    if (! vars->restrictions()->name().empty())
        if ((_imp->dir / vars->restrictions()->name()).stat().exists())
        {
            std::string v(file_contents(_imp->dir / vars->restrictions()->name()));
            if (! strip_leading(v, " \t\r\n").empty())
            {
                _imp->keys->restrictions = std::make_shared<EPlainTextSpecKey>(_imp->environment, vars->restrictions(),
                            eapi(), v, mkt_internal, is_installed());
                add_metadata_key(_imp->keys->restrictions);
            }
        }

    if (! vars->properties()->name().empty())
        if ((_imp->dir / vars->properties()->name()).stat().exists())
        {
            std::string v(file_contents(_imp->dir / vars->properties()->name()));
            if (! strip_leading(v, " \t\r\n").empty())
            {
                _imp->keys->properties = std::make_shared<EPlainTextSpecKey>(_imp->environment, vars->properties(),
                            eapi(), v, mkt_internal, is_installed());
                add_metadata_key(_imp->keys->properties);
            }
        }

    if (! vars->src_uri()->name().empty())
        if ((_imp->dir / vars->src_uri()->name()).stat().exists())
        {
            _imp->keys->src_uri = std::make_shared<EFetchableURIKey>(_imp->environment, shared_from_this(), vars->src_uri(),
                        file_contents(_imp->dir / vars->src_uri()->name()), mkt_dependencies);
            add_metadata_key(_imp->keys->src_uri);
        }

    if (! vars->short_description()->name().empty())
        if ((_imp->dir / vars->short_description()->name()).stat().exists())
        {
            _imp->keys->short_description = std::make_shared<LiteralMetadataValueKey<std::string> >(vars->short_description()->name(),
                        vars->short_description()->description(), mkt_significant, file_contents(_imp->dir / vars->short_description()->name()));
            add_metadata_key(_imp->keys->short_description);
        }

    if (! vars->long_description()->name().empty())
        if ((_imp->dir / vars->long_description()->name()).stat().exists())
        {
            std::string value(file_contents(_imp->dir / vars->long_description()->name()));
            if (! strip_leading(value, " \t\r\n").empty())
            {
                _imp->keys->long_description = std::make_shared<LiteralMetadataValueKey<std::string> >(vars->long_description()->name(),
                            vars->long_description()->description(), mkt_significant, value);
                add_metadata_key(_imp->keys->long_description);
            }
        }

    if (! vars->upstream_changelog()->name().empty())
        if ((_imp->dir / vars->upstream_changelog()->name()).stat().exists())
        {
            std::string value(file_contents(_imp->dir / vars->upstream_changelog()->name()));
            if (! strip_leading(value, " \t\r\n").empty())
            {
                _imp->keys->upstream_changelog = std::make_shared<ESimpleURIKey>(_imp->environment,
                            vars->upstream_changelog(), eapi(), value, mkt_normal, is_installed());
                add_metadata_key(_imp->keys->upstream_changelog);
            }
        }

    if (! vars->upstream_release_notes()->name().empty())
        if ((_imp->dir / vars->upstream_release_notes()->name()).stat().exists())
        {
            std::string value(file_contents(_imp->dir / vars->upstream_release_notes()->name()));
            if (! strip_leading(value, " \t\r\n").empty())
            {
                _imp->keys->upstream_release_notes = std::make_shared<ESimpleURIKey>(_imp->environment,
                            vars->upstream_release_notes(), eapi(), value, mkt_normal, is_installed());
                add_metadata_key(_imp->keys->upstream_release_notes);
            }
        }

    if (! vars->upstream_documentation()->name().empty())
        if ((_imp->dir / vars->upstream_documentation()->name()).stat().exists())
        {
            std::string value(file_contents(_imp->dir / vars->upstream_documentation()->name()));
            if (! strip_leading(value, " \t\r\n").empty())
            {
                _imp->keys->upstream_documentation = std::make_shared<ESimpleURIKey>(_imp->environment,
                            vars->upstream_documentation(), eapi(), value, mkt_normal, is_installed());
                add_metadata_key(_imp->keys->upstream_documentation);
            }
        }

    if (! vars->bugs_to()->name().empty())
        if ((_imp->dir / vars->bugs_to()->name()).stat().exists())
        {
            std::string value(file_contents(_imp->dir / vars->bugs_to()->name()));
            if (! strip_leading(value, " \t\r\n").empty())
            {
                _imp->keys->bugs_to = std::make_shared<EPlainTextSpecKey>(_imp->environment, vars->bugs_to(), eapi(), value, mkt_normal, is_installed());
                add_metadata_key(_imp->keys->bugs_to);
            }
        }

    if (! vars->remote_ids()->name().empty())
        if ((_imp->dir / vars->remote_ids()->name()).stat().exists())
        {
            std::string value(file_contents(_imp->dir / vars->remote_ids()->name()));
            if (! strip_leading(value, " \t\r\n").empty())
            {
                _imp->keys->remote_ids = std::make_shared<EPlainTextSpecKey>(_imp->environment,
                            vars->remote_ids(), eapi(), value, mkt_internal, is_installed());
                add_metadata_key(_imp->keys->remote_ids);
            }
        }

    if (! vars->homepage()->name().empty())
        if ((_imp->dir / vars->homepage()->name()).stat().exists())
        {
            _imp->keys->homepage = std::make_shared<ESimpleURIKey>(_imp->environment, vars->homepage(), eapi(),
                        file_contents(_imp->dir / vars->homepage()->name()), mkt_significant, is_installed());
            add_metadata_key(_imp->keys->homepage);
        }

    _imp->keys->installed_time = std::make_shared<EMTimeKey>("INSTALLED_TIME", "Installed time",
                _imp->dir / contents_filename(), mkt_normal);
    add_metadata_key(_imp->keys->installed_time);

    if (_imp->eapi->supported())
        _imp->keys->choices = std::make_shared<EChoicesKey>(_imp->environment, shared_from_this(), "PALUDIS_CHOICES",
                    _imp->eapi->supported()->ebuild_environment_variables()->description_choices(),
                    mkt_normal, make_null_shared_ptr(), std::bind(return_literal_function(make_null_shared_ptr())));
    else
        _imp->keys->choices = std::make_shared<EChoicesKey>(_imp->environment, shared_from_this(), "PALUDIS_CHOICES", "Choices", mkt_normal,
                    make_null_shared_ptr(), std::bind(return_literal_function(make_null_shared_ptr())));

    add_metadata_key(_imp->keys->choices);

    std::shared_ptr<Set<std::string> > from_repositories_value(std::make_shared<Set<std::string>>());
    if ((_imp->dir / "REPOSITORY").stat().exists())
        from_repositories_value->insert(file_contents(_imp->dir / "REPOSITORY"));
    if ((_imp->dir / "repository").stat().exists())
        from_repositories_value->insert(file_contents(_imp->dir / "repository"));
    if ((_imp->dir / "BINARY_REPOSITORY").stat().exists())
        from_repositories_value->insert(file_contents(_imp->dir / "BINARY_REPOSITORY"));
    if (! from_repositories_value->empty())
    {
        _imp->keys->from_repositories = std::make_shared<LiteralMetadataStringSetKey>("REPOSITORIES",
                    "From repositories", mkt_normal, from_repositories_value);
        add_metadata_key(_imp->keys->from_repositories);
    }

    if ((_imp->dir / "ASFLAGS").stat().exists())
    {
        _imp->keys->asflags = std::make_shared<LiteralMetadataValueKey<std::string> >("ASFLAGS", "ASFLAGS",
                    mkt_internal, file_contents(_imp->dir / "ASFLAGS"));
        add_metadata_key(_imp->keys->asflags);
    }

    if ((_imp->dir / "CBUILD").stat().exists())
    {
        _imp->keys->cbuild = std::make_shared<LiteralMetadataValueKey<std::string> >("CBUILD", "CBUILD",
                    mkt_internal, file_contents(_imp->dir / "CBUILD"));
        add_metadata_key(_imp->keys->cbuild);
    }

    if ((_imp->dir / "CFLAGS").stat().exists())
    {
        _imp->keys->cflags = std::make_shared<LiteralMetadataValueKey<std::string> >("CFLAGS", "CFLAGS",
                    mkt_internal, file_contents(_imp->dir / "CFLAGS"));
        add_metadata_key(_imp->keys->cflags);
    }

    if ((_imp->dir / "CHOST").stat().exists())
    {
        _imp->keys->chost = std::make_shared<LiteralMetadataValueKey<std::string> >("CHOST", "CHOST",
                    mkt_internal, file_contents(_imp->dir / "CHOST"));
        add_metadata_key(_imp->keys->chost);
    }

    if ((_imp->dir / "CONFIG_PROTECT").stat().exists())
    {
        _imp->keys->config_protect = std::make_shared<LiteralMetadataValueKey<std::string> >("CONFIG_PROTECT", "CONFIG_PROTECT",
                    mkt_internal, file_contents(_imp->dir / "CONFIG_PROTECT"));
        add_metadata_key(_imp->keys->config_protect);
    }

    if ((_imp->dir / "CONFIG_PROTECT_MASK").stat().exists())
    {
        _imp->keys->config_protect_mask = std::make_shared<LiteralMetadataValueKey<std::string> >("CONFIG_PROTECT_MASK", "CONFIG_PROTECT_MASK",
                    mkt_internal, file_contents(_imp->dir / "CONFIG_PROTECT_MASK"));
        add_metadata_key(_imp->keys->config_protect_mask);
    }

    if ((_imp->dir / "CXXFLAGS").stat().exists())
    {
        _imp->keys->cxxflags = std::make_shared<LiteralMetadataValueKey<std::string> >("CXXFLAGS", "CXXFLAGS",
                    mkt_internal, file_contents(_imp->dir / "CXXFLAGS"));
        add_metadata_key(_imp->keys->cxxflags);
    }

    if ((_imp->dir / "LDFLAGS").stat().exists())
    {
        _imp->keys->ldflags = std::make_shared<LiteralMetadataValueKey<std::string> >("LDFLAGS", "LDFLAGS",
                    mkt_internal, file_contents(_imp->dir / "LDFLAGS"));
        add_metadata_key(_imp->keys->ldflags);
    }

    if ((_imp->dir / "PKGMANAGER").stat().exists())
    {
        _imp->keys->pkgmanager = std::make_shared<LiteralMetadataValueKey<std::string> >("PKGMANAGER", "Installed using",
                    mkt_normal, file_contents(_imp->dir / "PKGMANAGER"));
        add_metadata_key(_imp->keys->pkgmanager);
    }

    if ((_imp->dir / "VDB_FORMAT").stat().exists())
    {
        _imp->keys->vdb_format = std::make_shared<LiteralMetadataValueKey<std::string> >("VDB_FORMAT", "VDB Format",
                    mkt_internal, file_contents(_imp->dir / "VDB_FORMAT"));
        add_metadata_key(_imp->keys->vdb_format);
    }
}

void
EInstalledRepositoryID::need_masks_added() const
{
}

const std::string
EInstalledRepositoryID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            if (_imp->keys && _imp->keys->slot)
                return stringify(name()) + "-" + stringify(version()) + ":" + stringify(_imp->keys->slot->parse_value().parallel_value()) + "::" +
                    stringify(repository_name());

            return stringify(name()) + "-" + stringify(version()) + "::" + stringify(repository_name());

        case idcf_no_version:
            if (_imp->keys && _imp->keys->slot)
                return stringify(name()) + ":" + stringify(_imp->keys->slot->parse_value().parallel_value()) + "::" +
                    stringify(repository_name());

            return stringify(name()) + "::" + stringify(repository_name());

        case idcf_version:
            return stringify(version());

        case idcf_no_name:
            if (_imp->keys && _imp->keys->slot)
                return stringify(version()) + ":" + stringify(_imp->keys->slot->parse_value().parallel_value()) + "::" +
                    stringify(repository_name());

            return stringify(version()) + "::" + stringify(repository_name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
EInstalledRepositoryID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            (slot_key() ? ":" + stringify(slot_key()->parse_value().parallel_value()) : "") + "::" + stringify(repository_name()),
            _imp->environment, { });
}

const QualifiedPackageName
EInstalledRepositoryID::name() const
{
    return _imp->name;
}

const VersionSpec
EInstalledRepositoryID::version() const
{
    return _imp->version;
}

const RepositoryName
EInstalledRepositoryID::repository_name() const
{
    return _imp->repository_name;
}

const std::shared_ptr<const EAPI>
EInstalledRepositoryID::eapi() const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->mutex);

    if (_imp->eapi)
        return _imp->eapi;

    Context context("When finding EAPI for '" + canonical_form(idcf_full) + "':");

    if ((_imp->dir / "EAPI").stat().exists())
        _imp->eapi = EAPIData::get_instance()->eapi_from_string(file_contents(_imp->dir / "EAPI"));
    else
    {
        Log::get_instance()->message("e.no_eapi", ll_debug, lc_context) << "No EAPI entry in '" << _imp->dir << "', pretending '"
            << _imp->environment->distribution() << "'";
        _imp->eapi = EAPIData::get_instance()->eapi_from_string(
                EExtraDistributionData::get_instance()->data_from_distribution(*DistributionData::get_instance()->distribution_from_string(
                        _imp->environment->distribution()))->default_eapi_when_unspecified());
    }

    return _imp->eapi;
}

const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
EInstalledRepositoryID::keywords_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::raw_use_key() const
{
    need_keys_added();
    return _imp->keys->raw_use;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::raw_iuse_key() const
{
    need_keys_added();
    return _imp->keys->raw_iuse;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::raw_iuse_effective_key() const
{
    need_keys_added();
    return _imp->keys->raw_iuse_effective;
}

const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EInstalledRepositoryID::raw_myoptions_key() const
{
    need_keys_added();
    return _imp->keys->raw_myoptions;
}

const std::shared_ptr<const MetadataSpecTreeKey<RequiredUseSpecTree> >
EInstalledRepositoryID::required_use_key() const
{
    need_keys_added();
    return _imp->keys->required_use;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::raw_use_expand_key() const
{
    need_keys_added();
    return _imp->keys->raw_use_expand;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::raw_use_expand_hidden_key() const
{
    need_keys_added();
    return _imp->keys->raw_use_expand_hidden;
}

const std::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >
EInstalledRepositoryID::license_key() const
{
    need_keys_added();
    return _imp->keys->license;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::behaviours_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::inherited_key() const
{
    need_keys_added();
    return _imp->keys->inherited;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::defined_phases_key() const
{
    need_keys_added();
    return _imp->keys->defined_phases;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EInstalledRepositoryID::dependencies_key() const
{
    need_keys_added();
    return _imp->keys->dependencies;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EInstalledRepositoryID::build_dependencies_key() const
{
    need_keys_added();
    return _imp->keys->build_dependencies;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EInstalledRepositoryID::run_dependencies_key() const
{
    need_keys_added();
    return _imp->keys->run_dependencies;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
EInstalledRepositoryID::post_dependencies_key() const
{
    need_keys_added();
    return _imp->keys->post_dependencies;
}

const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EInstalledRepositoryID::restrict_key() const
{
    need_keys_added();
    return _imp->keys->restrictions;
}

const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> >
EInstalledRepositoryID::properties_key() const
{
    need_keys_added();
    return _imp->keys->properties;
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >
EInstalledRepositoryID::choices_key() const
{
    need_keys_added();
    return _imp->keys->choices;
}

const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
EInstalledRepositoryID::fetches_key() const
{
    need_keys_added();
    return _imp->keys->src_uri;
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
EInstalledRepositoryID::homepage_key() const
{
    need_keys_added();
    return _imp->keys->homepage;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
EInstalledRepositoryID::short_description_key() const
{
    need_keys_added();
    return _imp->keys->short_description;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
EInstalledRepositoryID::long_description_key() const
{
    need_keys_added();
    return _imp->keys->long_description;
}

const std::shared_ptr<const MetadataTimeKey>
EInstalledRepositoryID::installed_time_key() const
{
    need_keys_added();
    return _imp->keys->installed_time;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
EInstalledRepositoryID::from_repositories_key() const
{
    need_keys_added();
    return _imp->keys->from_repositories;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
EInstalledRepositoryID::scm_revision_key() const
{
    need_keys_added();
    return _imp->keys->scm_revision;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
EInstalledRepositoryID::fs_location_key() const
{
    // Avoid loading whole metadata
    if (! _imp->fs_location)
    {
        std::unique_lock<std::recursive_mutex> lock(_imp->mutex);

        _imp->fs_location = std::make_shared<LiteralMetadataValueKey<FSPath> >(fs_location_raw_name(),
                    fs_location_human_name(), mkt_internal, _imp->dir);
        add_metadata_key(_imp->fs_location);
    }

    return _imp->fs_location;
}

bool
EInstalledRepositoryID::arbitrary_less_than_comparison(const PackageID &) const
{
    return false;
}

std::size_t
EInstalledRepositoryID::extra_hash_value() const
{
    return 0;
}

namespace
{
    struct SupportsActionQuery
    {
        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<ConfigAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return true;
        }
    };
}

bool
EInstalledRepositoryID::supports_action(const SupportsActionTestBase & b) const
{
    SupportsActionQuery q;
    return b.accept_returning<bool>(q);
}

namespace
{
    struct PerformAction
    {
        const Environment * const env;
        const std::shared_ptr<const erepository::ERepositoryID> id;

        void visit(const UninstallAction & a)
        {
            auto repo(env->fetch_repository(id->repository_name()));
            std::static_pointer_cast<const EInstalledRepository>(repo)->perform_uninstall(id, a);
        }

        void visit(const ConfigAction & a)
        {
            auto repo(env->fetch_repository(id->repository_name()));
            std::static_pointer_cast<const EInstalledRepository>(repo)->perform_config(id, a);
        }

        void visit(const InfoAction & a)
        {
            auto repo(env->fetch_repository(id->repository_name()));
            std::static_pointer_cast<const EInstalledRepository>(repo)->perform_info(id, a);
        }

        void visit(const InstallAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const PretendAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const FetchAction & a) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const PretendFetchAction & a) PALUDIS_ATTRIBUTE((noreturn));
    };

    void PerformAction::visit(const InstallAction & a)
    {
        throw ActionFailedError("Unsupported action: " + a.simple_name());
    }

    void PerformAction::visit(const PretendAction & a)
    {
        throw ActionFailedError("Unsupported action: " + a.simple_name());
    }

    void PerformAction::visit(const FetchAction & a)
    {
        throw ActionFailedError("Unsupported action: " + a.simple_name());
    }

    void PerformAction::visit(const PretendFetchAction & a)
    {
        throw ActionFailedError("Unsupported action: " + a.simple_name());
    }
}

void
EInstalledRepositoryID::perform_action(Action & a) const
{
    PerformAction b{_imp->environment, shared_from_this()};
    a.accept(b);
}

const std::shared_ptr<const MetadataValueKey<Slot> >
EInstalledRepositoryID::slot_key() const
{
    need_keys_added();
    return _imp->keys->slot;
}

const std::shared_ptr<const ChoiceValue>
EInstalledRepositoryID::make_choice_value(const std::shared_ptr<const Choice> & c, const UnprefixedChoiceName & v,
        const Tribool, const bool, const ChoiceOrigin origin, const std::string & override_description, const bool, const bool presumed) const
{
    if (! eapi()->supported())
        throw InternalError(PALUDIS_HERE, "Unsupported EAPI");

    std::string name_with_prefix;
    if (stringify(c->prefix()).empty())
        name_with_prefix = stringify(v);
    else
    {
        char use_expand_separator(eapi()->supported()->choices_options()->use_expand_separator());
        if (! use_expand_separator)
            throw InternalError(PALUDIS_HERE, "No use_expand_separator defined");
        name_with_prefix = stringify(c->prefix()) + std::string(1, use_expand_separator) + stringify(v);
    }

    bool enabled(false);
    if (raw_use_key())
    {
        auto ru(raw_use_key()->parse_value());
        enabled = (ru->end() != ru->find(name_with_prefix));
    }

    return create_e_choice_value(make_named_values<EChoiceValueParams>(
                n::choice_name_with_prefix() = ChoiceNameWithPrefix(name_with_prefix),
                n::choice_prefix_name() = c->prefix(),
                n::description() = override_description,
                n::enabled() = enabled,
                n::enabled_by_default() = enabled,
                n::locked() = true,
                n::presumed() = presumed,
                n::origin() = origin,
                n::unprefixed_choice_name() = v
                ));
}

void
EInstalledRepositoryID::add_build_options(const std::shared_ptr<Choices> & choices) const
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

        /* trace */
        build_options->add(std::make_shared<ELikeTraceChoiceValue>(
                        shared_from_this(), _imp->environment, build_options));
    }
}

void
EInstalledRepositoryID::purge_invalid_cache() const
{
}

void
EInstalledRepositoryID::can_drop_in_memory_cache() const
{
    std::unique_lock<std::recursive_mutex> lock(_imp->mutex);

    clear_metadata_keys();
    _imp->keys.reset();
}

bool
EInstalledRepositoryID::is_installed() const
{
    return true;
}

void
EInstalledRepositoryID::set_scm_revision(const std::string & s) const
{
    throw CannotChangeSCMRevision(stringify(*this), s);
}

