/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_profile.hh>
#include <paludis/repositories/e/e_repository_news.hh>
#include <paludis/repositories/e/e_repository_sets.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository_entries.hh>
#include <paludis/repositories/e/use_desc.hh>
#include <paludis/repositories/e/layout.hh>

#ifdef ENABLE_QA
#  include <paludis/repositories/e/qa/qa_controller.hh>
#endif

#include <paludis/repository_info.hh>
#include <paludis/config_file.hh>
#include <paludis/distribution.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/hook.hh>
#include <paludis/match_package.hh>
#include <paludis/query.hh>
#include <paludis/repository_name_cache.hh>
#include <paludis/syncer.hh>
#include <paludis/eapi.hh>
#include <paludis/action.hh>

#include <paludis/util/fs_entry.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/random.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/visitor-impl.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include <map>
#include <set>
#include <functional>
#include <algorithm>
#include <vector>
#include <list>

#include <strings.h>
#include <ctype.h>

/** \file
 * Implementation of ERepository.
 *
 * \ingroup grperepository
 */

using namespace paludis;

typedef MakeHashedMap<QualifiedPackageName, std::list<tr1::shared_ptr<const PackageDepSpec> > >::Type RepositoryMaskMap;
typedef MakeHashedMultiMap<std::string, std::string>::Type MirrorMap;
typedef MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> >::Type VirtualsMap;
typedef std::list<RepositoryEInterface::ProfilesDescLine> ProfilesDesc;

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
        ERepository * const repo;
        const ERepositoryParams params;

        tr1::shared_ptr<RepositoryNameCache> names_cache;

        mutable RepositoryMaskMap repo_mask;
        mutable bool has_repo_mask;

        mutable VirtualsMap our_virtuals;
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
        mutable tr1::shared_ptr<ERepositoryEntries> entries_ptr;
        mutable tr1::shared_ptr<Layout> layout;

        Implementation(ERepository * const, const ERepositoryParams &);
        ~Implementation();

        void need_profiles() const;
        void need_profiles_desc() const;
    };

    Implementation<ERepository>::Implementation(ERepository * const r,
            const ERepositoryParams & p) :
        repo(r),
        params(p),
        names_cache(new RepositoryNameCache(p.names_cache, r)),
        has_repo_mask(false),
        has_mirrors(false),
        has_profiles_desc(false),
        sets_ptr(new ERepositorySets(params.environment, r, p)),
        entries_ptr(ERepositoryEntriesMaker::get_instance()->find_maker(
                    params.entry_format)(params.environment, r, p)),
        layout(LayoutMaker::get_instance()->find_maker(
                    params.layout)(r, params.location, entries_ptr, params.master_repository ?
                        make_shared_ptr(new FSEntry(params.master_repository->params().location)) :
                        tr1::shared_ptr<FSEntry>()))
    {
    }

    Implementation<ERepository>::~Implementation()
    {
    }

    void
    Implementation<ERepository>::need_profiles() const
    {
        if (profile_ptr)
            return;

        profile_ptr.reset(new ERepositoryProfile(
                    params.environment, repo, repo->name(), *params.profiles,
                    EAPIData::get_instance()->eapi_from_string(params.eapi_when_unknown)->supported->ebuild_environment_variables->env_arch));
    }

    void
    Implementation<ERepository>::need_profiles_desc() const
    {
        if (has_profiles_desc)
            return;

        Context context("When loading profiles.desc:");

        bool found_one(false);
        tr1::shared_ptr<const FSEntrySequence> profiles_desc_files(layout->profiles_desc_files());
        for (FSEntrySequence::Iterator p(profiles_desc_files->begin()), p_end(profiles_desc_files->end()) ;
                p != p_end ; ++p)
        {
            if (! p->exists())
                continue;

            found_one = true;

            LineConfigFile f(*p, LineConfigFileOptions());
            for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ; line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                WhitespaceTokeniser::get_instance()->tokenise(*line,
                        std::back_inserter(tokens));
                if (tokens.size() < 3)
                    continue;

                FSEntrySequence profiles;
                profiles.push_back(layout->profiles_base_dir() / tokens.at(1));
                profiles_desc.push_back(RepositoryEInterface::ProfilesDescLine::create()
                        .arch(tokens.at(0))
                        .path(*profiles.begin())
                        .status(tokens.at(2))
                        .profile(tr1::shared_ptr<ERepositoryProfile>(new ERepositoryProfile(
                                    params.environment, repo, repo->name(), profiles,
                                    EAPIData::get_instance()->eapi_from_string(
                                        params.eapi_when_unknown)->supported->ebuild_environment_variables->env_arch))));
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
            RepositoryCapabilities::create()
            .installed_interface(0)
            .sets_interface(this)
            .syncable_interface(this)
            .use_interface(this)
            .world_interface(0)
            .environment_variable_interface(this)
            .mirrors_interface(this)
            .virtuals_interface(DistributionData::get_instance()->distribution_from_string(
                    p.environment->default_distribution())->support_old_style_virtuals ? this : 0)
            .provides_interface(0)
            .destination_interface(p.enable_destinations ? this : 0)
            .licenses_interface(this)
            .make_virtuals_interface(0)
            .e_interface(this)
#ifdef ENABLE_QA
            .qa_interface(this)
#else
            .qa_interface(0)
#endif
            .hook_interface(this),
            p.entry_format),
    PrivateImplementationPattern<ERepository>(new Implementation<ERepository>(this, p))
{
    // the info_vars and info_pkgs info is only added on demand, since it's
    // fairly slow to calculate.
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("entry_format", _imp->params.entry_format);
    config_info->add_kv("layout", _imp->params.layout);
    config_info->add_kv("location", stringify(_imp->params.location));
    config_info->add_kv("profiles", join(_imp->params.profiles->begin(),
                _imp->params.profiles->end(), " "));
    config_info->add_kv("cache", stringify(_imp->params.cache));
    config_info->add_kv("write_cache", stringify(_imp->params.write_cache));
    config_info->add_kv("names_cache", stringify(_imp->params.names_cache));
    config_info->add_kv("distdir", stringify(_imp->params.distdir));
    config_info->add_kv("eclassdirs", join(_imp->params.eclassdirs->begin(),
                _imp->params.eclassdirs->end(), " "));
    config_info->add_kv("securitydir", stringify(_imp->params.securitydir));
    config_info->add_kv("setsdir", stringify(_imp->params.setsdir));
    config_info->add_kv("newsdir", stringify(_imp->params.newsdir));
    config_info->add_kv("sync", _imp->params.sync);
    config_info->add_kv("sync_options", _imp->params.sync_options);
    config_info->add_kv("buildroot", stringify(_imp->params.buildroot));
    if (_imp->params.master_repository)
        config_info->add_kv("master_repository", stringify(_imp->params.master_repository->name()));
    config_info->add_kv("format", _imp->params.entry_format);
    config_info->add_kv("layout", _imp->params.layout);
    config_info->add_kv("eapi_when_unknown", _imp->params.eapi_when_unknown);
    config_info->add_kv("eapi_when_unspecified", _imp->params.eapi_when_unspecified);
    config_info->add_kv("profile_eapi", _imp->params.profile_eapi);

    _info->add_section(config_info);
}

ERepository::~ERepository()
{
}

bool
ERepository::do_has_category_named(const CategoryNamePart & c) const
{
    return _imp->layout->has_category_named(c);
}

bool
ERepository::do_has_package_named(const QualifiedPackageName & q) const
{
    return _imp->layout->has_package_named(q);
}

tr1::shared_ptr<const CategoryNamePartSet>
ERepository::do_category_names() const
{
    return _imp->layout->category_names();
}

tr1::shared_ptr<const QualifiedPackageNameSet>
ERepository::do_package_names(const CategoryNamePart & c) const
{
    return _imp->layout->package_names(c);
}

tr1::shared_ptr<const PackageIDSequence>
ERepository::do_package_ids(const QualifiedPackageName & n) const
{
    return _imp->layout->package_ids(n);
}

bool
ERepository::repository_masked(const PackageID & id) const
{
    if (! _imp->has_repo_mask)
    {
        Context context("When querying repository mask for '" + stringify(id) + "':");

        tr1::shared_ptr<const FSEntrySequence> repository_mask_files(_imp->layout->repository_mask_files());
        for (FSEntrySequence::Iterator p(repository_mask_files->begin()), p_end(repository_mask_files->end()) ;
                p != p_end ; ++p)
        {
            Context context_local("When reading '" + stringify(*p) + "':");

            if (p->exists())
            {
                LineConfigFile ff(*p, LineConfigFileOptions());
                for (LineConfigFile::Iterator line(ff.begin()), line_end(ff.end()) ;
                        line != line_end ; ++line)
                {
                    try
                    {
                        tr1::shared_ptr<const PackageDepSpec> a(new PackageDepSpec(*line, pds_pm_eapi_0));
                        if (a->package_ptr())
                            _imp->repo_mask[*a->package_ptr()].push_back(a);
                        else
                            Log::get_instance()->message(ll_warning, lc_context, "Loading package mask spec '"
                                    + stringify(*line) + "' failed because specification does not restrict to a "
                                    "unique package");
                    }
                    catch (const Exception & e)
                    {
                        Log::get_instance()->message(ll_warning, lc_context, "Loading package mask spec '"
                                + stringify(*line) + "' failed due to exception '" + e.message() + "' ("
                                + e.what() + ")");
                    }
                }
            }
        }

        _imp->has_repo_mask = true;
    }

    RepositoryMaskMap::iterator r(_imp->repo_mask.find(id.name()));
    if (_imp->repo_mask.end() == r)
        return false;
    else
        for (IndirectIterator<std::list<tr1::shared_ptr<const PackageDepSpec> >::const_iterator, const PackageDepSpec>
                k(r->second.begin()), k_end(r->second.end()) ; k != k_end ; ++k)
            if (match_package(*_imp->params.environment, *k, id))
                return true;

    return false;
}

UseFlagState
ERepository::do_query_use(const UseFlagName & f, const PackageID & e) const
{
    _imp->need_profiles();
    return _imp->profile_ptr->use_state_ignoring_masks(f, e);
}

bool
ERepository::do_query_use_mask(const UseFlagName & u, const PackageID & e) const
{
    _imp->need_profiles();
    return _imp->profile_ptr->use_masked(u, e) ||
        (arch_flags()->end() != arch_flags()->find(u) && use_enabled != do_query_use(u, e));
}

bool
ERepository::do_query_use_force(const UseFlagName & u, const PackageID & e) const
{
    _imp->need_profiles();
    return _imp->profile_ptr->use_forced(u, e);
}

tr1::shared_ptr<const UseFlagNameSet>
ERepository::do_arch_flags() const
{
    if (! _imp->arch_flags)
    {
        Context context("When loading arch list:");
        _imp->arch_flags.reset(new UseFlagNameSet);

        bool found_one(false);
        tr1::shared_ptr<const FSEntrySequence> arch_list_files(_imp->layout->arch_list_files());
        for (FSEntrySequence::Iterator p(arch_list_files->begin()), p_end(arch_list_files->end()) ;
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

tr1::shared_ptr<FSEntry>
ERepository::do_license_exists(const std::string & license) const
{
    tr1::shared_ptr<FSEntry> p;

    if (_imp->params.master_repository)
    {
        FSEntry l(_imp->params.master_repository->params().location / "licenses" / license);
        if (l.exists() && l.is_regular_file_or_symlink_to_regular_file())
            p.reset(new FSEntry(l));
    }

    FSEntry l(_imp->params.location / "licenses" / license);
    if (l.exists() && l.is_regular_file_or_symlink_to_regular_file())
        p.reset(new FSEntry(l));

    return p;
}

void
ERepository::need_mirrors() const
{
    if (! _imp->has_mirrors)
    {
        bool found_one(false);
        tr1::shared_ptr<const FSEntrySequence> mirror_files(_imp->layout->mirror_files());
        for (FSEntrySequence::Iterator p(mirror_files->begin()), p_end(mirror_files->end()) ;
                p != p_end ; ++p)
        {
            if (p->exists())
            {
                LineConfigFile mirrors(*p, LineConfigFileOptions());
                for (LineConfigFile::Iterator line(mirrors.begin()) ; line != mirrors.end() ; ++line)
                {
                    std::vector<std::string> ee;
                    WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(ee));
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
ERepository::do_package_set(const SetName & s) const
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
ERepository::do_sync() const
{
    Context context("When syncing repository '" + stringify(name()) + "':");

    if (_imp->params.sync.empty())
        return false;

    std::list<std::string> sync_list;
    WhitespaceTokeniser::get_instance()->tokenise(_imp->params.sync, std::back_inserter(sync_list));

    bool ok(false);
    for (std::list<std::string>::const_iterator s(sync_list.begin()),
            s_end(sync_list.end()) ; s != s_end ; ++s)
    {
        DefaultSyncer syncer(SyncerParams::create()
                                .environment(_imp->params.environment)
                                .local(stringify(_imp->params.location))
                                .remote(*s));
        SyncOptions opts(_imp->params.sync_options);
        try
        {
            syncer.sync(opts);
        }
        catch (const SyncFailedError & e)
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
    _imp.reset(new Implementation<ERepository>(this, _imp->params));
}

void
ERepository::update_news() const
{
    if (! _imp->news_ptr)
        _imp->news_ptr.reset(new ERepositoryNews(_imp->params.environment, this, _imp->params));

    _imp->news_ptr->update_news();
}

const tr1::shared_ptr<const Layout>
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

const tr1::shared_ptr<const ERepositoryEntries>
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

    return _imp->entries_ptr->get_environment_variable(for_package, var, _imp->profile_ptr);
}

tr1::shared_ptr<const RepositoryInfo>
ERepository::info(bool verbose) const
{
    tr1::shared_ptr<const RepositoryInfo> result_non_verbose(Repository::info(verbose));
    if (! verbose)
        return result_non_verbose;

    tr1::shared_ptr<RepositoryInfo> result(new RepositoryInfo);

    for (RepositoryInfo::SectionIterator s(result_non_verbose->begin_sections()),
            s_end(result_non_verbose->end_sections()) ; s != s_end ; ++s)
        result->add_section(*s);

    // don't inherit from master_repository, just causes clutter
    std::set<std::string> info_pkgs;
    if ((_imp->layout->info_packages_file(_imp->params.location / "profiles")).exists())
    {
        LineConfigFile vars(_imp->layout->info_packages_file(_imp->params.location / "profiles"), LineConfigFileOptions());
        info_pkgs.insert(vars.begin(), vars.end());
    }

    if (! info_pkgs.empty())
    {
        tr1::shared_ptr<RepositoryInfoSection> package_info(new RepositoryInfoSection("Package information"));
        for (std::set<std::string>::const_iterator i(info_pkgs.begin()),
                i_end(info_pkgs.end()) ; i != i_end ; ++i)
        {
            tr1::shared_ptr<const PackageIDSequence> q(
                    _imp->params.environment->package_database()->query(
                        query::Matches(PackageDepSpec(*i, pds_pm_eapi_0)) &
                        query::InstalledAtRoot(_imp->params.environment->root()),
                        qo_order_by_version));
            if (q->empty())
                package_info->add_kv(*i, "(none)");
            else
            {
                using namespace tr1::placeholders;
                std::set<VersionSpec> versions;
                std::copy(q->begin(), q->end(),
                        transform_inserter(std::inserter(versions, versions.begin()),
                            tr1::bind<const VersionSpec>(tr1::mem_fn(&PackageID::version), _1)));
                package_info->add_kv(*i, join(versions.begin(), versions.end(), ", "));
            }
        }

        result->add_section(package_info);
    }

#if 0
    // don't inherit from master_repository, just causes clutter
    std::set<std::string> info_vars;
    if (_imp->layout->info_variables_file(_imp->params.location / "profiles").exists())
    {
        LineConfigFile vars(_imp->layout->info_variables_file(_imp->params.location / "profiles"), LineConfigFileOptions());
        info_vars.insert(vars.begin(), vars.end());
    }

    if (! info_vars.empty() && ! info_pkgs.empty() &&
            ! package_ids(QualifiedPackageName(*info_pkgs.begin()))->empty())
    {
        PackageDatabaseEntry e(QualifiedPackageName(*info_pkgs.begin()),
                *package_ids(QualifiedPackageName(*info_pkgs.begin()))->last(),
                name());
        tr1::shared_ptr<RepositoryInfoSection> variable_info(new RepositoryInfoSection("Variable information"));
        for (std::set<std::string>::const_iterator i(info_vars.begin()),
                i_end(info_vars.end()) ; i != i_end ; ++i)
            variable_info->add_kv(*i, get_environment_variable(e, *i));

        result->add_section(variable_info);
    }
    else if (! info_vars.empty())
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Skipping info_vars for '" + stringify(name()) +
                "' because info_pkgs is not usable");
#endif

    return result;
}

std::string
ERepository::profile_variable(const std::string & s) const
{
    _imp->need_profiles();

    return _imp->profile_ptr->environment_variable(s);
}

ERepository::MirrorsIterator
ERepository::begin_mirrors(const std::string & s) const
{
    need_mirrors();
    return MirrorsIterator(_imp->mirrors.equal_range(s).first);
}

ERepository::MirrorsIterator
ERepository::end_mirrors(const std::string & s) const
{
    need_mirrors();
    return MirrorsIterator(_imp->mirrors.equal_range(s).second);
}

tr1::shared_ptr<const ERepository::VirtualsSequence>
ERepository::virtual_packages() const
{
    Context context("When loading virtual packages for repository '" +
            stringify(name()) + "'");

    _imp->need_profiles();

    tr1::shared_ptr<VirtualsSequence> result(new VirtualsSequence);

    for (ERepositoryProfile::VirtualsIterator i(_imp->profile_ptr->begin_virtuals()),
            i_end(_imp->profile_ptr->end_virtuals()) ; i != i_end ; ++i)
        result->push_back(RepositoryVirtualsEntry::create()
                .provided_by_spec(i->second)
                .virtual_name(i->first));

    return result;
}

tr1::shared_ptr<const UseFlagNameSet>
ERepository::do_use_expand_flags() const
{
    _imp->need_profiles();

    std::string expand_sep(stringify(EAPIData::get_instance()->eapi_from_string(_imp->params.profile_eapi
                    )->supported->ebuild_options->use_expand_separator));
    tr1::shared_ptr<UseFlagNameSet> result(new UseFlagNameSet);
    for (ERepositoryProfile::UseExpandIterator i(_imp->profile_ptr->begin_use_expand()),
            i_end(_imp->profile_ptr->end_use_expand()) ; i != i_end ; ++i)
    {
        std::list<std::string> values;
        WhitespaceTokeniser::get_instance()->tokenise(_imp->profile_ptr->environment_variable(
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
ERepository::do_use_expand_prefixes() const
{
    _imp->need_profiles();

    tr1::shared_ptr<UseFlagNameSet> result(new UseFlagNameSet);
    for (ERepositoryProfile::UseExpandIterator i(_imp->profile_ptr->begin_use_expand()),
            i_end(_imp->profile_ptr->end_use_expand()) ; i != i_end ; ++i)
    {
        std::string lower_i;
        std::transform(i->data().begin(), i->data().end(), std::back_inserter(lower_i), &::tolower);
        result->insert(UseFlagName(lower_i));
    }

    return result;
}

tr1::shared_ptr<const UseFlagNameSet>
ERepository::do_use_expand_hidden_prefixes() const
{
    _imp->need_profiles();

    tr1::shared_ptr<UseFlagNameSet> result(new UseFlagNameSet);
    for (ERepositoryProfile::UseExpandIterator i(_imp->profile_ptr->begin_use_expand_hidden()),
            i_end(_imp->profile_ptr->end_use_expand_hidden()) ; i != i_end ; ++i)
    {
        std::string lower_i;
        std::transform(i->data().begin(), i->data().end(), std::back_inserter(lower_i), &::tolower);
        result->insert(UseFlagName(lower_i));
    }

    return result;
}

void
ERepository::regenerate_cache() const
{
    _imp->names_cache->regenerate_cache();
}

tr1::shared_ptr<const CategoryNamePartSet>
ERepository::do_category_names_containing_package(const PackageNamePart & p) const
{
    if (! _imp->names_cache->usable())
        return Repository::do_category_names_containing_package(p);

    tr1::shared_ptr<const CategoryNamePartSet> result(
            _imp->names_cache->category_names_containing_package(p));

    return result ? result : Repository::do_category_names_containing_package(p);
}

ERepository::ProfilesIterator
ERepository::begin_profiles() const
{
    _imp->need_profiles_desc();
    return ProfilesIterator(_imp->profiles_desc.begin());
}

ERepository::ProfilesIterator
ERepository::end_profiles() const
{
    _imp->need_profiles_desc();
    return ProfilesIterator(_imp->profiles_desc.end());
}

ERepository::ProfilesIterator
ERepository::find_profile(const FSEntry & location) const
{
    _imp->need_profiles_desc();
    for (ProfilesDesc::const_iterator i(_imp->profiles_desc.begin()),
            i_end(_imp->profiles_desc.end()) ; i != i_end ; ++i)
        if (i->path == location)
            return ProfilesIterator(i);
    return ProfilesIterator(_imp->profiles_desc.end());
}

void
ERepository::set_profile(const ProfilesIterator & iter)
{
    Context context("When setting profile by iterator:");

    _imp->profile_ptr = iter->profile;

    if (DistributionData::get_instance()->distribution_from_string(_imp->params.environment->default_distribution())->support_old_style_virtuals)
        if (_imp->params.environment->package_database()->has_repository_named(RepositoryName("virtuals")))
            _imp->params.environment->package_database()->fetch_repository(
                    RepositoryName("virtuals"))->invalidate();
}

void
ERepository::set_profile_by_arch(const UseFlagName & arch)
{
    Context context("When setting profile by arch '" + stringify(arch) + "':");

    for (ProfilesIterator p(begin_profiles()), p_end(end_profiles()) ; p != p_end ; ++p)
        if (p->arch == stringify(arch) && p->status == "stable")
        {
            set_profile(p);
            return;
        }

    for (ProfilesIterator p(begin_profiles()), p_end(end_profiles()) ; p != p_end ; ++p)
        if (p->arch == stringify(arch))
        {
            set_profile(p);
            return;
        }

    throw ConfigurationError("Cannot find a profile appropriate for '" + stringify(arch) + "'");
}

std::string
ERepository::do_describe_use_flag(const UseFlagName & f,
        const PackageID & e) const
{
    if (_imp->use_desc.empty())
    {
        std::string expand_sep(stringify(EAPIData::get_instance()->eapi_from_string(
                        _imp->params.profile_eapi)->supported->ebuild_options->use_expand_separator));
        tr1::shared_ptr<const FSEntrySequence> use_desc_dirs(_imp->layout->use_desc_dirs());
        for (FSEntrySequence::Iterator p(use_desc_dirs->begin()), p_end(use_desc_dirs->end()) ;
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
    std::string f(e.repository()->format());
    return f == "ebuild" || f == "ebin";
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
ERepository::merge(const MergeOptions & o)
{
    _imp->entries_ptr->merge(o);
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

    erepository::QAController controller(
            _imp->params.environment,
            shared_from_this(),
            ignore_if,
            ignore_unless,
            minimum_level,
            reporter);

    controller.run();

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

        void visit(const SupportsActionTest<UninstallAction> &)
        {
        }
    };
}

bool
ERepository::do_some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

