/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/fs_path.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/set-fwd.hh>
#include <memory>

/** \file
 * Declaration for the ERepositoryParams class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    class Environment;
    class ERepository;

    typedef Sequence<std::shared_ptr<const ERepository> > ERepositorySequence;

    namespace n
    {
        typedef Name<struct name_append_repository_name_to_write_cache> append_repository_name_to_write_cache;
        typedef Name<struct name_auto_profiles> auto_profiles;
        typedef Name<struct name_binary_destination> binary_destination;
        typedef Name<struct name_binary_distdir> binary_distdir;
        typedef Name<struct name_binary_keywords_filter> binary_keywords_filter;
        typedef Name<struct name_binary_uri_prefix> binary_uri_prefix;
        typedef Name<struct name_builddir> builddir;
        typedef Name<struct name_cache> cache;
        typedef Name<struct name_distdir> distdir;
        typedef Name<struct name_eapi_when_unknown> eapi_when_unknown;
        typedef Name<struct name_eapi_when_unspecified> eapi_when_unspecified;
        typedef Name<struct name_eclassdirs> eclassdirs;
        typedef Name<struct name_entry_format> entry_format;
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_ignore_deprecated_profiles> ignore_deprecated_profiles;
        typedef Name<struct name_layout> layout;
        typedef Name<struct name_location> location;
        typedef Name<struct name_manifest_hashes> manifest_hashes;
        typedef Name<struct name_master_repositories> master_repositories;
        typedef Name<struct name_names_cache> names_cache;
        typedef Name<struct name_newsdir> newsdir;
        typedef Name<struct name_profile_eapi_when_unspecified> profile_eapi_when_unspecified;
        typedef Name<struct name_profile_layout> profile_layout;
        typedef Name<struct name_profiles> profiles;
        typedef Name<struct name_profiles_explicitly_set> profiles_explicitly_set;
        typedef Name<struct name_securitydir> securitydir;
        typedef Name<struct name_setsdir> setsdir;
        typedef Name<struct name_sync> sync;
        typedef Name<struct name_sync_options> sync_options;
        typedef Name<struct name_thin_manifests> thin_manifests;
        typedef Name<struct name_use_manifest> use_manifest;
        typedef Name<struct name_write_bin_uri_prefix> write_bin_uri_prefix;
        typedef Name<struct name_write_cache> write_cache;
    }

    namespace erepository
    {
#include <paludis/repositories/e/e_repository_params-se.hh>

        struct ERepositoryParams
        {
            NamedValue<n::append_repository_name_to_write_cache, bool> append_repository_name_to_write_cache;
            NamedValue<n::auto_profiles, bool> auto_profiles;
            NamedValue<n::binary_destination, bool> binary_destination;
            NamedValue<n::binary_distdir, FSPath> binary_distdir;
            NamedValue<n::binary_keywords_filter, std::string> binary_keywords_filter;
            NamedValue<n::binary_uri_prefix, std::string> binary_uri_prefix;
            NamedValue<n::builddir, FSPath> builddir;
            NamedValue<n::cache, FSPath> cache;
            NamedValue<n::distdir, FSPath> distdir;
            NamedValue<n::eapi_when_unknown, std::string> eapi_when_unknown;
            NamedValue<n::eapi_when_unspecified, std::string> eapi_when_unspecified;
            NamedValue<n::eclassdirs, std::shared_ptr<const FSPathSequence> > eclassdirs;
            NamedValue<n::entry_format, std::string> entry_format;
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::ignore_deprecated_profiles, bool> ignore_deprecated_profiles;
            NamedValue<n::layout, std::string> layout;
            NamedValue<n::location, FSPath> location;
            NamedValue<n::manifest_hashes, std::shared_ptr<const Set<std::string> > > manifest_hashes;
            NamedValue<n::master_repositories, std::shared_ptr<const ERepositorySequence> > master_repositories;
            NamedValue<n::names_cache, FSPath> names_cache;
            NamedValue<n::newsdir, FSPath> newsdir;
            NamedValue<n::profile_eapi_when_unspecified, std::string> profile_eapi_when_unspecified;
            NamedValue<n::profile_layout, std::string> profile_layout;
            NamedValue<n::profiles, std::shared_ptr<const FSPathSequence> > profiles;
            NamedValue<n::profiles_explicitly_set, bool> profiles_explicitly_set;
            NamedValue<n::securitydir, FSPath> securitydir;
            NamedValue<n::setsdir, FSPath> setsdir;
            NamedValue<n::sync, std::shared_ptr<Map<std::string, std::string> > > sync;
            NamedValue<n::sync_options, std::shared_ptr<Map<std::string, std::string> > > sync_options;
            NamedValue<n::thin_manifests, bool> thin_manifests;
            NamedValue<n::use_manifest, erepository::UseManifest> use_manifest;
            NamedValue<n::write_bin_uri_prefix, std::string> write_bin_uri_prefix;
            NamedValue<n::write_cache, FSPath> write_cache;
        };
    }

}

#endif
