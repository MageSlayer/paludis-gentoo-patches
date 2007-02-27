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

#include <paludis/environment.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

namespace paludis
{
    class PortageRepository;

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

#include <paludis/environments/no_config/no_config_environment-sr.hh>

    /**
     * An environment that uses a single repository, with no user configuration.
     *
     * \ingroup grpnoconfigenvironment
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoConfigEnvironment :
        public Environment,
        private PrivateImplementationPattern<NoConfigEnvironment>,
        private InstantiationPolicy<NoConfigEnvironment, instantiation_method::NonCopyableTag>
    {
        public:
            ///\name Basic operations
            ///\{

            NoConfigEnvironment(const NoConfigEnvironmentParams & params);

            virtual ~NoConfigEnvironment();

            ///\}

            virtual std::string paludis_command() const;
            virtual void set_paludis_command(const std::string &);

            /**
             * What is our top level directory for our main repository?
             */
            FSEntry main_repository_dir() const;

            virtual bool accept_keyword(const KeywordName &, const PackageDatabaseEntry * const, const bool) const;

            /**
             * Should we accept unstable keywords?
             */
            void set_accept_unstable(const bool value);

            std::tr1::shared_ptr<PortageRepository> portage_repository();
            std::tr1::shared_ptr<const PortageRepository> portage_repository() const;

            std::tr1::shared_ptr<PortageRepository> master_repository();
            std::tr1::shared_ptr<const PortageRepository> master_repository() const;

            virtual void force_use(std::tr1::shared_ptr<const PackageDepSpec>, const UseFlagName &,
                    const UseFlagState) PALUDIS_ATTRIBUTE((noreturn));

            virtual void clear_forced_use();
    };
}

#endif
