/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/repositories/e/e_repository_profile_file.hh>
#include <paludis/repositories/e/e_repository_profile.hh>
#include <paludis/repositories/e/e_repository_news.hh>
#include <paludis/repositories/e/e_repository_sets.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository_entries.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/eclass_mtimes.hh>
#include <paludis/repositories/e/extra_distribution_data.hh>
#include <paludis/repositories/e/use_desc.hh>
#include <paludis/repositories/e/layout.hh>
#include <paludis/repositories/e/info_metadata_key.hh>
#include <paludis/repositories/e/extra_distribution_data.hh>
#include <paludis/repositories/e/memoised_hashes.hh>

#include <paludis/util/config_file.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/distribution.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/dep_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/match_package.hh>
#include <paludis/repository_name_cache.hh>
#include <paludis/syncer.hh>
#include <paludis/action.hh>
#include <paludis/mask.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <paludis/about.hh>
#include <paludis/choice.hh>

#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/random.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/map.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/system.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/rmd160.hh>
#include <paludis/util/sha1.hh>
#include <paludis/util/sha256.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/destringify.hh>
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
typedef std::tr1::unordered_multimap<std::string, std::string, Hash<std::string> > MirrorMap;
typedef std::tr1::unordered_map<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec>, Hash<QualifiedPackageName> > VirtualsMap;
typedef std::list<RepositoryEInterface::ProfilesDescLine> ProfilesDesc;
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
            Mutex profiles_desc_mutex;
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

        mutable bool has_profiles_desc;
        mutable ProfilesDesc profiles_desc;

        mutable std::tr1::shared_ptr<ERepositoryProfile> profile_ptr;

        mutable std::tr1::shared_ptr<ERepositoryNews> news_ptr;

        mutable std::tr1::shared_ptr<ERepositorySets> sets_ptr;
        mutable std::tr1::shared_ptr<ERepositoryEntries> entries_ptr;
        mutable std::tr1::shared_ptr<Layout> layout;

        mutable EAPIForFileMap eapi_for_file_map;

        Implementation(ERepository * const, const ERepositoryParams &, std::tr1::shared_ptr<Mutexes> = make_shared_ptr(new Mutexes));
        ~Implementation();

        void need_profiles() const;
        void need_profiles_desc() const;

        std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;
        std::tr1::shared_ptr<const MetadataValueKey<std::string> > layout_key;
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
        std::list<std::tr1::shared_ptr<const MetadataKey> > about_keys;
    };

    Implementation<ERepository>::Implementation(ERepository * const r,
            const ERepositoryParams & p, std::tr1::shared_ptr<Mutexes> m) :
        repo(r),
        params(p),
        mutexes(m),
        names_cache(new RepositoryNameCache(p.names_cache(), r)),
        has_repo_mask(false),
        has_mirrors(false),
        has_profiles_desc(false),
        sets_ptr(new ERepositorySets(params.environment(), r, p)),
        entries_ptr(ERepositoryEntriesFactory::get_instance()->create(params.entry_format(), params.environment(), r, p)),
        layout(LayoutFactory::get_instance()->create(params.layout(), r, params.location(), entries_ptr, get_master_locations(
                        params.master_repositories()))),
        format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                    mkt_significant, params.entry_format())),
        layout_key(new LiteralMetadataValueKey<std::string> ("layout", "layout",
                    mkt_normal, params.layout())),
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
        e_updates_location_key(layout->e_updates_location_key())
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

        profile_ptr.reset(new ERepositoryProfile(
                    params.environment(), repo, repo->name(), *params.profiles(),
                    EAPIData::get_instance()->eapi_from_string(
                        params.eapi_when_unknown())->supported()->ebuild_environment_variables()->env_arch(),
                    params.profiles_explicitly_set()));
    }

    void
    Implementation<ERepository>::need_profiles_desc() const
    {
        if (has_profiles_desc)
            return;

        Lock l(mutexes->profiles_desc_mutex);

        if (has_profiles_desc)
            return;

        Context context("When loading profiles.desc:");

        bool found_one(false);
        std::tr1::shared_ptr<const FSEntrySequence> profiles_desc_files(layout->profiles_desc_files());
        for (FSEntrySequence::ConstIterator p(profiles_desc_files->begin()), p_end(profiles_desc_files->end()) ;
                p != p_end ; ++p)
        {
            if (! p->exists())
                continue;

            found_one = true;

            LineConfigFile f(*p, LineConfigFileOptions() + lcfo_disallow_continuations);
            for (LineConfigFile::ConstIterator line(f.begin()), line_end(f.end()) ; line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                tokenise_whitespace(*line, std::back_inserter(tokens));
                if (tokens.size() < 3)
                    continue;

                std::tr1::shared_ptr<FSEntrySequence> profiles(new FSEntrySequence);
                profiles->push_back(layout->profiles_base_dir() / tokens.at(1));
                try
                {
                    profiles_desc.push_back(make_named_values<RepositoryEInterface::ProfilesDescLine>(
                            value_for<n::arch>(tokens.at(0)),
                            value_for<n::path>(*profiles->begin()),
                            value_for<n::profile>(make_shared_ptr(new RepositoryEInterfaceProfilesDescLineProfile(
                                        make_named_values<RepositoryEInterfaceProfilesDescLineProfile>(
                                            value_for<n::arch_var_if_special>(EAPIData::get_instance()->eapi_from_string(params.eapi_when_unknown())->supported()->ebuild_environment_variables()->env_arch()),
                                            value_for<n::environment>(params.environment()),
                                            value_for<n::location>(profiles),
                                            value_for<n::mutex>(make_shared_ptr(new Mutex)),
                                            value_for<n::profiles_explicitly_set>(true),
                                            value_for<n::repository>(repo),
                                            value_for<n::repository_name>(repo->name()),
                                            value_for<n::value>(make_null_shared_ptr())
                                            )))),
                            value_for<n::status>(tokens.at(2))
                            ));
                }
                catch (const InternalError &)
                {
                    throw;
                }
                catch (const Exception & e)
                {
                    Log::get_instance()->message("e.profile.failure", ll_warning, lc_context) << "Not loading profile '"
                        << tokens.at(1) << "' due to exception '" << e.message() << "' (" << e.what() << ")";
                }
            }
        }

        if (! found_one)
            throw ERepositoryConfigurationError("No profiles.desc found (maybe this repository is not synced, or maybe "
                    "you need to specify its master");

        has_profiles_desc = true;
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
                value_for<n::e_interface>(this),
                value_for<n::environment_variable_interface>(this),
                value_for<n::make_virtuals_interface>(static_cast<RepositoryMakeVirtualsInterface *>(0)),
                value_for<n::manifest_interface>(this),
                value_for<n::mirrors_interface>(this),
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
                        for (std::vector<std::string>::const_iterator e(next(ee.begin())),
                                e_end(ee.end()) ; e != e_end ; ++e)
                            _imp->mirrors.insert(std::make_pair(ee.at(0), *e));
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
        master_mtime = master_mtime_file.mtime();

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

const std::tr1::shared_ptr<const ERepositoryProfile>
ERepository::profile() const
{
    _imp->need_profiles();
    return _imp->profile_ptr;
}

const std::tr1::shared_ptr<const ERepositoryEntries>
ERepository::entries() const
{
    return _imp->entries_ptr;
}

std::string
ERepository::get_environment_variable(
        const std::tr1::shared_ptr<const PackageID> & for_package,
        const std::string & var) const
{
    Context context("When fetching environment variable '" + var + "' from repository '"
            + stringify(name()) + "':");

    _imp->need_profiles();

    return _imp->entries_ptr->get_environment_variable(std::tr1::static_pointer_cast<const ERepositoryID>(for_package),
            var, _imp->profile_ptr);
}

std::string
ERepository::profile_variable(const std::string & s) const
{
    _imp->need_profiles();

    return _imp->profile_ptr->environment_variable(s);
}

ERepository::MirrorsConstIterator
ERepository::begin_mirrors(const std::string & s) const
{
    need_mirrors();
    return MirrorsConstIterator(_imp->mirrors.equal_range(s).first);
}

ERepository::MirrorsConstIterator
ERepository::end_mirrors(const std::string & s) const
{
    need_mirrors();
    return MirrorsConstIterator(_imp->mirrors.equal_range(s).second);
}

std::tr1::shared_ptr<const ERepository::VirtualsSequence>
ERepository::virtual_packages() const
{
    Context context("When loading virtual packages for repository '" +
            stringify(name()) + "'");

    _imp->need_profiles();

    std::tr1::shared_ptr<VirtualsSequence> result(new VirtualsSequence);

    for (ERepositoryProfile::VirtualsConstIterator i(_imp->profile_ptr->begin_virtuals()),
            i_end(_imp->profile_ptr->end_virtuals()) ; i != i_end ; ++i)
        result->push_back(make_named_values<RepositoryVirtualsEntry>(
                    value_for<n::provided_by_spec>(i->second),
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

ERepository::ProfilesConstIterator
ERepository::begin_profiles() const
{
    _imp->need_profiles_desc();
    return ProfilesConstIterator(_imp->profiles_desc.begin());
}

ERepository::ProfilesConstIterator
ERepository::end_profiles() const
{
    _imp->need_profiles_desc();
    return ProfilesConstIterator(_imp->profiles_desc.end());
}

ERepository::ProfilesConstIterator
ERepository::find_profile(const FSEntry & location) const
{
    _imp->need_profiles_desc();
    for (ProfilesDesc::const_iterator i(_imp->profiles_desc.begin()),
            i_end(_imp->profiles_desc.end()) ; i != i_end ; ++i)
        if ((*i).path() == location)
            return ProfilesConstIterator(i);
    return ProfilesConstIterator(_imp->profiles_desc.end());
}

void
ERepository::set_profile(const ProfilesConstIterator & iter)
{
    Context context("When setting profile by iterator:");

    Log::get_instance()->message("e.profile.using", ll_debug, lc_context)
        << "Using profile '" << ((*iter).path()) << "'";

    _imp->profile_ptr = (*iter).profile()->fetch();

    if ((*DistributionData::get_instance()->distribution_from_string(_imp->params.environment()->distribution()))
            .support_old_style_virtuals())
        if (_imp->params.environment()->package_database()->has_repository_named(RepositoryName("virtuals")))
            _imp->params.environment()->package_database()->fetch_repository(
                    RepositoryName("virtuals"))->invalidate();

    invalidate_masks();
}

void
ERepository::set_profile_by_arch(const std::string & arch)
{
    Context context("When setting profile by arch '" + stringify(arch) + "':");

    for (ProfilesConstIterator p(begin_profiles()), p_end(end_profiles()) ; p != p_end ; ++p)
        if ((*p).arch() == stringify(arch) && (*p).status() == "stable")
        {
            set_profile(p);
            return;
        }

    for (ProfilesConstIterator p(begin_profiles()), p_end(end_profiles()) ; p != p_end ; ++p)
        if ((*p).arch() == stringify(arch))
        {
            set_profile(p);
            return;
        }

    throw ConfigurationError("Cannot find a profile appropriate for '" + stringify(arch) + "'");
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

void
ERepository::merge(const MergeParams & p)
{
    _imp->entries_ptr->merge(p);
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

std::string
ERepository::accept_keywords_variable() const
{
    return EAPIData::get_instance()->eapi_from_string(
            eapi_for_file(*_imp->profiles_key->value()->begin())
            )->supported()->ebuild_environment_variables()->env_accept_keywords();
}

std::string
ERepository::arch_variable() const
{
    return EAPIData::get_instance()->eapi_from_string(
            eapi_for_file(*_imp->profiles_key->value()->begin())
            )->supported()->ebuild_environment_variables()->env_arch();
}

void
ERepository::need_keys_added() const
{
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
    bool profiles_explicitly_set(false);
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

