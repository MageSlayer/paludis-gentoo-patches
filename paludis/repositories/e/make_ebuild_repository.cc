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
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/environment.hh>
#include <paludis/distribution.hh>
#include <paludis/metadata_key.hh>

using namespace paludis;

std::tr1::shared_ptr<ERepository>
paludis::make_ebuild_repository(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    Context context("When making ebuild repository from repo_file '" + f("repo_file") + "':");

    std::string location(f("location"));
    if (location.empty())
        throw ERepositoryConfigurationError("Key 'location' not specified or empty");

    std::tr1::shared_ptr<KeyValueConfigFile> layout_conf((FSEntry(location) / "metadata/layout.conf").exists() ?
            new KeyValueConfigFile(FSEntry(location) / "metadata/layout.conf", KeyValueConfigFileOptions(),
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation)
            : 0);

    std::tr1::shared_ptr<const RepositoryName> master_repository_name;
    std::tr1::shared_ptr<const ERepository> master_repository;
    if (! f("master_repository").empty())
    {
        Context context_local("When finding configuration information for master_repository '"
                + stringify(f("master_repository")) + "':");

        master_repository_name.reset(new RepositoryName(f("master_repository")));

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
    tokenise_whitespace(f("profiles"), create_inserter<FSEntry>(std::back_inserter(*profiles)));

    if (profiles->empty())
    {
        if (master_repository)
            std::copy(master_repository->params().profiles->begin(),
                    master_repository->params().profiles->end(), profiles->back_inserter());
        else
            throw ERepositoryConfigurationError("No profiles have been specified");
    }

    std::tr1::shared_ptr<FSEntrySequence> eclassdirs(new FSEntrySequence);
    tokenise_whitespace(f("eclassdirs"), create_inserter<FSEntry>(std::back_inserter(*eclassdirs)));

    if (eclassdirs->empty())
    {
        if (master_repository)
            std::copy(master_repository->params().eclassdirs->begin(),
                    master_repository->params().eclassdirs->end(), eclassdirs->back_inserter());

        eclassdirs->push_back(location + "/eclass");
    }

    std::string distdir(f("distdir"));
    if (distdir.empty())
    {
        if (master_repository)
            distdir = stringify(master_repository->params().distdir);
        else
        {
            distdir = (*DistributionData::get_instance()->distribution_from_string(
                    env->distribution())).default_ebuild_distdir();
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
        write_cache = (*DistributionData::get_instance()->distribution_from_string(
                env->distribution())).default_ebuild_write_cache();

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
            eapi_when_unknown = (*DistributionData::get_instance()->distribution_from_string(
                    env->distribution())).default_ebuild_eapi_when_unknown();
    }

    std::string eapi_when_unspecified(f("eapi_when_unspecified"));
    if (eapi_when_unspecified.empty())
    {
        if (! layout_conf
                || (eapi_when_unspecified = layout_conf->get("eapi_when_unspecified")).empty())
            eapi_when_unspecified = (*DistributionData::get_instance()->distribution_from_string(
                    env->distribution())).default_ebuild_eapi_when_unspecified();
    }

    std::string profile_eapi(f("profile_eapi"));
    if (profile_eapi.empty())
    {
        if (! layout_conf
                || (profile_eapi = layout_conf->get("eapi_when_unspecified")).empty())
            profile_eapi = (*DistributionData::get_instance()->distribution_from_string(
                    env->distribution())).default_ebuild_profile_eapi();
    }

    std::string names_cache(f("names_cache"));
    if (names_cache.empty())
    {
        names_cache = (*DistributionData::get_instance()->distribution_from_string(
                env->distribution())).default_ebuild_names_cache();
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

    if (! f("sync_exclude").empty())
    {
        Log::get_instance()->message("e.ebuild.configuration.deprecated", ll_warning, lc_no_context)
            << "The sync_exclude key in '" << f("repo_file") << "' is deprecated in favour of sync_options = --exclude-from=";
        if (! sync_options.empty())
            sync_options += " ";
        sync_options += "--exclude-from='" + f("sync_exclude") + "'";
    }

    std::string builddir(f("builddir"));
    if (builddir.empty())
    {
        builddir = f("buildroot");
        if (builddir.empty())
        {
            if (master_repository)
                builddir = stringify(master_repository->params().builddir);
            else
                builddir = (*DistributionData::get_instance()->distribution_from_string(
                         env->distribution())).default_ebuild_builddir();
        }
        else
            Log::get_instance()->message("e.ebuild.configuration.deprecated", ll_warning, lc_context)
                << "Key 'buildroot' is deprecated, use 'builddir' instead";
    }

    std::string layout(f("layout"));
    if (layout.empty())
    {
        if (! layout_conf
                || (layout = layout_conf->get("layout")).empty())
            layout = (*DistributionData::get_instance()->distribution_from_string(
                    env->distribution())).default_ebuild_layout();
    }

    erepository::UseManifest use_manifest(erepository::manifest_use);
    if (! f("use_manifest").empty())
    {
        Context item_context("When handling use_manifest key:");
        use_manifest = destringify<erepository::UseManifest>(f("use_manifest"));
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
                .builddir(FSEntry(builddir).realpath_if_exists())));
}

std::tr1::shared_ptr<Repository>
paludis::make_ebuild_repository_wrapped(
        Environment * const env,
        const std::tr1::function<std::string (const std::string &)> & f)
{
    return make_ebuild_repository(env, f);
}

