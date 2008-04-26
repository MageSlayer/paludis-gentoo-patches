/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include "make_ebuild_repository.hh"
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/map.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/options.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/kc.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/environment.hh>
#include <paludis/distribution.hh>
#include <paludis/metadata_key.hh>

using namespace paludis;

std::tr1::shared_ptr<ERepository>
paludis::make_ebuild_repository(
        Environment * const env,
        std::tr1::shared_ptr<const Map<std::string, std::string> > m)
{
    std::string repo_file(m->end() == m->find("repo_file") ? std::string("?") :
            m->find("repo_file")->second);

    Context context("When making ebuild repository from repo_file '" + repo_file + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw ERepositoryConfigurationError("Key 'location' not specified or empty");

    std::tr1::shared_ptr<KeyValueConfigFile> layout_conf((FSEntry(location) / "metadata/layout.conf").exists() ?
            new KeyValueConfigFile(FSEntry(location) / "metadata/layout.conf",
                KeyValueConfigFileOptions())
            : 0);

    std::tr1::shared_ptr<const RepositoryName> master_repository_name;
    std::tr1::shared_ptr<const ERepository> master_repository;
    if (m->end() != m->find("master_repository") && ! m->find("master_repository")->second.empty())
    {
        Context context_local("When finding configuration information for master_repository '"
                + stringify(m->find("master_repository")->second) + "':");

        master_repository_name.reset(new RepositoryName(m->find("master_repository")->second));

        std::tr1::shared_ptr<const Repository> master_repository_uncasted(
                env->package_database()->fetch_repository(*master_repository_name));

        std::string format("unknown");
        if (master_repository_uncasted->format_key())
            format = master_repository_uncasted->format_key()->value();
        if (format != "ebuild")
            throw ERepositoryConfigurationError("Master repository format is '" +
                    stringify(format) + "', not 'ebuild'");

        master_repository = std::tr1::static_pointer_cast<const ERepository>(master_repository_uncasted);

        if (master_repository->params().master_repository)
            throw ERepositoryConfigurationError("Requested master repository has a master_repository of '" +
                    stringify(master_repository->params().master_repository->name()) + "', so it cannot "
                    "be used as a master repository");
    }

    std::tr1::shared_ptr<FSEntrySequence> profiles(new FSEntrySequence);
    if (m->end() != m->find("profiles"))
        tokenise_whitespace(m->find("profiles")->second,
                create_inserter<FSEntry>(std::back_inserter(*profiles)));

    if (profiles->empty())
    {
        if (master_repository)
            std::copy(master_repository->params().profiles->begin(),
                    master_repository->params().profiles->end(), profiles->back_inserter());
        else
            throw ERepositoryConfigurationError("No profiles have been specified");
    }

    std::tr1::shared_ptr<FSEntrySequence> eclassdirs(new FSEntrySequence);

    if (m->end() != m->find("eclassdirs"))
        tokenise_whitespace(m->find("eclassdirs")->second,
                create_inserter<FSEntry>(std::back_inserter(*eclassdirs)));

    if (eclassdirs->empty())
    {
        if (master_repository)
            std::copy(master_repository->params().eclassdirs->begin(),
                    master_repository->params().eclassdirs->end(), eclassdirs->back_inserter());

        eclassdirs->push_back(location + "/eclass");
    }

    std::string distdir;
    if (m->end() == m->find("distdir") || ((distdir = m->find("distdir")->second)).empty())
    {
        if (master_repository)
            distdir = stringify(master_repository->params().distdir);
        else
        {
            distdir = (*DistributionData::get_instance()->distribution_from_string(
                    env->default_distribution()))[k::default_ebuild_distdir()];
            if (distdir.empty())
                distdir = location + "/distfiles";
            else if ('/' != distdir.at(0))
                distdir = location + "/" + distdir;
        }
    }

    std::string setsdir;
    if (m->end() == m->find("setsdir") || ((setsdir = m->find("setsdir")->second)).empty())
        setsdir = location + "/sets";

    std::string securitydir;
    if (m->end() == m->find("securitydir") || ((securitydir = m->find("securitydir")->second)).empty())
        securitydir = location + "/metadata/glsa";

    std::string newsdir;
    if (m->end() == m->find("newsdir") || ((newsdir = m->find("newsdir")->second)).empty())
        newsdir = location + "/metadata/news";

    std::string cache;
    if (m->end() == m->find("cache") || ((cache = m->find("cache")->second)).empty())
    {
        cache = location + "/metadata/cache";
        if (! FSEntry(cache).exists())
            cache = "/var/empty";
    }

    std::string write_cache;
    if (m->end() == m->find("write_cache") || ((write_cache = m->find("write_cache")->second)).empty())
        write_cache = (*DistributionData::get_instance()->distribution_from_string(
                env->default_distribution()))[k::default_ebuild_write_cache()];

    bool append_repository_name_to_write_cache(true);
    if (m->end() != m->find("append_repository_name_to_write_cache") && ! m->find("append_repository_name_to_write_cache")->second.empty())
    {
        Context item_context("When handling append_repository_name_to_write_cache key:");
        append_repository_name_to_write_cache = destringify<bool>(m->find("append_repository_name_to_write_cache")->second);
    }

    bool ignore_deprecated_profiles(false);
    if (m->end() != m->find("ignore_deprecated_profiles") && ! m->find("ignore_deprecated_profiles")->second.empty())
    {
        Context item_context("When handling ignore_deprecated_profiles key:");
        ignore_deprecated_profiles = destringify<bool>(m->find("ignore_deprecated_profiles")->second);
    }

    std::string eapi_when_unknown;
    if (m->end() == m->find("eapi_when_unknown") || ((eapi_when_unknown = m->find("eapi_when_unknown")->second)).empty())
    {
        if (! layout_conf
                || (eapi_when_unknown = layout_conf->get("eapi_when_unknown")).empty())
            eapi_when_unknown = (*DistributionData::get_instance()->distribution_from_string(
                    env->default_distribution()))[k::default_ebuild_eapi_when_unknown()];
    }

    std::string eapi_when_unspecified;
    if (m->end() == m->find("eapi_when_unspecified") || ((eapi_when_unspecified = m->find("eapi_when_unspecified")->second)).empty())
    {
        if (! layout_conf 
                || (eapi_when_unspecified = layout_conf->get("eapi_when_unspecified")).empty())
            eapi_when_unspecified = (*DistributionData::get_instance()->distribution_from_string(
                    env->default_distribution()))[k::default_ebuild_eapi_when_unspecified()];
    }

    std::string profile_eapi;
    if (m->end() == m->find("profile_eapi") || ((profile_eapi = m->find("profile_eapi")->second)).empty())
    {
        if (! layout_conf
                || (profile_eapi = layout_conf->get("eapi_when_unspecified")).empty())
            profile_eapi = (*DistributionData::get_instance()->distribution_from_string(
                    env->default_distribution()))[k::default_ebuild_profile_eapi()];
    }

    std::string names_cache;
    if (m->end() == m->find("names_cache") || ((names_cache = m->find("names_cache")->second)).empty())
    {
        names_cache = (*DistributionData::get_instance()->distribution_from_string(
                env->default_distribution()))[k::default_ebuild_names_cache()];
        if (names_cache.empty())
        {
            Log::get_instance()->message("e.ebuild.configuration.no_names_cache", ll_warning, lc_no_context)
                << "The names_cache key is not set in '" << repo_file
                << "'. You should read the Paludis documentation and select an appropriate value.";
            names_cache = "/var/empty";
        }
    }

    std::string sync;
    if (m->end() != m->find("sync"))
        sync = m->find("sync")->second;

    std::string sync_options;
    if (m->end() != m->find("sync_options"))
        sync_options = m->find("sync_options")->second;

    if (m->end() != m->find("sync_exclude"))
    {
        Log::get_instance()->message("e.ebuild.configuration.deprecated", ll_warning, lc_no_context)
            << "The sync_exclude key in '" << repo_file << "' is deprecated in favour of sync_options = --exclude-from=";
        if (! sync_options.empty())
            sync_options += " ";
        sync_options += "--exclude-from='" + m->find("sync_exclude")->second + "'";
    }

    std::string builddir;
    if (m->end() == m->find("builddir") || ((builddir = m->find("builddir")->second)).empty())
    {
        if (m->end() == m->find("buildroot") || ((builddir = m->find("buildroot")->second)).empty())
            builddir = (*DistributionData::get_instance()->distribution_from_string(
                    env->default_distribution()))[k::default_ebuild_builddir()];
        else
            Log::get_instance()->message("e.ebuild.configuration.deprecated", ll_warning, lc_context)
                << "Key 'buildroot' is deprecated, use 'builddir' instead";
    }

    std::string layout;
    if (m->end() == m->find("layout") || ((layout = m->find("layout")->second)).empty())
    {
        if (! layout_conf
                || (layout = layout_conf->get("layout")).empty())
            layout = (*DistributionData::get_instance()->distribution_from_string(
                    env->default_distribution()))[k::default_ebuild_layout()];
    }

    erepository::UseManifest use_manifest(erepository::manifest_use);
    if (m->end() != m->find("use_manifest") && ! m->find("use_manifest")->second.empty())
    {
        Context item_context("When handling use_manifest key:");
        use_manifest = destringify<erepository::UseManifest>(m->find("use_manifest")->second);
    }

    bool binary_destination(false);
    if (m->end() != m->find("binary_destination") && ! m->find("binary_destination")->second.empty())
    {
        Context item_context("When handling binary_destination key:");
        binary_destination = destringify<bool>(m->find("binary_destination")->second);
    }

    std::string binary_uri_prefix;
    if (m->end() != m->find("binary_uri_prefix"))
        binary_uri_prefix = m->find("binary_uri_prefix")->second;

    std::string binary_distdir;
    if (m->end() == m->find("binary_distdir") || ((binary_distdir = m->find("binary_distdir")->second)).empty())
        binary_distdir = distdir;

    std::string binary_keywords;
    if (m->end() == m->find("binary_keywords") || ((binary_keywords = m->find("binary_keywords")->second)).empty())
    {
        if (binary_destination)
            throw ERepositoryConfigurationError("binary_destination = true, but binary_keywords is unset or empty");
    }

    return std::tr1::shared_ptr<ERepository>(new ERepository(ERepositoryParams::create()
                .entry_format("ebuild")
                .layout(layout)
                .environment(env)
                .location(location)
                .profiles(profiles)
                .cache(cache)
                .write_cache(write_cache)
                .names_cache(names_cache)
                .eclassdirs(eclassdirs)
                .distdir(distdir)
                .securitydir(securitydir)
                .setsdir(setsdir)
                .newsdir(newsdir)
                .sync(sync)
                .sync_options(sync_options)
                .master_repository(master_repository)
                .write_bin_uri_prefix("")
                .eapi_when_unknown(eapi_when_unknown)
                .eapi_when_unspecified(eapi_when_unspecified)
                .profile_eapi(profile_eapi)
                .use_manifest(use_manifest)
                .append_repository_name_to_write_cache(append_repository_name_to_write_cache)
                .ignore_deprecated_profiles(ignore_deprecated_profiles)
                .binary_destination(binary_destination)
                .binary_uri_prefix(binary_uri_prefix)
                .binary_distdir(binary_distdir)
                .binary_keywords(binary_keywords)
                .builddir(builddir)));
}

std::tr1::shared_ptr<Repository>
paludis::make_ebuild_repository_wrapped(
        Environment * const env,
        std::tr1::shared_ptr<const Map<std::string, std::string> > m)
{
    return make_ebuild_repository(env, m);
}

