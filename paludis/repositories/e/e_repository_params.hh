/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_PARAMS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_PARAMS_HH 1

#include <paludis/util/fs_entry.hh>
#include <paludis/util/named_value.hh>

/** \file
 * Declaration for the ERepositoryParams class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    class Environment;
    class PackageDatabase;
    class ERepository;

    typedef Sequence<std::shared_ptr<const ERepository> > ERepositorySequence;

    namespace n
    {
        typedef Name<struct append_repository_name_to_write_cache_name> append_repository_name_to_write_cache;
        typedef Name<struct auto_profiles_name> auto_profiles;
        typedef Name<struct binary_destination_name> binary_destination;
        typedef Name<struct binary_distdir_name> binary_distdir;
        typedef Name<struct binary_keywords_name> binary_keywords;
        typedef Name<struct binary_uri_prefix_name> binary_uri_prefix;
        typedef Name<struct builddir_name> builddir;
        typedef Name<struct cache_name> cache;
        typedef Name<struct distdir_name> distdir;
        typedef Name<struct eapi_when_unknown_name> eapi_when_unknown;
        typedef Name<struct eapi_when_unspecified_name> eapi_when_unspecified;
        typedef Name<struct eclassdirs_name> eclassdirs;
        typedef Name<struct entry_format_name> entry_format;
        typedef Name<struct environment_name> environment;
        typedef Name<struct ignore_deprecated_profiles_name> ignore_deprecated_profiles;
        typedef Name<struct layout_name> layout;
        typedef Name<struct location_name> location;
        typedef Name<struct master_repositories_name> master_repositories;
        typedef Name<struct names_cache_name> names_cache;
        typedef Name<struct newsdir_name> newsdir;
        typedef Name<struct profile_eapi_when_unspecified_name> profile_eapi_when_unspecified;
        typedef Name<struct profile_layout_name> profile_layout;
        typedef Name<struct profiles_name> profiles;
        typedef Name<struct profiles_explicitly_set_name> profiles_explicitly_set;
        typedef Name<struct securitydir_name> securitydir;
        typedef Name<struct setsdir_name> setsdir;
        typedef Name<struct sync_name> sync;
        typedef Name<struct sync_options_name> sync_options;
        typedef Name<struct use_manifest_name> use_manifest;
        typedef Name<struct write_bin_uri_prefix_name> write_bin_uri_prefix;
        typedef Name<struct write_cache_name> write_cache;
    }

    namespace erepository
    {
#include <paludis/repositories/e/e_repository_params-se.hh>

        struct ERepositoryParams
        {
            NamedValue<n::append_repository_name_to_write_cache, bool> append_repository_name_to_write_cache;
            NamedValue<n::auto_profiles, bool> auto_profiles;
            NamedValue<n::binary_destination, bool> binary_destination;
            NamedValue<n::binary_distdir, FSEntry> binary_distdir;
            NamedValue<n::binary_keywords, std::string> binary_keywords;
            NamedValue<n::binary_uri_prefix, std::string> binary_uri_prefix;
            NamedValue<n::builddir, FSEntry> builddir;
            NamedValue<n::cache, FSEntry> cache;
            NamedValue<n::distdir, FSEntry> distdir;
            NamedValue<n::eapi_when_unknown, std::string> eapi_when_unknown;
            NamedValue<n::eapi_when_unspecified, std::string> eapi_when_unspecified;
            NamedValue<n::eclassdirs, std::shared_ptr<const FSEntrySequence> > eclassdirs;
            NamedValue<n::entry_format, std::string> entry_format;
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::ignore_deprecated_profiles, bool> ignore_deprecated_profiles;
            NamedValue<n::layout, std::string> layout;
            NamedValue<n::location, FSEntry> location;
            NamedValue<n::master_repositories, std::shared_ptr<const ERepositorySequence> > master_repositories;
            NamedValue<n::names_cache, FSEntry> names_cache;
            NamedValue<n::newsdir, FSEntry> newsdir;
            NamedValue<n::profile_eapi_when_unspecified, std::string> profile_eapi_when_unspecified;
            NamedValue<n::profile_layout, std::string> profile_layout;
            NamedValue<n::profiles, std::shared_ptr<const FSEntrySequence> > profiles;
            NamedValue<n::profiles_explicitly_set, bool> profiles_explicitly_set;
            NamedValue<n::securitydir, FSEntry> securitydir;
            NamedValue<n::setsdir, FSEntry> setsdir;
            NamedValue<n::sync, std::string> sync;
            NamedValue<n::sync_options, std::string> sync_options;
            NamedValue<n::use_manifest, erepository::UseManifest> use_manifest;
            NamedValue<n::write_bin_uri_prefix, std::string> write_bin_uri_prefix;
            NamedValue<n::write_cache, FSEntry> write_cache;
        };
    }

}

#endif
