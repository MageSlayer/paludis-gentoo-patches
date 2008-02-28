/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/repositories/e/e_repository_profile.hh>
#include <paludis/repositories/e/e_repository_news.hh>
#include <paludis/repositories/e/e_repository_sets.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository_entries.hh>
#include <paludis/repositories/e/package_dep_spec.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/use_desc.hh>
#include <paludis/repositories/e/layout.hh>

#ifdef ENABLE_QA
#  include <paludis/repositories/e/qa/qa_controller.hh>
#endif

#include <paludis/util/config_file.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/distribution.hh>
#include <paludis/dep_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/hook.hh>
#include <paludis/match_package.hh>
#include <paludis/query.hh>
#include <paludis/repository_name_cache.hh>
#include <paludis/syncer.hh>
#include <paludis/action.hh>
#include <paludis/mask.hh>
#include <paludis/qa.hh>

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
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/kc.hh>
#include <paludis/util/rmd160.hh>
#include <paludis/util/sha1.hh>
#include <paludis/util/sha256.hh>

#include <map>
#include <set>
#include <functional>
#include <algorithm>
#include <vector>
#include <list>
#include <fstream>

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

typedef MakeHashedMap<QualifiedPackageName,
        std::list<std::pair<tr1::shared_ptr<const PackageDepSpec>, tr1::shared_ptr<const RepositoryMaskInfo> > > >::Type RepositoryMaskMap;
typedef MakeHashedMultiMap<std::string, std::string>::Type MirrorMap;
typedef MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> >::Type VirtualsMap;
typedef std::list<RepositoryEInterface::ProfilesDescLine> ProfilesDesc;

namespace
{
    class PkgInfoSectionKey :
        public MetadataSectionKey
    {
        private:
            mutable Mutex _mutex;
            mutable bool _added;

            const Environment * const _env;
            const FSEntry _f;
            const std::string & _p;

        protected:
            virtual void need_keys_added() const
            {
                Lock l(_mutex);
                if (_added)
                    return;
                _added = true;

                // don't inherit from master_repository, just causes clutter
                std::set<std::string> info_pkgs;
                if (_f.exists())
                {
                    LineConfigFile vars(_f, LineConfigFileOptions());
                    info_pkgs.insert(vars.begin(), vars.end());
                }

                if (! info_pkgs.empty())
                {
                    for (std::set<std::string>::const_iterator i(info_pkgs.begin()),
                            i_end(info_pkgs.end()) ; i != i_end ; ++i)
                    {
                        tr1::shared_ptr<MetadataKey> key;
                        tr1::shared_ptr<const PackageIDSequence> q(
                                _env->package_database()->query(
                                    query::Matches(erepository::parse_e_package_dep_spec(*i,
                                            *erepository::EAPIData::get_instance()->eapi_from_string(_p),
                                            tr1::shared_ptr<const PackageID>())) &
                                    query::InstalledAtRoot(_env->root()),
                                    qo_order_by_version));
                        if (q->empty())
                            key.reset(new LiteralMetadataValueKey<std::string>(*i, *i, mkt_normal, "(none)"));
                        else
                        {
                            using namespace tr1::placeholders;
                            tr1::shared_ptr<Set<std::string> > s(new Set<std::string>);
                            std::transform(indirect_iterator(q->begin()), indirect_iterator(q->end()), s->inserter(),
                                    tr1::bind(tr1::mem_fn(&PackageID::canonical_form), _1, idcf_version));
                            key.reset(new LiteralMetadataStringSetKey(*i, *i, mkt_normal, s));
                        }

                        add_metadata_key(key);
                    }
                }
            }

        public:
            PkgInfoSectionKey(const Environment * const e, const FSEntry & f, const std::string & p) :
                MetadataSectionKey("info_pkgs", "Package information", mkt_normal),
                _added(false),
                _env(e),
                _f(f),
                _p(p)
            {
            }

            ~PkgInfoSectionKey()
            {
            }
    };
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
        };

        ERepository * const repo;
        const ERepositoryParams params;

        const tr1::shared_ptr<Mutexes> mutexes;

        tr1::shared_ptr<RepositoryNameCache> names_cache;

        mutable RepositoryMaskMap repo_mask;
        mutable bool has_repo_mask;

        const std::map<QualifiedPackageName, QualifiedPackageName> provide_map;

        mutable tr1::shared_ptr<UseFlagNameSet> arch_flags;

        mutable bool has_mirrors;
        mutable MirrorMap mirrors;

        mutable bool has_profiles_desc;
        mutable ProfilesDesc profiles_desc;

        mutable std::list<tr1::shared_ptr<UseDesc> > use_desc;

        mutable tr1::shared_ptr<ERepositoryProfile> profile_ptr;

        mutable tr1::shared_ptr<ERepositoryNews> news_ptr;

        mutable tr1::shared_ptr<ERepositorySets> sets_ptr;
        mutable tr1::shared_ptr<erepository::ERepositoryEntries> entries_ptr;
        mutable tr1::shared_ptr<erepository::Layout> layout;

        Implementation(ERepository * const, const ERepositoryParams &, tr1::shared_ptr<Mutexes> = make_shared_ptr(new Mutexes));
        ~Implementation();

        void need_profiles() const;
        void need_profiles_desc() const;

        tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > layout_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key;
        tr1::shared_ptr<const MetadataCollectionKey<FSEntrySequence> > profiles_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > cache_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > write_cache_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > append_repository_name_to_write_cache_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > ignore_deprecated_profiles;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > names_cache_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > distdir_key;
        tr1::shared_ptr<const MetadataCollectionKey<FSEntrySequence> > eclassdirs_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > securitydir_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > setsdir_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > newsdir_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > sync_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > sync_options_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > builddir_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > master_repository_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > eapi_when_unknown_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > eapi_when_unspecified_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > profile_eapi_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > use_manifest_key;
        tr1::shared_ptr<const MetadataSectionKey> info_pkgs_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > binary_destination_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > binary_src_uri_prefix_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > binary_keywords;
    };

    Implementation<ERepository>::Implementation(ERepository * const r,
            const ERepositoryParams & p, tr1::shared_ptr<Mutexes> m) :
        repo(r),
        params(p),
        mutexes(m),
        names_cache(new RepositoryNameCache(p.names_cache, r)),
        has_repo_mask(false),
        has_mirrors(false),
        has_profiles_desc(false),
        sets_ptr(new ERepositorySets(params.environment, r, p)),
        entries_ptr(erepository::ERepositoryEntriesMaker::get_instance()->find_maker(
                    params.entry_format)(params.environment, r, p)),
        layout(erepository::LayoutMaker::get_instance()->find_maker(
                    params.layout)(r, params.location, entries_ptr, params.master_repository ?
                        make_shared_ptr(new FSEntry(params.master_repository->params().location)) :
                        tr1::shared_ptr<FSEntry>())),
        format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                    mkt_significant, params.entry_format)),
        layout_key(new LiteralMetadataValueKey<std::string> ("layout", "layout",
                    mkt_normal, params.layout)),
        location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                    mkt_significant, params.location)),
        profiles_key(new LiteralMetadataFSEntrySequenceKey(
                    "profiles", "profiles", mkt_normal, params.profiles)),
        cache_key(new LiteralMetadataValueKey<FSEntry> ("cache", "cache",
                    mkt_normal, params.cache)),
        write_cache_key(new LiteralMetadataValueKey<FSEntry> ("write_cache", "write_cache",
                    mkt_normal, params.write_cache)),
        append_repository_name_to_write_cache_key(new LiteralMetadataValueKey<std::string> (
                    "append_repository_name_to_write_cache", "append_repository_name_to_write_cache",
                    mkt_normal, stringify(params.append_repository_name_to_write_cache))),
        ignore_deprecated_profiles(new LiteralMetadataValueKey<std::string> (
                    "ignore_deprecated_profiles", "ignore_deprecated_profiles",
                    mkt_normal, stringify(params.ignore_deprecated_profiles))),
        names_cache_key(new LiteralMetadataValueKey<FSEntry> (
                    "names_cache", "names_cache", mkt_normal, params.names_cache)),
        distdir_key(new LiteralMetadataValueKey<FSEntry> (
                    "distdir", "distdir", mkt_normal, params.distdir)),
        eclassdirs_key(new LiteralMetadataFSEntrySequenceKey(
                    "eclassdirs", "eclassdirs", mkt_normal, params.eclassdirs)),
        securitydir_key(new LiteralMetadataValueKey<FSEntry> (
                    "securitydir", "securitydir", mkt_normal, params.securitydir)),
        setsdir_key(new LiteralMetadataValueKey<FSEntry> (
                    "setsdir", "setsdir", mkt_normal, params.setsdir)),
        newsdir_key(new LiteralMetadataValueKey<FSEntry> (
                    "newsdir", "newsdir", mkt_normal, params.newsdir)),
        sync_key(new LiteralMetadataValueKey<std::string> (
                    "sync", "sync", mkt_normal, params.sync)),
        sync_options_key(new LiteralMetadataValueKey<std::string> (
                    "sync_options", "sync_options", mkt_normal, params.sync_options)),
        builddir_key(new LiteralMetadataValueKey<FSEntry> (
                    "builddir", "builddir", mkt_normal, params.builddir)),
        master_repository_key(params.master_repository ?
                tr1::shared_ptr<MetadataValueKey<std::string> >(new LiteralMetadataValueKey<std::string> (
                        "master_repository", "master_repository", mkt_normal, stringify(params.master_repository->name()))) :
                tr1::shared_ptr<MetadataValueKey<std::string> >()),
        eapi_when_unknown_key(new LiteralMetadataValueKey<std::string> (
                    "eapi_when_unknown", "eapi_when_unknown", mkt_normal, params.eapi_when_unknown)),
        eapi_when_unspecified_key(new LiteralMetadataValueKey<std::string> (
                    "eapi_when_unspecified", "eapi_when_unspecified", mkt_normal, params.eapi_when_unspecified)),
        profile_eapi_key(new LiteralMetadataValueKey<std::string> (
                    "profile_eapi", "profile_eapi", mkt_normal, params.profile_eapi)),
        use_manifest_key(new LiteralMetadataValueKey<std::string> (
                    "use_manifest", "use_manifest", mkt_normal, stringify(params.use_manifest))),
        info_pkgs_key((layout->info_packages_file(params.location / "profiles")).exists() ?
                tr1::shared_ptr<MetadataSectionKey>(new PkgInfoSectionKey(
                        params.environment, layout->info_packages_file(params.location / "profiles"), params.profile_eapi)) :
                tr1::shared_ptr<MetadataSectionKey>()),
        binary_destination_key(new LiteralMetadataValueKey<std::string> (
                    "binary_destination", "binary_destination", mkt_normal, stringify(params.binary_destination))),
        binary_src_uri_prefix_key(new LiteralMetadataValueKey<std::string> (
                    "binary_uri_prefix", "binary_uri_prefix", mkt_normal, params.binary_uri_prefix)),
        binary_keywords(new LiteralMetadataValueKey<std::string> (
                    "binary_keywords", "binary_keywords", mkt_normal, params.binary_keywords))
    {
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
                    params.environment, repo, repo->name(), *params.profiles,
                    (*(*erepository::EAPIData::get_instance()->eapi_from_string(
                        params.eapi_when_unknown))[k::supported()])[k::ebuild_environment_variables()][k::env_arch()]));
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
        tr1::shared_ptr<const FSEntrySequence> profiles_desc_files(layout->profiles_desc_files());
        for (FSEntrySequence::ConstIterator p(profiles_desc_files->begin()), p_end(profiles_desc_files->end()) ;
                p != p_end ; ++p)
        {
            if (! p->exists())
                continue;

            found_one = true;

            LineConfigFile f(*p, LineConfigFileOptions());
            for (LineConfigFile::ConstIterator line(f.begin()), line_end(f.end()) ; line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                tokenise_whitespace(*line, std::back_inserter(tokens));
                if (tokens.size() < 3)
                    continue;

                FSEntrySequence profiles;
                profiles.push_back(layout->profiles_base_dir() / tokens.at(1));
                profiles_desc.push_back(RepositoryEInterface::ProfilesDescLine::named_create()
                        (k::arch(), tokens.at(0))
                        (k::path(), *profiles.begin())
                        (k::status(), tokens.at(2))
                        (k::profile(), tr1::shared_ptr<ERepositoryProfile>(new ERepositoryProfile(
                                    params.environment, repo, repo->name(), profiles,
                                    (*(*erepository::EAPIData::get_instance()->eapi_from_string(
                                        params.eapi_when_unknown))[k::supported()])[k::ebuild_environment_variables()][k::env_arch()]))));
            }
        }

        if (! found_one)
            throw ERepositoryConfigurationError("No profiles.desc found");

        has_profiles_desc = true;
    }
}

namespace
{
    RepositoryName
    fetch_repo_name(const FSEntry & tree_root)
    {
        try
        {
            do
            {
                FSEntry name_file(tree_root);
                name_file /= "profiles";
                name_file /= "repo_name";

                if (! name_file.is_regular_file())
                    break;

                LineConfigFile f(name_file, LineConfigFileOptions());
                if (f.begin() == f.end())
                    break;
                return RepositoryName(*f.begin());

            } while (false);
        }
        catch (...)
        {
        }

        std::string modified_location(tree_root.basename());
        std::replace(modified_location.begin(), modified_location.end(), '/', '-');

        Log::get_instance()->message(ll_qa, lc_no_context, "Couldn't open repo_name file in '"
                + stringify(tree_root) + "/profiles/', falling back to generated name 'x-" + modified_location +
                "' (ignore this message if you have yet to sync this repository).");

        return RepositoryName("x-" + modified_location);
    }
}

ERepository::ERepository(const ERepositoryParams & p) :
    Repository(fetch_repo_name(p.location),
            RepositoryCapabilities::named_create()
            (k::sets_interface(), this)
            (k::syncable_interface(), this)
            (k::use_interface(), this)
            (k::world_interface(), static_cast<RepositoryWorldInterface *>(0))
            (k::environment_variable_interface(), this)
            (k::mirrors_interface(), this)
            (k::virtuals_interface(), (*DistributionData::get_instance()->distribution_from_string(
                    p.environment->default_distribution()))[k::support_old_style_virtuals()] ? this : 0)
            (k::provides_interface(), static_cast<RepositoryProvidesInterface *>(0))
            (k::destination_interface(), p.binary_destination ? this : 0)
            (k::make_virtuals_interface(), static_cast<RepositoryMakeVirtualsInterface *>(0))
            (k::e_interface(), this)
#ifdef ENABLE_QA
            (k::qa_interface(), this)
#else
            (k::qa_interface(), static_cast<RepositoryQAInterface *>(0))
#endif
            (k::hook_interface(), this)
            (k::manifest_interface(), this)),
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
    add_metadata_key(_imp->profile_eapi_key);
    if (_imp->master_repository_key)
        add_metadata_key(_imp->master_repository_key);
    add_metadata_key(_imp->use_manifest_key);
    if (_imp->info_pkgs_key)
        add_metadata_key(_imp->info_pkgs_key);
    add_metadata_key(_imp->binary_destination_key);
    add_metadata_key(_imp->binary_src_uri_prefix_key);
    add_metadata_key(_imp->binary_keywords);
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

tr1::shared_ptr<const CategoryNamePartSet>
ERepository::category_names() const
{
    return _imp->layout->category_names();
}

tr1::shared_ptr<const QualifiedPackageNameSet>
ERepository::package_names(const CategoryNamePart & c) const
{
    return _imp->layout->package_names(c);
}

tr1::shared_ptr<const PackageIDSequence>
ERepository::package_ids(const QualifiedPackageName & n) const
{
    return _imp->layout->package_ids(n);
}

tr1::shared_ptr<const RepositoryMaskInfo>
ERepository::repository_masked(const PackageID & id) const
{
    Lock l(_imp->mutexes->repo_mask_mutex);

    if (! _imp->has_repo_mask)
    {
        Context context("When querying repository mask for '" + stringify(id) + "':");

        tr1::shared_ptr<const FSEntrySequence> repository_mask_files(_imp->layout->repository_mask_files());
        for (FSEntrySequence::ConstIterator p(repository_mask_files->begin()), p_end(repository_mask_files->end()) ;
                p != p_end ; ++p)
        {
            Context context_local("When reading '" + stringify(*p) + "':");

            if (p->exists())
            {
                erepository::MaskFile ff(*p, LineConfigFileOptions());
                for (erepository::MaskFile::ConstIterator line(ff.begin()), line_end(ff.end()) ;
                        line != line_end ; ++line)
                {
                    try
                    {
                        tr1::shared_ptr<const PackageDepSpec> a(new PackageDepSpec(erepository::parse_e_package_dep_spec(
                                        line->first,
                                        *erepository::EAPIData::get_instance()->eapi_from_string(_imp->params.profile_eapi),
                                        tr1::shared_ptr<const PackageID>())));
                        if (a->package_ptr())
                            _imp->repo_mask[*a->package_ptr()].push_back(std::make_pair(a, line->second));
                        else
                            Log::get_instance()->message(ll_warning, lc_context, "Loading package mask spec '"
                                    + stringify(line->first) + "' failed because specification does not restrict to a "
                                    "unique package");
                    }
                    catch (const Exception & e)
                    {
                        Log::get_instance()->message(ll_warning, lc_context, "Loading package mask spec '"
                                + stringify(line->first) + "' failed due to exception '" + e.message() + "' ("
                                + e.what() + ")");
                    }
                }
            }
        }

        _imp->has_repo_mask = true;
    }

    RepositoryMaskMap::iterator r(_imp->repo_mask.find(id.name()));
    if (_imp->repo_mask.end() == r)
        return tr1::shared_ptr<const RepositoryMaskInfo>();
    else
        for (std::list<std::pair<tr1::shared_ptr<const PackageDepSpec>, tr1::shared_ptr<const RepositoryMaskInfo> > >::const_iterator
                k(r->second.begin()), k_end(r->second.end()) ; k != k_end ; ++k)
            if (match_package(*_imp->params.environment, *k->first, id))
                return k->second;

    return tr1::shared_ptr<const RepositoryMaskInfo>();
}

UseFlagState
ERepository::query_use(const UseFlagName & f, const PackageID & e) const
{
    if (this != e.repository().get())
        return use_unspecified;

    const erepository::ERepositoryID & id(static_cast<const erepository::ERepositoryID &>(e));
    if (id.use_key())
    {
        if (id.use_key()->value()->end() != id.use_key()->value()->find(f))
            return use_enabled;
        else
            return use_disabled;
    }
    else
    {
        _imp->need_profiles();
        if (query_use_mask(f, e))
            return use_disabled;
        else if (query_use_force(f, e))
            return use_enabled;
        else
            return _imp->profile_ptr->use_state_ignoring_masks(f, e);
    }
}

bool
ERepository::query_use_mask(const UseFlagName & u, const PackageID & e) const
{
    if (this != e.repository().get())
        return use_unspecified;

    const erepository::ERepositoryID & id(static_cast<const erepository::ERepositoryID &>(e));
    if (id.use_key())
    {
        return (id.use_key()->value()->end() == id.use_key()->value()->find(u));
    }
    else
    {
        _imp->need_profiles();
        return _imp->profile_ptr->use_masked(u, e) ||
            (arch_flags()->end() != arch_flags()->find(u) &&
             use_enabled != _imp->profile_ptr->use_state_ignoring_masks(u, e));
    }
}

bool
ERepository::query_use_force(const UseFlagName & u, const PackageID & e) const
{
    if (this != e.repository().get())
        return use_unspecified;

    const erepository::ERepositoryID & id(static_cast<const erepository::ERepositoryID &>(e));
    if (id.use_key())
    {
        return (id.use_key()->value()->end() != id.use_key()->value()->find(u));
    }
    else
    {
        _imp->need_profiles();
        return _imp->profile_ptr->use_forced(u, e);
    }
}

tr1::shared_ptr<const UseFlagNameSet>
ERepository::arch_flags() const
{
    Lock l(_imp->mutexes->arch_flags_mutex);
    if (! _imp->arch_flags)
    {
        Context context("When loading arch list:");
        _imp->arch_flags.reset(new UseFlagNameSet);

        bool found_one(false);
        tr1::shared_ptr<const FSEntrySequence> arch_list_files(_imp->layout->arch_list_files());
        for (FSEntrySequence::ConstIterator p(arch_list_files->begin()), p_end(arch_list_files->end()) ;
                p != p_end ; ++p)
        {
            if (! p->exists())
                continue;

            LineConfigFile archs(*p, LineConfigFileOptions());
            std::copy(archs.begin(), archs.end(), create_inserter<UseFlagName>(_imp->arch_flags->inserter()));
            found_one = true;
        }

        if (! found_one)
        {
            Log::get_instance()->message(ll_qa, lc_no_context, "Couldn't find arch.list file for repository '"
                    + stringify(name()) + "', arch flags may incorrectly show up as unmasked");
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
        tr1::shared_ptr<const FSEntrySequence> mirror_files(_imp->layout->mirror_files());
        for (FSEntrySequence::ConstIterator p(mirror_files->begin()), p_end(mirror_files->end()) ;
                p != p_end ; ++p)
        {
            if (p->exists())
            {
                LineConfigFile mirrors(*p, LineConfigFileOptions());
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
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "No thirdpartymirrors file found in '"
                    + stringify(_imp->params.location / "profiles") + "', so mirror:// SRC_URI "
                    "components cannot be fetched");

        _imp->has_mirrors = true;
    }
}

tr1::shared_ptr<SetSpecTree::ConstItem>
ERepository::package_set(const SetName & s) const
{
    if (s.data() == "system")
    {
        _imp->need_profiles();
        return _imp->profile_ptr->system_packages();
    }

    return _imp->sets_ptr->package_set(s);
}

tr1::shared_ptr<const SetNameSet>
ERepository::sets_list() const
{
    return _imp->sets_ptr->sets_list();
}

bool
ERepository::sync() const
{
    Context context("When syncing repository '" + stringify(name()) + "':");

    if (_imp->params.sync.empty())
        return false;

    std::list<std::string> sync_list;
    tokenise_whitespace(_imp->params.sync, std::back_inserter(sync_list));

    bool ok(false);
    for (std::list<std::string>::const_iterator s(sync_list.begin()),
            s_end(sync_list.end()) ; s != s_end ; ++s)
    {
        DefaultSyncer syncer(SyncerParams::named_create()
                (k::environment(), _imp->params.environment)
                (k::local(), stringify(_imp->params.location))
                (k::remote(), *s)
                );
        SyncOptions opts(
                _imp->params.sync_options,
                _imp->layout->sync_filter_file(),
                "sync " + stringify(name()) + "> "
                );
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
        throw SyncFailedError(stringify(_imp->params.location), _imp->params.sync);

    return true;
}

void
ERepository::invalidate()
{
    _imp.reset(new Implementation<ERepository>(this, _imp->params, _imp->mutexes));
    _add_metadata_keys();
}

void
ERepository::invalidate_masks()
{
    _imp->layout->invalidate_masks();

    if ((*DistributionData::get_instance()->distribution_from_string(_imp->params.environment->default_distribution()))
            [k::support_old_style_virtuals()])
        if (_imp->params.environment->package_database()->has_repository_named(RepositoryName("virtuals")))
            _imp->params.environment->package_database()->fetch_repository(
                    RepositoryName("virtuals"))->invalidate_masks();
}

void
ERepository::update_news() const
{
    Lock l(_imp->mutexes->news_ptr_mutex);

    if (! _imp->news_ptr)
        _imp->news_ptr.reset(new ERepositoryNews(_imp->params.environment, this, _imp->params));

    _imp->news_ptr->update_news();
}

const tr1::shared_ptr<const erepository::Layout>
ERepository::layout() const
{
    return _imp->layout;
}

const tr1::shared_ptr<const ERepositoryProfile>
ERepository::profile() const
{
    _imp->need_profiles();
    return _imp->profile_ptr;
}

const tr1::shared_ptr<const erepository::ERepositoryEntries>
ERepository::entries() const
{
    return _imp->entries_ptr;
}

std::string
ERepository::get_environment_variable(
        const tr1::shared_ptr<const PackageID> & for_package,
        const std::string & var) const
{
    Context context("When fetching environment variable '" + var + "' from repository '"
            + stringify(name()) + "':");

    _imp->need_profiles();

    return _imp->entries_ptr->get_environment_variable(tr1::static_pointer_cast<const erepository::ERepositoryID>(for_package),
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

tr1::shared_ptr<const ERepository::VirtualsSequence>
ERepository::virtual_packages() const
{
    Context context("When loading virtual packages for repository '" +
            stringify(name()) + "'");

    _imp->need_profiles();

    tr1::shared_ptr<VirtualsSequence> result(new VirtualsSequence);

    for (ERepositoryProfile::VirtualsConstIterator i(_imp->profile_ptr->begin_virtuals()),
            i_end(_imp->profile_ptr->end_virtuals()) ; i != i_end ; ++i)
        result->push_back(RepositoryVirtualsEntry::named_create()
                (k::provided_by_spec(), i->second)
                (k::virtual_name(), i->first));

    return result;
}

tr1::shared_ptr<const UseFlagNameSet>
ERepository::use_expand_flags() const
{
    _imp->need_profiles();

    std::string expand_sep(stringify((*(*erepository::EAPIData::get_instance()->eapi_from_string(
                            (_imp->params.profile_eapi)))[k::supported()])[k::ebuild_options()].use_expand_separator));
    tr1::shared_ptr<UseFlagNameSet> result(new UseFlagNameSet);
    for (ERepositoryProfile::UseExpandConstIterator i(_imp->profile_ptr->begin_use_expand()),
            i_end(_imp->profile_ptr->end_use_expand()) ; i != i_end ; ++i)
    {
        std::list<std::string> values;
        tokenise_whitespace(_imp->profile_ptr->environment_variable(
                    stringify(*i)), std::back_inserter(values));
        for (std::list<std::string>::const_iterator j(values.begin()), j_end(values.end()) ;
                j != j_end ; ++j)
        {
            std::string f(stringify(*i) + expand_sep + *j), lower_f;
            std::transform(f.begin(), f.end(), std::back_inserter(lower_f), &::tolower);
            result->insert(UseFlagName(lower_f));
        }
    }

    return result;
}

tr1::shared_ptr<const UseFlagNameSet>
ERepository::use_expand_prefixes() const
{
    _imp->need_profiles();

    tr1::shared_ptr<UseFlagNameSet> result(new UseFlagNameSet);
    for (ERepositoryProfile::UseExpandConstIterator i(_imp->profile_ptr->begin_use_expand()),
            i_end(_imp->profile_ptr->end_use_expand()) ; i != i_end ; ++i)
    {
        std::string lower_i;
        std::transform(i->data().begin(), i->data().end(), std::back_inserter(lower_i), &::tolower);
        result->insert(UseFlagName(lower_i));
    }

    return result;
}

tr1::shared_ptr<const UseFlagNameSet>
ERepository::use_expand_hidden_prefixes() const
{
    _imp->need_profiles();

    tr1::shared_ptr<UseFlagNameSet> result(new UseFlagNameSet);
    for (ERepositoryProfile::UseExpandConstIterator i(_imp->profile_ptr->begin_use_expand_hidden()),
            i_end(_imp->profile_ptr->end_use_expand_hidden()) ; i != i_end ; ++i)
    {
        std::string lower_i;
        std::transform(i->data().begin(), i->data().end(), std::back_inserter(lower_i), &::tolower);
        result->insert(UseFlagName(lower_i));
    }

    return result;
}

char
ERepository::use_expand_separator(const PackageID & id) const
{
    if (this != id.repository().get())
        return '\0';
    const tr1::shared_ptr<const erepository::EAPI> & eapi(static_cast<const erepository::ERepositoryID &>(id).eapi());
    return (*eapi)[k::supported()] ? (*(*eapi)[k::supported()])[k::ebuild_options()].use_expand_separator : '\0';
}

void
ERepository::regenerate_cache() const
{
    _imp->names_cache->regenerate_cache();
}

tr1::shared_ptr<const CategoryNamePartSet>
ERepository::category_names_containing_package(const PackageNamePart & p) const
{
    if (! _imp->names_cache->usable())
        return Repository::category_names_containing_package(p);

    tr1::shared_ptr<const CategoryNamePartSet> result(
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
        if ((*i)[k::path()] == location)
            return ProfilesConstIterator(i);
    return ProfilesConstIterator(_imp->profiles_desc.end());
}

void
ERepository::set_profile(const ProfilesConstIterator & iter)
{
    Context context("When setting profile by iterator:");

    _imp->profile_ptr = (*iter)[k::profile()];

    if ((*DistributionData::get_instance()->distribution_from_string(_imp->params.environment->default_distribution()))
            [k::support_old_style_virtuals()])
        if (_imp->params.environment->package_database()->has_repository_named(RepositoryName("virtuals")))
            _imp->params.environment->package_database()->fetch_repository(
                    RepositoryName("virtuals"))->invalidate();

    invalidate_masks();
}

void
ERepository::set_profile_by_arch(const UseFlagName & arch)
{
    Context context("When setting profile by arch '" + stringify(arch) + "':");

    for (ProfilesConstIterator p(begin_profiles()), p_end(end_profiles()) ; p != p_end ; ++p)
        if ((*p)[k::arch()] == stringify(arch) && (*p)[k::status()] == "stable")
        {
            set_profile(p);
            return;
        }

    for (ProfilesConstIterator p(begin_profiles()), p_end(end_profiles()) ; p != p_end ; ++p)
        if ((*p)[k::arch()] == stringify(arch))
        {
            set_profile(p);
            return;
        }

    throw ConfigurationError("Cannot find a profile appropriate for '" + stringify(arch) + "'");
}

std::string
ERepository::describe_use_flag(const UseFlagName & f,
        const PackageID & e) const
{
    Lock l(_imp->mutexes->use_desc_mutex);

    if (_imp->use_desc.empty())
    {
        std::string expand_sep(stringify((*(*erepository::EAPIData::get_instance()->eapi_from_string(
                                _imp->params.profile_eapi))[k::supported()])[k::ebuild_options()].use_expand_separator));
        tr1::shared_ptr<const FSEntrySequence> use_desc_dirs(_imp->layout->use_desc_dirs());
        for (FSEntrySequence::ConstIterator p(use_desc_dirs->begin()), p_end(use_desc_dirs->end()) ;
                p != p_end ; ++p)
            _imp->use_desc.push_back(tr1::shared_ptr<UseDesc>(new UseDesc(*p, expand_sep)));
    }

    std::string result;
    for (std::list<tr1::shared_ptr<UseDesc> >::const_iterator i(_imp->use_desc.begin()),
            i_end(_imp->use_desc.end()) ; i != i_end ; ++i)
    {
        std::string new_result((*i)->describe(f, e));
        if (! new_result.empty())
            result = new_result;
    }
    return result;
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
        return (*(*static_cast<const erepository::ERepositoryID &>(e).eapi())[k::supported()])[k::can_be_pbin()];
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
ERepository::perform_hook(const Hook & hook) const
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    if (hook.name() == "sync_all_post"
            || hook.name() == "install_all_post"
            || hook.name() == "uninstall_all_post")
        update_news();

    return HookResult(0, "");
}

tr1::shared_ptr<const CategoryNamePartSet>
ERepository::unimportant_category_names() const
{
    tr1::shared_ptr<CategoryNamePartSet> result(make_shared_ptr(new CategoryNamePartSet));
    result->insert(CategoryNamePart("virtual"));
    return result;
}

#ifdef ENABLE_QA
namespace
{
    struct LibQAHandle
    {
        Mutex mutex;
        void * handle;
        void (* qa_checks_handle)(
                const Environment * const,
                const tr1::shared_ptr<const ERepository> &,
                const QACheckProperties & ignore_if,
                const QACheckProperties & ignore_unless,
                const QAMessageLevel minimum_level,
                QAReporter & reporter,
                const FSEntry & dir);

        LibQAHandle() :
            handle(0),
            qa_checks_handle(0)
        {
        }

        ~LibQAHandle()
        {
            if (0 != handle)
                dlclose(handle);
        }

    } libqahandle;
}
#endif

void
ERepository::check_qa(
        QAReporter & reporter,
        const QACheckProperties & ignore_if,
        const QACheckProperties & ignore_unless,
        const QAMessageLevel minimum_level,
        const FSEntry & dir
        ) const
{
#ifdef ENABLE_QA
    Context c("When performing QA checks for '" + stringify(dir) + "':");

    {
        Lock lock(libqahandle.mutex);

        if (0 == libqahandle.handle)
            libqahandle.handle = dlopen(getenv_with_default("PALUDIS_E_REPOSITORY_QA_SO",
                        "libpaludiserepositoryqa.so").c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (0 == libqahandle.handle)
        {
            reporter.message(QAMessage(dir, qaml_severe, "check_qa", "Got error '" + stringify(dlerror()) +
                        "' when dlopen(" + getenv_with_default("PALUDIS_E_REPOSITORY_QA_SO",
                                "libpaludiserepositoryqa.so") + ")"));
            return;
        }

        if (0 == libqahandle.qa_checks_handle)
            libqahandle.qa_checks_handle = STUPID_CAST(void (*)(
                        const Environment * const,
                        const tr1::shared_ptr<const ERepository> &,
                        const QACheckProperties &,
                        const QACheckProperties &,
                        const QAMessageLevel,
                        QAReporter &,
                        const FSEntry &),
                    dlsym(libqahandle.handle, "check_qa"));
        if (0 == libqahandle.qa_checks_handle)
        {
            reporter.message(QAMessage(dir, qaml_severe, "check_qa", "Got error '" + stringify(dlerror) +
                        "' when dlsym(libpaludisqa.so, \"check_qa\")"));
            return;
        }
    }

    (*libqahandle.qa_checks_handle)(_imp->params.environment, shared_from_this(), ignore_if, ignore_unless,
            minimum_level, reporter, dir);
#endif
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

        void visit(const SupportsActionTest<FetchAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<UninstallAction> &)
        {
        }

        void visit(const SupportsActionTest<InfoAction> &)
        {
            result = true;
        }
    };
}

bool
ERepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

void
ERepository::make_manifest(const QualifiedPackageName & qpn)
{
    FSEntry package_dir = _imp->layout->package_directory(qpn);

    FSEntry(package_dir / "Manifest").unlink();
    std::ofstream manifest(stringify(FSEntry(package_dir 
                    / "Manifest")).c_str());
    if (! manifest)
        throw ERepositoryConfigurationError("Couldn't open Manifest for writing.");

    tr1::shared_ptr<Map<FSEntry, std::string> > files = _imp->layout->manifest_files(qpn);

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

        std::ifstream file_stream(stringify(file).c_str());
        if (! file_stream)
            throw ERepositoryConfigurationError("Couldn't read " 
                    + stringify(file));

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

    tr1::shared_ptr<const PackageIDSequence> versions;
    versions = package_ids(qpn);

    std::set<std::string> done_files;

    for (PackageIDSequence::ConstIterator v(versions->begin()),
            v_end(versions->end()) ;
            v != v_end ; ++v)
    {
        tr1::shared_ptr<const PackageID> id = (*v);
        if (! id->fetches_key())
            continue;
        paludis::erepository::AAVisitor aa;
        id->fetches_key()->value()->accept(aa);

        for (paludis::erepository::AAVisitor::ConstIterator d(aa.begin()) ;
                d != aa.end() ; ++d)
        {
            if (done_files.count(*d))
                continue;
            done_files.insert(*d);

            FSEntry f(params().distdir / *d);

            std::ifstream file_stream(stringify(f).c_str());
            if (! file_stream)
                throw ERepositoryConfigurationError("Couldn't read "
                        + stringify(f));

            RMD160 rmd160sum(file_stream);
            manifest << "DIST " << f.basename() << " "
                << f.file_size()
                << " RMD160 " << rmd160sum.hexsum();

            file_stream.clear();
            file_stream.seekg(0, std::ios::beg);
            SHA1 sha1sum(file_stream);
            manifest << " SHA1 " << sha1sum.hexsum();

            file_stream.clear();
            file_stream.seekg(0, std::ios::beg);
            SHA256 sha256sum(file_stream);
            manifest << " SHA256 " << sha256sum.hexsum()
                << std::endl;
        }
    }
}

std::string
ERepository::accept_keywords_variable() const
{
    return (*(*erepository::EAPIData::get_instance()->eapi_from_string(params().profile_eapi))
            [k::supported()])[k::ebuild_environment_variables()][k::env_accept_keywords()];
}

std::string
ERepository::arch_variable() const
{
    return (*(*erepository::EAPIData::get_instance()->eapi_from_string(params().profile_eapi))
            [k::supported()])[k::ebuild_environment_variables()][k::env_arch()];
}

FSEntry
ERepository::info_variables_file(const FSEntry & f) const
{
    return layout()->info_variables_file(f);
}

void
ERepository::need_keys_added() const
{
}

const tr1::shared_ptr<const MetadataValueKey<std::string> >
ERepository::format_key() const
{
    return _imp->format_key;
}

const tr1::shared_ptr<const MetadataValueKey<FSEntry> >
ERepository::installed_root_key() const
{
    return tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

