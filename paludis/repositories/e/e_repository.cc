/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
 * Copyright (c) 2006 Danny van Dyk
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

#include "config.h"

#include <paludis/repositories/e/aa_visitor.hh>
#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_mask_file.hh>
#include <paludis/repositories/e/profile.hh>
#include <paludis/repositories/e/profile_file.hh>
#include <paludis/repositories/e/traditional_profile.hh>
#include <paludis/repositories/e/e_repository_news.hh>
#include <paludis/repositories/e/e_repository_sets.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/eclass_mtimes.hh>
#include <paludis/repositories/e/extra_distribution_data.hh>
#include <paludis/repositories/e/use_desc.hh>
#include <paludis/repositories/e/layout.hh>
#include <paludis/repositories/e/info_metadata_key.hh>
#include <paludis/repositories/e/extra_distribution_data.hh>
#include <paludis/repositories/e/memoised_hashes.hh>
#include <paludis/repositories/e/ebuild_id.hh>
#include <paludis/repositories/e/check_fetched_files_visitor.hh>
#include <paludis/repositories/e/fetch_visitor.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/can_skip_phase.hh>
#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repositories/e/pretend_fetch_visitor.hh>
#include <paludis/repositories/e/e_stripper.hh>
#include <paludis/repositories/e/myoptions_requirements_verifier.hh>

#include <paludis/about.hh>
#include <paludis/action.hh>
#include <paludis/choice.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/distribution.hh>
#include <paludis/elike_choices.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/match_package.hh>
#include <paludis/output_manager.hh>
#include <paludis/repository_name_cache.hh>
#include <paludis/syncer.hh>

#include <paludis/util/config_file.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/extract_host_from_url.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/map.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/options.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/random.hh>
#include <paludis/util/rmd160.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sha1.hh>
#include <paludis/util/sha256.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>

#include <tr1/functional>
#include <tr1/unordered_map>
#include <map>
#include <set>
#include <functional>
#include <algorithm>
#include <vector>
#include <list>

#include <strings.h>
#include <ctype.h>

#include <dlfcn.h>
#include <stdint.h>

#define STUPID_CAST(type, val) reinterpret_cast<type>(reinterpret_cast<uintptr_t>(val))

/** \file
 * Implementation of ERepository.
 *
 * \ingroup grperepository
 */

using namespace paludis;
using namespace paludis::erepository;

typedef std::tr1::unordered_map<QualifiedPackageName,
        std::list<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::tr1::shared_ptr<const RepositoryMaskInfo> > >,
        Hash<QualifiedPackageName> > RepositoryMaskMap;
typedef std::tr1::unordered_map<std::string, std::tr1::shared_ptr<MirrorsSequence> > MirrorMap;
typedef std::tr1::unordered_map<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec>, Hash<QualifiedPackageName> > VirtualsMap;

typedef std::map<FSEntry, std::string> EAPIForFileMap;

namespace
{
    std::tr1::shared_ptr<FSEntrySequence> get_master_locations(
            const std::tr1::shared_ptr<const ERepositorySequence> & r)
    {
        std::tr1::shared_ptr<FSEntrySequence> result;

        if (r)
        {
            result.reset(new FSEntrySequence);
            for (ERepositorySequence::ConstIterator e(r->begin()), e_end(r->end()) ;
                    e != e_end ; ++e)
                result->push_back((*e)->location_key()->value());
        }

        return result;
    }

    std::tr1::shared_ptr<Sequence<std::string> > get_master_names(
            const std::tr1::shared_ptr<const ERepositorySequence> & r)
    {
        std::tr1::shared_ptr<Sequence<std::string> > result;

        if (r)
        {
            result.reset(new Sequence<std::string>);
            for (ERepositorySequence::ConstIterator e(r->begin()), e_end(r->end()) ;
                    e != e_end ; ++e)
                result->push_back(stringify((*e)->name()));
        }

        return result;
    }
}

namespace paludis
{
    /**
     * Implementation data for a ERepository.
     *
     * \ingroup grperepository
     */
    template <>
    struct Implementation<ERepository>
    {
        struct Mutexes
        {
            Mutex repo_mask_mutex;
            Mutex arch_flags_mutex;
            Mutex mirrors_mutex;
            Mutex use_desc_mutex;
            Mutex profile_ptr_mutex;
            Mutex news_ptr_mutex;
            Mutex eapi_for_file_mutex;
        };

        ERepository * const repo;
        const ERepositoryParams params;

        const std::tr1::shared_ptr<Mutexes> mutexes;

        std::tr1::shared_ptr<RepositoryNameCache> names_cache;

        mutable RepositoryMaskMap repo_mask;
        mutable bool has_repo_mask;

        const std::map<QualifiedPackageName, QualifiedPackageName> provide_map;

        mutable std::tr1::shared_ptr<Set<UnprefixedChoiceName> > arch_flags;
        mutable std::tr1::shared_ptr<const UseDesc> use_desc;

        mutable bool has_mirrors;
        mutable MirrorMap mirrors;

        mutable std::tr1::shared_ptr<erepository::Profile> profile_ptr;
        mutable std::tr1::shared_ptr<const FSEntry> main_profile_path;

        mutable std::tr1::shared_ptr<ERepositoryNews> news_ptr;

        mutable std::tr1::shared_ptr<ERepositorySets> sets_ptr;
        const std::tr1::shared_ptr<Layout> layout;

        mutable EAPIForFileMap eapi_for_file_map;

        Implementation(ERepository * const, const ERepositoryParams &, std::tr1::shared_ptr<Mutexes> = make_shared_ptr(new Mutexes));
        ~Implementation();

        void need_profiles() const;

        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > layout_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > profile_layout_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key;
        std::tr1::shared_ptr<const MetadataCollectionKey<FSEntrySequence> > profiles_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > cache_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > write_cache_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > append_repository_name_to_write_cache_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > ignore_deprecated_profiles;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > names_cache_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > distdir_key;
        std::tr1::shared_ptr<const MetadataCollectionKey<FSEntrySequence> > eclassdirs_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > securitydir_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > setsdir_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > newsdir_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > sync_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > sync_options_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > builddir_key;
        std::tr1::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > > master_repositories_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > eapi_when_unknown_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > eapi_when_unspecified_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > profile_eapi_when_unspecified_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > use_manifest_key;
        std::tr1::shared_ptr<const MetadataSectionKey> info_pkgs_key;
        std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > info_vars_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > binary_destination_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > binary_src_uri_prefix_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > binary_keywords;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > accounts_repository_data_location_key;
        std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > e_updates_location_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > accept_keywords_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > sync_host_key;
        std::list<std::tr1::shared_ptr<const MetadataKey> > about_keys;

        std::tr1::shared_ptr<EclassMtimes> eclass_mtimes;
        time_t master_mtime;
    };

    Implementation<ERepository>::Implementation(ERepository * const r,
            const ERepositoryParams & p, std::tr1::shared_ptr<Mutexes> m) :
        repo(r),
        params(p),
        mutexes(m),
        names_cache(new RepositoryNameCache(p.names_cache(), r)),
        has_repo_mask(false),
        has_mirrors(false),
        sets_ptr(new ERepositorySets(params.environment(), r, p)),
        layout(LayoutFactory::get_instance()->create(params.layout(), r, params.location(), get_master_locations(
                        params.master_repositories()))),
        format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                    mkt_significant, params.entry_format())),
        layout_key(new LiteralMetadataValueKey<std::string> ("layout", "layout",
                    mkt_normal, params.layout())),
        profile_layout_key(new LiteralMetadataValueKey<std::string> ("profile_layout", "profile_layout",
                    mkt_normal, params.profile_layout())),
        location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                    mkt_significant, params.location())),
        profiles_key(new LiteralMetadataFSEntrySequenceKey(
                    "profiles", "profiles", mkt_normal, params.profiles())),
        cache_key(new LiteralMetadataValueKey<FSEntry> ("cache", "cache",
                    mkt_normal, params.cache())),
        write_cache_key(new LiteralMetadataValueKey<FSEntry> ("write_cache", "write_cache",
                    mkt_normal, params.write_cache())),
        append_repository_name_to_write_cache_key(new LiteralMetadataValueKey<std::string> (
                    "append_repository_name_to_write_cache", "append_repository_name_to_write_cache",
                    mkt_normal, stringify(params.append_repository_name_to_write_cache()))),
        ignore_deprecated_profiles(new LiteralMetadataValueKey<std::string> (
                    "ignore_deprecated_profiles", "ignore_deprecated_profiles",
                    mkt_normal, stringify(params.ignore_deprecated_profiles()))),
        names_cache_key(new LiteralMetadataValueKey<FSEntry> (
                    "names_cache", "names_cache", mkt_normal, params.names_cache())),
        distdir_key(new LiteralMetadataValueKey<FSEntry> (
                    "distdir", "distdir", mkt_normal, params.distdir())),
        eclassdirs_key(new LiteralMetadataFSEntrySequenceKey(
                    "eclassdirs", "eclassdirs", mkt_normal, params.eclassdirs())),
        securitydir_key(new LiteralMetadataValueKey<FSEntry> (
                    "securitydir", "securitydir", mkt_normal, params.securitydir())),
        setsdir_key(new LiteralMetadataValueKey<FSEntry> (
                    "setsdir", "setsdir", mkt_normal, params.setsdir())),
        newsdir_key(new LiteralMetadataValueKey<FSEntry> (
                    "newsdir", "newsdir", mkt_normal, params.newsdir())),
        sync_key(new LiteralMetadataValueKey<std::string> (
                    "sync", "sync", mkt_normal, params.sync())),
        sync_options_key(new LiteralMetadataValueKey<std::string> (
                    "sync_options", "sync_options", mkt_normal, params.sync_options())),
        builddir_key(new LiteralMetadataValueKey<FSEntry> (
                    "builddir", "builddir", mkt_normal, params.builddir())),
        master_repositories_key(params.master_repositories() ?
                std::tr1::shared_ptr<MetadataCollectionKey<Sequence<std::string> > >(new LiteralMetadataStringSequenceKey(
                        "master_repository", "master_repository", mkt_normal, get_master_names(params.master_repositories()))) :
                std::tr1::shared_ptr<MetadataCollectionKey<Sequence<std::string> > >()),
        eapi_when_unknown_key(new LiteralMetadataValueKey<std::string> (
                    "eapi_when_unknown", "eapi_when_unknown", mkt_normal, params.eapi_when_unknown())),
        eapi_when_unspecified_key(new LiteralMetadataValueKey<std::string> (
                    "eapi_when_unspecified", "eapi_when_unspecified", mkt_normal, params.eapi_when_unspecified())),
        profile_eapi_when_unspecified_key(new LiteralMetadataValueKey<std::string> (
                    "profile_eapi_when_unspecified", "profile_eapi_when_unspecified", mkt_normal, params.profile_eapi_when_unspecified())),
        use_manifest_key(new LiteralMetadataValueKey<std::string> (
                    "use_manifest", "use_manifest", mkt_normal, stringify(params.use_manifest()))),
        info_pkgs_key(layout->info_packages_files()->end() != std::find_if(layout->info_packages_files()->begin(),
                    layout->info_packages_files()->end(),
                    std::tr1::bind(std::tr1::mem_fn(&FSEntry::is_regular_file_or_symlink_to_regular_file),
                        std::tr1::placeholders::_1)) ?
                make_shared_ptr(new InfoPkgsMetadataKey(params.environment(), layout->info_packages_files(), repo)) :
                std::tr1::shared_ptr<InfoPkgsMetadataKey>()
                ),
        info_vars_key(layout->info_variables_files()->end() != std::find_if(layout->info_variables_files()->begin(),
                    layout->info_variables_files()->end(),
                    std::tr1::bind(std::tr1::mem_fn(&FSEntry::is_regular_file_or_symlink_to_regular_file),
                        std::tr1::placeholders::_1)) ?
                make_shared_ptr(new InfoVarsMetadataKey(layout->info_variables_files())) :
                std::tr1::shared_ptr<InfoVarsMetadataKey>()
                ),
        binary_destination_key(new LiteralMetadataValueKey<std::string> (
                    "binary_destination", "binary_destination", mkt_normal, stringify(params.binary_destination()))),
        binary_src_uri_prefix_key(new LiteralMetadataValueKey<std::string> (
                    "binary_uri_prefix", "binary_uri_prefix", mkt_normal, params.binary_uri_prefix())),
        binary_keywords(new LiteralMetadataValueKey<std::string> (
                    "binary_keywords", "binary_keywords", mkt_normal, params.binary_keywords())),
        accounts_repository_data_location_key(layout->accounts_repository_data_location_key()),
        e_updates_location_key(layout->e_updates_location_key()),
        sync_host_key(new LiteralMetadataValueKey<std::string> ("sync_host", "sync_host", mkt_internal, extract_host_from_url(params.sync()))),
        eclass_mtimes(new EclassMtimes(r, params.eclassdirs())),

        master_mtime(0)
    {
        if ((params.location() / "metadata" / "about.conf").is_regular_file_or_symlink_to_regular_file())
        {
            Context context("When loading about.conf:");
            KeyValueConfigFile k(params.location() / "metadata" / "about.conf", KeyValueConfigFileOptions(),
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);
            if (! k.get("description").empty())
                about_keys.push_back(make_shared_ptr(new LiteralMetadataValueKey<std::string>("description", "description",
                                mkt_significant, k.get("description"))));
            if (! k.get("summary").empty())
                about_keys.push_back(make_shared_ptr(new LiteralMetadataValueKey<std::string>("summary", "summary",
                                mkt_significant, k.get("summary"))));
            if (! k.get("status").empty())
                about_keys.push_back(make_shared_ptr(new LiteralMetadataValueKey<std::string>("status", "status",
                                mkt_significant, k.get("status"))));
            if (! k.get("maintainer").empty())
                about_keys.push_back(make_shared_ptr(new LiteralMetadataValueKey<std::string>("maintainer", "maintainer",
                                mkt_significant, k.get("maintainer"))));
            if (! k.get("homepage").empty())
                about_keys.push_back(make_shared_ptr(new LiteralMetadataValueKey<std::string>("homepage", "homepage",
                                mkt_significant, k.get("homepage"))));
        }

        FSEntry mtf(params.location() / "metadata" / "timestamp");
        if (mtf.exists())
            master_mtime = mtf.mtim().seconds();
    }

    Implementation<ERepository>::~Implementation()
    {
    }

    void
    Implementation<ERepository>::need_profiles() const
    {
        Lock l(mutexes->profile_ptr_mutex);

        if (profile_ptr)
            return;

        std::tr1::shared_ptr<const FSEntrySequence> profiles(params.profiles());

        if (params.auto_profiles())
        {
            FSEntry profiles_desc("/dev/null");
            for (FSEntrySequence::ConstIterator f(layout->profiles_desc_files()->begin()),
                    f_end(layout->profiles_desc_files()->end()) ;
                    f != f_end ; ++f)
                if (f->is_regular_file_or_symlink_to_regular_file())
                    profiles_desc = *f;

            std::tr1::shared_ptr<FSEntrySequence> auto_profiles(new FSEntrySequence);

            if (profiles_desc == FSEntry("/dev/null"))
            {
                auto_profiles->push_back(FSEntry("/var/empty"));
                main_profile_path.reset(new FSEntry("/var/empty"));
            }
            else
            {
                Context context("When loading profiles.desc file '" + stringify(profiles_desc) + "':");
                LineConfigFile f(profiles_desc, LineConfigFileOptions());
                for (LineConfigFile::ConstIterator line(f.begin()), line_end(f.end()) ;
                        line != line_end ; ++line)
                {
                    std::vector<std::string> tokens;
                    tokenise_whitespace(*line, std::back_inserter(tokens));
                    if (tokens.size() < 3)
                        continue;

                    FSEntry p(params.location() / "profiles" / tokens.at(1));
                    auto_profiles->push_back(p);
                    main_profile_path.reset(new FSEntry(p));
                    break;
                }
            }
            profiles = auto_profiles;
        }
        else if (params.profiles()->empty())
            main_profile_path.reset(new FSEntry("/var/empty"));
        else
            main_profile_path.reset(new FSEntry(*params.profiles()->begin()));

        profile_ptr = ProfileFactory::get_instance()->create(
                params.profile_layout(),
                params.environment(), repo, repo->name(), *profiles,
                EAPIData::get_instance()->eapi_from_string(
                    params.eapi_when_unknown())->supported()->ebuild_environment_variables()->env_arch(),
                params.profiles_explicitly_set());
    }
}

namespace
{
    RepositoryName
    fetch_repo_name(const FSEntry & tree_root)
    {
        bool illegal(false);
        try
        {
            do
            {
                FSEntry name_file(tree_root);
                name_file /= "profiles";
                name_file /= "repo_name";

                if (! name_file.is_regular_file())
                    break;

                LineConfigFile f(name_file, LineConfigFileOptions() + lcfo_disallow_comments + lcfo_disallow_continuations + lcfo_no_skip_blank_lines);
                if (f.begin() == f.end())
                    break;
                return RepositoryName(*f.begin());

            } while (false);
        }
        catch (const RepositoryNameError &)
        {
            illegal = true;
        }
        catch (...)
        {
        }

        std::string modified_location(tree_root.basename());
        std::replace(modified_location.begin(), modified_location.end(), '/', '-');

        if (illegal)
            Log::get_instance()->message("e.repo_name.invalid", ll_qa, lc_no_context)
                << "repo_name file in '" << tree_root << "/profiles/', specifies an illegal repository name, falling back to generated name 'x-"
                << modified_location << "'.";
        else
            Log::get_instance()->message("e.repo_name.unusable", ll_qa, lc_no_context)
                << "Couldn't open repo_name file in '" << tree_root << "/profiles/', falling back to generated name 'x-"
                << modified_location << "' (ignore this message if you have yet to sync this repository).";

        return RepositoryName("x-" + modified_location);
    }
}

ERepository::ERepository(const ERepositoryParams & p) :
    Repository(
            p.environment(),
            fetch_repo_name(p.location()),
            make_named_values<RepositoryCapabilities>(
                value_for<n::destination_interface>(p.binary_destination() ? this : 0),
                value_for<n::environment_variable_interface>(this),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(this),
                value_for<n::provides_interface>(static_cast<RepositoryProvidesInterface *>(0)),
                value_for<n::virtuals_interface>((*DistributionData::get_instance()->distribution_from_string(p.environment()->distribution())).support_old_style_virtuals() ? this : 0)
                )),
    PrivateImplementationPattern<ERepository>(new Implementation<ERepository>(this, p)),
    _imp(PrivateImplementationPattern<ERepository>::_imp)
{
    _add_metadata_keys();
}

ERepository::~ERepository()
{
}

void
ERepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->layout_key);
    add_metadata_key(_imp->profile_layout_key);
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->profiles_key);
    add_metadata_key(_imp->cache_key);
    add_metadata_key(_imp->write_cache_key);
    add_metadata_key(_imp->append_repository_name_to_write_cache_key);
    add_metadata_key(_imp->ignore_deprecated_profiles);
    add_metadata_key(_imp->names_cache_key);
    add_metadata_key(_imp->distdir_key);
    add_metadata_key(_imp->eclassdirs_key);
    add_metadata_key(_imp->securitydir_key);
    add_metadata_key(_imp->setsdir_key);
    add_metadata_key(_imp->newsdir_key);
    add_metadata_key(_imp->sync_key);
    add_metadata_key(_imp->sync_options_key);
    add_metadata_key(_imp->builddir_key);
    add_metadata_key(_imp->eapi_when_unknown_key);
    add_metadata_key(_imp->eapi_when_unspecified_key);
    add_metadata_key(_imp->profile_eapi_when_unspecified_key);
    if (_imp->master_repositories_key)
        add_metadata_key(_imp->master_repositories_key);
    add_metadata_key(_imp->use_manifest_key);
    if (_imp->info_pkgs_key)
        add_metadata_key(_imp->info_pkgs_key);
    if (_imp->info_vars_key)
        add_metadata_key(_imp->info_vars_key);
    add_metadata_key(_imp->binary_destination_key);
    add_metadata_key(_imp->binary_src_uri_prefix_key);
    add_metadata_key(_imp->binary_keywords);
    if (_imp->accounts_repository_data_location_key)
        add_metadata_key(_imp->accounts_repository_data_location_key);
    if (_imp->e_updates_location_key)
        add_metadata_key(_imp->e_updates_location_key);
    if (_imp->accept_keywords_key)
        add_metadata_key(_imp->accept_keywords_key);
    add_metadata_key(_imp->sync_host_key);

    std::for_each(_imp->about_keys.begin(), _imp->about_keys.end(), std::tr1::bind(
                std::tr1::mem_fn(&ERepository::add_metadata_key), this, std::tr1::placeholders::_1));
}

bool
ERepository::has_category_named(const CategoryNamePart & c) const
{
    return _imp->layout->has_category_named(c);
}

bool
ERepository::has_package_named(const QualifiedPackageName & q) const
{
    return _imp->layout->has_package_named(q);
}

std::tr1::shared_ptr<const CategoryNamePartSet>
ERepository::category_names() const
{
    return _imp->layout->category_names();
}

std::tr1::shared_ptr<const QualifiedPackageNameSet>
ERepository::package_names(const CategoryNamePart & c) const
{
    return _imp->layout->package_names(c);
}

std::tr1::shared_ptr<const PackageIDSequence>
ERepository::package_ids(const QualifiedPackageName & n) const
{
    return _imp->layout->package_ids(n);
}

std::tr1::shared_ptr<const RepositoryMaskInfo>
ERepository::repository_masked(const PackageID & id) const
{
    Lock l(_imp->mutexes->repo_mask_mutex);

    if (! _imp->has_repo_mask)
    {
        Context context("When querying repository mask for '" + stringify(id) + "':");

        using namespace std::tr1::placeholders;

        std::tr1::shared_ptr<const FSEntrySequence> repository_mask_files(_imp->layout->repository_mask_files());
        ProfileFile<MaskFile> repository_mask_file(this);
        std::for_each(repository_mask_files->begin(), repository_mask_files->end(),
                      std::tr1::bind(&ProfileFile<MaskFile>::add_file, std::tr1::ref(repository_mask_file), _1));

        for (ProfileFile<MaskFile>::ConstIterator
                line(repository_mask_file.begin()), line_end(repository_mask_file.end()) ;
                line != line_end ; ++line)
        {
            try
            {
                std::tr1::shared_ptr<const PackageDepSpec> a(new PackageDepSpec(parse_elike_package_dep_spec(
                                line->second.first, line->first->supported()->package_dep_spec_parse_options(),
                                line->first->supported()->version_spec_options(),
                                std::tr1::shared_ptr<const PackageID>())));
                if (a->package_ptr())
                    _imp->repo_mask[*a->package_ptr()].push_back(std::make_pair(a, line->second.second));
                else
                    Log::get_instance()->message("e.package_mask.bad_spec", ll_warning, lc_context)
                        << "Loading package mask spec '" << line->second.first << "' failed because specification does not restrict to a "
                        "unique package";
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                Log::get_instance()->message("e.package_mask.bad_spec", ll_warning, lc_context) << "Loading package mask spec '"
                    << line->second.first << "' failed due to exception '" << e.message() << "' ("
                    << e.what() << ")";
            }
        }

        _imp->has_repo_mask = true;
    }

    RepositoryMaskMap::iterator r(_imp->repo_mask.find(id.name()));
    if (_imp->repo_mask.end() == r)
        return std::tr1::shared_ptr<const RepositoryMaskInfo>();
    else
        for (std::list<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::tr1::shared_ptr<const RepositoryMaskInfo> > >::const_iterator
                k(r->second.begin()), k_end(r->second.end()) ; k != k_end ; ++k)
            if (match_package(*_imp->params.environment(), *k->first, id, MatchPackageOptions()))
                return k->second;

    return std::tr1::shared_ptr<const RepositoryMaskInfo>();
}

const std::tr1::shared_ptr<const Set<UnprefixedChoiceName> >
ERepository::arch_flags() const
{
    Lock l(_imp->mutexes->arch_flags_mutex);
    if (! _imp->arch_flags)
    {
        Context context("When loading arch list:");
        _imp->arch_flags.reset(new Set<UnprefixedChoiceName>);

        bool found_one(false);
        std::tr1::shared_ptr<const FSEntrySequence> arch_list_files(_imp->layout->arch_list_files());
        for (FSEntrySequence::ConstIterator p(arch_list_files->begin()), p_end(arch_list_files->end()) ;
                p != p_end ; ++p)
        {
            if (! p->exists())
                continue;

            LineConfigFile archs(*p, LineConfigFileOptions() + lcfo_disallow_continuations);
            std::copy(archs.begin(), archs.end(), create_inserter<UnprefixedChoiceName>(_imp->arch_flags->inserter()));
            found_one = true;
        }

        if (! found_one)
        {
            Log::get_instance()->message("e.arch_list.missing", ll_qa, lc_no_context)
                << "Couldn't find arch.list file for repository '"
                << stringify(name()) << "', arch flags may incorrectly show up as unmasked";
        }
    }

    return _imp->arch_flags;
}

void
ERepository::need_mirrors() const
{
    Lock l(_imp->mutexes->mirrors_mutex);

    if (! _imp->has_mirrors)
    {
        bool found_one(false);
        std::tr1::shared_ptr<const FSEntrySequence> mirror_files(_imp->layout->mirror_files());
        for (FSEntrySequence::ConstIterator p(mirror_files->begin()), p_end(mirror_files->end()) ;
                p != p_end ; ++p)
        {
            if (p->exists())
            {
                LineConfigFile mirrors(*p, LineConfigFileOptions() + lcfo_disallow_continuations);
                for (LineConfigFile::ConstIterator line(mirrors.begin()) ; line != mirrors.end() ; ++line)
                {
                    std::vector<std::string> ee;
                    tokenise_whitespace(*line, std::back_inserter(ee));
                    if (! ee.empty())
                    {
                        /* pick up to five random mirrors only */
                        static Random r;
                        std::random_shuffle(next(ee.begin()), ee.end(), r);
                        if (ee.size() > 6)
                            ee.resize(6);

                        std::tr1::shared_ptr<MirrorsSequence> ms(new MirrorsSequence);
                        for (std::vector<std::string>::const_iterator e(next(ee.begin())),
                                e_end(ee.end()) ; e != e_end ; ++e)
                            ms->push_back(*e);

                        _imp->mirrors.insert(std::make_pair(ee.at(0), ms));
                    }
                }
            }

            found_one = true;
        }

        if (! found_one)
            Log::get_instance()->message("e.thirdpartymirrors.missing", ll_warning, lc_no_context) <<
                "No thirdpartymirrors file found in '"
                << (_imp->params.location() / "profiles") << "', so mirror:// SRC_URI "
                "components cannot be fetched";

        _imp->has_mirrors = true;
    }
}

bool
ERepository::sync(const std::tr1::shared_ptr<OutputManager> & output_manager) const
{
    Context context("When syncing repository '" + stringify(name()) + "':");

    if (_imp->params.sync().empty())
        return false;

    std::list<std::string> sync_list;
    tokenise_whitespace(_imp->params.sync(), std::back_inserter(sync_list));

    bool ok(false);
    for (std::list<std::string>::const_iterator s(sync_list.begin()),
            s_end(sync_list.end()) ; s != s_end ; ++s)
    {
        DefaultSyncer syncer(make_named_values<SyncerParams>(
                    value_for<n::environment>(_imp->params.environment()),
                    value_for<n::local>(stringify(_imp->params.location())),
                    value_for<n::remote>(*s)
                ));
        SyncOptions opts(make_named_values<SyncOptions>(
                    value_for<n::filter_file>(_imp->layout->sync_filter_file()),
                    value_for<n::options>(_imp->params.sync_options()),
                    value_for<n::output_manager>(output_manager)
                ));
        try
        {
            syncer.sync(opts);
        }
        catch (const SyncFailedError &)
        {
            continue;
        }

        ok = true;
        break;
    }

    if (! ok)
        throw SyncFailedError(stringify(_imp->params.location()), _imp->params.sync());

    return true;
}

void
ERepository::invalidate()
{
    _imp.reset(new Implementation<ERepository>(this, _imp->params, _imp->mutexes));
    _add_metadata_keys();
}

void
ERepository::purge_invalid_cache() const
{
    Context context("When purging invalid write_cache:");

    FSEntry write_cache(_imp->params.write_cache());
    if (write_cache == FSEntry("/var/empty"))
        return;

    if (_imp->params.append_repository_name_to_write_cache())
        write_cache /= stringify(name());

    if (! write_cache.is_directory_or_symlink_to_directory())
        return;

    const std::tr1::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(
                _imp->params.eapi_when_unknown()));

    std::tr1::shared_ptr<EclassMtimes> eclass_mtimes(new EclassMtimes(this, _imp->params.eclassdirs()));
    time_t master_mtime(0);
    FSEntry master_mtime_file(_imp->params.location() / "metadata" / "timestamp");
    if (master_mtime_file.exists())
        master_mtime = master_mtime_file.mtim().seconds();

    for (DirIterator dc(write_cache, DirIteratorOptions() + dio_inode_sort), dc_end ; dc != dc_end ; ++dc)
    {
        if (! dc->is_directory_or_symlink_to_directory())
            continue;

        for (DirIterator dp(*dc, DirIteratorOptions() + dio_inode_sort), dp_end ; dp != dp_end ; ++dp)
        {
            if (! dp->is_regular_file_or_symlink_to_regular_file())
                continue;

            try
            {
                CategoryNamePart cnp(dc->basename());
                std::string pv(dp->basename());
                VersionSpec v(elike_get_remove_trailing_version(pv, eapi->supported()->version_spec_options()));
                PackageNamePart p(pv);

                std::tr1::shared_ptr<const PackageIDSequence> ids(_imp->layout->package_ids(cnp + p));
                bool found(false);
                for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                        i != i_end ; ++i)
                {
                    /* 00 is *not* equal to 0 here */
                    if (stringify((*i)->version()) != stringify(v))
                        continue;

                    std::tr1::static_pointer_cast<const ERepositoryID>(*i)->purge_invalid_cache();

                    found = true;
                    break;
                }

                if (! found)
                    FSEntry(*dp).unlink();
            }
            catch (const Exception & e)
            {
                Log::get_instance()->message("e.ebuild.purge_write_cache.ignoring", ll_warning, lc_context)
                    << "Ignoring exception '" << e.message() << "' (" << e.what() << ") when purging invalid write_cache entries";
            }
        }
    }
}

void
ERepository::invalidate_masks()
{
    _imp->layout->invalidate_masks();

    if ((*DistributionData::get_instance()->distribution_from_string(_imp->params.environment()->distribution()))
            .support_old_style_virtuals())
        if (_imp->params.environment()->package_database()->has_repository_named(RepositoryName("virtuals")))
            _imp->params.environment()->package_database()->fetch_repository(
                    RepositoryName("virtuals"))->invalidate_masks();
}

void
ERepository::update_news() const
{
    Lock l(_imp->mutexes->news_ptr_mutex);

    if (! _imp->news_ptr)
        _imp->news_ptr.reset(new ERepositoryNews(_imp->params.environment(), this, _imp->params));

    _imp->news_ptr->update_news();
}

const std::tr1::shared_ptr<const Layout>
ERepository::layout() const
{
    return _imp->layout;
}

const std::tr1::shared_ptr<const Profile>
ERepository::profile() const
{
    _imp->need_profiles();
    return _imp->profile_ptr;
}

std::string
ERepository::profile_variable(const std::string & s) const
{
    _imp->need_profiles();

    return _imp->profile_ptr->environment_variable(s);
}

std::tr1::shared_ptr<const ERepository::VirtualsSequence>
ERepository::virtual_packages() const
{
    Context context("When loading virtual packages for repository '" +
            stringify(name()) + "'");

    _imp->need_profiles();

    std::tr1::shared_ptr<VirtualsSequence> result(new VirtualsSequence);

    for (Map<QualifiedPackageName, PackageDepSpec>::ConstIterator i(_imp->profile_ptr->virtuals()->begin()),
            i_end(_imp->profile_ptr->virtuals()->end()) ; i != i_end ; ++i)
        result->push_back(make_named_values<RepositoryVirtualsEntry>(
                    value_for<n::provided_by_spec>(make_shared_copy(i->second)),
                    value_for<n::virtual_name>(i->first)
                ));

    return result;
}

void
ERepository::regenerate_cache() const
{
    _imp->names_cache->regenerate_cache();
}

std::tr1::shared_ptr<const CategoryNamePartSet>
ERepository::category_names_containing_package(const PackageNamePart & p) const
{
    if (! _imp->names_cache->usable())
        return Repository::category_names_containing_package(p);

    std::tr1::shared_ptr<const CategoryNamePartSet> result(
            _imp->names_cache->category_names_containing_package(p));

    return result ? result : Repository::category_names_containing_package(p);
}

const ERepositoryParams &
ERepository::params() const
{
    return _imp->params;
}

bool
ERepository::is_suitable_destination_for(const PackageID & e) const
{
    std::string f(e.repository()->format_key() ? e.repository()->format_key()->value() : "");
    if (f == "ebuild")
        return static_cast<const ERepositoryID &>(e).eapi()->supported()->can_be_pbin();
    else
        return false;
}

bool
ERepository::is_default_destination() const
{
    return false;
}

bool
ERepository::want_pre_post_phases() const
{
    return false;
}

HookResult
ERepository::perform_hook(const Hook & hook)
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    if (hook.name() == "sync_all_post"
            || hook.name() == "install_all_post"
            || hook.name() == "uninstall_all_post")
        update_news();

    return make_named_values<HookResult>(value_for<n::max_exit_status>(0), value_for<n::output>(""));
}

std::tr1::shared_ptr<const CategoryNamePartSet>
ERepository::unimportant_category_names() const
{
    std::tr1::shared_ptr<CategoryNamePartSet> result(make_shared_ptr(new CategoryNamePartSet));
    result->insert(CategoryNamePart("virtual"));
    return result;
}

namespace
{
    struct SupportsActionQuery
    {
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

        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return true;
        }
    };
}

bool
ERepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    return a.accept_returning<bool>(q);
}

void
ERepository::make_manifest(const QualifiedPackageName & qpn)
{
    FSEntry package_dir = _imp->layout->package_directory(qpn);

    FSEntry(package_dir / "Manifest").unlink();
    SafeOFStream manifest(FSEntry(package_dir / "Manifest"));
    if (! manifest)
        throw ERepositoryConfigurationError("Couldn't open Manifest for writing.");

    std::tr1::shared_ptr<Map<FSEntry, std::string> > files = _imp->layout->manifest_files(qpn);

    for (Map<FSEntry, std::string>::ConstIterator f(files->begin()) ;
            f != files->end() ; ++f)
    {
        FSEntry file(f->first);
        std::string filename = file.basename();
        std::string file_type(f->second);

        if ("AUX" == file_type)
        {
            filename = stringify(file).substr(stringify(package_dir / "files").length()+1);
        }

        SafeIFStream file_stream(file);

        RMD160 rmd160sum(file_stream);
        manifest << file_type << " " << filename << " "
            << file.file_size() << " RMD160 " << rmd160sum.hexsum();

        file_stream.clear();
        file_stream.seekg(0, std::ios::beg);
        SHA1 sha1sum(file_stream);
        manifest << " SHA1 " << sha1sum.hexsum();

        file_stream.clear();
        file_stream.seekg(0, std::ios::beg);
        SHA256 sha256sum(file_stream);
        manifest << " SHA256 " << sha256sum.hexsum() << std::endl;
    }

    std::tr1::shared_ptr<const PackageIDSequence> versions;
    versions = package_ids(qpn);

    std::set<std::string> done_files;

    for (PackageIDSequence::ConstIterator v(versions->begin()),
            v_end(versions->end()) ;
            v != v_end ; ++v)
    {
        std::tr1::shared_ptr<const PackageID> id = (*v);
        if (! id->fetches_key())
            continue;
        AAVisitor aa;
        id->fetches_key()->value()->root()->accept(aa);

        for (AAVisitor::ConstIterator d(aa.begin()) ;
                d != aa.end() ; ++d)
        {
            if (done_files.count(*d))
                continue;
            done_files.insert(*d);

            FSEntry f(params().distdir() / *d);

            SafeIFStream file_stream(f);

            MemoisedHashes * hashes = MemoisedHashes::get_instance();

            manifest << "DIST " << f.basename() << " "
                << f.file_size()
                << " RMD160 " << hashes->get<RMD160>(f, file_stream)
                << " SHA1 " << hashes->get<SHA1>(f, file_stream)
                << " SHA256 " << hashes->get<SHA256>(f, file_stream)
                << std::endl;
        }
    }
}

void
ERepository::need_keys_added() const
{
    Lock l(_imp->mutexes->profile_ptr_mutex);

    if (! _imp->accept_keywords_key)
    {
        _imp->need_profiles();

        std::string k, v;

        v = EAPIData::get_instance()->eapi_from_string(eapi_for_file(*_imp->main_profile_path)
                )->supported()->ebuild_environment_variables()->env_accept_keywords();
        if (! v.empty())
            k = _imp->profile_ptr->environment_variable(v);

        if (k.empty())
        {
            v = EAPIData::get_instance()->eapi_from_string(eapi_for_file(*_imp->main_profile_path)
                    )->supported()->ebuild_environment_variables()->env_arch();
            if (! v.empty())
                k = _imp->profile_ptr->environment_variable(v);
        }

        _imp->accept_keywords_key.reset(new LiteralMetadataValueKey<std::string>(v,
                    "Default accepted keywords", mkt_internal, k));
        add_metadata_key(_imp->accept_keywords_key);
    }

    return;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
ERepository::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
ERepository::location_key() const
{
    return _imp->location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
ERepository::installed_root_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
ERepository::info_vars_key() const
{
    return _imp->info_vars_key;
}

RepositoryName
ERepository::repository_factory_name(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> & key_function)
{
    Context context("When finding repository name for e repository from repo_file '" + key_function("repo_file") + "':");

    if (key_function("location").empty())
        throw ERepositoryConfigurationError("Key 'location' unspecified or empty");
    return fetch_repo_name(FSEntry(key_function("location")));
}

std::tr1::shared_ptr<Repository>
ERepository::repository_factory_create(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    Context context("When making ebuild repository from repo_file '" + f("repo_file") + "':");

    std::string location(f("location"));
    if (location.empty())
        throw ERepositoryConfigurationError("Key 'location' not specified or empty");
    if ('/' != location.at(0))
        throw ERepositoryConfigurationError("Key 'location' must start with a / (relative paths are not allowed)");

    std::tr1::shared_ptr<KeyValueConfigFile> layout_conf((FSEntry(location) / "metadata/layout.conf").exists() ?
            new KeyValueConfigFile(FSEntry(location) / "metadata/layout.conf", KeyValueConfigFileOptions(),
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation)
            : 0);

    std::tr1::shared_ptr<ERepositorySequence> master_repositories;
    if (! f("master_repository").empty())
    {
        if (layout_conf)
        {
            Log::get_instance()->message("e.ebuild.configuration.master_repository", ll_warning, lc_context) << "Key 'master_repository' in '"
                << f("repo_file") << "' will override '" << (FSEntry(location) / "metadata/layout.conf")
                << "'.";
        }

        Context context_local("When finding configuration information for master_repository '"
                + stringify(f("master_repository")) + "':");

        RepositoryName master_repository_name(f("master_repository"));
        std::tr1::shared_ptr<Repository> master_repository_uncasted(
                env->package_database()->fetch_repository(master_repository_name));

        std::string format("unknown");
        if (master_repository_uncasted->format_key())
            format = master_repository_uncasted->format_key()->value();

        if (format != "ebuild")
            throw ERepositoryConfigurationError("Master repository format is '" +
                    stringify(format) + "', not 'ebuild'");

        std::tr1::shared_ptr<ERepository> master_repository(std::tr1::static_pointer_cast<ERepository>(master_repository_uncasted));
        master_repositories.reset(new ERepositorySequence);
        master_repositories->push_back(master_repository);
    }
    else if (layout_conf)
    {
        std::list<std::string> tokens;
        tokenise_whitespace(layout_conf->get("masters"), std::back_inserter(tokens));
        for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                t != t_end ; ++t)
        {
            Context context_local("When finding configuration information for master '" + *t + "':");

            RepositoryName master_repository_name(*t);
            try
            {
                std::tr1::shared_ptr<Repository> master_repository_uncasted(
                        env->package_database()->fetch_repository(master_repository_name));

                std::string format("unknown");
                if (master_repository_uncasted->format_key())
                    format = master_repository_uncasted->format_key()->value();

                if (format != "ebuild")
                    throw ERepositoryConfigurationError("Master repository format is '" +
                            stringify(format) + "', not 'ebuild'");

                std::tr1::shared_ptr<ERepository> master_repository(std::tr1::static_pointer_cast<ERepository>(master_repository_uncasted));
                if (! master_repositories)
                    master_repositories.reset(new ERepositorySequence);
                master_repositories->push_back(master_repository);
            }
            catch (const NoSuchRepositoryError &)
            {
                throw ERepositoryConfigurationError("According to '" + stringify(FSEntry(location) / "metadata/layout.conf")
                        + "', the repository specified by '" + f("repo_file") + "' requires repository '" + *t +
                        "', which you do not have available");
            }
        }
    }

    std::tr1::shared_ptr<FSEntrySequence> profiles(new FSEntrySequence);
    bool profiles_explicitly_set(false), auto_profiles(false);
    tokenise_whitespace(f("profiles"), create_inserter<FSEntry>(std::back_inserter(*profiles)));
    if (profiles->empty())
    {
        if (master_repositories)
            std::copy((*master_repositories->begin())->params().profiles()->begin(),
                    (*master_repositories->begin())->params().profiles()->end(), profiles->back_inserter());
        else if (FSEntry(location).is_directory_or_symlink_to_directory() &&
                (DirIterator(FSEntry(location)) != DirIterator()))
        {
            /* only require profiles = if we've definitely been synced. requiring profiles = on
             * unsynced doesn't play nice with layout.conf specifying masters. */
            throw ERepositoryConfigurationError("No profiles have been specified");
        }
    }
    else if (f("profiles") == "(auto)")
    {
        profiles.reset(new FSEntrySequence);
        if (master_repositories)
            std::copy((*master_repositories->begin())->params().profiles()->begin(),
                    (*master_repositories->begin())->params().profiles()->end(), profiles->back_inserter());
        else
            auto_profiles = true;
    }
    else
        profiles_explicitly_set = true;

    std::tr1::shared_ptr<FSEntrySequence> eclassdirs(new FSEntrySequence);
    tokenise_whitespace(f("eclassdirs"), create_inserter<FSEntry>(std::back_inserter(*eclassdirs)));
    if (eclassdirs->empty())
    {
        if (master_repositories)
        {
            for (ERepositorySequence::ConstIterator e(master_repositories->begin()),
                    e_end(master_repositories->end()) ; e != e_end ; ++e)
                std::copy((*e)->params().eclassdirs()->begin(), (*e)->params().eclassdirs()->end(), eclassdirs->back_inserter());
        }
        eclassdirs->push_back(location + "/eclass");
    }

    std::string distdir(f("distdir"));
    if (distdir.empty())
    {
        if (master_repositories)
            distdir = stringify((*master_repositories->begin())->params().distdir());
        else
        {
            distdir = EExtraDistributionData::get_instance()->data_from_distribution(
                    *DistributionData::get_instance()->distribution_from_string(
                        env->distribution()))->default_distdir();
            if (distdir.empty())
                distdir = location + "/distfiles";
            else if ('/' != distdir.at(0))
                distdir = location + "/" + distdir;
        }
    }

    std::string setsdir(f("setsdir"));
    if (setsdir.empty())
        setsdir = location + "/sets";

    std::string securitydir(f("securitydir"));
    if (securitydir.empty())
        securitydir = location + "/metadata/glsa";

    std::string newsdir(f("newsdir"));
    if (newsdir.empty())
        newsdir = location + "/metadata/news";

    std::string cache(f("cache"));
    if (cache.empty())
    {
        cache = location + "/metadata/cache";
        if (! FSEntry(cache).exists())
            cache = "/var/empty";
    }

    std::string write_cache(f("write_cache"));
    if (write_cache.empty())
        write_cache = EExtraDistributionData::get_instance()->data_from_distribution(*DistributionData::get_instance()->distribution_from_string(
                env->distribution()))->default_write_cache();

    bool append_repository_name_to_write_cache(true);
    if (! f("append_repository_name_to_write_cache").empty())
    {
        Context item_context("When handling append_repository_name_to_write_cache key:");
        append_repository_name_to_write_cache = destringify<bool>(f("append_repository_name_to_write_cache"));
    }

    bool ignore_deprecated_profiles(false);
    if (! f("ignore_deprecated_profiles").empty())
    {
        Context item_context("When handling ignore_deprecated_profiles key:");
        ignore_deprecated_profiles = destringify<bool>(f("ignore_deprecated_profiles"));
    }

    std::string eapi_when_unknown(f("eapi_when_unknown"));
    if (eapi_when_unknown.empty())
    {
        if (! layout_conf
                || (eapi_when_unknown = layout_conf->get("eapi_when_unknown")).empty())
            eapi_when_unknown = EExtraDistributionData::get_instance()->data_from_distribution(
                    *DistributionData::get_instance()->distribution_from_string(
                        env->distribution()))->default_eapi_when_unknown();
    }

    std::string eapi_when_unspecified(f("eapi_when_unspecified"));
    if (eapi_when_unspecified.empty())
    {
        if (! layout_conf
                || (eapi_when_unspecified = layout_conf->get("eapi_when_unspecified")).empty())
            eapi_when_unspecified = EExtraDistributionData::get_instance()->data_from_distribution(
                    *DistributionData::get_instance()->distribution_from_string(
                        env->distribution()))->default_eapi_when_unspecified();
    }

    std::string profile_eapi(f("profile_eapi_when_unspecified"));
    if (profile_eapi.empty())
    {
        profile_eapi = f("profile_eapi");
        if (! profile_eapi.empty())
            Log::get_instance()->message("e.ebuild.configuration.profile_eapi", ll_warning, lc_context) <<
                "Key 'profile_eapi' in '" + f("repo_file") + "' is deprecated, use profile_eapi_when_unspecified";
    }

    if (profile_eapi.empty())
    {
        if (! layout_conf
                || (profile_eapi = layout_conf->get("profile_eapi_when_unspecified")).empty())
            profile_eapi = EExtraDistributionData::get_instance()->data_from_distribution(
                    *DistributionData::get_instance()->distribution_from_string(
                        env->distribution()))->default_profile_eapi();
    }

    std::string names_cache(f("names_cache"));
    if (names_cache.empty())
    {
        names_cache = EExtraDistributionData::get_instance()->data_from_distribution(
                *DistributionData::get_instance()->distribution_from_string(
                    env->distribution()))->default_names_cache();
        if (names_cache.empty())
        {
            Log::get_instance()->message("e.ebuild.configuration.no_names_cache", ll_warning, lc_no_context)
                << "The names_cache key is not set in '" << f("repo_file")
                << "'. You should read the Paludis documentation and select an appropriate value.";
            names_cache = "/var/empty";
        }
    }

    std::string sync(f("sync"));

    std::string sync_options(f("sync_options"));

    std::string builddir(f("builddir"));
    if (builddir.empty())
    {
        if (master_repositories)
            builddir = stringify((*master_repositories->begin())->params().builddir());
        else
            builddir = EExtraDistributionData::get_instance()->data_from_distribution(
                    *DistributionData::get_instance()->distribution_from_string(
                        env->distribution()))->default_buildroot();
    }

    std::string layout(f("layout"));
    if (layout.empty())
    {
        if (! layout_conf
                || (layout = layout_conf->get("layout")).empty())
            layout = EExtraDistributionData::get_instance()->data_from_distribution(
                    *DistributionData::get_instance()->distribution_from_string(
                        env->distribution()))->default_layout();
    }

    std::string profile_layout(f("profile_layout"));
    if (profile_layout.empty())
    {
        if (! layout_conf
                || (profile_layout = layout_conf->get("profile_layout")).empty())
            profile_layout = EExtraDistributionData::get_instance()->data_from_distribution(
                    *DistributionData::get_instance()->distribution_from_string(
                        env->distribution()))->default_profile_layout();
    }

    UseManifest use_manifest(manifest_use);
    if (! f("use_manifest").empty())
    {
        Context item_context("When handling use_manifest key:");
        use_manifest = destringify<UseManifest>(f("use_manifest"));
    }

    bool binary_destination(false);
    if (! f("binary_destination").empty())
    {
        Context item_context("When handling binary_destination key:");
        binary_destination = destringify<bool>(f("binary_destination"));
    }

    std::string binary_uri_prefix(f("binary_uri_prefix"));

    std::string binary_distdir(f("binary_distdir"));

    std::string binary_keywords(f("binary_keywords"));

    if (binary_keywords.empty())
    {
        if (binary_destination)
            throw ERepositoryConfigurationError("binary_destination = true, but binary_keywords is unset or empty");
    }

    return std::tr1::shared_ptr<ERepository>(new ERepository(make_named_values<ERepositoryParams>(
                    value_for<n::append_repository_name_to_write_cache>(append_repository_name_to_write_cache),
                    value_for<n::auto_profiles>(auto_profiles),
                    value_for<n::binary_destination>(binary_destination),
                    value_for<n::binary_distdir>(binary_distdir),
                    value_for<n::binary_keywords>(binary_keywords),
                    value_for<n::binary_uri_prefix>(binary_uri_prefix),
                    value_for<n::builddir>(FSEntry(builddir).realpath_if_exists()),
                    value_for<n::cache>(cache),
                    value_for<n::distdir>(FSEntry(distdir).realpath_if_exists()),
                    value_for<n::eapi_when_unknown>(eapi_when_unknown),
                    value_for<n::eapi_when_unspecified>(eapi_when_unspecified),
                    value_for<n::eclassdirs>(eclassdirs),
                    value_for<n::entry_format>("ebuild"),
                    value_for<n::environment>(env),
                    value_for<n::ignore_deprecated_profiles>(ignore_deprecated_profiles),
                    value_for<n::layout>(layout),
                    value_for<n::location>(FSEntry(location).realpath_if_exists()),
                    value_for<n::master_repositories>(master_repositories),
                    value_for<n::names_cache>(FSEntry(names_cache).realpath_if_exists()),
                    value_for<n::newsdir>(FSEntry(newsdir).realpath_if_exists()),
                    value_for<n::profile_eapi_when_unspecified>(profile_eapi),
                    value_for<n::profile_layout>(profile_layout),
                    value_for<n::profiles>(profiles),
                    value_for<n::profiles_explicitly_set>(profiles_explicitly_set),
                    value_for<n::securitydir>(FSEntry(securitydir).realpath_if_exists()),
                    value_for<n::setsdir>(FSEntry(setsdir).realpath_if_exists()),
                    value_for<n::sync>(sync),
                    value_for<n::sync_options>(sync_options),
                    value_for<n::use_manifest>(use_manifest),
                    value_for<n::write_bin_uri_prefix>(""),
                    value_for<n::write_cache>(FSEntry(write_cache).realpath_if_exists())
                        )));
}

std::tr1::shared_ptr<const RepositoryNameSet>
ERepository::repository_factory_dependencies(
        const Environment * const,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    std::tr1::shared_ptr<RepositoryNameSet> result(new RepositoryNameSet);
    if (! f("master_repository").empty())
        result->insert(RepositoryName(f("master_repository")));
    else
    {
        std::string location(f("location"));
        if (location.empty())
            throw ERepositoryConfigurationError("Key 'location' not specified or empty");

        std::tr1::shared_ptr<KeyValueConfigFile> layout_conf((FSEntry(location) / "metadata/layout.conf").exists() ?
                new KeyValueConfigFile(FSEntry(location) / "metadata/layout.conf", KeyValueConfigFileOptions(),
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation)
                : 0);

        if (layout_conf)
        {
            std::list<std::string> tokens;
            tokenise_whitespace(layout_conf->get("masters"), std::back_inserter(tokens));
            for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                    t != t_end ; ++t)
                result->insert(RepositoryName(*t));
        }
    }

    return result;
}

const std::tr1::shared_ptr<const UseDesc>
ERepository::use_desc() const
{
    Lock l(_imp->mutexes->use_desc_mutex);
    if (! _imp->use_desc)
    {
        _imp->use_desc.reset(new UseDesc(_imp->layout->use_desc_files()));
    }

    return _imp->use_desc;
}

const std::string
ERepository::eapi_for_file(const FSEntry & f) const
{
    FSEntry dir(f.dirname());
    Lock lock(_imp->mutexes->eapi_for_file_mutex);
    EAPIForFileMap::const_iterator i(_imp->eapi_for_file_map.find(dir));
    if (i == _imp->eapi_for_file_map.end())
    {
        Context context("When finding the EAPI to use for file '" + stringify(f) + "':");
        if ((dir / "eapi").is_regular_file_or_symlink_to_regular_file())
        {
            LineConfigFile file(dir / "eapi", LineConfigFileOptions() + lcfo_disallow_continuations);
            if (file.begin() == file.end())
            {
                Log::get_instance()->message("e.ebuild.profile_eapi_file.empty", ll_warning, lc_no_context)
                    << "File '" << (dir / "eapi") << "' has no content";
                i = _imp->eapi_for_file_map.insert(std::make_pair(
                            dir, _imp->params.profile_eapi_when_unspecified())).first;
            }
            else
                i = _imp->eapi_for_file_map.insert(std::make_pair(dir, *file.begin())).first;
        }
        else
            i = _imp->eapi_for_file_map.insert(std::make_pair(
                        dir, _imp->params.profile_eapi_when_unspecified())).first;
    }
    return i->second;
}

namespace
{
    std::tr1::shared_ptr<const SetSpecTree> get_system_set(const std::tr1::shared_ptr<const SetSpecTree> s)
    {
        return s;
    }

    std::tr1::shared_ptr<const SetSpecTree> get_set(
            const std::tr1::shared_ptr<const ERepositorySets> & s,
            const SetName & n)
    {
        return s->package_set(n);
    }
}

void
ERepository::populate_sets() const
{
    const std::tr1::shared_ptr<const SetNameSet> sets(_imp->sets_ptr->sets_list());
    for (SetNameSet::ConstIterator s(sets->begin()), s_end(sets->end()) ;
            s != s_end ; ++s)
    {
        if (stringify(*s) == "system")
        {
            _imp->need_profiles();
            _imp->params.environment()->add_set(
                    *s,
                    SetName(stringify(*s) + "::" + stringify(name())),
                    std::tr1::bind(&get_system_set, _imp->profile_ptr->system_packages()),
                    true);
        }
        else
        {
            _imp->params.environment()->add_set(
                    *s,
                    SetName(stringify(*s) + "::" + stringify(name())),
                    std::tr1::bind(&get_set, _imp->sets_ptr, *s),
                    true);

            if (stringify(*s) != "security" && stringify(*s) != "insecurity")
                _imp->params.environment()->add_set(
                        SetName(stringify(*s) + "*"),
                        SetName(stringify(*s) + "::" + stringify(name()) + "*"),
                        std::tr1::bind(&get_set, _imp->sets_ptr, SetName(stringify(*s) + "*")),
                        true);
        }
    }
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
ERepository::accept_keywords_key() const
{
    need_keys_added();
    return _imp->accept_keywords_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
ERepository::sync_host_key() const
{
    return _imp->sync_host_key;
}

namespace
{
    struct Suffixes :
        InstantiationPolicy<Suffixes, instantiation_method::SingletonTag>
    {
        KeyValueConfigFile file;

        Suffixes() :
            file(FSEntry(getenv_with_default("PALUDIS_SUFFIXES_FILE", DATADIR "/paludis/ebuild_entries_suffixes.conf")),
                    KeyValueConfigFileOptions(), &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation)
        {
        }

        bool is_known_suffix(const std::string & s) const
        {
            return ! file.get("suffix_" + s + "_known").empty();
        }

        std::string guess_eapi(const std::string & s) const
        {
            return file.get("guess_eapi_" + s);
        }

        std::string manifest_key(const std::string & s) const
        {
            std::string result(file.get("manifest_key_" + s));
            if (result.empty())
            {
                Log::get_instance()->message("e.ebuild.unknown_manifest_key", ll_warning, lc_context)
                    << "Don't know what the manifest key for files with suffix '" << s << "' is, guessing 'MISC'";
                return "MISC";
            }
            else
                return result;
        }
    };
}

const std::tr1::shared_ptr<const ERepositoryID>
ERepository::make_id(const QualifiedPackageName & q, const FSEntry & f) const
{
    Context context("When creating ID for '" + stringify(q) + "' from '" + stringify(f) + "':");

    std::tr1::shared_ptr<EbuildID> result(new EbuildID(q, extract_package_file_version(q, f),
                _imp->params.environment(),
                shared_from_this(), f, _guess_eapi(q, f),
                _imp->master_mtime, _imp->eclass_mtimes));
    return result;
}

namespace
{
    class AFinder :
        private InstantiationPolicy<AFinder, instantiation_method::NonCopyableTag>
    {
        private:
            std::list<std::pair<const FetchableURIDepSpec *, const URILabelsDepSpec *> > _specs;
            std::list<const URILabelsDepSpec *> _labels;

            const Environment * const env;
            const std::tr1::shared_ptr<const PackageID> id;

        public:
            AFinder(const Environment * const e, const std::tr1::shared_ptr<const PackageID> & i) :
                env(e),
                id(i)
            {
                _labels.push_back(0);
            }

            void visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node)
            {
                _specs.push_back(std::make_pair(node.spec().get(), *_labels.begin()));
            }

            void visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type & node)
            {
                *_labels.begin() = node.spec().get();
            }

            void visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node)
            {
                _labels.push_front(*_labels.begin());
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                _labels.pop_front();
            }

            void visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
            {
                if (node.spec()->condition_met())
                {
                    _labels.push_front(*_labels.begin());
                    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                    _labels.pop_front();
                }
            }

            typedef std::list<std::pair<const FetchableURIDepSpec *,
                    const URILabelsDepSpec *> >::const_iterator ConstIterator;

            ConstIterator begin()
            {
                return _specs.begin();
            }

            ConstIterator end() const
            {
                return _specs.end();
            }
    };
}

namespace
{
    std::string make_use(const Environment * const,
            const ERepositoryID & id,
            std::tr1::shared_ptr<const Profile> profile)
    {
        if (! id.eapi()->supported())
        {
            Log::get_instance()->message("e.ebuild.unknown_eapi", ll_warning, lc_context)
                << "Can't make the USE string for '" << id << "' because its EAPI is unsupported";
            return "";
        }

        std::string use;

        if (id.choices_key())
        {
            for (Choices::ConstIterator k(id.choices_key()->value()->begin()),
                    k_end(id.choices_key()->value()->end()) ;
                    k != k_end ; ++k)
            {
                if ((*k)->prefix() == canonical_build_options_prefix())
                    continue;

                for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                        i != i_end ; ++i)
                    if ((*i)->enabled())
                        use += stringify((*i)->name_with_prefix()) + " ";
            }
        }

        if (! id.eapi()->supported()->ebuild_environment_variables()->env_arch().empty())
            use += profile->environment_variable(id.eapi()->supported()->ebuild_environment_variables()->env_arch()) + " ";

        return use;
    }

    std::tr1::shared_ptr<Map<std::string, std::string> >
    make_expand(const Environment * const,
            const ERepositoryID & e,
            std::tr1::shared_ptr<const Profile> profile)
    {
        std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(
            new Map<std::string, std::string>);

        if (! e.eapi()->supported())
        {
            Log::get_instance()->message("e.ebuild.unknown_eapi", ll_warning, lc_context)
                << "Can't make the USE_EXPAND strings for '" << e << "' because its EAPI is unsupported";
            return expand_vars;
        }

        if (! e.choices_key())
            return expand_vars;

        for (Set<std::string>::ConstIterator x(profile->use_expand()->begin()), x_end(profile->use_expand()->end()) ;
                x != x_end ; ++x)
        {
            expand_vars->insert(stringify(*x), "");

            Choices::ConstIterator k(std::find_if(e.choices_key()->value()->begin(), e.choices_key()->value()->end(),
                        std::tr1::bind(std::equal_to<std::string>(), *x,
                            std::tr1::bind(std::tr1::mem_fn(&Choice::raw_name), std::tr1::placeholders::_1))));
            if (k == e.choices_key()->value()->end())
                continue;

            for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                    i != i_end ; ++i)
                if ((*i)->enabled())
                {
                    std::string value;
                    Map<std::string, std::string>::ConstIterator v(expand_vars->find(stringify(*x)));
                    if (expand_vars->end() != v)
                    {
                        value = v->second;
                        if (! value.empty())
                            value.append(" ");
                        expand_vars->erase(v);
                    }
                    value.append(stringify((*i)->unprefixed_name()));
                    expand_vars->insert(stringify(*x), value);
                }
        }

        return expand_vars;
    }
}

namespace
{
    bool
    check_userpriv(const FSEntry & f, const Environment * env, bool mandatory)
    {
        Context c("When checking permissions on '" + stringify(f) + "' for userpriv:");

        if (! getenv_with_default("PALUDIS_BYPASS_USERPRIV_CHECKS", "").empty())
            return false;

        if (f.exists())
        {
            if (f.group() != env->reduced_gid())
            {
                if (mandatory)
                    throw ConfigurationError("Directory '" + stringify(f) + "' owned by group '" + get_group_name(f.group())
                            + "', not '" + get_group_name(env->reduced_gid()) + "'");
                else
                    Log::get_instance()->message("e.ebuild.userpriv_disabled", ll_warning, lc_context) << "Directory '" <<
                        f << "' owned by group '" << get_group_name(f.group()) << "', not '"
                        << get_group_name(env->reduced_gid()) << "', so cannot enable userpriv";
                return false;
            }
            else if (! f.has_permission(fs_ug_group, fs_perm_write))
            {
                if (mandatory)
                    throw ConfigurationError("Directory '" + stringify(f) + "' does not have group write permission");
                else
                    Log::get_instance()->message("e.ebuild.userpriv_disabled", ll_warning, lc_context) << "Directory '" <<
                        f << "' does not have group write permission, cannot enable userpriv";
                return false;
            }
        }

        return true;
    }

    const std::tr1::shared_ptr<const MirrorsSequence>
    get_mirrors_fn(const std::string & m, const MirrorMap & map)
    {
        MirrorMap::const_iterator i(map.find(m));
        if (i == map.end())
            return make_shared_ptr(new MirrorsSequence);
        else
            return i->second;
    }
}

void
ERepository::fetch(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const FetchAction & fetch_action) const
{
    using namespace std::tr1::placeholders;

    Context context("When fetching '" + stringify(*id) + "':");

    bool fetch_restrict(false), userpriv_restrict(false);
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(_imp->params.environment());
        if (id->restrict_key())
            id->restrict_key()->value()->root()->accept(restricts);

        for (DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec>::ConstIterator i(restricts.begin()), i_end(restricts.end()) ;
                i != i_end ; ++i)
        {
            if (id->eapi()->supported()->ebuild_options()->restrict_fetch()->end() !=
                    std::find(id->eapi()->supported()->ebuild_options()->restrict_fetch()->begin(),
                        id->eapi()->supported()->ebuild_options()->restrict_fetch()->end(), (*i)->text()))
                fetch_restrict = true;
            if ("userpriv" == (*i)->text() || "nouserpriv" == (*i)->text())
                userpriv_restrict = true;
        }
    }

    bool fetch_userpriv_ok(_imp->params.environment()->reduced_gid() != getgid() &&
            check_userpriv(FSEntry(_imp->params.distdir()), _imp->params.environment(),
                id->eapi()->supported()->userpriv_cannot_use_root()));

    std::string archives, all_archives;
    {
        std::set<std::string> already_in_archives;

        /* make A */
        AFinder f(_imp->params.environment(), id);
        if (id->fetches_key())
            id->fetches_key()->value()->root()->accept(f);

        for (AFinder::ConstIterator i(f.begin()), i_end(f.end()) ; i != i_end ; ++i)
        {
            const FetchableURIDepSpec * const spec(static_cast<const FetchableURIDepSpec *>(i->first));

            if (already_in_archives.end() == already_in_archives.find(spec->filename()))
            {
                archives.append(spec->filename());
                already_in_archives.insert(spec->filename());
            }
            archives.append(" ");
        }

        /* make AA */
        if (! id->eapi()->supported()->ebuild_environment_variables()->env_aa().empty())
        {
            AAVisitor g;
            if (id->fetches_key())
                id->fetches_key()->value()->root()->accept(g);
            std::set<std::string> already_in_all_archives;

            for (AAVisitor::ConstIterator gg(g.begin()), gg_end(g.end()) ; gg != gg_end ; ++gg)
            {
                if (already_in_all_archives.end() == already_in_all_archives.find(*gg))
                {
                    all_archives.append(*gg);
                    already_in_all_archives.insert(*gg);
                }
                all_archives.append(" ");
            }
        }
        else
            all_archives = "AA-not-set-for-this-EAPI";
    }

    /* Strip trailing space. Some ebuilds rely upon this. From kde-meta.eclass:
     *     [[ -n ${A/${TARBALL}/} ]] && unpack ${A/${TARBALL}/}
     * Rather annoying.
     */
    archives = strip_trailing(archives, " ");
    all_archives = strip_trailing(all_archives, " ");

    std::tr1::shared_ptr<OutputManager> output_manager(fetch_action.options.make_output_manager()(fetch_action));

    CheckFetchedFilesVisitor c(_imp->params.environment(), id, _imp->params.distdir(),
            fetch_action.options.fetch_parts()[fp_unneeded], fetch_restrict,
            ((_imp->layout->package_directory(id->name())) / "Manifest"),
            _imp->params.use_manifest(),
            output_manager, fetch_action.options.exclude_unmirrorable(),
            fetch_action.options.ignore_unfetched());

    if (id->fetches_key())
    {
        /* always use mirror://gentoo/, where gentoo is the name of our first master repository,
         * or our name if there's no master. */
        std::string mirrors_name(
                (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                stringify((*_imp->params.master_repositories()->begin())->name()) :
                stringify(name()));

        if (fetch_action.options.fetch_parts()[fp_regulars] && ! fetch_action.options.ignore_unfetched())
        {
            need_mirrors();

            FetchVisitor f(_imp->params.environment(), id, *id->eapi(),
                    _imp->params.distdir(), fetch_action.options.fetch_parts()[fp_unneeded],
                    fetch_userpriv_ok, mirrors_name,
                    id->fetches_key()->initial_label(), fetch_action.options.safe_resume(),
                    output_manager, std::tr1::bind(&get_mirrors_fn, std::tr1::placeholders::_1, std::tr1::cref(_imp->mirrors)));
            id->fetches_key()->value()->root()->accept(f);
        }

        id->fetches_key()->value()->root()->accept(c);
    }

    if ( (fetch_action.options.fetch_parts()[fp_extras]) && ((c.need_nofetch()) ||
            ((! fetch_action.options.ignore_unfetched()) && (! id->eapi()->supported()->ebuild_phases()->ebuild_fetch_extra().empty()))))
    {
        bool userpriv_ok((! userpriv_restrict) && (_imp->params.environment()->reduced_gid() != getgid()) &&
                check_userpriv(FSEntry(_imp->params.builddir()), _imp->params.environment(),
                    id->eapi()->supported()->userpriv_cannot_use_root()));
        std::string use(make_use(_imp->params.environment(), *id, profile()));
        std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                    _imp->params.environment(), *id, profile()));

        std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(layout()->exlibsdirs(id->name()));

        EAPIPhases fetch_extra_phases(id->eapi()->supported()->ebuild_phases()->ebuild_fetch_extra());
        if ((! fetch_action.options.ignore_unfetched()) && (fetch_extra_phases.begin_phases() != fetch_extra_phases.end_phases()))
        {
            FSEntry package_builddir(_imp->params.builddir() / (stringify(id->name().category()) + "-" +
                    stringify(id->name().package()) + "-" + stringify(id->version()) + "-fetch_extra"));

            for (EAPIPhases::ConstIterator phase(fetch_extra_phases.begin_phases()), phase_end(fetch_extra_phases.end_phases()) ;
                    phase != phase_end ; ++phase)
            {
                if (can_skip_phase(id, *phase))
                    continue;

                EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                        value_for<n::builddir>(_imp->params.builddir()),
                        value_for<n::clearenv>(phase->option("clearenv")),
                        value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                        value_for<n::distdir>(_imp->params.distdir()),
                        value_for<n::ebuild_dir>(layout()->package_directory(id->name())),
                        value_for<n::ebuild_file>(id->fs_location_key()->value()),
                        value_for<n::eclassdirs>(_imp->params.eclassdirs()),
                        value_for<n::environment>(_imp->params.environment()),
                        value_for<n::exlibsdirs>(exlibsdirs),
                        value_for<n::files_dir>(layout()->package_directory(id->name()) / "files"),
                        value_for<n::maybe_output_manager>(output_manager),
                        value_for<n::package_builddir>(package_builddir),
                        value_for<n::package_id>(id),
                        value_for<n::portdir>(
                            (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                            (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
                        value_for<n::root>("/"),
                        value_for<n::sandbox>(phase->option("sandbox")),
                        value_for<n::sydbox>(phase->option("sydbox")),
                        value_for<n::userpriv>(phase->option("userpriv") && userpriv_ok)
                        ));

                EbuildFetchExtraCommand fetch_extra_cmd(command_params,
                        make_named_values<EbuildFetchExtraCommandParams>(
                        value_for<n::a>(archives),
                        value_for<n::aa>(all_archives),
                        value_for<n::expand_vars>(expand_vars),
                        value_for<n::loadsaveenv_dir>(package_builddir / "temp"),
                        value_for<n::profiles>(_imp->params.profiles()),
                        value_for<n::slot>(id->slot_key() ? stringify(id->slot_key()->value()) : ""),
                        value_for<n::use>(use),
                        value_for<n::use_expand>(join(profile()->use_expand()->begin(), profile()->use_expand()->end(), " ")),
                        value_for<n::use_expand_hidden>(join(profile()->use_expand_hidden()->begin(), profile()->use_expand_hidden()->end(), " "))
                        ));

                if (! fetch_extra_cmd())
                    throw ActionFailedError("Fetch of '" + stringify(*id) + "' failed");
            }
        }

        if (c.need_nofetch())
        {
            EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_nofetch());
            for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
                    phase != phase_end ; ++phase)
            {
                EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                        value_for<n::builddir>(_imp->params.builddir()),
                        value_for<n::clearenv>(phase->option("clearenv")),
                        value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                        value_for<n::distdir>(_imp->params.distdir()),
                        value_for<n::ebuild_dir>(layout()->package_directory(id->name())),
                        value_for<n::ebuild_file>(id->fs_location_key()->value()),
                        value_for<n::eclassdirs>(_imp->params.eclassdirs()),
                        value_for<n::environment>(_imp->params.environment()),
                        value_for<n::exlibsdirs>(exlibsdirs),
                        value_for<n::files_dir>(layout()->package_directory(id->name()) / "files"),
                        value_for<n::maybe_output_manager>(output_manager),
                        value_for<n::package_builddir>(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-nofetch")),
                        value_for<n::package_id>(id),
                        value_for<n::portdir>(
                            (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                            (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
                        value_for<n::root>("/"),
                        value_for<n::sandbox>(phase->option("sandbox")),
                        value_for<n::sydbox>(phase->option("sydbox")),
                        value_for<n::userpriv>(phase->option("userpriv") && userpriv_ok)
                        ));

                EbuildNoFetchCommand nofetch_cmd(command_params,
                        make_named_values<EbuildNoFetchCommandParams>(
                        value_for<n::a>(archives),
                        value_for<n::aa>(all_archives),
                        value_for<n::expand_vars>(expand_vars),
                        value_for<n::profiles>(_imp->params.profiles()),
                        value_for<n::use>(use),
                        value_for<n::use_expand>(join(profile()->use_expand()->begin(), profile()->use_expand()->end(), " ")),
                        value_for<n::use_expand_hidden>(join(profile()->use_expand_hidden()->begin(), profile()->use_expand_hidden()->end(), " "))
                        ));

                if (! nofetch_cmd())
                {
                    std::copy(c.failures()->begin(), c.failures()->end(),
                            fetch_action.options.errors()->back_inserter());
                    throw ActionFailedError("Fetch of '" + stringify(*id) + "' failed");
                }
            }
        }
    }

    if (! c.failures()->empty())
    {
        std::copy(c.failures()->begin(), c.failures()->end(),
                fetch_action.options.errors()->back_inserter());
        throw ActionFailedError("Fetch of '" + stringify(*id) + "' failed");
    }

    output_manager->succeeded();
}

void
ERepository::pretend_fetch(const std::tr1::shared_ptr<const ERepositoryID> & id,
        PretendFetchAction & a) const
{
    using namespace std::tr1::placeholders;

    Context context("When pretending to fetch ID '" + stringify(*id) + "':");

    if (id->fetches_key())
    {
        PretendFetchVisitor f(_imp->params.environment(), id, *id->eapi(),
                params().distdir(), a.options.fetch_parts()[fp_unneeded],
                id->fetches_key()->initial_label(), a);
        id->fetches_key()->value()->root()->accept(f);
    }
}

namespace
{
    bool slot_is_same(const std::tr1::shared_ptr<const PackageID> & a,
            const std::tr1::shared_ptr<const PackageID> & b)
    {
        if (a->slot_key())
            return b->slot_key() && a->slot_key()->value() == b->slot_key()->value();
        else
            return ! b->slot_key();
    }

    void used_this_for_config_protect(std::string & s, const std::string & v)
    {
        s = v;
    }

    std::tr1::shared_ptr<OutputManager> this_output_manager(const std::tr1::shared_ptr<OutputManager> & o, const Action &)
    {
        return o;
    }

    void installed_this(const FSEntry &)
    {
    }

    bool ignore_merged(const std::tr1::shared_ptr<const FSEntrySet> & s,
            const FSEntry & f)
    {
        return s->end() != s->find(f);
    }
}

void
ERepository::install(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const InstallAction & install_action) const
{
    using namespace std::tr1::placeholders;

    Context context("When installing '" + stringify(*id) + "'" +
            (install_action.options.replacing()->empty() ? "" : " replacing { '"
             + join(indirect_iterator(install_action.options.replacing()->begin()),
                 indirect_iterator(install_action.options.replacing()->end()), "', '") + "' }") + ":");

    std::tr1::shared_ptr<OutputManager> output_manager(install_action.options.make_output_manager()(install_action));

    bool userpriv_restrict, test_restrict, strip_restrict;
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(_imp->params.environment());
        if (id->restrict_key())
            id->restrict_key()->value()->root()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));

        test_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "test"));

        strip_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "strip")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nostrip"));
    }

    std::string archives, all_archives;
    {
        std::set<std::string> already_in_archives;

        /* make A */
        AFinder f(_imp->params.environment(), id);
        if (id->fetches_key())
            id->fetches_key()->value()->root()->accept(f);

        for (AFinder::ConstIterator i(f.begin()), i_end(f.end()) ; i != i_end ; ++i)
        {
            const FetchableURIDepSpec * const spec(static_cast<const FetchableURIDepSpec *>(i->first));

            if (already_in_archives.end() == already_in_archives.find(spec->filename()))
            {
                archives.append(spec->filename());
                already_in_archives.insert(spec->filename());
            }
            archives.append(" ");
        }

        /* make AA */
        if (! id->eapi()->supported()->ebuild_environment_variables()->env_aa().empty())
        {
            AAVisitor g;
            if (id->fetches_key())
                id->fetches_key()->value()->root()->accept(g);
            std::set<std::string> already_in_all_archives;

            for (AAVisitor::ConstIterator gg(g.begin()), gg_end(g.end()) ; gg != gg_end ; ++gg)
            {
                if (already_in_all_archives.end() == already_in_all_archives.find(*gg))
                {
                    all_archives.append(*gg);
                    already_in_all_archives.insert(*gg);
                }
                all_archives.append(" ");
            }
        }
        else
            all_archives = "AA-not-set-for-this-EAPI";
    }

    /* Strip trailing space. Some ebuilds rely upon this. From kde-meta.eclass:
     *     [[ -n ${A/${TARBALL}/} ]] && unpack ${A/${TARBALL}/}
     * Rather annoying.
     */
    archives = strip_trailing(archives, " ");
    all_archives = strip_trailing(all_archives, " ");

    /* make use */
    std::string use(make_use(_imp->params.environment(), *id, profile()));

    /* add expand to use (iuse isn't reliable for use_expand things), and make the expand
     * environment variables */
    std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                _imp->params.environment(), *id, profile()));

    std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(layout()->exlibsdirs(id->name()));

    bool userpriv_ok((! userpriv_restrict) && (_imp->params.environment()->reduced_gid() != getgid()) &&
            check_userpriv(FSEntry(_imp->params.distdir()),  _imp->params.environment(), id->eapi()->supported()->userpriv_cannot_use_root()) &&
            check_userpriv(FSEntry(_imp->params.builddir()), _imp->params.environment(), id->eapi()->supported()->userpriv_cannot_use_root()));

    FSEntry package_builddir(_imp->params.builddir() / (stringify(id->name().category()) + "-" +
            stringify(id->name().package()) + "-" + stringify(id->version())));

    std::string used_config_protect;
    std::tr1::shared_ptr<FSEntrySet> merged_entries(new FSEntrySet);

    std::tr1::shared_ptr<const ChoiceValue> preserve_work_choice(
            id->choices_key()->value()->find_by_name_with_prefix(
                ELikePreserveWorkChoiceValue::canonical_name_with_prefix()));

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_install());
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        bool skip(false);
        do
        {
            switch (install_action.options.want_phase()(phase->equal_option("skipname")))
            {
                case wp_yes:
                    continue;

                case wp_skip:
                    skip = true;
                    continue;

                case wp_abort:
                    throw ActionAbortedError("Told to abort install");

                case last_wp:
                    break;
            }

            throw InternalError(PALUDIS_HERE, "bad want_phase");
        } while (false);

        if (skip)
            continue;

        if (can_skip_phase(id, *phase))
        {
            output_manager->stdout_stream() << "--- No need to do anything for " << phase->equal_option("skipname") << " phase" << std::endl;
            continue;
        }

        if (phase->option("tidyup") && preserve_work_choice && preserve_work_choice->enabled())
        {
            output_manager->stdout_stream() << "--- Skipping " << phase->equal_option("skipname")
                << " phase to preserve work" << std::endl;
            continue;
        }

        if (phase->option("merge"))
        {
            if (! (*install_action.options.destination()).destination_interface())
                throw ActionFailedError("Can't install '" + stringify(*id)
                        + "' to destination '" + stringify(install_action.options.destination()->name())
                        + "' because destination does not provide destination_interface");

            MergerOptions extra_merger_options;
            if (preserve_work_choice && preserve_work_choice->enabled())
                extra_merger_options += mo_nondestructive;

            Timestamp build_start_time(FSEntry(package_builddir / "temp" / "build_start_time").mtim());
            (*install_action.options.destination()).destination_interface()->merge(
                    make_named_values<MergeParams>(
                        value_for<n::build_start_time>(build_start_time),
                        value_for<n::environment_file>(package_builddir / "temp" / "loadsaveenv"),
                        value_for<n::image_dir>(package_builddir / "image"),
                        value_for<n::merged_entries>(merged_entries),
                        value_for<n::options>(id->eapi()->supported()->merger_options() | extra_merger_options),
                        value_for<n::output_manager>(output_manager),
                        value_for<n::package_id>(id),
                        value_for<n::perform_uninstall>(install_action.options.perform_uninstall()),
                        value_for<n::used_this_for_config_protect>(std::tr1::bind(
                                &used_this_for_config_protect, std::tr1::ref(used_config_protect), std::tr1::placeholders::_1))
                        ));
        }
        else if (phase->option("strip"))
        {
            if (! strip_restrict)
            {
                std::string libdir("lib");
                FSEntry root(install_action.options.destination()->installed_root_key() ?
                        stringify(install_action.options.destination()->installed_root_key()->value()) : "/");
                if ((root / "usr" / "lib").is_symbolic_link())
                {
                    libdir = (root / "usr" / "lib").readlink();
                    if (std::string::npos != libdir.find_first_of("./"))
                        libdir = "lib";
                }

                Log::get_instance()->message("e.ebuild.libdir", ll_debug, lc_context) << "Using '" << libdir << "' for libdir";

                std::tr1::shared_ptr<const ChoiceValue> strip_choice(id->choices_key()->value()->find_by_name_with_prefix(
                            ELikeStripChoiceValue::canonical_name_with_prefix()));
                std::tr1::shared_ptr<const ChoiceValue> split_choice(id->choices_key()->value()->find_by_name_with_prefix(
                            ELikeSplitChoiceValue::canonical_name_with_prefix()));

                EStripper stripper(make_named_values<EStripperOptions>(
                        value_for<n::debug_dir>(package_builddir / "image" / "usr" / libdir / "debug"),
                        value_for<n::image_dir>(package_builddir / "image"),
                        value_for<n::output_manager>(output_manager),
                        value_for<n::package_id>(id),
                        value_for<n::split>(split_choice && split_choice->enabled()),
                        value_for<n::strip>(strip_choice && strip_choice->enabled())
                        ));
                stripper.strip();
            }
        }
        else if ((! phase->option("prepost")) ||
                ((*install_action.options.destination()).destination_interface() &&
                 (*install_action.options.destination()).destination_interface()->want_pre_post_phases()))
        {
            if (phase->option("optional_tests"))
            {
                if (test_restrict)
                    continue;

                std::tr1::shared_ptr<const ChoiceValue> choice(id->choices_key()->value()->find_by_name_with_prefix(
                            ELikeOptionalTestsChoiceValue::canonical_name_with_prefix()));
                if (choice && ! choice->enabled())
                    continue;
            }
            else if (phase->option("recommended_tests"))
            {
                if (test_restrict)
                    continue;

                std::tr1::shared_ptr<const ChoiceValue> choice(id->choices_key()->value()->find_by_name_with_prefix(
                            ELikeRecommendedTestsChoiceValue::canonical_name_with_prefix()));
                if (choice && ! choice->enabled())
                    continue;
            }
            else if (phase->option("expensive_tests"))
            {
                std::tr1::shared_ptr<const ChoiceValue> choice(id->choices_key()->value()->find_by_name_with_prefix(
                            ELikeExpensiveTestsChoiceValue::canonical_name_with_prefix()));
                if (choice && ! choice->enabled())
                    continue;
            }

            EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                    value_for<n::builddir>(_imp->params.builddir()),
                    value_for<n::clearenv>(phase->option("clearenv")),
                    value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                    value_for<n::distdir>(_imp->params.distdir()),
                    value_for<n::ebuild_dir>(layout()->package_directory(id->name())),
                    value_for<n::ebuild_file>(id->fs_location_key()->value()),
                    value_for<n::eclassdirs>(_imp->params.eclassdirs()),
                    value_for<n::environment>(_imp->params.environment()),
                    value_for<n::exlibsdirs>(exlibsdirs),
                    value_for<n::files_dir>(layout()->package_directory(id->name()) / "files"),
                    value_for<n::maybe_output_manager>(output_manager),
                    value_for<n::package_builddir>(package_builddir),
                    value_for<n::package_id>(id),
                    value_for<n::portdir>(
                        (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                        (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
                    value_for<n::root>(install_action.options.destination()->installed_root_key() ?
                        stringify(install_action.options.destination()->installed_root_key()->value()) :
                        "/"),
                    value_for<n::sandbox>(phase->option("sandbox")),
                    value_for<n::sydbox>(phase->option("sydbox")),
                    value_for<n::userpriv>(phase->option("userpriv") && userpriv_ok)
                    ));

            EbuildInstallCommandParams install_params(
                    make_named_values<EbuildInstallCommandParams>(
                            value_for<n::a>(archives),
                            value_for<n::aa>(all_archives),
                            value_for<n::config_protect>(profile_variable("CONFIG_PROTECT")),
                            value_for<n::config_protect_mask>(profile_variable("CONFIG_PROTECT_MASK")),
                            value_for<n::expand_vars>(expand_vars),
                            value_for<n::loadsaveenv_dir>(package_builddir / "temp"),
                            value_for<n::profiles>(_imp->params.profiles()),
                            value_for<n::replacing_ids>(install_action.options.replacing()),
                            value_for<n::slot>(id->slot_key() ? stringify(id->slot_key()->value()) : ""),
                            value_for<n::use>(use),
                            value_for<n::use_expand>(join(profile()->use_expand()->begin(), profile()->use_expand()->end(), " ")),
                            value_for<n::use_expand_hidden>(join(profile()->use_expand_hidden()->begin(), profile()->use_expand_hidden()->end(), " "))
                            ));

            EbuildInstallCommand cmd(command_params, install_params);
            cmd();
        }
    }

    for (PackageIDSequence::ConstIterator i(install_action.options.replacing()->begin()), i_end(install_action.options.replacing()->end()) ;
            i != i_end ; ++i)
    {
        Context local_context("When cleaning '" + stringify(**i) + "':");
        if ((*i)->name() == id->name() && (*i)->version() == id->version())
            continue;

        if (id->eapi()->supported()->ebuild_phases()->ebuild_new_upgrade_phase_order())
            if ((*i)->name() == id->name() && slot_is_same(*i, id))
                continue;

        UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                    value_for<n::config_protect>(used_config_protect),
                    value_for<n::if_for_install_id>(id),
                    value_for<n::ignore_for_unmerge>(std::tr1::bind(&ignore_merged, merged_entries,
                            std::tr1::placeholders::_1)),
                    value_for<n::is_overwrite>(false),
                    value_for<n::make_output_manager>(std::tr1::bind(&this_output_manager, output_manager, std::tr1::placeholders::_1))
                    ));
        install_action.options.perform_uninstall()(*i, uo);
    }

    output_manager->succeeded();
}

void
ERepository::info(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const InfoAction & a) const
{
    using namespace std::tr1::placeholders;

    Context context("When infoing '" + stringify(*id) + "':");

    std::tr1::shared_ptr<OutputManager> output_manager(a.options.make_output_manager()(a));

    bool userpriv_restrict;
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(_imp->params.environment());
        if (id->restrict_key())
            id->restrict_key()->value()->root()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));
    }
    bool userpriv_ok((! userpriv_restrict) && (_imp->params.environment()->reduced_gid() != getgid()) &&
            check_userpriv(FSEntry(_imp->params.builddir()), _imp->params.environment(), id->eapi()->supported()->userpriv_cannot_use_root()));

    /* make use */
    std::string use(make_use(_imp->params.environment(), *id, profile()));

    /* add expand to use (iuse isn't reliable for use_expand things), and make the expand
     * environment variables */
    std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                _imp->params.environment(), *id, profile()));

    std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(layout()->exlibsdirs(id->name()));

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_info());
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (phase->option("installed=true"))
            continue;

        EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                value_for<n::builddir>(_imp->params.builddir()),
                value_for<n::clearenv>(phase->option("clearenv")),
                value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                value_for<n::distdir>(_imp->params.distdir()),
                value_for<n::ebuild_dir>(layout()->package_directory(id->name())),
                value_for<n::ebuild_file>(id->fs_location_key()->value()),
                value_for<n::eclassdirs>(_imp->params.eclassdirs()),
                value_for<n::environment>(_imp->params.environment()),
                value_for<n::exlibsdirs>(exlibsdirs),
                value_for<n::files_dir>(layout()->package_directory(id->name()) / "files"),
                value_for<n::maybe_output_manager>(output_manager),
                value_for<n::package_builddir>(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-info")),
                value_for<n::package_id>(id),
                value_for<n::portdir>(
                    (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                    (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
                value_for<n::root>(stringify(_imp->params.environment()->root())),
                value_for<n::sandbox>(phase->option("sandbox")),
                value_for<n::sydbox>(phase->option("sydbox")),
                value_for<n::userpriv>(phase->option("userpriv") && userpriv_ok)
                ));

        EbuildInfoCommandParams info_params(
                make_named_values<EbuildInfoCommandParams>(
                value_for<n::expand_vars>(expand_vars),
                value_for<n::info_vars>(info_vars_key() ?
                    info_vars_key()->value() : make_shared_ptr(new const Set<std::string>)),
                value_for<n::load_environment>(static_cast<const FSEntry *>(0)),
                value_for<n::profiles>(_imp->params.profiles()),
                value_for<n::use>(use),
                value_for<n::use_ebuild_file>(true),
                value_for<n::use_expand>(join(profile()->use_expand()->begin(), profile()->use_expand()->end(), " ")),
                value_for<n::use_expand_hidden>(join(profile()->use_expand_hidden()->begin(), profile()->use_expand_hidden()->end(), " "))
                ));

        EbuildInfoCommand cmd(command_params, info_params);
        cmd();
    }
}

std::string
ERepository::get_environment_variable(
        const std::tr1::shared_ptr<const PackageID> & id_uncasted,
        const std::string & var) const
{
    const std::tr1::shared_ptr<const ERepositoryID> id(std::tr1::static_pointer_cast<const ERepositoryID>(id_uncasted));

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_variable());

    int c(std::distance(phases.begin_phases(), phases.end_phases()));
    if (1 != c)
        throw EAPIConfigurationError("EAPI '" + id->eapi()->name() + "' defines "
                + (c == 0 ? "no" : stringify(c)) + " ebuild variable phases but expected exactly one");

    bool userpriv_restrict;
    {
        using namespace std::tr1::placeholders;

        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(_imp->params.environment());
        if (id->restrict_key())
            id->restrict_key()->value()->root()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));
    }
    bool userpriv_ok((! userpriv_restrict) && (_imp->params.environment()->reduced_gid() != getgid()) &&
            check_userpriv(FSEntry(_imp->params.builddir()), _imp->params.environment(), id->eapi()->supported()->userpriv_cannot_use_root()));

    std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(layout()->exlibsdirs(id->name()));

    EbuildVariableCommand cmd(make_named_values<EbuildCommandParams>(
            value_for<n::builddir>(_imp->params.builddir()),
            value_for<n::clearenv>(phases.begin_phases()->option("clearenv")),
            value_for<n::commands>(join(phases.begin_phases()->begin_commands(), phases.begin_phases()->end_commands(), " ")),
            value_for<n::distdir>(_imp->params.distdir()),
            value_for<n::ebuild_dir>(layout()->package_directory(id->name())),
            value_for<n::ebuild_file>(id->fs_location_key()->value()),
            value_for<n::eclassdirs>(_imp->params.eclassdirs()),
            value_for<n::environment>(_imp->params.environment()),
            value_for<n::exlibsdirs>(exlibsdirs),
            value_for<n::files_dir>(layout()->package_directory(id->name()) / "files"),
            value_for<n::maybe_output_manager>(make_null_shared_ptr()),
            value_for<n::package_builddir>(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-variable")),
            value_for<n::package_id>(id),
            value_for<n::portdir>(
                (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
            value_for<n::root>("/"),
            value_for<n::sandbox>(phases.begin_phases()->option("sandbox")),
            value_for<n::sydbox>(phases.begin_phases()->option("sydbox")),
            value_for<n::userpriv>(phases.begin_phases()->option("userpriv") && userpriv_ok)
            ),

            var);

    if (! cmd())
        throw ActionFailedError("Couldn't get environment variable '" + stringify(var) +
                "' for package '" + stringify(*id) + "'");

    return cmd.result();
}

void
ERepository::merge(const MergeParams & m)
{
    Context context("When merging '" + stringify(*m.package_id()) + "' at '" + stringify(m.image_dir())
            + "' to E repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(*m.package_id()))
        throw ActionFailedError("Not a suitable destination for '" + stringify(*m.package_id()) + "'");

    FSEntry binary_ebuild_location(layout()->binary_ebuild_location(
                m.package_id()->name(), m.package_id()->version(),
                "pbin-1+" + std::tr1::static_pointer_cast<const ERepositoryID>(m.package_id())->eapi()->name()));

    binary_ebuild_location.dirname().dirname().mkdir();
    binary_ebuild_location.dirname().mkdir();

    WriteBinaryEbuildCommand write_binary_ebuild_command(
            make_named_values<WriteBinaryEbuildCommandParams>(
            value_for<n::binary_distdir>(_imp->params.binary_distdir()),
            value_for<n::binary_ebuild_location>(binary_ebuild_location),
            value_for<n::builddir>(_imp->params.builddir()),
            value_for<n::destination_repository>(this),
            value_for<n::environment>(_imp->params.environment()),
            value_for<n::environment_file>(m.environment_file()),
            value_for<n::image>(m.image_dir()),
            value_for<n::maybe_output_manager>(m.output_manager()),
            value_for<n::merger_options>(std::tr1::static_pointer_cast<const ERepositoryID>(m.package_id())->eapi()->supported()->merger_options()),
            value_for<n::package_id>(std::tr1::static_pointer_cast<const ERepositoryID>(m.package_id()))
            ));

    write_binary_ebuild_command();
}

bool
ERepository::is_package_file(const QualifiedPackageName & n, const FSEntry & e) const
{
    Context context("When working out whether '" + stringify(e) + "' is a package file for '" + stringify(n) + "':");

    if (0 != e.basename().compare(0, stringify(n.package()).length() + 1, stringify(n.package()) + "-"))
        return false;

    std::string::size_type p(e.basename().rfind('.'));
    if (std::string::npos == p)
        return false;

    std::string suffix(e.basename().substr(p + 1));
    return Suffixes::get_instance()->is_known_suffix(suffix);
}

VersionSpec
ERepository::extract_package_file_version(const QualifiedPackageName & n, const FSEntry & e) const
{
    Context context("When extracting version from '" + stringify(e) + "':");
    std::string::size_type p(e.basename().rfind('.'));
    if (std::string::npos == p)
        throw InternalError(PALUDIS_HERE, "got npos");
    return VersionSpec(strip_leading_string(e.basename().substr(0, p), stringify(n.package()) + "-"),
            EAPIData::get_instance()->eapi_from_string(
                _imp->params.eapi_when_unknown())->supported()->version_spec_options());
}

bool
ERepository::pretend(
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const PretendAction & a) const
{
    using namespace std::tr1::placeholders;

    Context context("When running pretend for '" + stringify(*id) + "':");

    if (! id->eapi()->supported())
        return false;

    bool result(true);

    if (! id->raw_myoptions_key())
        if (id->eapi()->supported()->ebuild_phases()->ebuild_pretend().empty())
            return result;

    bool userpriv_restrict;
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(_imp->params.environment());
        if (id->restrict_key())
            id->restrict_key()->value()->root()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));
    }
    bool userpriv_ok((! userpriv_restrict) && (_imp->params.environment()->reduced_gid() != getgid()) &&
            check_userpriv(FSEntry(_imp->params.builddir()), _imp->params.environment(), id->eapi()->supported()->userpriv_cannot_use_root()));

    std::string use(make_use(_imp->params.environment(), *id, profile()));
    std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                _imp->params.environment(), *id, profile()));

    std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(layout()->exlibsdirs(id->name()));

    std::tr1::shared_ptr<OutputManager> output_manager;

    if (id->raw_myoptions_key())
    {
        MyOptionsRequirementsVerifier verifier(id);
        id->raw_myoptions_key()->value()->root()->accept(verifier);

        if (verifier.unmet_requirements() && ! verifier.unmet_requirements()->empty())
        {
            EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_bad_options());
            if (phases.begin_phases() == phases.end_phases())
                throw InternalError(PALUDIS_HERE, "using myoptions but no ebuild_bad_options phase");

            for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
                    phase != phase_end ; ++phase)
            {
                if (! output_manager)
                    output_manager = a.options.make_output_manager()(a);

                EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                            value_for<n::builddir>(_imp->params.builddir()),
                            value_for<n::clearenv>(phase->option("clearenv")),
                            value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                            value_for<n::distdir>(_imp->params.distdir()),
                            value_for<n::ebuild_dir>(layout()->package_directory(id->name())),
                            value_for<n::ebuild_file>(id->fs_location_key()->value()),
                            value_for<n::eclassdirs>(_imp->params.eclassdirs()),
                            value_for<n::environment>(_imp->params.environment()),
                            value_for<n::exlibsdirs>(exlibsdirs),
                            value_for<n::files_dir>(layout()->package_directory(id->name()) / "files"),
                            value_for<n::maybe_output_manager>(output_manager),
                            value_for<n::package_builddir>(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-bad_options")),
                            value_for<n::package_id>(id),
                            value_for<n::portdir>(
                                (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                                (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
                            value_for<n::root>(stringify(_imp->params.environment()->root())),
                            value_for<n::sandbox>(phase->option("sandbox")),
                            value_for<n::sydbox>(phase->option("sydbox")),
                            value_for<n::userpriv>(phase->option("userpriv") && userpriv_ok)
                            ));

                EbuildBadOptionsCommand bad_options_cmd(command_params,
                        make_named_values<EbuildBadOptionsCommandParams>(
                            value_for<n::expand_vars>(expand_vars),
                            value_for<n::profiles>(_imp->params.profiles()),
                            value_for<n::unmet_requirements>(verifier.unmet_requirements()),
                            value_for<n::use>(use),
                            value_for<n::use_expand>(join(profile()->use_expand()->begin(), profile()->use_expand()->end(), " ")),
                            value_for<n::use_expand_hidden>(join(profile()->use_expand_hidden()->begin(), profile()->use_expand_hidden()->end(), " "))
                            ));

                if (! bad_options_cmd())
                    throw ActionFailedError("Bad options phase died");
            }

            result = false;
        }
    }

    if (id->eapi()->supported()->ebuild_phases()->ebuild_pretend().empty())
        return result;

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_pretend());
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (can_skip_phase(id, *phase))
            continue;

        if (! output_manager)
            output_manager = a.options.make_output_manager()(a);

        EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                value_for<n::builddir>(_imp->params.builddir()),
                value_for<n::clearenv>(phase->option("clearenv")),
                value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                value_for<n::distdir>(_imp->params.distdir()),
                value_for<n::ebuild_dir>(layout()->package_directory(id->name())),
                value_for<n::ebuild_file>(id->fs_location_key()->value()),
                value_for<n::eclassdirs>(_imp->params.eclassdirs()),
                value_for<n::environment>(_imp->params.environment()),
                value_for<n::exlibsdirs>(exlibsdirs),
                value_for<n::files_dir>(layout()->package_directory(id->name()) / "files"),
                value_for<n::maybe_output_manager>(output_manager),
                value_for<n::package_builddir>(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-pretend")),
                value_for<n::package_id>(id),
                value_for<n::portdir>(
                    (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                    (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
                value_for<n::root>(stringify(_imp->params.environment()->root())),
                value_for<n::sandbox>(phase->option("sandbox")),
                value_for<n::sydbox>(phase->option("sydbox")),
                value_for<n::userpriv>(phase->option("userpriv") && userpriv_ok)
                ));

        EbuildPretendCommand pretend_cmd(command_params,
                make_named_values<EbuildPretendCommandParams>(
                value_for<n::expand_vars>(expand_vars),
                value_for<n::profiles>(_imp->params.profiles()),
                value_for<n::use>(use),
                value_for<n::use_expand>(join(profile()->use_expand()->begin(), profile()->use_expand()->end(), " ")),
                value_for<n::use_expand_hidden>(join(profile()->use_expand_hidden()->begin(), profile()->use_expand_hidden()->end(), " "))
                ));

        if (! pretend_cmd())
            return false;
    }

    return result;
}

const std::string
ERepository::get_package_file_manifest_key(const FSEntry & e, const QualifiedPackageName & q) const
{
    if (! is_package_file(q, e))
        return "";

    std::string::size_type p(e.basename().rfind('.'));
    if (std::string::npos == p)
        return "EBUILD";

    std::string suffix(e.basename().substr(p + 1));
    return Suffixes::get_instance()->manifest_key(suffix);
}

const std::string
ERepository::binary_ebuild_name(const QualifiedPackageName & q, const VersionSpec & v, const std::string & e) const
{
    return stringify(q.package()) + "-" + stringify(v) + "." + e;
}

const std::string
ERepository::_guess_eapi(const QualifiedPackageName &, const FSEntry & e) const
{
    std::string::size_type p(e.basename().rfind('.'));
    if (std::string::npos == p)
        return "";

    std::string suffix(e.basename().substr(p + 1));
    return Suffixes::get_instance()->guess_eapi(suffix);
}

