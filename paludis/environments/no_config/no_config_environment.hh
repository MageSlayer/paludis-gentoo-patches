/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/environment_implementation.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/sr.hh>
#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>

namespace paludis
{
    namespace no_config_environment
    {
        /**
         * The type of repository to use for a NoConfigEnvironment.
         *
         * \see NoConfigEnvironment
         * \ingroup grpnoconfigenvironment
         */
        enum RepositoryType
        {
            ncer_ebuild,
            ncer_vdb,
            ncer_auto
        };

#include <paludis/environments/no_config/no_config_environment-sr.hh>
    }

    /**
     * An environment that uses a single repository, with no user configuration.
     *
     * \ingroup grpnoconfigenvironment
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoConfigEnvironment :
        public EnvironmentImplementation,
        private PrivateImplementationPattern<NoConfigEnvironment>,
        private InstantiationPolicy<NoConfigEnvironment, instantiation_method::NonCopyableTag>
    {
        protected:
            virtual bool accept_keywords(tr1::shared_ptr<const KeywordNameCollection>, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            ///\name Basic operations
            ///\{

            NoConfigEnvironment(const no_config_environment::Params & params);

            virtual ~NoConfigEnvironment();

            ///\}

            ///\name NoConfigEnvironment-specific configuration options
            ///\{

            /**
             * What is our top level directory for our main repository?
             */
            FSEntry main_repository_dir() const;

            /**
             * Should we accept unstable keywords?
             */
            void set_accept_unstable(const bool value);

            ///\}

            ///\name NoConfigEnvironment-specific repository information
            ///\{

            /**
             * Fetch our 'main' repository.
             */
            tr1::shared_ptr<Repository> main_repository();

            /**
             * Fetch our 'main' repository.
             */
            tr1::shared_ptr<const Repository> main_repository() const;

            /**
             * Fetch our 'master' repository (may be zero).
             */
            tr1::shared_ptr<Repository> master_repository();

            /**
             * Fetch our 'master' repository (may be zero).
             */
            tr1::shared_ptr<const Repository> master_repository() const;

            ///\}

            virtual tr1::shared_ptr<PackageDatabase> package_database()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const PackageDatabase> package_database() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string paludis_command() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void set_paludis_command(const std::string &);
    };
}

#endif
