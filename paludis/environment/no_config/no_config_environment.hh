/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENT_NO_CONFIG_NO_CONFIG_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENT_NO_CONFIG_NO_CONFIG_ENVIRONMENT_HH 1

#include <paludis/environment.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

namespace paludis
{
    /**
     * The type of repository to use for a NoConfigEnvironment.
     *
     * \see NoConfigEnvironment
     * \ingroup grpnoconfigenvironment
     */
    enum NoConfigEnvironmentRepositoryType
    {
        ncer_portage,
        ncer_vdb,
        ncer_auto
    };

#include <paludis/environment/no_config/no_config_environment-sr.hh>

    /**
     * An environment that uses a single repository, with no user configuration.
     *
     * \ingroup grpnoconfigenvironment
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoConfigEnvironment :
        private PrivateImplementationPattern<NoConfigEnvironment>,
        public Environment
    {
        public:
            ///\name Basic operations
            ///\{

            NoConfigEnvironment(const NoConfigEnvironmentParams & params);

            virtual ~NoConfigEnvironment();

            ///\}

            virtual std::string paludis_command() const;

            /**
             * What is our top level directory for our main repository?
             */
            FSEntry main_repository_dir() const;

            virtual bool accept_keyword(const KeywordName &, const PackageDatabaseEntry * const) const;

            ///\name Iterate over our profiles
            ///\{

            typedef libwrapiter::ForwardIterator<NoConfigEnvironment, const NoConfigEnvironmentProfilesDescLine> ProfilesIterator;
            ProfilesIterator begin_profiles() const;
            ProfilesIterator end_profiles() const;

            ///\}

            ///\name Profile functions
            ///\{

            void set_profile(const FSEntry & location);
            void set_profile(const ProfilesIterator & iter);

            ///\}

    };
}

#endif
