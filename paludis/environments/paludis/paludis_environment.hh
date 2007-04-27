/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEFAULT_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_DEFAULT_ENVIRONMENT_HH 1

#include <paludis/package_database.hh>
#include <paludis/environment_implementation.hh>

/** \file
 * Declarations for the PaludisEnvironment class.
 *
 * \ingroup grppaludisenvironment
 */

namespace paludis
{
    class PaludisConfig;

    /**
     * The PaludisEnvironment is an Environment that corresponds to the normal
     * operating evironment.
     *
     * \ingroup grppaludisenvironment
     */
    class PALUDIS_VISIBLE PaludisEnvironment :
        public EnvironmentImplementation,
        public InstantiationPolicy<PaludisEnvironment, instantiation_method::SingletonTag>,
        private PrivateImplementationPattern<PaludisEnvironment>
   {
        protected:
            virtual bool accept_keywords(std::tr1::shared_ptr<const KeywordNameCollection>, const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_license(const std::string &, const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_breaks_portage(const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool masked_by_user(const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool unmasked_by_user(const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<CompositeDepSpec> local_set(const SetName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            ///\name Basic operations
            ///\{

            PaludisEnvironment(const std::string &);

            ~PaludisEnvironment();

            ///\}

            ///\name PaludisEnvironment-specific information
            ///\{

            /**
             * The config directory.
             */
            std::string config_dir() const;

            ///\}

            virtual bool query_use(const UseFlagName &, const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const UseFlagNameCollection> known_use_expand_names(
                    const UseFlagName &, const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntryCollection> bashrc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntryCollection> syncers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntryCollection> fetchers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntryCollection> hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const FSEntry root() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual uid_t reduced_uid() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual gid_t reduced_gid() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const MirrorsCollection> mirrors(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const SetNameCollection> set_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual int perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string paludis_command() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void set_paludis_command(const std::string &);

            virtual std::tr1::shared_ptr<PackageDatabase> package_database()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const PackageDatabase> package_database() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}
#endif
